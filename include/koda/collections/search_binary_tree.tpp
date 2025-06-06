#include <koda/utils/comparation.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>

namespace koda {

constexpr SearchBinaryTree::SearchBinaryTree(size_t string_size) noexcept
    : string_size_{string_size} {}

constexpr SearchBinaryTree::SearchBinaryTree(SearchBinaryTree&& other) noexcept
    : root_{std::exchange(other.root_, nullptr)},
      dictionary_start_index_{other.dictionary_start_index_},
      buffer_start_index_{other.buffer_start_index_},
      string_size_{other.string_size_} {}

constexpr SearchBinaryTree& SearchBinaryTree::operator=(
    SearchBinaryTree&& other) noexcept {
    Destroy();
    root_ = std::exchange(other.root_, nullptr);
    dictionary_start_index_ = other.dictionary_start_index_;
    buffer_start_index_ = other.buffer_start_index_;
    string_size_ = other.string_size_;
    return *this;
}

constexpr void SearchBinaryTree::AddString(StringView string) {
    assert(string.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    InsertNewNode(string.data());
    ++buffer_start_index_;
}

constexpr bool SearchBinaryTree::RemoveString(StringView string) {
    Node* node = FindNodeToRemoval(string);

    if (!node) [[unlikely]] {
        return false;
    }

    if (!--node->ref_counter) {
        RemoveNode(node);
    }

    ++dictionary_start_index_;
    return true;
}

constexpr SearchBinaryTree::RepeatitionMarker SearchBinaryTree::FindMatch(
    StringView buffer) const {
    assert(
        buffer.size() <= string_size_ &&
        "Inserted string have to have fixed size not bigger than string_size_");

    auto [position, length] = FindString(buffer.data(), buffer.size());

    if (!length) {
        return {0, 0};
    }

    return {// Calculate relative offset from the start of the dictionary
            position - dictionary_start_index_, length};
}

constexpr SearchBinaryTree::~SearchBinaryTree() { Destroy(); }

constexpr SearchBinaryTree::Node::Node(const uint8_t* key,
                                       size_t insertion_index, Node* parent,
                                       Color color)
    : key{std::move(key)},
      insertion_index{insertion_index},
      parent{parent},
      color{color} {}

constexpr SearchBinaryTree::NodePool::Scheduler::~Scheduler() {
    pool.ReturnNode(node);
}

constexpr SearchBinaryTree::NodePool::NodePool(NodePool&& other) noexcept
    : handle_{std::exchange(other.handle_, nullptr)} {}

constexpr SearchBinaryTree::NodePool& SearchBinaryTree::NodePool::operator=(
    NodePool&& other) noexcept {
    Destroy();
    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
}

constexpr void SearchBinaryTree::NodePool::ReturnNode(Node* handle) {
    handle->left = handle_;
    handle_ = handle;
}

constexpr SearchBinaryTree::NodePool::Scheduler
SearchBinaryTree::NodePool::ScheduleForReturn(Node* node) {
    return Scheduler{*this, node};
}

constexpr SearchBinaryTree::Node* SearchBinaryTree::NodePool::GetNode(
    const uint8_t* key, size_t insertion_index, Node* parent,
    Node::Color color) {
    if (!handle_) {
        return new Node{key, insertion_index, parent, color};
    }
    Node* node = handle_;
    handle_ = node->left;

    *node = Node{key, insertion_index, parent, color};
    return node;
}

constexpr SearchBinaryTree::NodePool::~NodePool() { Destroy(); }

constexpr void SearchBinaryTree::NodePool::Destroy() {
    for (Node* node = handle_; node;) {
        std::unique_ptr<Node> handle{node};
        node = node->left;
    }
}

constexpr void SearchBinaryTree::RotateLeft(Node* node) {
    Node* right = node->right;
    Node* right_left = right->left;

    node->right = right_left;
    right->left = node;

    RotateHelper(node, right_left, right);
}

constexpr void SearchBinaryTree::RotateRight(Node* node) {
    Node* left = node->left;
    Node* left_right = left->right;

    node->left = left_right;
    left->right = node;

    RotateHelper(node, left_right, left);
}

constexpr void SearchBinaryTree::RotateHelper(Node* node, Node* child,
                                              Node* root) {
    Node* parent = node->parent;
    root->parent = parent;
    node->parent = root;
    if (child) {
        child->parent = node;
    }
    if (parent) {
        if (node == parent->right) {
            parent->right = root;
        } else {
            parent->left = root;
        }
    } else {
        root_ = root;
    }
}

constexpr void SearchBinaryTree::InsertNewNode(const uint8_t* key) {
    if (!root_) [[unlikely]] {
        root_ = pool_.GetNode(key, buffer_start_index_);
        return;
    }
    if (auto inserted = TryToInserLeaf(key)) {
        BuildNode(key, inserted->first, inserted->second);
        assert(inserted->first->parent && "Parent has to exist");
        if (inserted->first->parent->color == Node::Color::kRed) {
            FixInsertionImbalance(inserted->first);
        }
    }
}

constexpr void SearchBinaryTree::UpdateNodeReference(Node* node,
                                                     const uint8_t* key) {
    ++node->ref_counter;
    node->key = key;
    node->insertion_index = buffer_start_index_ - dictionary_start_index_;
}

constexpr void SearchBinaryTree::BuildNode(const uint8_t* key, Node*& node,
                                           Node* parent) {
    node = pool_.GetNode(key, buffer_start_index_, parent, Node::Color::kRed);
}

constexpr std::optional<SearchBinaryTree::NodeSpot>
SearchBinaryTree::TryToInserLeaf(const uint8_t* key) {
    const StringView key_view{key, string_size_};
    Node** node = &root_;
    Node* parent = nullptr;
    while (*node) {
        switch (
            OrderCast(key_view <=> StringView{(*node)->key, string_size_})) {
            case WeakOrdering::kEquivalent:
                UpdateNodeReference(*node, key);
                return std::nullopt;
            case WeakOrdering::kLess:
                parent = *node;
                node = &(*node)->left;
                break;
            case WeakOrdering::kGreater:
                parent = *node;
                node = &(*node)->right;
                break;
            default:
                std::unreachable();
        };
    }
    return NodeSpot{*node, parent};
}

constexpr void SearchBinaryTree::FixInsertionImbalance(Node* node) {
    Node* parent = node->parent;

    for (; parent && parent->color == Node::Color::kRed;) {
        Node* grand_parent = parent->parent;

        if (!grand_parent) {
            parent->color = Node::Color::kBlack;
            return;
        }

        if (FixLocalInsertionImbalance(node, parent, grand_parent)) {
            return;
        }

        parent->color = Node::Color::kBlack;
        grand_parent->color = Node::Color::kRed;
        node = grand_parent;
        parent = node->parent;
    }
}

constexpr bool SearchBinaryTree::FixLocalInsertionImbalance(
    Node*& node, Node*& parent, Node*& grand_parent) {
    Node* uncle = grand_parent->right == parent ? grand_parent->left
                                                : grand_parent->right;
    if (!uncle || uncle->color == Node::Color::kBlack) {
        if (grand_parent->right == parent) {
            FixLocalInsertionImbalanceRight(node, parent, grand_parent, uncle);
        } else {
            FixLocalInsertionImbalanceLeft(node, parent, grand_parent, uncle);
        }
        parent->color = Node::Color::kBlack;
        grand_parent->color = Node::Color::kRed;
        return true;
    }
    uncle->color = Node::Color::kBlack;
    return false;
}

constexpr void SearchBinaryTree::FixLocalInsertionImbalanceRight(
    Node*& node, Node*& parent, Node*& grand_parent, Node* uncle) {
    if (node == parent->left) {
        RotateRight(parent);
        node = parent;
        parent = grand_parent->right;
    }
    RotateLeft(grand_parent);
}

constexpr void SearchBinaryTree::FixLocalInsertionImbalanceLeft(
    Node*& node, Node*& parent, Node*& grand_parent, Node* uncle) {
    if (node == parent->right) {
        RotateLeft(parent);
        node = parent;
        parent = grand_parent->left;
    }
    RotateRight(grand_parent);
}

constexpr std::pair<size_t, size_t> SearchBinaryTree::FindString(
    const uint8_t* buffer, size_t length) const {
    std::pair<size_t, size_t> match{};
    for (const Node* node = root_; node;) {
        auto prefix_length = FindCommonPrefixSize(buffer, node->key, length);
        if (prefix_length == length) {
            return {node->insertion_index, length};
        }
        UpdateMatchInfo(match, prefix_length, node);

        if (StringView{buffer, length} < StringView{node->key, string_size_}) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return match;
}

/*static*/ constexpr void SearchBinaryTree::UpdateMatchInfo(
    std::pair<size_t, size_t>& match_info, size_t prefix_length,
    const Node* node) noexcept {
    if (match_info.second < prefix_length) {
        match_info.first = node->insertion_index;
        match_info.second = prefix_length;
    }
}

constexpr size_t SearchBinaryTree::FindCommonPrefixSize(
    const uint8_t* buffer, const uint8_t* node, size_t length) const noexcept {
    for (size_t i = 0; i < length; ++i) {
        if (buffer[i] != node[i]) {
            return i;
        }
    }
    return length;
}

constexpr SearchBinaryTree::Node* SearchBinaryTree::FindNodeToRemoval(
    StringView key_view) {
    for (Node* node = root_; node;) {
        switch (OrderCast(key_view <=> StringView{node->key, string_size_})) {
            case WeakOrdering::kEquivalent:
                return node;
            case WeakOrdering::kLess:
                node = node->left;
                break;
            case WeakOrdering::kGreater:
                node = node->right;
                break;
            default:
                std::unreachable();
        };
    }
    return nullptr;
}

constexpr SearchBinaryTree::Node* SearchBinaryTree::FindSuccessor(Node* node) {
    for (; node->left; node = node->left);
    return node;
}

constexpr void SearchBinaryTree::RemoveNodeWithTwoChildren(Node* node) {
    Node* succesor = FindSuccessor(node->right);
    node->key = succesor->key;
    node->ref_counter = succesor->ref_counter;
    node->insertion_index = succesor->insertion_index;

    if (succesor->right) {
        return RemoveNodeWithOneChildren(succesor, succesor->right);
    }
    RemoveChildlessNode(succesor);
}

constexpr void SearchBinaryTree::RemoveNodeWithOneChildren(Node* node,
                                                           Node* children) {
    children->color = Node::Color::kBlack;
    children->parent = node->parent;

    if (node->parent) {
        if (node == node->parent->left) {
            node->parent->left = children;
        } else {
            node->parent->right = children;
        }
    } else {
        root_ = children;
    }

    pool_.ReturnNode(node);
}

constexpr void SearchBinaryTree::RemoveRootNode() {
    pool_.ReturnNode(std::exchange(root_, nullptr));
}

constexpr void SearchBinaryTree::PrepareToRemoveRedChildlessNode(Node* node) {
    if (node->parent->right == node) {
        node->parent->right = nullptr;
    } else {
        node->parent->left = nullptr;
    }
}

constexpr void SearchBinaryTree::RemoveBlackChildlessNodeRightPathSiblingIsRed(
    Node* node, Node* parent, Node* sibling, Node* left_nephew,
    Node* right_nephew) {
    RotateRight(parent);
    parent->color = Node::Color::kRed;
    sibling->color = Node::Color::kBlack;
    sibling = right_nephew;

    left_nephew = right_nephew->left;
    if (left_nephew && left_nephew->color == Node::Color::kRed) {
        return RemoveNodeRotateParentRightPath(parent, sibling, left_nephew);
    }
    right_nephew = right_nephew->right;
    if (right_nephew && right_nephew->color == Node::Color::kRed) {
        return RemoveNodeRotateSiblingRightPath(parent, sibling, right_nephew);
    }

    sibling->color = Node::Color::kRed;
    parent->color = Node::Color::kBlack;
}

constexpr bool SearchBinaryTree::RemoveBlackChildlessNodeRightPath(
    Node* node, Node* parent) {
    Node* sibling = parent->left;
    Node* left_nephew = sibling->left;
    Node* right_nephew = sibling->right;

    if (sibling->color == Node::Color::kRed) {
        RemoveBlackChildlessNodeRightPathSiblingIsRed(
            node, parent, sibling, left_nephew, right_nephew);
        return true;
    }

    if (left_nephew && left_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateParentRightPath(parent, sibling, left_nephew);
        return true;
    }

    if (right_nephew && right_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateSiblingRightPath(parent, sibling, right_nephew);
        return true;
    }

    sibling->color = Node::Color::kRed;

    if (parent->color == Node::Color::kRed) {
        parent->color = Node::Color::kBlack;
        return true;
    }
    return false;
}

constexpr void SearchBinaryTree::RemoveBlackChildlessNodeLeftPathSiblingIsRed(
    Node* node, Node* parent, Node* sibling, Node* left_nephew,
    Node* right_nephew) {
    RotateLeft(parent);
    parent->color = Node::Color::kRed;
    sibling->color = Node::Color::kBlack;
    sibling = left_nephew;

    right_nephew = left_nephew->right;
    if (right_nephew && right_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateParentLeftPath(parent, sibling, right_nephew);
    }
    left_nephew = left_nephew->left;
    if (left_nephew && left_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateSiblingLeftPath(parent, sibling, left_nephew);
    }

    sibling->color = Node::Color::kRed;
    parent->color = Node::Color::kBlack;
}

constexpr bool SearchBinaryTree::RemoveBlackChildlessNodeLeftPath(
    Node* node, Node* parent) {
    Node* sibling = parent->right;
    Node* left_nephew = sibling->left;
    Node* right_nephew = sibling->right;

    if (sibling->color == Node::Color::kRed) {
        RemoveBlackChildlessNodeLeftPathSiblingIsRed(node, parent, sibling,
                                                     left_nephew, right_nephew);
        return true;
    }

    if (right_nephew && right_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateParentLeftPath(parent, sibling, right_nephew);
        return true;
    }

    if (left_nephew && left_nephew->color == Node::Color::kRed) {
        RemoveNodeRotateSiblingLeftPath(parent, sibling, left_nephew);
        return true;
    }

    sibling->color = Node::Color::kRed;

    if (parent->color == Node::Color::kRed) {
        parent->color = Node::Color::kBlack;
        return true;
    }
    return false;
}

constexpr void SearchBinaryTree::RemoveBlackChildlessNode(Node* node) {
    if (node->parent->right == node) {
        node->parent->right = nullptr;
        if (RemoveBlackChildlessNodeRightPath(node, node->parent)) {
            return;
        }
    } else {
        node->parent->left = nullptr;
        if (RemoveBlackChildlessNodeLeftPath(node, node->parent)) {
            return;
        }
    }

    for (node = node->parent; Node* parent = node->parent;
         node = node->parent) {
        if (parent->right == node) {
            if (RemoveBlackChildlessNodeRightPath(node, parent)) {
                return;
            }
        } else {
            if (RemoveBlackChildlessNodeLeftPath(node, parent)) {
                return;
            }
        }
    }
}

constexpr void SearchBinaryTree::RemoveChildlessNode(Node* node) {
    if (node == root_) {
        return RemoveRootNode();
    }

    // will deallocate node after all logic has been handled
    auto handle = pool_.ScheduleForReturn(node);

    if (node->color == Node::Color::kRed) {
        return PrepareToRemoveRedChildlessNode(node);
    }

    RemoveBlackChildlessNode(node);
}

constexpr void SearchBinaryTree::RemoveNode(Node* node) {
    if (node->left && node->right) {
        return RemoveNodeWithTwoChildren(node);
    }

    if (node->left || node->right) {
        return RemoveNodeWithOneChildren(node,
                                         node->left ? node->left : node->right);
    }

    RemoveChildlessNode(node);
}

constexpr void SearchBinaryTree::RemoveNodeRotateSiblingRightPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateLeft(sibling);
    sibling->color = Node::Color::kRed;
    nephew->color = Node::Color::kBlack;
    RemoveNodeRotateParentRightPath(parent, nephew, sibling);
}

constexpr void SearchBinaryTree::RemoveNodeRotateSiblingLeftPath(Node* parent,
                                                                 Node* sibling,
                                                                 Node* nephew) {
    RotateRight(sibling);
    sibling->color = Node::Color::kRed;
    nephew->color = Node::Color::kBlack;
    RemoveNodeRotateParentLeftPath(parent, nephew, sibling);
}

constexpr void SearchBinaryTree::RemoveNodeRotateParentRightPath(Node* parent,
                                                                 Node* sibling,
                                                                 Node* nephew) {
    RotateRight(parent);
    sibling->color = parent->color;
    parent->color = Node::Color::kBlack;
    nephew->color = Node::Color::kBlack;
}

constexpr void SearchBinaryTree::RemoveNodeRotateParentLeftPath(Node* parent,
                                                                Node* sibling,
                                                                Node* nephew) {
    RotateLeft(parent);
    sibling->color = parent->color;
    parent->color = Node::Color::kBlack;
    nephew->color = Node::Color::kBlack;
}

constexpr void SearchBinaryTree::Destroy() {
    // Instead of calling a recursive destructor call, deallocate tree in place
    // in order to avoid stack overflow for large structures!
    for (Node* node = root_; root_;) {
        if (node->left != nullptr) {
            node = std::exchange(node->left, nullptr);
            continue;
        }

        if (node->right != nullptr) {
            node = std::exchange(node->right, nullptr);
            continue;
        }

        // We're leaving - destroy node !
        std::unique_ptr<Node> destroy_handle{node};
        if (!node->parent) {
            return;  // root can be left with dangling pointer
        }
        node = node->parent;
    }
}

}  // namespace koda

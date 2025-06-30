#include <koda/utils/comparation.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>

namespace koda {

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::SearchBinaryTree(
    size_t string_size, const AllocatorTp& allocator) noexcept
    : pool_{allocator}, string_size_{string_size} {}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::SearchBinaryTree(
    SearchBinaryTree&& other) noexcept
    : root_{std::exchange(other.root_, nullptr)},
      pool_{std::move(other.pool_)},
      dictionary_start_index_{other.dictionary_start_index_},
      buffer_start_index_{other.buffer_start_index_},
      string_size_{other.string_size_} {}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>&
SearchBinaryTree<Tp, AllocatorTp>::operator=(
    SearchBinaryTree&& other) noexcept {
    Destroy();
    root_ = std::exchange(other.root_, nullptr);
    pool_ = std::move(other.pool_);
    dictionary_start_index_ = other.dictionary_start_index_;
    buffer_start_index_ = other.buffer_start_index_;
    string_size_ = other.string_size_;
    return *this;
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::AddString(StringView string) {
    assert(string.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    InsertNewNode(string.data());
    ++buffer_start_index_;
}

template <typename Tp, typename AllocatorTp>
constexpr bool SearchBinaryTree<Tp, AllocatorTp>::RemoveString(
    StringView string) {
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

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::RepeatitionMarker
SearchBinaryTree<Tp, AllocatorTp>::FindMatch(StringView buffer) const {
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

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr size_t SearchBinaryTree<Tp, AllocatorTp>::string_size()
    const noexcept {
    return string_size_;
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp
SearchBinaryTree<Tp, AllocatorTp>::get_allocator() const {
    return pool_.get_allocator();
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::~SearchBinaryTree() {
    Destroy();
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::Node::Node(const ValueType* key,
                                                        size_t insertion_index,
                                                        Node* parent,
                                                        Color color)
    : key{std::move(key)},
      insertion_index{insertion_index},
      parent{parent},
      color{color} {}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodePool::Scheduler::~Scheduler() {
    pool.ReturnNode(node);
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodePool::NodePool(
    const AllocatorTp& allocator) noexcept
    : allocator_{allocator} {}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodePool::NodePool(
    NodePool&& other) noexcept
    : allocator_{std::move(other.allocator_)},
      handle_{std::exchange(other.handle_, nullptr)} {}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodePool&
SearchBinaryTree<Tp, AllocatorTp>::NodePool::operator=(
    NodePool&& other) noexcept {
    Destroy();
    allocator_ = std::move(other.allocator_);
    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::NodePool::ReturnNode(
    Node* handle) {
    handle->left = handle_;
    handle_ = handle;
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodePool::Scheduler
SearchBinaryTree<Tp, AllocatorTp>::NodePool::ScheduleForReturn(Node* node) {
    return Scheduler{*this, node};
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::Node*
SearchBinaryTree<Tp, AllocatorTp>::NodePool::GetNode(const ValueType* key,
                                                     size_t insertion_index,
                                                     Node* parent,
                                                     Node::Color color) {
    if (!handle_) {
        Node* node = NodeTraits::allocate(allocator_, 1);
        NodeTraits::construct(allocator_, node, key, insertion_index, parent,
                              color);
        return node;
    }
    Node* node = handle_;
    handle_ = node->left;

    *node = Node{key, insertion_index, parent, color};
    return node;
}

template <typename Tp, typename AllocatorTp>
constexpr AllocatorTp
SearchBinaryTree<Tp, AllocatorTp>::NodePool::get_allocator() const {
    return allocator_;
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodePool::~NodePool() {
    Destroy();
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::NodePool::Destroy() {
    for (Node* node = handle_; node;) {
        Node* old_node = node;
        node = node->left;
        NodeTraits::destroy(allocator_, old_node);
        NodeTraits::deallocate(allocator_, old_node, 1);
    }
}

// Recursiveless tree iterator!
template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodeIterator::NodeIterator(
    pointer_type node, pointer_type previous) noexcept
    : current_{node}, previous_{previous} {}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr SearchBinaryTree<Tp,
                                         AllocatorTp>::NodeIterator::value_type
SearchBinaryTree<Tp, AllocatorTp>::NodeIterator::operator*() const noexcept {
    return *current_;
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr SearchBinaryTree<
    Tp, AllocatorTp>::NodeIterator::pointer_type
SearchBinaryTree<Tp, AllocatorTp>::NodeIterator::operator->() const noexcept {
    return current_;
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodeIterator&
SearchBinaryTree<Tp, AllocatorTp>::NodeIterator::operator++() noexcept {
    // If iterator came from parent then visit left subtree (if present)
    // If iterator came from left subtree then visit the right subtree (if
    // present) Otherwise visit parent (repeat untill parent is a nullptr)
    while (current_) {
        auto previous = current_;
        if (previous_ == current_->parent) {
            if (current_->left) {
                current_ = current_->left;
                previous_ = previous;
                return *this;
            }
        }
        if (previous_ == current_->left) {
            if (current_->right) {
                current_ = current_->right;
                previous_ = previous;
                return *this;
            }
        }
        current_ = current_->parent;
        previous_ = previous;
    }
    return *this;
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr SearchBinaryTree<Tp, AllocatorTp>::NodeIterator
SearchBinaryTree<Tp, AllocatorTp>::NodeIterator::operator++(int) noexcept {
    auto temp = *this;
    ++(*this);
    return temp;
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr bool
SearchBinaryTree<Tp, AllocatorTp>::NodeIterator::operator==(
    const NodeIterator& other) const noexcept {
    return (current_ == other.current_) && (previous_ == other.previous_);
}

template <typename Tp, typename AllocatorTp>
constexpr std::ranges::subrange<
    typename SearchBinaryTree<Tp, AllocatorTp>::NodeIterator,
    typename SearchBinaryTree<Tp, AllocatorTp>::NodeIterator>
SearchBinaryTree<Tp, AllocatorTp>::nodes() const {
    return std::ranges::subrange{NodeIterator{root_, nullptr},
                                 NodeIterator{nullptr, root_}};
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RotateLeft(Node* node) {
    Node* right = node->right;
    Node* right_left = right->left;

    node->right = right_left;
    right->left = node;

    RotateHelper(node, right_left, right);
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RotateRight(Node* node) {
    Node* left = node->left;
    Node* left_right = left->right;

    node->left = left_right;
    left->right = node;

    RotateHelper(node, left_right, left);
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RotateHelper(Node* node,
                                                               Node* child,
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

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::InsertNewNode(
    const ValueType* key) {
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

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::UpdateNodeReference(
    Node* node, const ValueType* key) {
    ++node->ref_counter;
    node->key = key;
    node->insertion_index = buffer_start_index_ - dictionary_start_index_;
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::BuildNode(
    const ValueType* key, Node*& node, Node* parent) {
    node = pool_.GetNode(key, buffer_start_index_, parent, Node::Color::kRed);
}

template <typename Tp, typename AllocatorTp>
constexpr std::optional<typename SearchBinaryTree<Tp, AllocatorTp>::NodeSpot>
SearchBinaryTree<Tp, AllocatorTp>::TryToInserLeaf(const ValueType* key) {
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

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::FixInsertionImbalance(
    Node* node) {
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

template <typename Tp, typename AllocatorTp>
constexpr bool SearchBinaryTree<Tp, AllocatorTp>::FixLocalInsertionImbalance(
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

template <typename Tp, typename AllocatorTp>
constexpr void
SearchBinaryTree<Tp, AllocatorTp>::FixLocalInsertionImbalanceRight(
    Node*& node, Node*& parent, Node*& grand_parent, Node* uncle) {
    if (node == parent->left) {
        RotateRight(parent);
        node = parent;
        parent = grand_parent->right;
    }
    RotateLeft(grand_parent);
}

template <typename Tp, typename AllocatorTp>
constexpr void
SearchBinaryTree<Tp, AllocatorTp>::FixLocalInsertionImbalanceLeft(
    Node*& node, Node*& parent, Node*& grand_parent, Node* uncle) {
    if (node == parent->right) {
        RotateLeft(parent);
        node = parent;
        parent = grand_parent->left;
    }
    RotateRight(grand_parent);
}

template <typename Tp, typename AllocatorTp>
constexpr std::pair<size_t, size_t>
SearchBinaryTree<Tp, AllocatorTp>::FindString(const ValueType* buffer,
                                              size_t length) const {
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

template <typename Tp, typename AllocatorTp>
/*static*/ constexpr void SearchBinaryTree<Tp, AllocatorTp>::UpdateMatchInfo(
    std::pair<size_t, size_t>& match_info, size_t prefix_length,
    const Node* node) noexcept {
    if (match_info.second < prefix_length) {
        match_info.first = node->insertion_index;
        match_info.second = prefix_length;
    }
}

template <typename Tp, typename AllocatorTp>
constexpr size_t SearchBinaryTree<Tp, AllocatorTp>::FindCommonPrefixSize(
    const ValueType* buffer, const ValueType* node,
    size_t length) const noexcept {
    for (size_t i = 0; i < length; ++i) {
        if (buffer[i] != node[i]) {
            return i;
        }
    }
    return length;
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::Node*
SearchBinaryTree<Tp, AllocatorTp>::FindNodeToRemoval(StringView key_view) {
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

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::Node*
SearchBinaryTree<Tp, AllocatorTp>::FindSuccessor(Node* node) {
    for (; node->left; node = node->left);
    return node;
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RemoveNodeWithTwoChildren(
    Node* node) {
    Node* succesor = FindSuccessor(node->right);
    node->key = succesor->key;
    node->ref_counter = succesor->ref_counter;
    node->insertion_index = succesor->insertion_index;

    if (succesor->right) {
        return RemoveNodeWithOneChildren(succesor, succesor->right);
    }
    RemoveChildlessNode(succesor);
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RemoveNodeWithOneChildren(
    Node* node, Node* children) {
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

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RemoveRootNode() {
    pool_.ReturnNode(std::exchange(root_, nullptr));
}

template <typename Tp, typename AllocatorTp>
constexpr void
SearchBinaryTree<Tp, AllocatorTp>::PrepareToRemoveRedChildlessNode(Node* node) {
    if (node->parent->right == node) {
        node->parent->right = nullptr;
    } else {
        node->parent->left = nullptr;
    }
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::
    RemoveBlackChildlessNodeRightPathSiblingIsRed(Node* node, Node* parent,
                                                  Node* sibling,
                                                  Node* left_nephew,
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

template <typename Tp, typename AllocatorTp>
constexpr bool
SearchBinaryTree<Tp, AllocatorTp>::RemoveBlackChildlessNodeRightPath(
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

template <typename Tp, typename AllocatorTp>
constexpr void
SearchBinaryTree<Tp, AllocatorTp>::RemoveBlackChildlessNodeLeftPathSiblingIsRed(
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

template <typename Tp, typename AllocatorTp>
constexpr bool
SearchBinaryTree<Tp, AllocatorTp>::RemoveBlackChildlessNodeLeftPath(
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

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RemoveBlackChildlessNode(
    Node* node) {
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

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RemoveChildlessNode(
    Node* node) {
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

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::RemoveNode(Node* node) {
    if (node->left && node->right) {
        return RemoveNodeWithTwoChildren(node);
    }

    if (node->left || node->right) {
        return RemoveNodeWithOneChildren(node,
                                         node->left ? node->left : node->right);
    }

    RemoveChildlessNode(node);
}

template <typename Tp, typename AllocatorTp>
constexpr void
SearchBinaryTree<Tp, AllocatorTp>::RemoveNodeRotateSiblingRightPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateLeft(sibling);
    sibling->color = Node::Color::kRed;
    nephew->color = Node::Color::kBlack;
    RemoveNodeRotateParentRightPath(parent, nephew, sibling);
}

template <typename Tp, typename AllocatorTp>
constexpr void
SearchBinaryTree<Tp, AllocatorTp>::RemoveNodeRotateSiblingLeftPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateRight(sibling);
    sibling->color = Node::Color::kRed;
    nephew->color = Node::Color::kBlack;
    RemoveNodeRotateParentLeftPath(parent, nephew, sibling);
}

template <typename Tp, typename AllocatorTp>
constexpr void
SearchBinaryTree<Tp, AllocatorTp>::RemoveNodeRotateParentRightPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateRight(parent);
    sibling->color = parent->color;
    parent->color = Node::Color::kBlack;
    nephew->color = Node::Color::kBlack;
}

template <typename Tp, typename AllocatorTp>
constexpr void
SearchBinaryTree<Tp, AllocatorTp>::RemoveNodeRotateParentLeftPath(
    Node* parent, Node* sibling, Node* nephew) {
    RotateLeft(parent);
    sibling->color = parent->color;
    parent->color = Node::Color::kBlack;
    nephew->color = Node::Color::kBlack;
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::Destroy() {
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

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::CheckInvariants() const {
#ifdef KODA_CHECKED_BUILD
    ValidateRedNodeConstraint();
#endif  // KODA_CHECKED_BUILD
}

#ifdef KODA_CHECKED_BUILD

// Two red nodes cannot coexist
constexpr void ValidateRedNodeConstraint() const {
    for (const Node* node : nodes()) {
        if (node->color == Node::Color::kRed) {
            if (node->left && node->left->color == Node::Color::kRed) {
                throw std::logic_error{"Red Node invariant broken!"};
            }

            if (node->right && node->right->color == Node::Color::kRed) {
                throw std::logic_error{"Red Node invariant broken!"};
            }
        }
    }
}

#endif  // KODA_CHECKED_BUILD

}  // namespace koda

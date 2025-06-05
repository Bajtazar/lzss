#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/comparation.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <ranges>

namespace koda {

SearchBinaryTree::SearchBinaryTree(size_t string_size) noexcept
    : string_size_{string_size} {}

SearchBinaryTree::SearchBinaryTree(SearchBinaryTree&& other) noexcept
    : root_{std::exchange(other.root_, nullptr)},
      dictionary_start_index_{other.dictionary_start_index_},
      buffer_start_index_{other.buffer_start_index_},
      string_size_{other.string_size_} {}

SearchBinaryTree& SearchBinaryTree::operator=(
    SearchBinaryTree&& other) noexcept {
    Destroy();
    root_ = std::exchange(other.root_, nullptr);
    dictionary_start_index_ = other.dictionary_start_index_;
    buffer_start_index_ = other.buffer_start_index_;
    string_size_ = other.string_size_;
    return *this;
}

void SearchBinaryTree::AddString(StringView string) {
    assert(string.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    InsertNewNode(string.data());
    ++buffer_start_index_;
}

bool SearchBinaryTree::RemoveString(StringView string) {
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

SearchBinaryTree::RepeatitionMarker SearchBinaryTree::FindMatch(
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

SearchBinaryTree::~SearchBinaryTree() { Destroy(); }

SearchBinaryTree::Node::Node(const uint8_t* key, size_t insertion_index,
                             Node* parent, Color color)
    : key{std::move(key)},
      insertion_index{insertion_index},
      parent{parent},
      color{color} {}

void SearchBinaryTree::RotateLeft(Node* node) {
    Node* right = node->right;
    Node* right_left = right->left;

    node->right = right_left;
    right->left = node;

    RotateHelper(node, right_left, right);
}

void SearchBinaryTree::RotateRight(Node* node) {
    Node* left = node->left;
    Node* left_right = left->right;

    node->left = left_right;
    left->right = node;

    RotateHelper(node, left_right, left);
}

void SearchBinaryTree::RotateHelper(Node* node, Node* child, Node* root) {
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

void SearchBinaryTree::RotateLeftRight(Node*& node) {
    RotateLeft(node->left);
    RotateRight(node);
}

void SearchBinaryTree::RotateRightLeft(Node*& node) {
    RotateRight(node->right);
    RotateLeft(node);
}

void SearchBinaryTree::InsertNewNode(const uint8_t* key) {
    if (!root_) [[unlikely]] {
        root_ = new Node{key, buffer_start_index_};
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

void SearchBinaryTree::UpdateNodeReference(Node* node, const uint8_t* key) {
    ++node->ref_counter;
    node->key = key;
    node->insertion_index = buffer_start_index_ - dictionary_start_index_;
}

void SearchBinaryTree::BuildNode(const uint8_t* key, Node*& node,
                                 Node* parent) {
    node = new Node{key, buffer_start_index_, parent, Node::Color::kRed};
}

void SearchBinaryTree::dumpTree() {
    if (root_) {
        std::cout << "===================\n";
        dumpTree(root_, "");
        std::cout << "===================\n";
    }
}

void SearchBinaryTree::dumpTree(Node* parent, std::string view) {
    if (parent->left) {
        dumpTree(parent->left, view + "|\t");
    }

    std::cout << parent << "]\t" << view << "|------->";

    if (parent->color == Node::Color::kRed) {
        std::cout << "\033[0;31m";
    } else {
        std::cout << "\033[1;30m";
    }

    std::cout << std::string_view{reinterpret_cast<const char*>(parent->key), 4}
              << "<\033[0m\n";
    if (parent->right) {
        dumpTree(parent->right, view + "|\t");
    }
}

std::optional<SearchBinaryTree::NodeSpot> SearchBinaryTree::TryToInserLeaf(
    const uint8_t* key) {
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

void SearchBinaryTree::FixInsertionImbalance(Node* node) {
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

bool SearchBinaryTree::FixLocalInsertionImbalance(Node*& node, Node*& parent,
                                                  Node*& grand_parent) {
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

void SearchBinaryTree::FixLocalInsertionImbalanceRight(Node*& node,
                                                       Node*& parent,
                                                       Node*& grand_parent,
                                                       Node* uncle) {
    if (node == parent->left) {
        RotateRight(parent);
        node = parent;
        parent = grand_parent->right;
    }
    RotateLeft(grand_parent);
}

void SearchBinaryTree::FixLocalInsertionImbalanceLeft(Node*& node,
                                                      Node*& parent,
                                                      Node*& grand_parent,
                                                      Node* uncle) {
    if (node == parent->right) {
        RotateLeft(parent);
        node = parent;
        parent = grand_parent->left;
    }
    RotateRight(grand_parent);
}

std::pair<size_t, size_t> SearchBinaryTree::FindString(const uint8_t* buffer,
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

/*static*/ void SearchBinaryTree::UpdateMatchInfo(
    std::pair<size_t, size_t>& match_info, size_t prefix_length,
    const Node* node) noexcept {
    if (match_info.second < prefix_length) {
        match_info.first = node->insertion_index;
        match_info.second = prefix_length;
    }
}

size_t SearchBinaryTree::FindCommonPrefixSize(const uint8_t* buffer,
                                              const uint8_t* node,
                                              size_t length) const noexcept {
    for (size_t i = 0; i < length; ++i) {
        if (buffer[i] != node[i]) {
            return i;
        }
    }
    return length;
}

SearchBinaryTree::Node* SearchBinaryTree::FindNodeToRemoval(
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

SearchBinaryTree::Node* SearchBinaryTree::FindSuccessor(Node* node) {
    for (; node->left; node = node->left);
    return node;
}

void SearchBinaryTree::RemoveNodeWithTwoChildren(Node* node) {
    Node* succesor = FindSuccessor(node->right);
    node->key = succesor->key;
    node->ref_counter = succesor->ref_counter;
    node->insertion_index = succesor->insertion_index;
    return RemoveNode(
        succesor);  // will call one of the special functions - swap later
}

void SearchBinaryTree::RemoveNodeWithOneChildren(Node* node, Node* children) {
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

    delete node;
}

void SearchBinaryTree::RemoveNode(Node* node) {
    if (node->left && node->right) {
        return RemoveNodeWithTwoChildren(node);
    }

    if (node->left || node->right) {
        return RemoveNodeWithOneChildren(node,
                                         node->left ? node->left : node->right);
    }

    if (node == root_) {
        delete root_;
        root_ = nullptr;
        return;
    }

    if (node->color == Node::Color::kRed) {
        if (node->parent->right == node) {
            node->parent->right = nullptr;
        } else {
            node->parent->left = nullptr;
        }
        delete node;
        return;
    }

    std::unique_ptr<Node> to_remove_node{node};
    bool direction, start = true;

    if (node->parent->right == node) {
        node->parent->right = nullptr;
        direction = true;
    } else {
        node->parent->left = nullptr;
        direction = false;
    }

    for (; Node* parent = node->parent; start = false) {
        if ((start && direction) || (!start && parent->right == node)) {
            // Right path
            Node* sibling = parent->left;
            Node* left_nephew = sibling->left;
            Node* right_nephew = sibling->right;

            if (sibling->color == Node::Color::kRed) {
                RotateRight(parent);
                parent->color = Node::Color::kRed;
                sibling->color = Node::Color::kBlack;
                sibling = right_nephew;

                left_nephew = right_nephew->left;
                if (left_nephew && left_nephew->color == Node::Color::kRed) {
                    return RemoveNodeRotateParentRightPath(parent, sibling,
                                                           left_nephew);
                }
                right_nephew = right_nephew->right;
                if (right_nephew && right_nephew->color == Node::Color::kRed) {
                    return RemoveNodeRotateSiblingRightPath(parent, sibling,
                                                            right_nephew);
                }

                sibling->color = Node::Color::kRed;
                parent->color = Node::Color::kBlack;
                return;
            }

            if (left_nephew && left_nephew->color == Node::Color::kRed) {
                return RemoveNodeRotateParentRightPath(parent, sibling,
                                                       left_nephew);
            }

            if (right_nephew && right_nephew->color == Node::Color::kRed) {
                return RemoveNodeRotateSiblingRightPath(parent, sibling,
                                                        right_nephew);
            }

            if (parent->color == Node::Color::kRed) {
                sibling->color = Node::Color::kRed;
                parent->color = Node::Color::kBlack;
                return;
            }

            sibling->color = Node::Color::kRed;
        } else {
            // Left path
            Node* sibling = parent->right;
            Node* right_nephew = sibling->right;
            Node* left_nephew = sibling->left;

            if (sibling->color == Node::Color::kRed) {
                RotateLeft(parent);
                parent->color = Node::Color::kRed;
                sibling->color = Node::Color::kBlack;
                sibling = left_nephew;

                right_nephew = left_nephew->right;
                if (right_nephew && right_nephew->color == Node::Color::kRed) {
                    return RemoveNodeRotateParentLeftPath(parent, sibling,
                                                          right_nephew);
                }
                left_nephew = left_nephew->left;
                if (left_nephew && left_nephew->color == Node::Color::kRed) {
                    return RemoveNodeRotateSiblingLeftPath(parent, sibling,
                                                           left_nephew);
                }

                sibling->color = Node::Color::kRed;
                parent->color = Node::Color::kBlack;
                return;
            }

            if (right_nephew && right_nephew->color == Node::Color::kRed) {
                return RemoveNodeRotateParentLeftPath(parent, sibling,
                                                      right_nephew);
            }

            if (left_nephew && left_nephew->color == Node::Color::kRed) {
                return RemoveNodeRotateSiblingLeftPath(parent, sibling,
                                                       left_nephew);
            }

            if (parent->color == Node::Color::kRed) {
                sibling->color = Node::Color::kRed;
                parent->color = Node::Color::kBlack;
                return;
            }

            sibling->color = Node::Color::kRed;
        }
        node = parent;
    }
}

void SearchBinaryTree::RemoveNodeRotateSiblingRightPath(Node* parent,
                                                        Node* sibling,
                                                        Node* nephew) {
    RotateLeft(sibling);
    sibling->color = Node::Color::kRed;
    nephew->color = Node::Color::kBlack;
    RemoveNodeRotateParentRightPath(parent, nephew, sibling);
}

void SearchBinaryTree::RemoveNodeRotateSiblingLeftPath(Node* parent,
                                                       Node* sibling,
                                                       Node* nephew) {
    RotateRight(sibling);
    sibling->color = Node::Color::kRed;
    nephew->color = Node::Color::kBlack;
    RemoveNodeRotateParentLeftPath(parent, nephew, sibling);
}

void SearchBinaryTree::RemoveNodeRotateParentRightPath(Node* parent,
                                                       Node* sibling,
                                                       Node* nephew) {
    RotateRight(parent);
    sibling->color = parent->color;
    parent->color = Node::Color::kBlack;
    nephew->color = Node::Color::kBlack;
}

void SearchBinaryTree::RemoveNodeRotateParentLeftPath(Node* parent,
                                                      Node* sibling,
                                                      Node* nephew) {
    RotateLeft(parent);
    sibling->color = parent->color;
    parent->color = Node::Color::kBlack;
    nephew->color = Node::Color::kBlack;
}

void SearchBinaryTree::Destroy() { /*delete root_;*/ }

}  // namespace koda

#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/comparation.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>
#include <iostream>

namespace koda {

SearchBinaryTree::SearchBinaryTree(size_t string_size) noexcept
    : string_size_{string_size} {}

void SearchBinaryTree::AddString(StringView string) {
    assert(string.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    InsertNewNode(string.data());
    ++buffer_start_index_;
}

void SearchBinaryTree::RemoveString(StringView string) {
    // auto iter = tree_.find(string);

    // if (iter == tree_.end()) {
    //     throw std::runtime_error{std::format(
    //         "Unknown string ({}) has been given",
    //         std::string_view{reinterpret_cast<const char*>(string.data()),
    //                          string.size()})};
    // }

    // // If it is present only one time in the dictionary then delete
    // if (iter->second.second == 1) {
    //     tree_.erase(iter);
    // } else {
    //     // Decrease a reference counter
    //     ++(iter->second.second);
    // }

    ++dictionary_start_index_;
}

SearchBinaryTree::RepeatitionMarker SearchBinaryTree::FindMatch(
    StringView buffer) const {
    assert(buffer.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    auto [position, length] = FindString(buffer.data());

    if (!length) {
        return {0, 0};
    }

    return {// Calculate relative offset from the start of the dictionary
            position - dictionary_start_index_, length};
}

SearchBinaryTree::Node::Node(const uint8_t* key, size_t insertion_index,
                             Node* parent, Color color)
    : key{std::move(key)},
      insertion_index{insertion_index},
      parent{parent},
      color{color} {}

SearchBinaryTree::Node::~Node() {
    if (left) {
        delete left;
    }
    if (right) {
        delete right;
    }
}

void SearchBinaryTree::RotateLeft(Node*& node) {
    Node* right = node->right;
    node->right = right->left;
    right->parent = node->parent;
    node->parent = right;
    right->left = node;
    node = right;
}

void SearchBinaryTree::RotateRight(Node*& node) {
    Node* left = node->left;
    node->left = left->right;
    left->parent = node->parent;
    node->parent = left;
    left->right = node;
    node = left;
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
        root_ = new Node{key, buffer_start_index_, nullptr};
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
    node->insertion_index = buffer_start_index_;
}

void SearchBinaryTree::BuildNode(const uint8_t* key, Node*& node,
                                 Node* parent) {
    node = new Node{key, buffer_start_index_, parent, Node::Color::kRed};
}

void SearchBinaryTree::dumpTree() {
    if (root_) {
        std::cout << root_ << "]\t";
        dumpTree(root_, "");
    }
}

void SearchBinaryTree::dumpTree(Node* parent, std::string view) {
    if (parent->left) {
        dumpTree(parent->left, view + "\t");
    }
    std::cout << parent << "]\t";
    std::cout << view << "|" << std::string_view{reinterpret_cast<const char*>(parent->key), 4} << "\n";
    if (parent->right) {
        dumpTree(parent->right, view + "\t");
    }
}

std::optional<SearchBinaryTree::NodeSpot> SearchBinaryTree::TryToInserLeaf(
    const uint8_t* key) {
    const StringView key_view{key, string_size_};
    Node** node = &root_;
    Node* parent = nullptr;
    while (*node) {
        switch (OrderCast(key_view <=> StringView{(*node)->key, string_size_})) {
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

    if (!node->parent) {
        root_ = node;
        return;
    }

    while (Node* parent = node->parent) {
        if (parent->color == Node::Color::kBlack) {
            return;
        }

        Node* grandparent = parent->parent;

        if (!grandparent) {
            parent->color = Node::Color::kBlack;
            return;
        }

        if (parent->parent->right) {
            // dir == right
            Node* uncle = grandparent->left;

            if (!uncle || uncle->color == Node::Color::kBlack) {
                if (node == parent->left) {
                    RotateRight(parent);
                    node = parent;
                    parent = grandparent->right;
                }

                RotateLeft(grandparent);
                parent->color = Node::Color::kBlack;
                grandparent->color = Node::Color::kRed;
                return;
            }
            uncle->color = Node::Color::kBlack;
        } else {
            // dir == left
            Node* uncle = grandparent->right;

            if (!uncle || uncle->color == Node::Color::kBlack) {
                if (node == parent->right) {
                    RotateLeft(parent);
                    node = parent;
                    parent = grandparent->left;
                }

                RotateRight(grandparent);
                parent->color = Node::Color::kBlack;
                grandparent->color = Node::Color::kRed;
                return;
            }
            uncle->color = Node::Color::kBlack;
        }

        parent->color = Node::Color::kBlack;
        grandparent->color = Node::Color::kRed;
        node = grandparent;
    };


    // Node* parent = node->parent;
    // Node* grand_parent = parent->parent;

    // while (parent->color == Node::Color::kRed) {
    //     FixLocalInsertionImbalance(node, parent, grand_parent);
    // }
    // root_->color = Node::Color::kBlack;
}

void SearchBinaryTree::FixLocalInsertionImbalance(Node*& node, Node*& parent,
                                                  Node*& grand_parent) {
    if (parent == grand_parent->left) {
        if (grand_parent->right->color == Node::Color::kRed) {
            FixInsertionGrandparentNodeColoring(node, grand_parent);
        } else {
            FixInsertionLeftGrandparentChildOrientation(node, parent,
                                                        grand_parent);
        }
    } else {
        if (grand_parent->left->color == Node::Color::kRed) {
            FixInsertionGrandparentNodeColoring(node, grand_parent);
        } else {
            FixInsertionRightGrandparentChildOrientation(node, parent,
                                                         grand_parent);
        }
    }
}

void SearchBinaryTree::FixInsertionGrandparentNodeColoring(
    Node*& node, Node*& grand_parent) {
    grand_parent->right->color = Node::Color::kBlack;
    grand_parent->left->color = Node::Color::kBlack;
    grand_parent->color = Node::Color::kRed;
    node = grand_parent;
}

void SearchBinaryTree::FixInsertionLeftGrandparentChildOrientation(
    Node*& node, Node*& parent, Node*& grand_parent) {
    if (node == parent->right) {
        RotateLeft(node = parent);
    }
    parent->color = Node::Color::kBlack;
    grand_parent->color = Node::Color::kRed;
    RotateRight(grand_parent);
}

void SearchBinaryTree::FixInsertionRightGrandparentChildOrientation(
    Node*& node, Node*& parent, Node*& grand_parent) {
    if (node == parent->left) {
        RotateRight(node = parent);
    }
    parent->color = Node::Color::kBlack;
    grand_parent->color = Node::Color::kRed;
    RotateLeft(grand_parent);
}

std::pair<size_t, size_t> SearchBinaryTree::FindString(
    const uint8_t* buffer) const {
    std::pair<size_t, size_t> match{};
    for (const Node* node = root_; node;) {
        auto prefix_length = FindCommonPrefixSize(buffer, node->key);
        if (prefix_length == string_size_) {
            return {node->insertion_index, string_size_};
        }
        UpdateMatchInfo(match, prefix_length, node);

        if (MakeView(buffer, prefix_length) <
            MakeView(node->key, prefix_length)) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return match;
}

SearchBinaryTree::StringView SearchBinaryTree::MakeView(
    const uint8_t* buffer, size_t prefix_length) const {
    return StringView{buffer + prefix_length, string_size_ - prefix_length};
}

/*static*/ void SearchBinaryTree::UpdateMatchInfo(
    std::pair<size_t, size_t>& match_info, size_t prefix_length,
    const Node* node) noexcept {
    if (match_info.second < prefix_length) {
        match_info.first = node->insertion_index;
        match_info.second = prefix_length;
    }
}

size_t SearchBinaryTree::FindCommonPrefixSize(
    const uint8_t* buffer, const uint8_t* node) const noexcept {
    for (size_t i = 0; i < string_size_; ++i) {
        if (buffer[i] != node[i]) {
            return i;
        }
    }
    return string_size_;
}

}  // namespace koda

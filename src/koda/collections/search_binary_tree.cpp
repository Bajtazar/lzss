#include <koda/collections/search_binary_tree.hpp>

#include <algorithm>
#include <assert>
#include <format>
#include <iostream>
#include <ranges>

namespace koda {

void SearchBinaryTree::AddString(StringView string) {
    auto iter = tree_.find(string);
    if (iter != tree_.end()) {
        // Referense youngest string so removal of the older references
        // does not invalidate index
        iter->second.first = buffer_start_index_;
        // Increase a reference counter of this string
        ++(iter->second.second);
    } else {
        // String is copied here since it can outlive its creator in circular
        // buffer
        tree_.emplace(string, std::forward_as_tuple(buffer_start_index_, 1));
    }
    ++buffer_start_index_;
}

void SearchBinaryTree::RemoveString(StringView string) {
    auto iter = tree_.find(string);

    if (iter == tree_.end()) {
        throw std::runtime_error{std::format(
            "Unknown string ({}) has been given",
            std::string_view{reinterpret_cast<const char*>(string.data()),
                             string.size()})};
    }

    // If it is present only one time in the dictionary then delete
    if (iter->second.second == 1) {
        tree_.erase(iter);
    } else {
        // Decrease a reference counter
        ++(iter->second.second);
    }

    ++dictionary_start_index_;
}

SearchBinaryTree::RepeatitionMarker SearchBinaryTree::FindMatch(
    StringView buffer) const {
    auto iter = tree_.lower_bound(buffer);
    if (iter == tree_.end()) {
        // No string has been found
        return {0, 0};
    }
    const auto common_length = FindCommonPrefixSize(buffer, iter->first);
    if (common_length == 0) {
        return {0, 0};
    }
    return {// Calculate relative offset from the start of the dictionary
            iter->second.first - dictionary_start_index_, common_length};
}

SearchBinaryTree::Node::Node(uint8_t* key, size_t insertion_index, Node* parent,
                             Color color)
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

void SearchBinaryTree::InsertNewNode(uint8_t* key) {
    if (!root_) [[unlikely]] {
        return root_.reset(
            new Node{.key = key, .insertion_index = buffer_start_index_});
    }
    if (auto inserted = TryToInserLeaf(key)) {
        BuildNode(key, inserted->first, inserted->second);
        assert(inserted->first->parent && "Parent has to exist");
        if (inserted->first->parent.color == Node::Color::kRed) {
            FixInsertionImbalance(inserted->first);
        }
    }
}

void SearchBinaryTree::UpdateNodeReference(Node* node, uint8_t* key) {
    ++node->ref_counter;
    node->key = key;
}

void SearchBinaryTree::BuildNode(uint8_t* key, Node*& node, Node* parent) {
    node.reset(
        new Node{.key = key, .parent = parent, .color = Node::Color::kRed});
}

std::optional<SearchBinaryTree::NodeSpot> SearchBinaryTree::TryToInserLeaf(
    uint8_t* key) {
    const StringView key_view{key, string_size_};
    for (Node *node = root_.get(), parent = nullptr; node; parent = node) {
        switch (key_view <=> StringView{node->key, string_size_}) {
            case std::weak_ordering::equivalent:
                UpdateNodeReference(node, key);
                return std::nullopt;
            case std::weak_ordering::less:
                node = node->left;
                break;
            case std::weak_ordering::greater:
                node = node->right;
                break;
            default:
                std::unreachable();
        };
    }
    [[asume(parent != nullptr)]];
    return {std::in_place, node.get(), parent};
}

void SearchBinaryTree::FixInsertionImbalance(Node*& node) {
    Node* parent = node->parent;
    Node* grand_parent = parent->parent;

    while (parent->color == Node::Color::kRed) {
        if (parent == grand_parent->left) {
            if (grand_parent->right == Node::Color::kRed) {
                grand_parent->right->color = Node::Color::kBlack;
                grand_parent->left->color = Node::Color::kBlack;
                grand_parent->color = Node::Color::kRed;
                node = grand_parent;
            } else {
                if (node == parent->right) {
                    RotateLeft(node = parent);
                }
                parent->color = Node::Color::kBlack;
                grand_parent->color = Node::Color::kRed;
                RotateRight(grand_parent);
            }

        } else {
            if (grand_parent->left == Node::Color::kRed) {
                grand_parent->right->color = Node::Color::kBlack;
                grand_parent->left->color = Node::Color::kBlack;
                grand_parent->color = Node::Color::kRed;
                node = grand_parent;
            } else {
                if (node == parent->left) {
                    RotateRight(node = parent);
                }
                parent->color = Node::Color::kBlack;
                grand_parent->color = Node::Color::kRed;
                RotateLeft(grand_parent);
            }
        }
    }
    root_->color = Node::Color::kBlack;
}

/*static*/ size_t SearchBinaryTree::FindCommonPrefixSize(
    StringView buffer, StringView node) noexcept {
    for (auto [index, comp] :
         std::views::zip(buffer, node) | std::views::enumerate) {
        const auto& [left, right] = comp;
        if (left != right) {
            return index;
        }
    }
    [[assume(buffer.size() <= node.size())]];
    return buffer.size();
}

}  // namespace koda

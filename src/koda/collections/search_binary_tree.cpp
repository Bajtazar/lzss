#include <koda/collections/search_binary_tree.hpp>

#include <algorithm>
#include <format>
#include <ranges>
#include <iostream>

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

SearchBinaryTree::Node::Node(uint8_t* key, Node* parent)
    : key{std::move(key)}, parent{parent} {}

void SearchBinaryTree::RotateLeft(std::unique_ptr<Node> node) {
    auto right = std::move(node->right);
    node->right = std::move(right->left);
    right->parent = node->parent;
    node->parent = right.get();
    right->left = std::move(node);
}

void SearchBinaryTree::RotateRight(std::unique_ptr<Node> node) {
    auto left = std::move(node->left);
    node->left = std::move(left->right);
    left->parent = node->parent;
    node->parent = left.get();
    left->right = std::move(node);
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

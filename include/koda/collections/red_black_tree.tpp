#pragma once

#include <algorithm>
#include <cassert>
#include <ranges>

namespace koda {

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::RedBlackTree(
    const AllocatorTp& allocator) noexcept
    : pool_{allocator} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::RedBlackTree(
    RedBlackTree&& other) noexcept
    : root_{std::exchange(other.root_, nullptr)},
      pool_{std::move(other.pool_)} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>&
RedBlackTree<ValueTp, AllocatorTp>::operator=(RedBlackTree&& other) noexcept {
    Destroy();
    root_ = std::exchange(other.root_, nullptr);
    pool_ = std::move(other.pool_);
    return *this;
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::~RedBlackTree() {
    Destroy();
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::Destroy() {
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

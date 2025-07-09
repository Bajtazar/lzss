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
[[nodiscard]] constexpr AllocatorTp
RedBlackTree<ValueTp, AllocatorTp>::get_allocator() const {
    return pool_.get_allocator();
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

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::Node::Node(ValueTp value,
                                                         Node* parent,
                                                         Color color)
    : value{std::move(value)}, parent{parent}, color{color} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp,
                       AllocatorTp>::NodePool::Scheduler::~Scheduler() {
    pool.ReturnNode(node);
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool::NodePool(
    const AllocatorTp& allocator) noexcept
    : allocator_{allocator} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool::NodePool(
    NodePool&& other) noexcept
    : allocator_{std::move(other.allocator_)},
      handle_{std::exchange(other.handle_, nullptr)} {}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool&
RedBlackTree<ValueTp, AllocatorTp>::NodePool::operator=(
    NodePool&& other) noexcept {
    Destroy();
    allocator_ = std::move(other.allocator_);
    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::NodePool::ReturnNode(
    Node* handle) {
    handle->left = handle_;
    handle_ = handle;
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool::Scheduler
RedBlackTree<ValueTp, AllocatorTp>::NodePool::ScheduleForReturn(Node* node) {
    return Scheduler{*this, node};
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::Node*
RedBlackTree<ValueTp, AllocatorTp>::NodePool::GetNode(ValueTp value,
                                                      Node* parent,
                                                      Node::Color color) {
    if (!handle_) {
        Node* node = NodeTraits::allocate(allocator_, 1);
        NodeTraits::construct(allocator_, node, std::move(value), parent,
                              color);
        return node;
    }
    Node* node = handle_;
    handle_ = node->left;

    *node = Node{std::move(value), parent, color};
    return node;
}

template <typename ValueTp, typename AllocatorTp>
constexpr AllocatorTp
RedBlackTree<ValueTp, AllocatorTp>::NodePool::get_allocator() const {
    return allocator_;
}

template <typename ValueTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodePool::~NodePool() {
    Destroy();
}

template <typename ValueTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, AllocatorTp>::NodePool::Destroy() {
    for (Node* node = handle_; node;) {
        Node* old_node = node;
        node = node->left;
        NodeTraits::destroy(allocator_, old_node);
        NodeTraits::deallocate(allocator_, old_node, 1);
    }
}

// Recursiveless tree iterator!
template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIterator<
    IsConst>::NodeIterator(pointer_type node, pointer_type previous) noexcept
    : current_{node}, previous_{previous} {}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIterator<
    IsConst>::value_type
RedBlackTree<ValueTp, AllocatorTp>::NodeIterator<IsConst>::operator*()
    const noexcept {
    return *current_;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIterator<
    IsConst>::pointer_type
RedBlackTree<ValueTp, AllocatorTp>::NodeIterator<IsConst>::operator->()
    const noexcept {
    return current_;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
constexpr RedBlackTree<ValueTp, AllocatorTp>::NodeIterator<IsConst>&
RedBlackTree<ValueTp,
             AllocatorTp>::NodeIterator<IsConst>::operator++() noexcept {
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

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<ValueTp,
                                     AllocatorTp>::NodeIterator<IsConst>
RedBlackTree<ValueTp, AllocatorTp>::NodeIterator<IsConst>::operator++(
    int) noexcept {
    auto temp = *this;
    ++(*this);
    return temp;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr bool
RedBlackTree<ValueTp, AllocatorTp>::NodeIterator<IsConst>::operator==(
    const NodeIterator& other) const noexcept {
    return (current_ == other.current_) && (previous_ == other.previous_);
}

}  // namespace koda

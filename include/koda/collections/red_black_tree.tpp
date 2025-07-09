#pragma once

#include <algorithm>
#include <cassert>
#include <ranges>

namespace koda {

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::RedBlackTree(
    const AllocatorTp& allocator) noexcept
    : pool_{allocator} {}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::RedBlackTree(
    RedBlackTree&& other) noexcept
    : root_{std::exchange(other.root_, nullptr)},
      pool_{std::move(other.pool_)} {}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>&
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::operator=(
    RedBlackTree&& other) noexcept {
    Destroy();
    root_ = std::exchange(other.root_, nullptr);
    pool_ = std::move(other.pool_);
    return *this;
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::~RedBlackTree() {
    Destroy();
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::get_allocator() const {
    return pool_.get_allocator();
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::Destroy() {
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

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::Node::Node(
    ValueTp value, Node* parent, Color color)
    : value{std::move(value)}, parent{parent}, color{color} {}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp,
                       AllocatorTp>::NodePool::Scheduler::~Scheduler() {
    pool.ReturnNode(node);
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::NodePool(
    const AllocatorTp& allocator) noexcept
    : allocator_{allocator} {}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::NodePool(
    NodePool&& other) noexcept
    : allocator_{std::move(other.allocator_)},
      handle_{std::exchange(other.handle_, nullptr)} {}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool&
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::operator=(
    NodePool&& other) noexcept {
    Destroy();
    allocator_ = std::move(other.allocator_);
    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, DerivedTp,
                            AllocatorTp>::NodePool::ReturnNode(Node* handle) {
    handle->left = handle_;
    handle_ = handle;
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::Scheduler
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::ScheduleForReturn(
    Node* node) {
    return Scheduler{*this, node};
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::Node*
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::GetNode(
    ValueTp value, Node* parent, Node::Color color) {
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

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr AllocatorTp
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::get_allocator() const {
    return allocator_;
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::~NodePool() {
    Destroy();
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr void
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodePool::Destroy() {
    for (Node* node = handle_; node;) {
        Node* old_node = node;
        node = node->left;
        NodeTraits::destroy(allocator_, old_node);
        NodeTraits::deallocate(allocator_, old_node, 1);
    }
}

// Recursiveless tree iterator!
template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
template <bool IsConst>
constexpr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::NodeIteratorBase(pointer_type node,
                               pointer_type previous) noexcept
    : current_{node}, previous_{previous} {}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<
    ValueTp, DerivedTp, AllocatorTp>::NodeIteratorBase<IsConst>::value_type
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::operator*() const noexcept {
    return *current_;
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<
    ValueTp, DerivedTp, AllocatorTp>::NodeIteratorBase<IsConst>::pointer_type
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::operator->() const noexcept {
    return current_;
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
template <bool IsConst>
constexpr RedBlackTree<ValueTp, DerivedTp,
                       AllocatorTp>::NodeIteratorBase<IsConst>&
RedBlackTree<ValueTp, DerivedTp,
             AllocatorTp>::NodeIteratorBase<IsConst>::operator++() noexcept {
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

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr RedBlackTree<ValueTp, DerivedTp,
                                     AllocatorTp>::NodeIteratorBase<IsConst>
RedBlackTree<ValueTp, DerivedTp,
             AllocatorTp>::NodeIteratorBase<IsConst>::operator++(int) noexcept {
    auto temp = *this;
    ++(*this);
    return temp;
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr bool
RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::NodeIteratorBase<
    IsConst>::operator==(const NodeIteratorBase& other) const noexcept {
    return (current_ == other.current_) && (previous_ == other.previous_);
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr const NodePtr RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::root()
    const noexcept {
    return root_;
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
template <std::three_way_comparable_with<ValueTp> KeyTp>
constexpr void RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::InsertNode(
    ValueTp value) {
    if (!root_) [[unlikely]] {
        root_ = pool_.GetNode(std::move(value), buffer_start_index_);
        return CheckInvariants();
    }
    if (auto inserted =
            static_cast<DerivedTp*>(this)->FindInsertionLocation(value)) {
        BuildNode(std::move(value), inserted->first, inserted->second);
        assert(inserted->first->parent && "Parent has to exist");
        if (inserted->first->parent->color == Node::Color::kRed) {
            FixInsertionImbalance(inserted->first);
        }
    }
    CheckInvariants();
}

template <typename ValueTp, typename DerivedTp, typename AllocatorTp>
constexpr void RedBlackTree<ValueTp, DerivedTp, AllocatorTp>::RemoveNode(
    NodePtr node) {
    if (node->left && node->right) {
        RemoveNodeWithTwoChildren(node);
        return CheckInvariants();
    }

    if (node->left || node->right) {
        RemoveNodeWithOneChild(node, node->left ? node->left : node->right);
        return CheckInvariants();
    }

    RemoveChildlessNode(node);
    CheckInvariants();
}

}  // namespace koda

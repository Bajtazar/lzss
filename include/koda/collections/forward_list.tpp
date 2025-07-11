#pragma once

#include <utility>

namespace koda {

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
constexpr ForwardList<ValueTp, AllocatorTp>::Iterator<IsConst>::Iterator(
    pointer_type node) noexcept
    : node_{node} {}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr ForwardList<ValueTp,
                                    AllocatorTp>::Iterator<IsConst>::value_type
ForwardList<ValueTp, AllocatorTp>::Iterator<IsConst>::operator*()
    const noexcept {
    return node_->value;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::Iterator<
    IsConst>::pointer_type
ForwardList<ValueTp, AllocatorTp>::Iterator<IsConst>::operator->()
    const noexcept {
    return &(*(*this));
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
constexpr ForwardList<ValueTp, AllocatorTp>::Iterator<IsConst>&
ForwardList<ValueTp, AllocatorTp>::Iterator<IsConst>::operator++() noexcept {
    node_ = node_->next;
    return *this;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::Iterator<IsConst>
ForwardList<ValueTp, AllocatorTp>::Iterator<IsConst>::operator++(int) noexcept {
    auto temp = *this;
    node_ = node_->next;
    return temp;
}

template <typename ValueTp, typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr bool
ForwardList<ValueTp, AllocatorTp>::Iterator<IsConst>::operator==(
    const Iterator& right) const noexcept {
    return node_ == right.node_;
}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::ForwardList(
    const AllocatorTp& allocator)
    : pool_{allocator} {}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::ForwardList(
    ForwardList&& other) noexcept
    : pool_{std::move(other.pool_)},
      root_{std::exchange(other.root_, nullptr)},
      size_{std::exchange(other.size_, 0)} {}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>&
ForwardList<ValueTp, AllocatorTp>::operator=(ForwardList&& other) noexcept {
    Destroy();
    pool_ = std::move(other.pool_);
    root_ = std::exchange(other.root_, nullptr);
    size_ = std::exchange(other.size_, 0);
    return *this;
}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::~ForwardList() {
    Destroy();
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr size_t ForwardList<ValueTp, AllocatorTp>::size()
    const noexcept {
    return size_;
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr bool ForwardList<ValueTp, AllocatorTp>::empty()
    const noexcept {
    return !size_;
}

template <typename ValueTp, typename AllocatorTp>
template <typename... Args>
    requires std::constructible_from<ValueTp, Args...>
constexpr ForwardList<ValueTp, AllocatorTp>::iterator
ForwardList<ValueTp, AllocatorTp>::PushFront(Args&&... args) {
    Node* node = pool_.GetNode(std::forward<Args>(args)...);
    node->next = root_;
    root_ = node;
    ++size_;
    return iterator{node};
}

template <typename ValueTp, typename AllocatorTp>
constexpr void ForwardList<ValueTp, AllocatorTp>::PopFront() {
    Node* node = root_;
    root_ = node->next;
    --size_;
    pool_.ReturnNode(node);
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] AllocatorTp ForwardList<ValueTp, AllocatorTp>::get_allocator()
    const {
    return pool_.get_allocator();
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::iterator
ForwardList<ValueTp, AllocatorTp>::begin() noexcept {
    return iterator{root_};
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::iterator
ForwardList<ValueTp, AllocatorTp>::end() noexcept {
    return iterator{nullptr};
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::const_iterator
ForwardList<ValueTp, AllocatorTp>::begin() const noexcept {
    return const_iterator{root_};
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::const_iterator
ForwardList<ValueTp, AllocatorTp>::end() const noexcept {
    return const_iterator{nullptr};
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::const_iterator
ForwardList<ValueTp, AllocatorTp>::cbegin() const noexcept {
    return begin();
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::const_iterator
ForwardList<ValueTp, AllocatorTp>::cend() const noexcept {
    return end();
}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::NodePool::NodePool(
    const AllocatorTp& allocator)
    : allocator_{allocator} {}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::NodePool::NodePool(
    NodePool&& pool) noexcept
    : allocator_{std::move(pool.allocator_)},
      handle_{std::exchange(pool.handle_)} {}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::NodePool&
ForwardList<ValueTp, AllocatorTp>::NodePool::operator=(
    NodePool&& pool) noexcept {
    Destroy();
    allocator_ = std::move(pool.allocator_);
    handle_ = std::exchange(pool.handle_);
    return *this;
}

template <typename ValueTp, typename AllocatorTp>
constexpr void ForwardList<ValueTp, AllocatorTp>::NodePool::ReturnNode(
    Node* handle) {
    handle->left = handle_;
    handle_ = handle;
    std::destroy_at(&handle_->value);
}

template <typename ValueTp, typename AllocatorTp>
template <typename... Args>
    requires std::constructible_from<ValueTp, Args...>
constexpr ForwardList<ValueTp, AllocatorTp>::Node*
ForwardList<ValueTp, AllocatorTp>::NodePool::GetNode(Args&&... args) {
    if (!handle_) {
        Node* node = NodeTraits::allocate(allocator_, 1);
        NodeTraits::construct(allocator_, node, std::forward<Args>(args)...);
        return node;
    }
    Node* node = handle_;
    handle_ = node->left;

    NodeTraits::construct(allocator_, node, std::forward<Args>(args)...);
    return node;
}

template <typename ValueTp, typename AllocatorTp>
constexpr AllocatorTp
ForwardList<ValueTp, AllocatorTp>::NodePool::get_allocator() const {
    return allocator_;
}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::NodeAllocatorTp&
ForwardList<ValueTp, AllocatorTp>::NodePool::get_node_allocator() noexcept {
    return allocator_;
}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::NodePool::~NodePool() {
    Destroy();
}

template <typename ValueTp, typename AllocatorTp>
constexpr void ForwardList<ValueTp, AllocatorTp>::NodePool::Destroy() {
    for (Node* node = handle_; node;) {
        Node* old_node = node;
        node = node->left;
        NodeTraits::deallocate(allocator_, old_node, 1);
    }
}

template <typename ValueTp, typename AllocatorTp>
constexpr void ForwardList<ValueTp, AllocatorTp>::Destroy() {
    for (Node* node = root_; node;) {
        auto next_node = node->next;
        NodeTraits::destroy(pool_.get_node_allocator(), node);
        NodeTraits::deallocate(pool_.get_node_allocator(), node, 1);
        node = next_node;
    }
}

}  // namespace koda

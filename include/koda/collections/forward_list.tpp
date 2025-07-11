#pragma once

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
[[nodiscard]] constexpr ForwardList<ValueTp, AllocatorTp>::pointer_type
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
    const Iterator& left, const Iterator& right) const noexcept {
    return left.node_ == right.node_;
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
constexpr ForwardList<ValueTp, AllocatorTp>
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
template <typename... Args>
    requires std::constructible_from<ValueTp, Args...>
constexpr iterator ForwardList<ValueTp, AllocatorTp>::PushFront(
    Args&&... args) {
    Node* node = pool_.GetNode(std::forward<Args>(args)...);
    node->next = root_;
    root_ = node;
    return iterator{node};
}

template <typename ValueTp, typename AllocatorTp>
constexpr void ForwardList<ValueTp, AllocatorTp>::PopFront() {
    Node* node = root_;
    root_ = node->next;
    pool_.ReturnNode(node);
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] AllocatorTp ForwardList<ValueTp, AllocatorTp>::get_allocator()
    const {
    return pool_.get_allocator();
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp ForwardList<ValueTp, AllocatorTp>::iterator
    AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::begin() noexcept {
    return iterator{root_};
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp ForwardList<ValueTp, AllocatorTp>::iterator
    AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::end() noexcept {
    return iterator{nullptr};
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::const_iterator AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::begin() const noexcept {
    return const_iterator{root_};
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::const_iterator AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::end() const noexcept {
    return const_iterator{nullptr};
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::const_iterator AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::cbegin() const noexcept {
    return begin();
}

template <typename ValueTp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::const_iterator AllocatorTp
    ForwardList<ValueTp, AllocatorTp>::cend() const noexcept {
    return end();
}

template <typename ValueTp, typename AllocatorTp>
constexpr ForwardList<ValueTp, AllocatorTp>::Destroy() {
    for (Node* node = root_; node;) {
        auto next_node = node->next;
        NodeTraits::destroy(pool_.get_node_allocator(), node);
        NodeTraits::deallocate(pool_.get_node_allocator(), node, 1);
        node = next_node;
    }
}

}  // namespace koda

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
constexpr ForwardList<ValueTp, AllocatorTp>::Destroy() {
    for (Node* node = root_; node;) {
        auto next_node = node->next;
        NodeTraits::destroy(pool_.get_allocator(), node);
        NodeTraits::dealloate(pool_.get_allocator(), node, 1);
        node = next_node;
    }
}

}  // namespace koda

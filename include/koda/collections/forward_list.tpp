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

}  // namespace koda

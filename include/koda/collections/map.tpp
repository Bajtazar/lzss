#pragma once

namespace koda {

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Map(
    const ComparatorTp& comparator, const AllocatorTp& allocator)
    : RedBlackTree<std::pair<const KeyTp, ValueTp>, Map,
                   AllocatorTp>{allocator},
      comparator_{comparator} {}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <bool IsConst>
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<IsConst>::Iterator(
    UnderlyingIter iterator) noexcept
    : iterator_{std::move(iterator)} {}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::Iterator<IsConst>::value_type
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<IsConst>::operator*()
    const noexcept {
    return (*iterator_).value;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::Iterator<IsConst>::pointer_type
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<IsConst>::operator->()
    const noexcept {
    return &(*(*this));
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <bool IsConst>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<IsConst>&
Map<KeyTp, ValueTp, ComparatorTp,
    AllocatorTp>::Iterator<IsConst>::operator++() noexcept {
    ++iterator_;
    return *this;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::Iterator<IsConst>
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<IsConst>::operator++(
    int) noexcept {
    auto temp = *this;
    ++(*this);
    return temp;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr bool
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<IsConst>::operator==(
    const Iterator& other) const noexcept {
    return iterator_ == other.iterator_;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Insert(entry_type entry) {}

}  // namespace koda

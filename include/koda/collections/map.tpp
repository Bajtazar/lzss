#pragma once

namespace koda {

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Map(
    const ComparatorTp& comparator, const AllocatorTp& allocator)
    : RedBlackImpl{allocator}, comparator_{comparator} {}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <bool IsConst>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<
    IsConst>::Iterator(UnderlyingIter iterator) noexcept
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
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Insert(entry_type entry) {
    auto* node = this->InsertNode(std::move(entry));
    return iterator{NodeIterator{node, node->parent}};
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Emplace(key_type key,
                                                        value_type value) {
    auto* node = this->InsertNode(entry_type{std::move(key), std::move(value)});
    return iterator{NodeIterator{node, node->parent}};
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <typename... KeyArgs, typename... ValueArgs>
    requires(std::constructible_from<KeyTp, KeyArgs...> &&
             std::constructible_from<ValueTp, ValueArgs...>)
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Emplace(
    std::piecewise_construct_t, std::tuple<KeyArgs...> key_args,
    std::tuple<ValueArgs...> value_args) {
    auto* node = this->InsertNode(
        entry_type{std::make_from_tuple<key_type>(std::move(key_args)),
                   std::make_from_tuple<value_type>(std::move(value_args))});
    return iterator{NodeIterator{node, node->parent}};
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <typename KeyLookupTp>
    requires std::predicate<ComparatorTp, KeyTp, KeyLookupTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Find(KeyLookupTp&& key) {
    for (Node* node = this->root(); node;) {
        switch (OrderCast(comparator_(key, node->value.first))) {
            case WeakOrdering::kEquivalent:
                return const_iterator{NodeConstIterator{node, node->parent}};
            case WeakOrdering::kLess:
                node = node->left;
                break;
            case WeakOrdering::kGreater:
                node = node->right;
                break;
            default:
                std::unreachable();
        };
    }
    return cend();
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <typename KeyLookupTp>
    requires std::predicate<ComparatorTp, KeyTp, KeyLookupTp>
constexpr bool Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Remove(
    KeyLookupTp&& key) {
    if (auto iterator = Find(std::forward<KeyLookupTp>(key));
        iterator != cend()) {
        return Remove(iterator);
    }
    return false;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr bool Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Remove(
    const_iterator position) {
    if (position == cend()) {
        return false;
    }
    this->RemoveNode(*position.iterator_);
    return true;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::begin() noexcept {
    return iterator{this->node_begin()};
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::end() noexcept {
    return iterator{this->node_end()};
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::begin() const noexcept {
    return const_iterator{this->node_begin()};
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::end() const noexcept {
    return const_iterator{this->node_end()};
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::cbegin() const noexcept {
    return begin();
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::cend() const noexcept {
    return end();
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::NodeInsertionLocation
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::FindInsertionLocation(
    const entry_type& entry) {
    Node** node = &this->root();
    Node* parent = nullptr;
    while (*node) {
        switch (OrderCast(comparator_(entry.first, (*node)->value.first))) {
            case WeakOrdering::kEquivalent:
                return std::nullopt;
            case WeakOrdering::kLess:
                parent = *node;
                node = &(*node)->left;
                break;
            case WeakOrdering::kGreater:
                parent = *node;
                node = &(*node)->right;
                break;
            default:
                std::unreachable();
        };
    }
    return NodeInsertionLocation{std::in_place, *node, parent};
}

}  // namespace koda

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
template <std::ranges::input_range Range>
    requires SpecializationOf<std::ranges::range_value_t<Range>, std::pair>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Map(
    Range&& range, const ComparatorTp& comparator, const AllocatorTp& allocator)
    : Map{comparator, allocator} {
    for (auto&& [key, value] : range) {
        Emplace(std::forward_like<Range>(key), std::forward_like<Range>(value));
    }
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Map(
    std::initializer_list<entry_type> init, const ComparatorTp& comparator,
    const AllocatorTp& allocator)
    : Map{comparator, allocator} {
    for (auto&& value : init) {
        Insert(std::move(value));
    }
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Map(const Map& other)
    requires std::is_copy_constructible_v<entry_type>
    : size_{other.size_}, comparator_{other.comparator_} {
    this->CloneFrom(other);
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>&
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::operator=(const Map& other)
    requires std::is_copy_constructible_v<entry_type>
{
    this->Destroy();
    size_ = other.size_;
    comparator_ = other.comparator_;
    this->CloneFrom(other);

    return *this;
}

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
                            AllocatorTp>::Iterator<IsConst>::reference_type
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
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<IsConst>&
Map<KeyTp, ValueTp, ComparatorTp,
    AllocatorTp>::Iterator<IsConst>::operator--() noexcept {
    --iterator_;
    return *this;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <bool IsConst>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::Iterator<IsConst>
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Iterator<IsConst>::operator--(
    int) noexcept {
    auto temp = *this;
    --(*this);
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
    return InsertHelper(this->InsertNode(std::move(entry)));
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Emplace(key_type key,
                                                        value_type value) {
    return InsertHelper(
        this->InsertNode(entry_type{std::move(key), std::move(value)}));
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
    return InsertHelper(this->InsertNode(
        entry_type{std::make_from_tuple<key_type>(std::move(key_args)),
                   std::make_from_tuple<value_type>(std::move(value_args))}));
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <typename KeyLookupTp>
    requires Invocable<ComparatorTp, std::weak_ordering, KeyTp, KeyLookupTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Find(KeyLookupTp&& key) const {
    return FindHelper(*this, std::forward<KeyLookupTp>(key));
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <typename KeyLookupTp>
    requires Invocable<ComparatorTp, std::weak_ordering, KeyTp, KeyLookupTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Find(KeyLookupTp&& key) {
    return FindHelper(*this, std::forward<KeyLookupTp>(key));
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <typename KeyLookupTp>
    requires Invocable<ComparatorTp, std::weak_ordering, KeyTp, KeyLookupTp>
constexpr bool Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Remove(
    KeyLookupTp&& key) {
    if (iterator iter = Find(std::forward<KeyLookupTp>(key)); iter != end()) {
        return Remove(iter);
    }
    return false;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr bool Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::Remove(
    iterator position) {
    if (position == end()) {
        return false;
    }
    this->RemoveNode(&(*position.iterator_));
    --size_;
    return true;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr size_t
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::size() const noexcept {
    return size_;
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr bool
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::empty() const noexcept {
    return !size_;
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
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::reverse_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::rbegin() noexcept {
    return std::make_reverse_iterator(end());
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::reverse_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::rend() noexcept {
    return std::make_reverse_iterator(begin());
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_reverse_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::rbegin() const noexcept {
    return std::make_reverse_iterator(end());
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_reverse_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::rend() const noexcept {
    return std::make_reverse_iterator(begin());
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_reverse_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::rcbegin() const noexcept {
    return rbegin();
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
[[nodiscard]] constexpr Map<KeyTp, ValueTp, ComparatorTp,
                            AllocatorTp>::const_reverse_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::rcend() const noexcept {
    return rend();
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

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
template <typename Self, typename KeyLookupTp>
    requires Invocable<ComparatorTp, std::weak_ordering, KeyTp, KeyLookupTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp,
              AllocatorTp>::Iterator<std::is_const_v<Self>>
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::FindHelper(Self& self,
                                                           KeyLookupTp&& key) {
    using NodeTp =
        std::conditional_t<std::is_const_v<Self>, const Node*, Node*>;

    for (NodeTp node = self.root(); node;) {
        switch (OrderCast(self.comparator_(key, node->value.first))) {
            case WeakOrdering::kEquivalent:
                return NodeToIter(node);
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
    return self.end();
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::InsertHelper(Node* node) {
    if (node) {
        ++size_;
        return iterator{NodeIterator{node}};
    }
    return end();
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
/*static*/ constexpr Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::NodeToIter(Node* node) {
    return iterator{NodeIterator{node, Map::kIteratorFromNode}};
}

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp,
          typename AllocatorTp>
/*static*/ constexpr Map<KeyTp, ValueTp, ComparatorTp,
                         AllocatorTp>::const_iterator
Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>::NodeToIter(const Node* node) {
    return const_iterator{NodeConstIterator{node, Map::kIteratorFromNode}};
}

}  // namespace koda

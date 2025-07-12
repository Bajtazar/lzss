#pragma once

#include <koda/collections/red_black_tree.hpp>
#include <koda/utils/comparation.hpp>
#include <koda/utils/concepts.hpp>

#include <concepts>
#include <iterator>

namespace koda {

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp =
              ThreeWayComparator,
          typename AllocatorTp =
              std::allocator<std::pair<const KeyTp, ValueTp>>>
class Map : public RedBlackTree<std::pair<const KeyTp, ValueTp>,

                                AllocatorTp> {
    using RedBlackImpl =
        RedBlackTree<std::pair<const KeyTp, ValueTp>, AllocatorTp>;

    template <bool IsConst>
    class Iterator {
        using UnderlyingIter = typename RedBlackImpl::NodeIteratorBase<IsConst>;

       public:
        using value_type =
            std::conditional_t<IsConst, const std::pair<const KeyTp, ValueTp>,
                               std::pair<const KeyTp, ValueTp>>;
        using reference_type = value_type&;
        using pointer_type = value_type*;
        using difference_type = std::ptrdiff_t;

        constexpr explicit Iterator(
            UnderlyingIter iterator = UnderlyingIter{}) noexcept;

        [[nodiscard]] constexpr reference_type operator*() const noexcept;

        [[nodiscard]] constexpr pointer_type operator->() const noexcept;

        constexpr Iterator& operator++() noexcept;

        [[nodiscard]] constexpr Iterator operator++(int) noexcept;

        constexpr Iterator& operator--() noexcept;

        [[nodiscard]] constexpr Iterator operator--(int) noexcept;

        [[nodiscard]] constexpr bool operator==(
            const Iterator& other) const noexcept;

        friend class Map;

       private:
        UnderlyingIter iterator_;
    };

   public:
    using entry_type = std::pair<const KeyTp, ValueTp>;
    using value_type = ValueTp;
    using key_type = const KeyTp;
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr explicit Map(const ComparatorTp& comparator = ComparatorTp{},
                           const AllocatorTp& allocator = AllocatorTp{});

    template <std::ranges::input_range Range>
        requires SpecializationOf<std::ranges::range_value_t<Range>, std::pair>
    constexpr explicit Map(Range&& range,
                           const ComparatorTp& comparator = ComparatorTp{},
                           const AllocatorTp& allocator = AllocatorTp{});

    constexpr Map(std::initializer_list<entry_type> init,
                  const ComparatorTp& comparator = ComparatorTp{},
                  const AllocatorTp& allocator = AllocatorTp{});

    constexpr Map(const Map& other)
        requires std::is_copy_constructible_v<entry_type>;
    constexpr Map(Map&& other) = default;

    constexpr Map& operator=(const Map& other)
        requires std::is_copy_constructible_v<entry_type>;
    constexpr Map& operator=(Map&& other) = default;

    constexpr ~Map() noexcept override = default;

    constexpr iterator Insert(entry_type entry);

    constexpr iterator Emplace(key_type key, value_type value);

    template <typename... KeyArgs, typename... ValueArgs>
        requires(std::constructible_from<KeyTp, KeyArgs...> &&
                 std::constructible_from<ValueTp, ValueArgs...>)
    constexpr iterator Emplace(std::piecewise_construct_t,
                               std::tuple<KeyArgs...> key_args,
                               std::tuple<ValueArgs...> value_args);

    template <typename KeyLookupTp>
        requires Invocable<ComparatorTp, std::weak_ordering, KeyTp, KeyLookupTp>
    [[nodiscard]] constexpr iterator Find(KeyLookupTp&& key);

    template <typename KeyLookupTp>
        requires Invocable<ComparatorTp, std::weak_ordering, KeyTp, KeyLookupTp>
    [[nodiscard]] constexpr const_iterator Find(KeyLookupTp&& key) const;

    template <typename KeyLookupTp>
        requires Invocable<ComparatorTp, std::weak_ordering, KeyTp, KeyLookupTp>
    constexpr bool Remove(KeyLookupTp&& key);

    constexpr bool Remove(iterator position);

    [[nodiscard]] constexpr size_t size() const noexcept;

    [[nodiscard]] constexpr bool empty() const noexcept;

    [[nodiscard]] constexpr iterator begin() noexcept;

    [[nodiscard]] constexpr iterator end() noexcept;

    [[nodiscard]] constexpr const_iterator begin() const noexcept;

    [[nodiscard]] constexpr const_iterator end() const noexcept;

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept;

    [[nodiscard]] constexpr const_iterator cend() const noexcept;

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept;

    [[nodiscard]] constexpr reverse_iterator rend() noexcept;

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept;

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept;

    [[nodiscard]] constexpr const_reverse_iterator rcbegin() const noexcept;

    [[nodiscard]] constexpr const_reverse_iterator rcend() const noexcept;

   private:
    using NodeInsertionLocation = RedBlackImpl::NodeInsertionLocation;
    using NodeIterator = RedBlackImpl::NodeIterator;
    using NodeConstIterator = RedBlackImpl::NodeConstIterator;
    using Node = RedBlackImpl::Node;

    size_t size_ = 0;
    [[no_unique_address]] ComparatorTp comparator_;

    constexpr NodeInsertionLocation FindInsertionLocation(
        const entry_type& value) override final;

    template <typename KeyLookupTp, typename Self>
        requires Invocable<ComparatorTp, std::weak_ordering, KeyTp, KeyLookupTp>
    constexpr auto FindHelper(this Self&& self, KeyLookupTp&& key);

    constexpr iterator InsertHelper(Node* node);

    static constexpr iterator NodeToIter(Node* node);

    static constexpr const_iterator NodeToIter(const Node* node);
};

}  // namespace koda

#include <koda/collections/map.tpp>

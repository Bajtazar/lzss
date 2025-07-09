#pragma once

#include <koda/collections/red_black_tree.hpp>
#include <koda/utils/comparation.hpp>
#include <koda/utils/concepts.hpp>

#include <concepts>

namespace koda {

template <typename KeyTp, typename ValueTp,
          Invocable<std::weak_ordering, KeyTp, KeyTp> ComparatorTp =
              ThreeWayComparator,
          typename AllocatorTp =
              std::allocator<std::pair<const KeyTp, ValueTp>>>
class Map : public RedBlackTree<std::pair<const KeyTp, ValueTp>,
                                Map<KeyTp, ValueTp, ComparatorTp, AllocatorTp>,
                                AllocatorTp> {
    template <bool IsConst>
    class Iterator {
        using UnderlyingIter =
            RedBlackTree<std::pair<const KeyTp, ValueTp>, Map,
                         AllocatorTp>::NodeIteratorBase<IsConst>;

       public:
        using value_type =
            std::conditional_t<IsConst, const std::pair<const KeyTp, ValueTp>&,
                               std::pair<const KeyTp, ValueTp>&>;
        using pointer_type =
            std::conditional_t<IsConst, const std::pair<const KeyTp, ValueTp>*,
                               std::pair<const KeyTp, ValueTp>*>;
        using difference_type = std::ptrdiff_t;

        constexpr explicit Iterator(
            UnderlyingIter iterator = UnderlyingIter{}) noexcept;

        [[nodiscard]] constexpr value_type operator*() const noexcept;

        [[nodiscard]] constexpr pointer_type operator->() const noexcept;

        constexpr Iterator& operator++() noexcept;

        [[nodiscard]] constexpr Iterator operator++(int) noexcept;

        [[nodiscard]] constexpr bool operator==(
            const Iterator& other) const noexcept;

       private:
        UnderlyingIter iterator_;
    };

   public:
    using entry_type = std::pair<const KeyTp, ValueTp>;
    using value_type = ValueTp;
    using key_type = const KeyTp;
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    constexpr explicit Map(const ComparatorTp& comparator = ComparatorTp{},
                           const AllocatorTp& allocator = AllocatorTp{});

    constexpr Map(const Map& map) = delete;
    constexpr Map(Map&& map) = default;

    constexpr Map& operator=(const Map& map) = delete;
    constexpr Map& operator=(Map&& map) = default;

    constexpr ~Map() noexcept override = default;

    constexpr iterator Insert(entry_type entry);

    template <typename... Args>
    constexpr iterator Emplace(Args&&... args);

    template <typename KeyLookupTp>
        requires std::predicate<ComparatorTp, KeyTp, KeyLookupTp>
    [[nodiscard]] constexpr const_iterator Find(KeyLookupTp&& key);

    template <typename KeyLookupTp>
        requires std::predicate<ComparatorTp, KeyTp, KeyLookupTp>
    constexpr bool Remove(KeyLookupTp&& key);

    constexpr bool Remove(const_iterator position);

    [[nodiscard]] constexpr iterator begin() noexcept;

    [[nodiscard]] constexpr iterator end() noexcept;

    [[nodiscard]] constexpr const_iterator begin() const noexcept;

    [[nodiscard]] constexpr const_iterator end() const noexcept;

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept;

    [[nodiscard]] constexpr const_iterator cend() const noexcept;

    friend class RedBlackTree<std::pair<const KeyTp, ValueTp>, Map,
                              AllocatorTp>;

   private:
    using RedBlackImpl =
        RedBlackTree<std::pair<const KeyTp, ValueTp>, Map, AllocatorTp>;
    using NodeInsertionLocation = RedBlackImpl::NodeInsertionLocation;

    [[no_unique_address]] ComparatorTp comparator_;

    constexpr NodeInsertionLocation FindInsertionLocation(
        const entry_type& value) override final;
};

}  // namespace koda

#include <koda/collections/map.tpp>

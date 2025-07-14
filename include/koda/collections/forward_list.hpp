#pragma once

#include <concepts>
#include <iterator>
#include <memory>
#include <type_traits>

namespace koda {

template <typename ValueTp, typename AllocatorTp = std::allocator<ValueTp>>
class ForwardList {
    struct Node;

    template <bool IsConst>
    class Iterator {
        using UnderlyingPtr = std::conditional_t<IsConst, const Node*, Node*>;

       public:
        using value_type = std::conditional_t<IsConst, const ValueTp, ValueTp>;
        using reference_type = value_type&;
        using pointer_type = value_type*;
        using difference_type = std::ptrdiff_t;

        constexpr explicit Iterator(UnderlyingPtr node = nullptr) noexcept;

        [[nodiscard]] constexpr reference_type operator*() const noexcept;

        [[nodiscard]] constexpr pointer_type operator->() const noexcept;

        constexpr Iterator& operator++() noexcept;

        [[nodiscard]] constexpr Iterator operator++(int) noexcept;

        [[nodiscard]] constexpr bool operator==(
            const Iterator& right) const noexcept;

       private:
        UnderlyingPtr node_;
    };

   public:
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
    using value_type = ValueTp;

    constexpr explicit ForwardList(
        const AllocatorTp& allocator = AllocatorTp{});

    template <std::ranges::input_range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, ValueTp>
    constexpr explicit ForwardList(
        Range&& range, const AllocatorTp& allocator = AllocatorTp{});

    constexpr ForwardList(std::initializer_list<ValueTp> init,
                          const AllocatorTp& allocator = AllocatorTp{});

    constexpr ForwardList(const ForwardList& other) noexcept = delete;
    constexpr ForwardList(ForwardList&& other) noexcept;

    constexpr ForwardList& operator=(const ForwardList& other) noexcept =
        delete;
    constexpr ForwardList& operator=(ForwardList&& other) noexcept;

    [[nodiscard]] constexpr size_t size() const noexcept;

    [[nodiscard]] constexpr bool empty() const noexcept;

    template <typename... Args>
        requires std::constructible_from<ValueTp, Args...>
    constexpr iterator PushFront(Args&&... args);

    constexpr void PopFront();

    [[nodiscard]] AllocatorTp get_allocator() const;

    [[nodiscard]] constexpr value_type& front() noexcept;

    [[nodiscard]] constexpr const value_type& front() const noexcept;

    [[nodiscard]] constexpr iterator begin() noexcept;

    [[nodiscard]] constexpr iterator end() noexcept;

    [[nodiscard]] constexpr const_iterator begin() const noexcept;

    [[nodiscard]] constexpr const_iterator end() const noexcept;

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept;

    [[nodiscard]] constexpr const_iterator cend() const noexcept;

    constexpr ~ForwardList();

   private:
    struct Node {
        template <typename... Args>
            requires std::constructible_from<ValueTp, Args...>
        explicit constexpr Node(Args&&... args);

        ValueTp value;
        Node* next = nullptr;
    };

    using ValueTraits = std::allocator_traits<AllocatorTp>;
    using NodeTraits = typename ValueTraits::rebind_traits<Node>;
    using NodeAllocatorTp = typename ValueTraits::rebind_alloc<Node>;

    class NodePool {
       public:
        constexpr explicit NodePool(const AllocatorTp& allocator);

        constexpr NodePool(NodePool&& pool) noexcept;
        constexpr NodePool(const NodePool& pool) = delete;

        constexpr NodePool& operator=(NodePool&& pool) noexcept;
        constexpr NodePool& operator=(const NodePool& pool) = delete;

        constexpr void ReturnNode(Node* handle);

        template <typename... Args>
            requires std::constructible_from<ValueTp, Args...>
        constexpr Node* GetNode(Args&&... args);

        constexpr AllocatorTp get_allocator() const;

        constexpr NodeAllocatorTp& get_node_allocator() noexcept;

        constexpr ~NodePool();

       private:
        [[no_unique_address]] NodeAllocatorTp allocator_;
        Node* handle_ = nullptr;

        constexpr void Destroy();
    };

    NodePool pool_;
    Node* root_ = nullptr;
    size_t size_ = 0;

    constexpr void Destroy();
};

}  // namespace koda

#include <koda/collections/forward_list.tpp>

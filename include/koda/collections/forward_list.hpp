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
       public:
        using value_type = std::conditional_t<IsConst, const ValueTp, ValueTp>;
        using reference_type = value_type&;
        using pointer_type = value_type*;
        using difference_type = std::ptrdiff_t;

        constexpr explicit Iterator(pointer_type node = nullptr) noexcept;

        [[nodiscard]] constexpr value_type operator*() const noexcept;

        [[nodiscard]] constexpr pointer_type operator->() const noexcept;

        constexpr Iterator& operator++() noexcept;

        [[nodiscard]] constexpr Iterator operator++(int) noexcept;

        [[nodiscard]] constexpr bool operator==(
            const Iterator& left, const Iterator& right) const noexcept;

       private:
        Node* node;
    };

   public:
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
    using value_type = ValueTp;

    constexpr explicit ForwardList() noexcept = default;

    constexpr ForwardList(const ForwardList&) noexcept = delete;
    constexpr ForwardList(ForwardList&&) noexcept = default;

    constexpr ForwardList& operator=(const ForwardList&) noexcept = delete;
    constexpr ForwardList& operator=(ForwardList&&) noexcept = default;

    [[nodiscard]] constexpr size_t size() const noexcept;

    template <typename... Args>
        requires std::constructible_from<ValueTp, Args...>
    constexpr iterator PushFront(Args&&... args);

    constexpr void PopFront();

    [[nodiscard]] AllocatorTp get_allocator() const;

    [[nodiscard]] constexpr iterator begin() noexcept;

    [[nodiscard]] constexpr iterator end() noexcept;

    [[nodiscard]] constexpr const_iterator begin() const noexcept;

    [[nodiscard]] constexpr const_iterator end() const noexcept;

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept;

    [[nodiscard]] constexpr const_iterator cend() const noexcept;

    constexpr ~ForwardList() noexcept = default;

   private:
    struct Node {
        ValueTp value;
        std::unique_ptr<Node> next;
    };

    class NodePool {
       public:
        struct Scheduler {
            NodePool& pool;
            Node* node;

            constexpr ~Scheduler();
        };

        constexpr explicit NodePool(const AllocatorTp& allocator) noexcept;

        constexpr NodePool(NodePool&& pool) noexcept;
        constexpr NodePool(const NodePool& pool) = delete;

        constexpr NodePool& operator=(NodePool&& pool) noexcept;
        constexpr NodePool& operator=(const NodePool& pool) = delete;

        constexpr void ReturnNode(std::unique_ptr<Node> handle);

        template <typename... Args>
            requires std::constructible_from<ValueTp, Args...>
        constexpr std::unique_ptr<Node> GetNode(Args&&... args);

        constexpr AllocatorTp get_allocator() const;

        constexpr ~NodePool();

       private:
        using ValueTraits = std::allocator_traits<AllocatorTp>;
        using NodeTraits = typename ValueTraits::rebind_traits<Node>;
        using NodeAllocatorTp = typename ValueTraits::rebind_alloc<Node>;

        [[no_unique_address]] NodeAllocatorTp allocator_;
        std::unique_ptr<Node> handle_ = nullptr;

        constexpr void Destroy();
    };

    std::unique_ptr<Node> root_ = nullptr;
    size_t size_;
};

}  // namespace koda

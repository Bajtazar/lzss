#pragma once

#include <cinttypes>
#include <concepts>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace koda {

template <typename ValueTp, typename AllocatorTp = std::allocator<ValueTp>>
class RedBlackTree {
   public:
    constexpr RedBlackTree(
        const AllocatorTp& allocator = AllocatorTp{}) noexcept;

    [[nodiscard]] constexpr AllocatorTp get_allocator() const;

    constexpr virtual ~RedBlackTree();

   protected:
    struct Node {
        enum class Color : bool { kBlack = 0, kRed = 1 };

        constexpr explicit Node(ValueTp value, Node* parent = nullptr,
                                Color color = Color::kBlack);

        constexpr Node(Node&&) noexcept = default;
        constexpr Node(const Node&) = delete;

        constexpr Node& operator=(Node&&) noexcept = default;
        constexpr Node& operator=(const Node&) = delete;

        ValueTp value;
        Node* parent;
        Node* left = nullptr;
        Node* right = nullptr;
        Color color;
    };

    using ValueTraits = std::allocator_traits<AllocatorTp>;
    using NodeTraits = typename ValueTraits::rebind_traits<Node>;
    using NodeAllocatorTp = typename ValueTraits::rebind_alloc<Node>;

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

        constexpr void ReturnNode(Node* handle);

        constexpr Scheduler ScheduleForReturn(Node* node);

        constexpr Node* GetNode(ValueTp value, Node* parent = nullptr,
                                Node::Color color = Node::Color::kBlack);

        constexpr AllocatorTp get_allocator() const;

        constexpr NodeAllocatorTp& get_node_allocator() noexcept;

        constexpr ~NodePool();

       private:
        [[no_unique_address]] NodeAllocatorTp allocator_;
        Node* handle_ = nullptr;

        constexpr void Destroy();
    };

    struct IteratorFromBeginingT {};
    struct IteratorFromSentinelT {};
    struct IteratorFromNodeT {};

    static constexpr IteratorFromBeginingT kIteratorFromBegining{};
    static constexpr IteratorFromSentinelT kIteratorFromSentinel{};
    static constexpr IteratorFromNodeT kIteratorFromNode{};

    template <bool IsConst>
    class NodeIteratorBase {
       public:
        using value_type = std::conditional_t<IsConst, const Node&, Node&>;
        using pointer_type = std::conditional_t<IsConst, const Node*, Node*>;
        using difference_type = std::ptrdiff_t;

        constexpr NodeIteratorBase(
            pointer_type node = nullptr,
            IteratorFromBeginingT = kIteratorFromBegining) noexcept;

        constexpr NodeIteratorBase(pointer_type node,
                                   IteratorFromSentinelT) noexcept;

        constexpr NodeIteratorBase(pointer_type node,
                                   IteratorFromNodeT) noexcept;

        [[nodiscard]] constexpr value_type operator*() const noexcept;

        [[nodiscard]] constexpr pointer_type operator->() const noexcept;

        constexpr NodeIteratorBase& operator++() noexcept;

        [[nodiscard]] constexpr NodeIteratorBase operator++(int) noexcept;

        constexpr NodeIteratorBase& operator--() noexcept;

        [[nodiscard]] constexpr NodeIteratorBase operator--(int) noexcept;

        [[nodiscard]] constexpr bool operator==(
            const NodeIteratorBase& other) const noexcept;

       private:
        // Since Node alignment is larger than 2 then it's youngest bit could be
        // used to store information whether the iterator stores a sentinel.
        // However that would require a lot of pointer masking during data
        // access slowing down the iterator itself
        pointer_type current_;
        bool is_sentinel_;

        static constexpr pointer_type FindLeftmost(pointer_type node) noexcept;

        static constexpr pointer_type FindRightmost(pointer_type node) noexcept;
    };

    using NodeIterator = NodeIteratorBase<false>;
    using NodeConstIterator = NodeIteratorBase<true>;

    using NodePtr = Node*;
    using NodeInsertionLocation = std::optional<std::pair<Node*&, Node*>>;

    constexpr explicit RedBlackTree(RedBlackTree&& other) noexcept;
    constexpr explicit RedBlackTree(const RedBlackTree& other) = delete;

    constexpr RedBlackTree& operator=(RedBlackTree&& other) noexcept;
    constexpr RedBlackTree& operator=(const RedBlackTree& other) = delete;

    constexpr virtual NodeInsertionLocation FindInsertionLocation(
        const ValueTp& value) = 0;

    constexpr NodePtr& root() noexcept;

    constexpr const NodePtr& root() const noexcept;

    constexpr NodePtr InsertNode(ValueTp value);

    constexpr void RemoveNode(NodePtr node);

    constexpr NodeIterator node_begin() noexcept;

    constexpr NodeConstIterator node_begin() const noexcept;

    constexpr NodeIterator node_end() noexcept;

    constexpr NodeConstIterator node_end() const noexcept;

    constexpr void CloneFrom(const RedBlackTree& source)
        requires std::is_copy_constructible_v<ValueTp>;

    constexpr void Destroy();

   private:
    NodePool pool_;
    Node* root_ = nullptr;

    constexpr void RotateLeft(Node* node);

    constexpr void RotateRight(Node* node);

    constexpr void RotateHelper(Node* node, Node* child, Node* root);

    constexpr Node* BuildNode(ValueTp&& value, Node*& node, Node* parent);

    constexpr void FixInsertionImbalance(Node* node);

    constexpr bool FixLocalInsertionImbalance(Node*& node, Node*& parent,
                                              Node*& grand_parent);

    constexpr void FixLocalInsertionImbalanceRight(Node*& node, Node*& parent,
                                                   Node*& grand_parent,
                                                   Node* uncle);

    constexpr void FixLocalInsertionImbalanceLeft(Node*& node, Node*& parent,
                                                  Node*& grand_parent,
                                                  Node* uncle);

    constexpr Node* FindSuccessor(Node* node);

    constexpr void RemoveNodeRotateSiblingRightPath(Node* parent, Node* sibling,
                                                    Node* nephew);

    constexpr void RemoveNodeRotateSiblingLeftPath(Node* parent, Node* sibling,
                                                   Node* nephew);

    constexpr void RemoveNodeRotateParentRightPath(Node* parent, Node* sibling,
                                                   Node* nephew);

    constexpr void RemoveNodeRotateParentLeftPath(Node* parent, Node* sibling,
                                                  Node* nephew);

    constexpr void RemoveNodeWithTwoChildren(Node* node);

    constexpr void RemoveNodeWithOneChild(Node* node, Node* children);

    constexpr void RemoveRootNode();

    constexpr void PrepareToRemoveRedChildlessNode(Node* node);

    constexpr void RemoveBlackChildlessNodeRightPathSiblingIsRed(
        Node* node, Node* parent, Node* sibling, Node* left_nephew,
        Node* right_nephew);

    constexpr bool RemoveBlackChildlessNodeRightPath(Node* node, Node* parent);

    constexpr void RemoveBlackChildlessNodeLeftPathSiblingIsRed(
        Node* node, Node* parent, Node* sibling, Node* left_nephew,
        Node* right_nephew);

    constexpr bool RemoveBlackChildlessNodeLeftPath(Node* node, Node* parent);

    constexpr void RemoveChildlessNode(Node* node);

    constexpr void RemoveBlackChildlessNode(Node* node);

    constexpr void CloneNodeChildren(const Node* node, Node* clone)
        requires std::is_copy_constructible_v<ValueTp>;

    constexpr void PopulateClonedRoot(const Node* root)
        requires std::is_copy_constructible_v<ValueTp>;

    // Only runs when checked build is explicitly requested
    constexpr void CheckInvariants() const;

#ifdef KODA_CHECKED_BUILD
    static constexpr std::ranges::subrange<NodeConstIterator, NodeConstIterator>
    nodes(const Node* root) noexcept;

    constexpr void ValidateRedNodeConstraint() const;

    constexpr void ValidateBlackNodeConstraint() const;

#endif  // KODA_CHECKED_BUILD
};

}  // namespace koda

#include <koda/collections/red_black_tree.tpp>

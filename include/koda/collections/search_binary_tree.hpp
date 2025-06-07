#pragma once

#include <cinttypes>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace koda {

template <typename Tp, typename AllocatorTp = std::allocator<Tp>>
class SearchBinaryTree {
   public:
    using ValueType = Tp;
    using StringView = std::basic_string_view<ValueType>;

    struct [[nodiscard]] RepeatitionMarker {
        size_t match_position;
        size_t match_length;

        [[nodiscard]] inline constexpr operator bool() const noexcept {
            return match_length != 0;
        }

        [[nodiscard]] constexpr auto operator<=>(
            const RepeatitionMarker&) const noexcept = default;
    };

    constexpr explicit SearchBinaryTree(
        size_t string_size,
        const AllocatorTp& allocator = AllocatorTp{}) noexcept;

    constexpr explicit SearchBinaryTree(SearchBinaryTree&& other) noexcept;
    constexpr explicit SearchBinaryTree(const SearchBinaryTree& other) = delete;

    constexpr SearchBinaryTree& operator=(SearchBinaryTree&& other) noexcept;
    constexpr SearchBinaryTree& operator=(const SearchBinaryTree& other) =
        delete;

    constexpr void AddString(StringView string);

    constexpr bool RemoveString(StringView string);

    constexpr RepeatitionMarker FindMatch(StringView buffer) const;

    [[nodiscard]] constexpr size_t string_size() const noexcept;

    [[nodiscard]] constexpr AllocatorTp get_allocator() const;

    constexpr ~SearchBinaryTree();

   private:
    struct Node {
        enum class Color : bool { kBlack = 0, kRed = 1 };

        constexpr explicit Node(const ValueType* key, size_t insertion_index,
                                Node* parent = nullptr,
                                Color color = Color::kBlack);

        constexpr Node(Node&&) noexcept = default;
        constexpr Node(const Node&) = delete;

        constexpr Node& operator=(Node&&) noexcept = default;
        constexpr Node& operator=(const Node&) = delete;

        size_t ref_counter = 1;
        size_t insertion_index;
        const ValueType* key;
        Node* parent;
        Node* left = nullptr;
        Node* right = nullptr;
        Color color;
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

        constexpr void ReturnNode(Node* handle);

        constexpr Scheduler ScheduleForReturn(Node* node);

        constexpr Node* GetNode(const ValueType* key, size_t insertion_index,
                                Node* parent = nullptr,
                                Node::Color color = Node::Color::kBlack);

        constexpr AllocatorTp get_allocator() const;

        constexpr ~NodePool();

       private:
        using ValueTraits = std::allocator_traits<AllocatorTp>;
        using NodeTraits = typename ValueTraits::rebind_traits<Node>;
        using NodeAllocatorTp = typename ValueTraits::rebind_alloc<Node>;

        [[no_unique_address]] NodeAllocatorTp allocator_;
        Node* handle_ = nullptr;

        constexpr void Destroy();
    };

    using NodeSpot = std::pair<Node*&, Node*>;

    size_t dictionary_start_index_ = 0;
    size_t buffer_start_index_ = 0;
    size_t string_size_;
    NodePool pool_;
    Node* root_ = nullptr;

    constexpr void RotateLeft(Node* node);

    constexpr void RotateRight(Node* node);

    constexpr void RotateHelper(Node* node, Node* child, Node* root);

    constexpr void InsertNewNode(const ValueType* key);

    constexpr void UpdateNodeReference(Node* node, const ValueType* key);

    constexpr void BuildNode(const ValueType* key, Node*& node, Node* parent);

    constexpr std::optional<NodeSpot> TryToInserLeaf(const ValueType* key);

    constexpr void FixInsertionImbalance(Node* node);

    constexpr bool FixLocalInsertionImbalance(Node*& node, Node*& parent,
                                              Node*& grand_parent);

    constexpr void FixLocalInsertionImbalanceRight(Node*& node, Node*& parent,
                                                   Node*& grand_parent,
                                                   Node* uncle);

    constexpr void FixLocalInsertionImbalanceLeft(Node*& node, Node*& parent,
                                                  Node*& grand_parent,
                                                  Node* uncle);

    constexpr std::pair<size_t, size_t> FindString(const ValueType* buffer,
                                                   size_t length) const;

    constexpr static void UpdateMatchInfo(std::pair<size_t, size_t>& match_info,
                                          size_t prefix_length,
                                          const Node* node) noexcept;

    constexpr size_t FindCommonPrefixSize(const ValueType* buffer,
                                          const ValueType* node,
                                          size_t length) const noexcept;

    constexpr Node* FindNodeToRemoval(StringView key_view);

    constexpr Node* FindSuccessor(Node* node);

    constexpr void RemoveNode(Node* node);

    constexpr void RemoveNodeRotateSiblingRightPath(Node* parent, Node* sibling,
                                                    Node* nephew);

    constexpr void RemoveNodeRotateSiblingLeftPath(Node* parent, Node* sibling,
                                                   Node* nephew);

    constexpr void RemoveNodeRotateParentRightPath(Node* parent, Node* sibling,
                                                   Node* nephew);

    constexpr void RemoveNodeRotateParentLeftPath(Node* parent, Node* sibling,
                                                  Node* nephew);

    constexpr void RemoveNodeWithTwoChildren(Node* node);

    constexpr void RemoveNodeWithOneChildren(Node* node, Node* children);

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

    constexpr void Destroy();
};

}  // namespace koda

#include <koda/collections/search_binary_tree.tpp>

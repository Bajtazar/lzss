#pragma once

#include <cinttypes>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace koda {

class SearchBinaryTree {
   public:
    using StringView = std::basic_string_view<uint8_t>;

    struct [[nodiscard]] RepeatitionMarker {
        size_t match_position;
        size_t match_length;

        [[nodiscard]] inline constexpr operator bool() const noexcept {
            return match_length != 0;
        }

        [[nodiscard]] constexpr auto operator<=>(
            const RepeatitionMarker&) const noexcept = default;
    };

    explicit SearchBinaryTree(size_t string_size) noexcept;

    explicit SearchBinaryTree(SearchBinaryTree&& other) noexcept;
    explicit SearchBinaryTree(const SearchBinaryTree& other) = delete;

    SearchBinaryTree& operator=(SearchBinaryTree&& other) noexcept;
    SearchBinaryTree& operator=(const SearchBinaryTree& other) = delete;

    void AddString(StringView string);

    bool RemoveString(StringView string);

    RepeatitionMarker FindMatch(StringView buffer) const;

    void dumpTree();

    ~SearchBinaryTree();

   private:
    struct Node {
        enum class Color : bool { kBlack = 0, kRed = 1 };

        explicit Node(const uint8_t* key, size_t insertion_index,
                      Node* parent = nullptr, Color color = Color::kBlack);

        Node(Node&&) = delete;
        Node(const Node&) = delete;

        Node& operator=(Node&&) = delete;
        Node& operator=(const Node&) = delete;

        size_t ref_counter = 1;
        size_t insertion_index;
        const uint8_t* key;
        Node* parent;
        Node* left = nullptr;
        Node* right = nullptr;
        Color color;
    };

    void dumpTree(Node* parent, std::string view);

    using NodeSpot = std::pair<Node*&, Node*>;

    Node* root_ = nullptr;
    size_t dictionary_start_index_ = 0;
    size_t buffer_start_index_ = 0;
    size_t string_size_;

    void RotateLeft(Node* node);

    void RotateRight(Node* node);

    void RotateHelper(Node* node, Node* child, Node* root);

    void RotateLeftRight(Node*& node);

    void RotateRightLeft(Node*& node);

    void InsertNewNode(const uint8_t* key);

    void UpdateNodeReference(Node* node, const uint8_t* key);

    void BuildNode(const uint8_t* key, Node*& node, Node* parent);

    std::optional<NodeSpot> TryToInserLeaf(const uint8_t* key);

    void FixInsertionImbalance(Node* node);

    bool FixLocalInsertionImbalance(Node*& node, Node*& parent,
                                    Node*& grand_parent);

    void FixLocalInsertionImbalanceRight(Node*& node, Node*& parent,
                                         Node*& grand_parent, Node* uncle);

    void FixLocalInsertionImbalanceLeft(Node*& node, Node*& parent,
                                        Node*& grand_parent, Node* uncle);

    std::pair<size_t, size_t> FindString(const uint8_t* buffer,
                                         size_t length) const;

    static void UpdateMatchInfo(std::pair<size_t, size_t>& match_info,
                                size_t prefix_length,
                                const Node* node) noexcept;

    size_t FindCommonPrefixSize(const uint8_t* buffer, const uint8_t* node,
                                size_t length) const noexcept;

    Node* FindNodeToRemoval(StringView key_view);

    Node* FindSuccessor(Node* node);

    void RemoveNode(Node* node);

    void RemoveNodeRotateSiblingRightPath(Node* parent, Node* sibling,
                                          Node* nephew);

    void RemoveNodeRotateSiblingLeftPath(Node* parent, Node* sibling,
                                         Node* nephew);

    void RemoveNodeRotateParentRightPath(Node* parent, Node* sibling,
                                         Node* nephew);

    void RemoveNodeRotateParentLeftPath(Node* parent, Node* sibling,
                                        Node* nephew);

    void RemoveNodeWithTwoChildren(Node* node);

    void RemoveNodeWithOneChildren(Node* node, Node* children);

    void Destroy();
};

}  // namespace koda

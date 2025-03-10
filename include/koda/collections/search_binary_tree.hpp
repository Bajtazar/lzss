#pragma once

#include <cinttypes>
#include <cstdlib>
#include <map>
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

    explicit SearchBinaryTree() noexcept = default;

    void AddString(StringView string);

    void RemoveString(StringView string);

    RepeatitionMarker FindMatch(StringView buffer) const;

   private:
    struct Node {
        enum class Color : bool { kBlack = 0, kRed = 1 };

        explicit Node(uint8_t* key, size_t insertion_index,
                      Node* parent = nullptr, Color color = Color::kBlack);

        Node(Node&&) = delete;
        Node(const Node&) = delete;

        Node& operator=(Node&&) = delete;
        Node& operator=(const Node&) = delete;

        uint8_t* key;
        size_t ref_counter = 1;
        size_t insertion_index;
        Node* parent = nullptr;
        Node* left = nullptr;
        Node* right = nullptr;
        Color color;

        ~Node();
    };

    using NodeSpot = std::pair<Node*&, Node*>;

    std::unique_ptr<Node> root_ = nullptr;
    size_t dictionary_start_index_ = 0;
    size_t buffer_start_index_ = 0;
    size_t string_size_;

    void RotateLeft(Node*& node);

    void RotateRight(Node*& node);

    void RotateLeftRight(Node*& node);

    void RotateRightLeft(Node*& node);

    void InsertNewNode(uint8_t* key);

    void UpdateNodeReference(Node* node, uint8_t* key);

    void BuildNode(uint8_t* key, Node*& node, Node* parent);

    std::optional<NodeSpot> TryToInserLeaf(uint8_t* key);

    void FixInsertionImbalance(Node*& node);

    static size_t FindCommonPrefixSize(StringView buffer,
                                       StringView node) noexcept;
};

}  // namespace koda

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

        explicit Node(uint8_t* key, Node* parent = nullptr);

        uint8_t* key;
        size_t ref_counter = 1;
        Node* parent = nullptr;
        std::unique_ptr<Node> left = nullptr;
        std::unique_ptr<Node> right = nullptr;
        Color color_ = Color::kBlack;
    };

    using NodeSpot = std::pair<std::unique_ptr<Node>&, Node*>;

    std::unique_ptr<Node> root_ = nullptr;
    size_t dictionary_start_index_ = 0;
    size_t buffer_start_index_ = 0;
    size_t string_size_;

    void RotateLeft(std::unique_ptr<Node>& node);

    void RotateRight(std::unique_ptr<Node>& node);

    void RotateLeftRight(std::unique_ptr<Node>& node);

    void RotateRightLeft(std::unique_ptr<Node>& node);

    void InsertNewNode(uint8_t* key);

    void UpdateNodeReference(std::unique_ptr<Node>& node, uint8_t* key);

    std::optional<NodeSpot> TryToInserLeaf(uint8_t* key);

    static size_t FindCommonPrefixSize(StringView buffer,
                                       StringView node) noexcept;
};

}  // namespace koda

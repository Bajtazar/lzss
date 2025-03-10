#pragma once

#include <cinttypes>
#include <cstdlib>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <memory>

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
        explicit Node(uint8_t* key, Node* parent = nullptr);

        uint8_t* key;
        size_t ref_counter = 1;
        Node* parent = nullptr;
        std::unique_ptr<Node> left = nullptr;
        std::unique_ptr<Node> right = nullptr;
    };

    std::unique_ptr<Node> tree_;
    size_t dictionary_start_index_ = 0;
    size_t buffer_start_index_ = 0;
    size_t string_size_;

    void RotateLeft(std::unique_ptr<Node>& node);

    void RotateRight(std::unique_ptr<Node>& node);

    void RotateLeftRight(std::unique_ptr<Node>& node);

    void RotateRightLeft(std::unique_ptr<Node>& node);

    static size_t FindCommonPrefixSize(StringView buffer,
                                       StringView node) noexcept;
};

}  // namespace koda

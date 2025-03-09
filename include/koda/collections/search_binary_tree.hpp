#pragma once

#include <cinttypes>
#include <cstdlib>
#include <map>
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
    };

    explicit SearchBinaryTree() noexcept = default;

    void AddString(StringView string);

    void RemoveString(StringView string);

    RepeatitionMarker FindMatch(StringView buffer);

   private:
    using String = std::basic_string<uint8_t>;
    using BinaryTree = std::map<String, std::pair<size_t, size_t>>;

    BinaryTree tree_;
    size_t dictionary_start_index_ = 0;
    size_t buffer_start_index_ = 0;
};

}  // namespace koda

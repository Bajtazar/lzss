#pragma once

#include <koda/collections/red_black_tree.hpp>

#include <cinttypes>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace koda {

namespace details {

template <typename Tp>
struct SearchBinaryTreeEntry {
    size_t ref_counter = 1;
    size_t insertion_index;
    const Tp* key;
};

}  // namespace details

template <typename Tp, typename AllocatorTp = std::allocator<Tp>>
class SearchBinaryTree
    : public RedBlackTree<details::SearchBinaryTreeEntry<Tp>, AllocatorTp> {
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

    constexpr explicit SearchBinaryTree(SearchBinaryTree&& other) noexcept =
        default;
    constexpr explicit SearchBinaryTree(const SearchBinaryTree& other) = delete;

    constexpr SearchBinaryTree& operator=(SearchBinaryTree&& other) noexcept =
        default;
    constexpr SearchBinaryTree& operator=(const SearchBinaryTree& other) =
        delete;

    constexpr void AddString(StringView string);

    constexpr bool RemoveString(StringView string);

    constexpr RepeatitionMarker FindMatch(StringView buffer) const;

    [[nodiscard]] constexpr size_t string_size() const noexcept;

    [[nodiscard]] constexpr size_t size() const noexcept;

    constexpr ~SearchBinaryTree() override = default;

   private:
    using Entry = details::SearchBinaryTreeEntry<Tp>;
    using RedBlackImpl = RedBlackTree<Entry, AllocatorTp>;
    using Node = RedBlackImpl::Node;
    using NodeInsertionLocation = RedBlackImpl::NodeInsertionLocation;

    size_t dictionary_start_index_ = 0;
    size_t buffer_start_index_ = 0;
    size_t string_size_;

    constexpr void UpdateNodeReference(Node* node, const ValueType* key);

    constexpr NodeInsertionLocation FindInsertionLocation(
        const Entry& value) override final;

    constexpr std::pair<size_t, size_t> FindString(const ValueType* buffer,
                                                   size_t length) const;

    constexpr static void UpdateMatchInfo(std::pair<size_t, size_t>& match_info,
                                          size_t prefix_length,
                                          const Node* node) noexcept;

    constexpr size_t FindCommonPrefixSize(const ValueType* buffer,
                                          const ValueType* node,
                                          size_t length) const noexcept;

    constexpr Node* FindNodeToRemoval(StringView key_view);
};

}  // namespace koda

#include <koda/collections/search_binary_tree.tpp>

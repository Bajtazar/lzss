#include <koda/utils/comparation.hpp>

#include <cassert>

namespace koda {

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::SearchBinaryTree(
    size_t string_size, const AllocatorTp& allocator) noexcept
    : RedBlackTree<details::SearchBinaryTreeEntry<Tp>,
                   SearchBinaryTree<Tp, AllocatorTp>, AllocatorTp>{allocator},
      string_size_{string_size} {}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::AddString(StringView string) {
    assert(string.size() == string_size_ &&
           "Inserted string have to have fixed size equal to string_size_");

    InsertNode(Entry{1, buffer_start_index_, string.data()});
    ++buffer_start_index_;
}

template <typename Tp, typename AllocatorTp>
constexpr bool SearchBinaryTree<Tp, AllocatorTp>::RemoveString(
    StringView string) {
    Node* node = FindNodeToRemoval(string);

    if (!node) [[unlikely]] {
        return false;
    }

    if (!--node->ref_counter) {
        RemoveNode(node);
    }

    ++dictionary_start_index_;
    return true;
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::RepeatitionMarker
SearchBinaryTree<Tp, AllocatorTp>::FindMatch(StringView buffer) const {
    assert(
        buffer.size() <= string_size_ &&
        "Inserted string have to have fixed size not bigger than string_size_");

    auto [position, length] = FindString(buffer.data(), buffer.size());

    if (!length) {
        return {0, 0};
    }

    return {// Calculate relative offset from the start of the dictionary
            position - dictionary_start_index_, length};
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr size_t SearchBinaryTree<Tp, AllocatorTp>::string_size()
    const noexcept {
    return string_size_;
}

template <typename Tp, typename AllocatorTp>
constexpr void SearchBinaryTree<Tp, AllocatorTp>::UpdateNodeReference(
    Node* node, const ValueType* key) {
    ++node->ref_counter;
    node->key = key;
    node->insertion_index = buffer_start_index_ - dictionary_start_index_;
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::NodeInsertionLocation
SearchBinaryTree<Tp, AllocatorTp>::FindInsertionLocation(const Entry& entry) {
    const ValueType* key = entry.key;
    const StringView key_view{key, string_size_};
    Node** node = &root();
    Node* parent = nullptr;
    while (*node) {
        switch (
            OrderCast(key_view <=> StringView{(*node)->key, string_size_})) {
            case WeakOrdering::kEquivalent:
                UpdateNodeReference(*node, key);
                return std::nullopt;
            case WeakOrdering::kLess:
                parent = *node;
                node = &(*node)->left;
                break;
            case WeakOrdering::kGreater:
                parent = *node;
                node = &(*node)->right;
                break;
            default:
                std::unreachable();
        };
    }
    return NodeSpot{*node, parent};
}

template <typename Tp, typename AllocatorTp>
constexpr std::pair<size_t, size_t>
SearchBinaryTree<Tp, AllocatorTp>::FindString(const ValueType* buffer,
                                              size_t length) const {
    std::pair<size_t, size_t> match{};
    for (const Node* node = root(); node;) {
        auto prefix_length = FindCommonPrefixSize(buffer, node->key, length);
        if (prefix_length == length) {
            return {node->insertion_index, length};
        }
        UpdateMatchInfo(match, prefix_length, node);

        if (StringView{buffer, length} < StringView{node->key, string_size_}) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return match;
}

template <typename Tp, typename AllocatorTp>
/*static*/ constexpr void SearchBinaryTree<Tp, AllocatorTp>::UpdateMatchInfo(
    std::pair<size_t, size_t>& match_info, size_t prefix_length,
    const Node* node) noexcept {
    if (match_info.second < prefix_length) {
        match_info.first = node->insertion_index;
        match_info.second = prefix_length;
    }
}

template <typename Tp, typename AllocatorTp>
constexpr size_t SearchBinaryTree<Tp, AllocatorTp>::FindCommonPrefixSize(
    const ValueType* buffer, const ValueType* node,
    size_t length) const noexcept {
    for (size_t i = 0; i < length; ++i) {
        if (buffer[i] != node[i]) {
            return i;
        }
    }
    return length;
}

template <typename Tp, typename AllocatorTp>
constexpr SearchBinaryTree<Tp, AllocatorTp>::Node*
SearchBinaryTree<Tp, AllocatorTp>::FindNodeToRemoval(StringView key_view) {
    for (Node* node = root(); node;) {
        switch (OrderCast(key_view <=> StringView{node->key, string_size_})) {
            case WeakOrdering::kEquivalent:
                return node;
            case WeakOrdering::kLess:
                node = node->left;
                break;
            case WeakOrdering::kGreater:
                node = node->right;
                break;
            default:
                std::unreachable();
        };
    }
    return nullptr;
}

}  // namespace koda

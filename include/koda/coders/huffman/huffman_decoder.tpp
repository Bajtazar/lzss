#pragma once

#include <koda/utils/utils.hpp>

namespace koda {

template <typename Token>
constexpr HuffmanDecoder<Token>::HuffmanDecoder(
    const HuffmanTable<Token>& table)
    : root_{BuildTree(table)} {}

template <typename Token>
constexpr auto HuffmanDecoder<Token>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    auto input_iter = std::ranges::begin(input);
    const auto input_sent = std::ranges::end(input);
    auto output_iter = std::ranges::begin(output);
    const auto output_sent = std::ranges::end(output);

    for (; (input_iter != input_sent) && (output_iter != output_sent);
         ++input_iter) {
        if (const Token* token = std::get_if<Token>(&root_)) {
            *output_iter = *token;
            continue;
        }

        [[assume(std::holds_alternative<NodePtr>(root_))]];

        if (!processed_) {
            processed_ = std::get<NodePtr>(root_).get();
        }

        if (*input_iter) {
            ProcessBit(output_iter, processed_->right);
        } else {
            ProcessBit(output_iter, processed_->left);
        }
    }
}

template <typename Token>
constexpr void HuffmanDecoder<Token>::ProcessBit(auto& output_iter,
                                                 NodeOrLeaf& next) {
    if (const Token* token = std::get_if<Token>(&next)) {
        processed_ = nullptr;
        *output_iter++ = *token;
        return;
    }
    processed_ = std::get<NodePtr>(&next).get();
}

template <typename Token>
constexpr auto HuffmanDecoder<Token>::Initialize(BitInputRange auto&& input) {
    return AsSubrange(std::forward<decltype(input)>(input));
}

template <typename Token>
constexpr HuffmanDecoder<Token>::TreeBuilder(const HuffmanTable<Token>& table)
    : root_{new Node{}} {
    // Populate unwinding table with original table
    ProcessUnwindingTableEntry(std::get<NodePtr>(root_).get(), table);
    ProcessUnwindingTable();
}

template <typename Token>
constexpr HuffmanDecoder<Token>::NodePtr HuffmanDecoder<Token>::root() && {
    return std::move(root_);
}

template <typename Token>
constexpr void HuffmanDecoder<Token>::ProcessUnwindingTableEntry(
    Node* entry_node, const auto& entry_table) {
    std::vector<HuffmanTableEntry> left;
    std::vector<HuffmanTableEntry> right;

    for (auto [token, symbol] : entry_table) {
        if (symbol.empty()) [[unlikely]] {
            throw std::runtime_error{"Invalid huffman table detected!"};
        }
        auto bit = symbol.front();
        symbol.erase(symbol.begin());
        if (bit) {
            right.emplace_back(token, std::move(symbol));
        } else {
            left.emplace_back(token, std::move(symbol));
        }
    }

    InsertUnwindingEntry(entry_node->left, std::move(left));
    InsertUnwindingEntry(entry_node->right, std::move(right));
}

template <typename Token>
constexpr void HuffmanDecoder<Token>::ProcessUnwindingTable() {
    while (!unwinding_table_.empty()) {
        auto entry_iter = unwinding_table_.begin();
        auto [parent, entry_table] = std::move(*entry_iter);
        unwinding_table_.Remove(entry_iter);

        ProcessUnwindingTableEntry(parent, entry_table);
    }
}

template <typename Token>
constexpr void HuffmanDecoder<Token>::InsertUnwindingEntry(
    NodeOrLeaf& hook, std::vector<HuffmanTableEntry>&& child_table) {
    if (child_table.empty()) {
        return;
    }
    if (child_table.size() == 1) {
        if (child_table.front().second.empty()) [[unlikely]] {
            throw std::runtime_error{"Invalid huffman table detected!"};
        }
        hook = std::move(child_table.front().first);
        return;
    }
    std::unique_ptr<Node> new_child{new Node{}};
    unwinding_table_.Emplace(new_child.get(), std::move(child_table));
    hook = std::move(new_child);
}

template <typename Token>
/*static*/ constexpr HuffmanDecoder<Token>::NodeOrLeaf
HuffmanDecoder<Token>::BuildTree(const HuffmanTable<Token>& table) {
    if (table.size() == 1) {
        if (!table.begin()->second.empty()) [[unlikely]] {
            throw std::runtime_error{
                "For a distribution with only one element symbol should not "
                "exist"};
        }
        return table.begin()->first;
    }
    return TreeBuilder{table}.root();
}

}  // namespace koda

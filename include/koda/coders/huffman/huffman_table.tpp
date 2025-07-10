#pragma once

#include <variant>

namespace koda {

template <std::integral Token>
[[nodiscard]] /*static*/ constexpr WeakOrdering
HuffmanTableEntryComparator::operator()(
    const std::pair<const Token, std::vector<bool>>& left,
    const std::pair<const Token, std::vector<bool>>& right) noexcept {
    const auto& left_binary = left.second;
    const auto& right_binary = right.second;
    if (left_binary.size() < right_binary.size()) {
        return true;
    }
    if (left_binary.size() > right_binary.size()) {
        return false;
    }
    return left_binary < right_binary;
}

}  // namespace koda

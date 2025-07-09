#pragma once

namespace koda {

template <std::integral LToken, std::integral RToken>
[[nodiscard]] /*static*/ constexpr bool HuffmanTableEntryComparator::operator()(
    const HuffmanTableEntry<LToken>& left,
    const HuffmanTableEntry<RToken>& right) noexcept {
    if (left.size() < right.size()) {
        return true;
    }
    if (left.size() > right.size()) {
        return false;
    }
    return left < right;
}

}  // namespace koda

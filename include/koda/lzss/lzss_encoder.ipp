#pragma once

namespace koda {

constexpr LzssIntermediateToken::LzssIntermediateToken(uint8_t symbol) noexcept
    : holds_distance_match_pair_{false}, symbol_{symbol} {}

constexpr LzssIntermediateToken::LzssIntermediateToken(
    size_t match_position, size_t match_length) noexcept
    : holds_distance_match_pair_{true},
      repeatition_marker_{match_position, match_length} {}

[[nodiscard]] constexpr bool LzssIntermediateToken::holds_symbol()
    const noexcept {
    return holds_distance_match_pair_ == false;
}

[[nodiscard]] constexpr bool LzssIntermediateToken::holds_marker()
    const noexcept {
    return holds_distance_match_pair_ == true;
}

[[nodiscard]] constexpr std::optional<uint8_t>
LzssIntermediateToken::get_symbol() const noexcept {
    if (holds_symbol()) {
        return symbol_;
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<LzssIntermediateToken::RepeatitionMarker>
LzssIntermediateToken::get_marker() const noexcept {
    if (holds_marker()) {
        return repeatition_marker_;
    }
    return std::nullopt;
}

}  // namespace koda

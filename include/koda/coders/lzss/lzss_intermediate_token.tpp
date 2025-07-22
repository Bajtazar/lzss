#pragma once

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
constexpr LzssIntermediateToken<InputToken, PositionTp, LengthTp>::
    LzssIntermediateToken(InputToken symbol) noexcept
    : symbol_{symbol}, holds_distance_match_pair_{false} {}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
constexpr LzssIntermediateToken<InputToken, PositionTp, LengthTp>::
    LzssIntermediateToken(PositionTp match_position,
                          LengthTp match_length) noexcept
    : repeatition_marker_{match_position, match_length},
      holds_distance_match_pair_{true} {}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
constexpr LzssIntermediateToken<InputToken, PositionTp, LengthTp>::
    LzssIntermediateToken(const LzssIntermediateToken& other) noexcept {
    if ((holds_distance_match_pair_ = other.holds_distance_match_pair_)) {
        repeatition_marker_ = other.repeatition_marker_;
    } else {
        symbol_ = other.symbol_;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
constexpr LzssIntermediateToken<InputToken, PositionTp, LengthTp>&
LzssIntermediateToken<InputToken, PositionTp, LengthTp>::operator=(
    const LzssIntermediateToken& other) noexcept {
    Destroy();
    if ((holds_distance_match_pair_ = other.holds_distance_match_pair_)) {
        repeatition_marker_ = other.repeatition_marker_;
    } else {
        symbol_ = other.symbol_;
    }
    return *this;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr std::partial_ordering
LzssIntermediateToken<InputToken, PositionTp, LengthTp>::operator<=>(
    const LzssIntermediateToken& right) const noexcept {
    if (auto symbol = get_symbol()) {
        if (auto other = right.get_symbol()) {
            return *symbol <=> *other;
        }
        return std::partial_ordering::unordered;
    }
    if (auto other = right.get_marker()) {
        return *get_marker() <=> *other;
    }
    return std::partial_ordering::unordered;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr bool
LzssIntermediateToken<InputToken, PositionTp, LengthTp>::operator==(
    const LzssIntermediateToken& right) const noexcept {
    if (auto symbol = get_symbol()) {
        if (auto other = right.get_symbol()) {
            return *symbol == *other;
        }
        return false;
    }
    if (auto other = right.get_marker()) {
        return *get_marker() == *other;
    }
    return false;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
constexpr LzssIntermediateToken<InputToken, PositionTp,
                                LengthTp>::~LzssIntermediateToken() {
    Destroy();
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
constexpr void
LzssIntermediateToken<InputToken, PositionTp, LengthTp>::Destroy() {
    if (holds_distance_match_pair_) {
        repeatition_marker_.~RepeatitionMarker();
    } else {
        symbol_.~Symbol();
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr bool LzssIntermediateToken<
    InputToken, PositionTp, LengthTp>::holds_symbol() const noexcept {
    return holds_distance_match_pair_ == false;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr bool LzssIntermediateToken<
    InputToken, PositionTp, LengthTp>::holds_marker() const noexcept {
    return holds_distance_match_pair_ == true;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr std::optional<InputToken> LzssIntermediateToken<
    InputToken, PositionTp, LengthTp>::get_symbol() const noexcept {
    if (holds_symbol()) {
        return symbol_;
    }
    return std::nullopt;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr std::optional<typename LzssIntermediateToken<
    InputToken, PositionTp, LengthTp>::RepeatitionMarker>
LzssIntermediateToken<InputToken, PositionTp, LengthTp>::get_marker()
    const noexcept {
    if (holds_marker()) {
        return repeatition_marker_;
    }
    return std::nullopt;
}

}  // namespace koda

namespace std {

template <typename Token, typename Position, typename Length>
constexpr auto
formatter<koda::LzssIntermediateToken<Token, Position, Length>>::parse(
    std::format_parse_context& ctx) {
    return ctx.begin();
}

template <typename Token, typename Position, typename Length>
auto formatter<koda::LzssIntermediateToken<Token, Position, Length>>::format(
    const koda::LzssIntermediateToken<Token, Position, Length>& obj,
    std::format_context& ctx) const {
    if (auto symbol = obj.get_symbol()) {
        return std::format_to(ctx.out(), "LzssIntermediateToken(symbol={})",
                              *symbol);
    }
    const auto [pos, len] = *obj.get_marker();
    return std::format_to(
        ctx.out(), "LzssIntermediateToken(position={}, length={})", pos, len);
}

}  // namespace std

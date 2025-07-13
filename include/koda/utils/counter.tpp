#pragma once

namespace koda {

template <typename Token, std::integral CountTp>
constexpr Counter<Token, CountTp>::Counter(
    std::ranges::input_range auto&& range) {
    CountRange(std::forward<decltype(range)>(range));
}

template <typename Token, std::integral CountTp>
constexpr void Counter<Token, CountTp>::Count(const Token& token) {
    if (auto iter = counter_.Find(token); iter != counter_.end()) {
        ++(iter->second);
        return;
    }
    counter_.Emplace(token, 1);
}

template <typename Token, std::integral CountTp>
constexpr void Counter<Token, CountTp>::CountRange(
    std::ranges::input_range auto&& range) {
    for (auto&& token : range) {
        Count(token);
    }
}

template <typename Token, std::integral CountTp>
[[nodiscard]] constexpr auto&& Counter<Token, CountTp>::counted(
    this auto&& self) noexcept {
    return std::forward_like<decltype(self)>(self.counter_);
}

}  // namespace koda

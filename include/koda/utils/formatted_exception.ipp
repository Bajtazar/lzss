#pragma once

namespace koda {

template <typename... Args>
FormattedException::FormattedException(
    std::format_string<Args...> format_string, Args&&... args)
    : message_{std::move(format_string), std::forward<Args>(args)...} {}

[[nodiscard]] inline const char* FormattedException::what() const noexcept {
    return message_.cstr();
}

}  // namespace koda

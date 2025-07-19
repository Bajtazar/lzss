#pragma once

#include <format>
#include <stdexcept>
#include <string>

namespace koda {

class FormattedException final : public std::exception {
   public:
    template <typename... Args>
    explicit FormattedException(std::format_string<Args...> format_string,
                                Args&&... args);

    [[nodiscard]] inline const char* what() override const noexcept;

   private:
    std::string message_;
};

}  // namespace koda

#include <koda/utils/formatted_exception.ipp>

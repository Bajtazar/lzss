#pragma once

#include <cinttypes>
#include <concepts>
#include <istream>
#include <ostream>

namespace koda {

template <std::integral InputToken, std::integral OutputToken>
class EncoderBase {
   public:
    using InputTokenStream = std::basic_istream<InputToken>;
    using OutputTokenStream = std::basic_ostream<OutputToken>;

    constexpr explicit EncoderBase() noexcept = default;

    virtual void operator()(InputTokenStream& input,
                            OutputTokenStream& output) const = 0;

    virtual ~EncoderBase() noexcept = default;

   protected:
    constexpr EncoderBase(const EncoderBase&) noexcept = default;
    constexpr EncoderBase(EncoderBase&&) noexcept = default;

    constexpr EncoderBase& operator=(const EncoderBase&) noexcept = default;
    constexpr EncoderBase& operator=(EncoderBase&&) noexcept = default;
};

}  // namespace koda

#pragma once

#include <concepts>
#include <istream>
#include <ostream>

namespace koda {

template <std::integral InputToken>
class EncoderBase {
   public:
    using InputTokenStream = std::basic_istream<InputToken>;
    using OutputBinaryStream = std::basic_ostream<bool>;

    constexpr explicit EncoderBase() noexcept = default;

    virtual void operator()(InputTokenStream& input,
                            OutputBinaryStream& output) const = 0;

    virtual ~EncoderBase() noexcept = default;

   protected:
    constexpr EncoderBase(const EncoderBase&) noexcept = default;
    constexpr EncoderBase(EncoderBase&&) noexcept = default;

    constexpr EncoderBase& operator=(const EncoderBase&) noexcept = default;
    constexpr EncoderBase& operator=(EncoderBase&&) noexcept = default;
};

}  // namespace koda

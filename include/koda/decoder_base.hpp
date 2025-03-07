#pragma once

#include <concepts>
#include <istream>
#include <ostream>

namespace koda {

template <std::integral OutputToken>
class DecoderBase {
   public:
    using InputBinaryStream = std::basic_istream<bool>;
    using OutputTokenStream = std::basic_ostream<OutputToken>;

    constexpr explicit DecoderBase() noexcept = default;

    virtual void operator()(InputBinaryStream& input,
                            OutputTokenStream& output) const = 0;

    virtual ~DecoderBase() noexcept = default;

   protected:
    constexpr DecoderBase(const DecoderBase&) noexcept = default;
    constexpr DecoderBase(DecoderBase&&) noexcept = default;

    constexpr DecoderBase& operator=(const EncoderBase&) noexcept = default;
    constexpr DecoderBase& operator=(DecoderBase&&) noexcept = default;
};

}  // namespace koda

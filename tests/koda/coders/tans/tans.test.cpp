#include <koda/coders/tans/tans_decoder.hpp>
#include <koda/coders/tans/tans_encoder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/counter.hpp>

static constexpr std::string_view kTestString =
    "The number theoretic transform is based on generalizing the $ N$ th "
    "primitive root of unity (see §3.12) to a ``quotient ring'' instead of "
    "the usual field of complex numbers. Let $ W_N$ denote a primitive $ "
    "N$ th root of unity. We have been using $ W_N = \\exp(-j2\\pi/N)$ in "
    "the field of complex numbers, and it of course satisfies $ W_N^N=1$ , "
    "making it a root of unity; it also has the property that $ W_N^k$ "
    "visits all of the ``DFT frequency points'' on the unit circle in the "
    "$ z$ plane, as $ k$ goes from 0 to $ N-1$";

BeginConstexprTest(TansTest, NormalTest) {
    std::string sequence{kTestString};
    koda::TansInitTable table{koda::Counter{kTestString}.counted(), 0, 1, 512};

    koda::TansEncoder encoder{table};
    koda::TansDecoder decoder{table};

    std::vector<bool> stream;
    std::string reconstruction;

    encoder(sequence, stream | koda::views::InsertFromBack);

    decoder(sequence.size(), stream | std::views::reverse,
            reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(sequence, reconstruction | std::views::reverse);
}
EndConstexprTest;

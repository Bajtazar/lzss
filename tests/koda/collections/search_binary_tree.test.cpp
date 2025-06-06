#include <koda/collections/search_binary_tree.hpp>
#include <koda/tests/tests.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cstring>
#include <ranges>
#include <set>
#include <vector>

#include "viewable_vector.hpp"

using namespace koda::tests;

namespace {

template <size_t Window, size_t Length>
    requires(Length >= Window)
constexpr std::array<ViewableVector<uint8_t>, Length - Window>
BuildSamplesFromString(const char (&sentence)[Length]) {
    std::array<ViewableVector<uint8_t>, Length - Window> result;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = ConvertToString(
            std::ranges::subrange{&sentence[i], &sentence[i + Window]});
    }
    return result;
}

template <size_t Size>
constexpr auto MakeSamples() {
    return BuildSamplesFromString<Size>("ala ma kota a kot ma ale");
}

}  // namespace

BeginConstexprTest(SearchBinaryTreeTest, Creation) {
    auto vector = MakeSamples<4>();
    koda::SearchBinaryTree<uint8_t> tree{4};

    for (auto const& element : vector) {
        tree.AddString(element);
    }

    for (auto const& element : vector) {
        ConstexprAssertTrue(tree.FindMatch(element));
    }

    ConstexprAssertTrue(tree.FindMatch("abcd"_u8));
    ConstexprAssertTrue(tree.FindMatch("abc"_u8));
    ConstexprAssertTrue(tree.FindMatch("ab"_u8));
    ConstexprAssertTrue(tree.FindMatch("a"_u8));
    ConstexprAssertFalse(tree.FindMatch("xyzo"_u8));
    ConstexprAssertFalse(tree.FindMatch("xyz"_u8));
    ConstexprAssertFalse(tree.FindMatch("xy"_u8));
    ConstexprAssertFalse(tree.FindMatch("x"_u8));
}
EndConstexprTest(SearchBinaryTreeTest, Creation);

BeginConstexprTest(SearchBinaryTreeTest, Uniqueness) {
    using Marker = koda::SearchBinaryTree<uint8_t>::RepeatitionMarker;

    auto vector = MakeSamples<4>();
    koda::SearchBinaryTree<uint8_t> tree{4};

    for (auto const& element : vector) {
        tree.AddString(element);
    }

    const auto al_result = tree.FindMatch("al"_u8);
    ConstexprAssertTrue(al_result == Marker(0, 2) ||
                        al_result == Marker(22, 2));

    ConstexprEqual(tree.FindMatch("abcd"_u8), Marker(0, 1));
    ConstexprEqual(tree.FindMatch("ala"_u8), Marker(0, 3));
    ConstexprEqual(tree.FindMatch(" ale"_u8), Marker(20, 4));

    const auto kot_result = tree.FindMatch("kot"_u8);

    ConstexprAssertTrue(kot_result == Marker(14, 3) ||
                        kot_result == Marker(7, 3));
    ConstexprEqual(tree.FindMatch("kota"_u8), Marker(7, 4));
    ConstexprEqual(tree.FindMatch("kot "_u8), Marker(14, 4));

    const auto ma_result = tree.FindMatch("ma"_u8);

    ConstexprAssertTrue(ma_result == Marker(4, 2) ||
                        ma_result == Marker(18, 2));
}
EndConstexprTest(SearchBinaryTreeTest, Uniqueness);

BeginConstexprTest(SearchBinaryTreeTest, Removal) {
    auto vector = MakeSamples<4>();
    koda::SearchBinaryTree<uint8_t> tree{4};

    for (auto const& element : vector) {
        tree.AddString(element);
    }

    for (auto const& element : vector) {
        ConstexprAssertTrue(tree.RemoveString(element));
    }

    for (auto const& element : vector) {
        ConstexprAssertFalse(tree.RemoveString(element));
    }
}
EndConstexprTest(SearchBinaryTreeTest, Removal);

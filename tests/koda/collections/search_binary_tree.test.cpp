#include <koda/collections/search_binary_tree.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstring>
#include <ranges>
#include <set>

template <std::ranges::input_range Range>
static std::basic_string<uint8_t> ConvertToString(Range&& range) {
    std::basic_string<uint8_t> result;
    std::ranges::transform(
        std::forward<Range>(range), std::back_inserter(result),
        [](const char token) { return static_cast<uint8_t>(token); });
    if (result.back() == 0) {
        result.pop_back();
    }
    return result;
}

template <size_t Window, size_t Length>
    requires(Length >= Window)
static std::array<std::basic_string<uint8_t>, Length - Window>
BuildSamplesFromString(const char (&sentence)[Length]) {
    std::array<std::basic_string<uint8_t>, Length - Window> result;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = ConvertToString(
            std::ranges::subrange{&sentence[i], &sentence[i + Window]});
    }
    return result;
}

static const auto kSampleString =
    BuildSamplesFromString<4>("ala ma kota a kot ma ale");

static const auto kOneOfRepeated = ConvertToString(" kot");

TEST(SearchBinaryTreeTest, Creation) {
    koda::SearchBinaryTree tree{4};

    for (auto const& element : kSampleString) {
        tree.AddString(element);
    }

    for (auto const& element : kSampleString) {
        ASSERT_TRUE(tree.FindMatch(element));
    }

    ASSERT_TRUE(tree.FindMatch(ConvertToString("abcd")));
    ASSERT_TRUE(tree.FindMatch(ConvertToString("abc")));
    ASSERT_TRUE(tree.FindMatch(ConvertToString("ab")));
    ASSERT_TRUE(tree.FindMatch(ConvertToString("a")));
    ASSERT_FALSE(tree.FindMatch(ConvertToString("xyzo")));
    ASSERT_FALSE(tree.FindMatch(ConvertToString("xyz")));
    ASSERT_FALSE(tree.FindMatch(ConvertToString("xy")));
    ASSERT_FALSE(tree.FindMatch(ConvertToString("x")));
    // Empty string is always inside!
    ASSERT_TRUE(tree.FindMatch(ConvertToString("")));
}

TEST(SearchBinaryTreeTest, Uniqueness) {
    using Marker = koda::SearchBinaryTree::RepeatitionMarker;

    koda::SearchBinaryTree tree{4};

    for (auto const& element : kSampleString) {
        tree.AddString(element);
    }

    const auto al_result = tree.FindMatch(ConvertToString("al"));

    std::cout << al_result.match_length << "\n";
    std::cout << al_result.match_position << "\n";
    ASSERT_TRUE(al_result == Marker(0, 2) || al_result == Marker(22, 2));

    ASSERT_EQ(tree.FindMatch(ConvertToString("abcd")), Marker(0, 1));
    ASSERT_EQ(tree.FindMatch(ConvertToString("ala")), Marker(0, 3));
    ASSERT_EQ(tree.FindMatch(ConvertToString("ale")), Marker(21, 3));

    const auto kot_result = tree.FindMatch(ConvertToString("kot"));

    ASSERT_TRUE(kot_result == Marker(14, 3) || kot_result == Marker(7, 3));
    ASSERT_EQ(tree.FindMatch(ConvertToString("kota")), Marker(7, 4));
    ASSERT_EQ(tree.FindMatch(ConvertToString("kot ")), Marker(14, 4));

    const auto ma_result = tree.FindMatch(ConvertToString("ma"));

    ASSERT_TRUE(ma_result == Marker(4, 2) || ma_result == Marker(18, 2));
}

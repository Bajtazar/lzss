#include <koda/collections/search_binary_tree.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstring>
#include <ranges>

template <std::ranges::input_range Range>
static std::basic_string<uint8_t> ConvertToString(Range&& range) {
    std::basic_string<uint8_t> result;
    std::ranges::transform(
        std::forward<Range>(range), std::back_inserter(result),
        [](const char token) { return static_cast<uint8_t>(token); });
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

#include <iostream>

static const auto kSampleString =
    BuildSamplesFromString<4>("ala ma kota a kot ma ale");

static const auto kOneOfRepeated = ConvertToString(" kot");

TEST(SearchBinaryTreeTest, Creation) {
    koda::SearchBinaryTree tree;

    for (auto const& element : kSampleString) {
        tree.AddString(element);
    }

    for (auto const& element : kSampleString) {
        ASSERT_TRUE(tree.FindMatch(element));
    }
}

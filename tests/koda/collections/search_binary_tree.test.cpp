#include <koda/collections/search_binary_tree.hpp>
#include <koda/tests/tests.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cstring>
#include <ranges>
#include <set>
#include <vector>

namespace {

template <typename Tp>
struct ViewableVector : public std::vector<Tp> {
    constexpr operator std::basic_string_view<uint8_t>() const noexcept {
        return {this->data(), this->size()};
    }
};

template <std::ranges::input_range Range>
constexpr ViewableVector<uint8_t> ConvertToString(Range&& range) {
    ViewableVector<uint8_t> result;
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
constexpr std::array<ViewableVector<uint8_t>, Length - Window>
BuildSamplesFromString(const char (&sentence)[Length]) {
    std::array<ViewableVector<uint8_t>, Length - Window> result;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = ConvertToString(
            std::ranges::subrange{&sentence[i], &sentence[i + Window]});
    }
    return result;
}

template <size_t Window, size_t Length>
constexpr std::basic_string_view<uint8_t> AsVV(const char (&sentence)[Length]) {
    return ConvertToString(
        std::ranges::subrange{std::begin(sentence), std::end(sentence)});
}

template <size_t Size>
constexpr auto MakeSamples() {
    return BuildSamplesFromString<Size>("ala ma kota a kot ma ale");
}

}  // namespace

BeginConstexprTest(SearchBinaryTreeTest, Creation) {
    auto vector = MakeSamples<4>();
    koda::SearchBinaryTree tree{4};

    for (auto const& element : vector) {
        tree.AddString(element);
    }

    for (auto const& element : vector) {
        ConstexprAssertTrue(tree.FindMatch(element));
    }

    ConstexprAssertTrue(tree.FindMatch(ConvertToString("abcd")));
    ConstexprAssertTrue(tree.FindMatch(ConvertToString("abc")));
    ConstexprAssertTrue(tree.FindMatch(ConvertToString("ab")));
    ConstexprAssertTrue(tree.FindMatch(ConvertToString("a")));
    ConstexprAssertFalse(tree.FindMatch(ConvertToString("xyzo")));
    ConstexprAssertFalse(tree.FindMatch(ConvertToString("xyz")));
    ConstexprAssertFalse(tree.FindMatch(ConvertToString("xy")));
    ConstexprAssertFalse(tree.FindMatch(ConvertToString("x")));
}
EndConstexprTest(SearchBinaryTreeTest, Creation);

BeginConstexprTest(SearchBinaryTreeTest, Uniqueness) {
    using Marker = koda::SearchBinaryTree::RepeatitionMarker;

    auto vector = MakeSamples<4>();
    koda::SearchBinaryTree tree{4};

    for (auto const& element : vector) {
        tree.AddString(element);
    }

    const auto al_result = tree.FindMatch(ConvertToString("al"));
    ConstexprAssertTrue(al_result == Marker(0, 2) ||
                        al_result == Marker(22, 2));

    ConstexprEqual(tree.FindMatch(ConvertToString("abcd")), Marker(0, 1));
    ConstexprEqual(tree.FindMatch(ConvertToString("ala")), Marker(0, 3));
    ConstexprEqual(tree.FindMatch(ConvertToString(" ale")), Marker(20, 4));

    const auto kot_result = tree.FindMatch(ConvertToString("kot"));

    ConstexprAssertTrue(kot_result == Marker(14, 3) ||
                        kot_result == Marker(7, 3));
    ConstexprEqual(tree.FindMatch(ConvertToString("kota")), Marker(7, 4));
    ConstexprEqual(tree.FindMatch(ConvertToString("kot ")), Marker(14, 4));

    const auto ma_result = tree.FindMatch(ConvertToString("ma"));

    ConstexprAssertTrue(ma_result == Marker(4, 2) ||
                        ma_result == Marker(18, 2));
}
EndConstexprTest(SearchBinaryTreeTest, Uniqueness);

BeginConstexprTest(SearchBinaryTreeTest, Removal) {
    auto vector = MakeSamples<4>();
    koda::SearchBinaryTree tree{4};

    for (auto const& element : vector) {
        tree.AddString(element);
    }

    for (auto const& element : vector) {
        ConstexprAssertTrue(tree.RemoveString(element));
    }
}
EndConstexprTest(SearchBinaryTreeTest, Removal);

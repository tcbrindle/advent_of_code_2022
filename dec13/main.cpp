
#include "../common.hpp"

auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

constexpr
auto compare_list_impl(std::string_view& lhs, std::string_view& rhs)
    -> std::strong_ordering
{
    assert(lhs.at(0) == '[' && rhs.at(0) == '[');

    lhs.remove_prefix(1);
    rhs.remove_prefix(1);

    while (true) {
        char l = lhs.at(0), r = rhs.at(0);

        if (l == ']' && r == ']') {
            lhs.remove_prefix(1);
            rhs.remove_prefix(1);
            return std::strong_ordering::equal;
        }
        if (l == ']') {
            return std::strong_ordering::less;
        }
        if (r == ']') {
            return std::strong_ordering::greater;
        }

        if (l == ',' && r == ',') {
            lhs.remove_prefix(1);
            rhs.remove_prefix(1);
            continue;
        }

        if (l == '[' && r == '[') {
            auto r = compare_list_impl(lhs, rhs);
            if (r != 0) {
                return r;
            }
            continue;
        }

        if (l == '[' && is_digit(r)) {
            auto end = flux::find_if_not(rhs, is_digit);

            auto string = '[' + std::string(rhs.substr(0, end)) + ']';
            rhs.remove_prefix(end);
            std::string_view temp(string);

            auto r = compare_list_impl(lhs, temp);
            if (r != 0) {
                return r;
            }
            assert(temp.empty());
            continue;
        }

        if (is_digit(l) && r == '[') {
            auto end = flux::find_if_not(lhs, is_digit);

            auto string = '[' + std::string(lhs.substr(0, end)) + ']';
            lhs.remove_prefix(end);
            std::string_view temp(string);

            auto r = compare_list_impl(temp, rhs);
            if (r != 0) {
                return r;
            }
            assert(temp.empty());
            continue;
        }

        if (is_digit(l) && is_digit(r)) {
            auto lend = flux::find_if_not(lhs, is_digit);
            auto rend = flux::find_if_not(rhs, is_digit);

            int left = aoc::try_parse<int>(lhs.substr(0, lend)).value();
            int right = aoc::try_parse<int>(rhs.substr(0, rend)).value();

            lhs.remove_prefix(lend);
            rhs.remove_prefix(rend);

            auto r = left <=> right;
            if (r != 0) {
                return r;
            }
            continue;
        }

        assert(false && "Should not get here");
    }
};

auto compare = [](std::string_view lhs, std::string_view rhs) {
    return compare_list_impl(lhs, rhs);
};

auto part1 = [](std::string_view input)
{
    return flux::zip(flux::ints(1), flux::split_string(input, "\n\n"))
            .filter([](auto pair) {
                auto [_, lines] = pair;
                auto nl = lines.find('\n');
                auto left = lines.substr(0, nl);
                auto right = lines.substr(nl + 1);
                return std::is_lt(compare(left, right));
            })
            .map([](auto pair) { return pair.first; })
            .sum();
};

auto part2 = [](std::string_view input) {
    auto vec = flux::split_string(input, "\n")
                .filter([](auto sv) { return !sv.empty(); })
                .map([](auto sv) { return std::string(sv); })
                .cache_last() // bugbug
                .to<std::vector>();

    vec.push_back("[[2]]");
    vec.push_back("[[6]]");

    flux::sort(vec, [](auto const& lhs, auto const& rhs) {
        return std::is_lt(compare(lhs, rhs));
    });

    return flux::zip(flux::ints(1), std::ref(vec))
            .filter([&](auto p) { return p.second == "[[2]]" || p.second == "[[6]]"; })
            .map([](auto p) { return p.first; })
            .product();
};

constexpr auto test_data =
R"([1,1,3,1,1]
[1,1,5,1,1]

[[1],[2,3,4]]
[[1],4]

[9]
[[8,7,6]]

[[4,4],4,4]
[[4,4],4,4,4]

[7,7,7,7]
[7,7,7]

[]
[3]

[[[]]]
[[]]

[1,[2,[3,[4,[5,6,7]]]],8,9]
[1,[2,[3,[4,[5,6,0]]]],8,9]
)";

auto test = []{
    return part1(test_data) == 13
        && part2(test_data) == 140;
};
static_assert(test());

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    fmt::print("Part 1: {}\n", part1(input));
    fmt::print("Part 2: {}\n", part2(input));
}
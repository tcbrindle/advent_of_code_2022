
#include "../common.hpp"

struct assignment { int from, to; };

auto parse_line = [](std::string_view line) {
    auto parse_assignment = [](std::string_view str) {
        auto dash = str.find('-');
        return assignment{.from = aoc::try_parse<int>(str.substr(0, dash)).value(),
                          .to = aoc::try_parse<int>(str.substr(dash + 1)).value()};
    };

    auto comma = line.find(',');
    return std::pair(parse_assignment(line.substr(0, comma)),
                     parse_assignment(line.substr(comma + 1)));
};

auto overlap_p1 = [](assignment a, assignment b) {
    return a.from <= b.from && a.to >= b.to;
};

auto overlap_p2 = [](assignment a, assignment b) {
    return b.from <= a.to && b.from >= a.from;
};

template <auto OverlapFn>
auto calculate = [](std::string_view input) {
    return flux::split_string(input, '\n')
            .filter([](auto s) { return s.size() > 0; })
            .map(parse_line)
            .count_if([](auto p) {
                return OverlapFn(p.first, p.second) || OverlapFn(p.second, p.first);
            });
};

auto part1 = calculate<overlap_p1>;
auto part2 = calculate<overlap_p2>;

constexpr auto test_data =
R"(2-4,6-8
2-3,4-5
5-7,7-9
2-8,3-7
6-6,4-6
2-6,4-8
)";

int main(int argc, char** argv)
{
    static_assert(part1(test_data) == 2);
    static_assert(part2(test_data) == 4);

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    fmt::print("Part 1: {}\n", part1(input));
    fmt::print("Part 2: {}\n", part2(input));
}
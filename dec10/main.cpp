
#include "../common.hpp"

auto process = [](std::string_view input) -> flux::generator<int> {
    int x = 1;

    for (auto line : flux::split_string(input, '\n').view()) {
        co_yield x;
        if (line.starts_with("addx")) {
            co_yield x;
            x += aoc::try_parse<int>(line.substr(5)).value();
        }
    }
};

auto part1 = [](std::string_view input) {
    return flux::zip(flux::ints(1), process(input))
            .filter([](auto& p) { return (p.first % 40) == 20; })
            .map([](auto p) { return p.first * p.second; })
            .sum();
};

auto part2 = [](std::string_view input) {
    std::string display;
    int beam_row = 0;

    for (int x : process(input).view()) {
        display += (std::abs(beam_row - x) < 2) ? '#' : '.';

        if (++beam_row == 40) {
            display += '\n';
            beam_row = 0;
        }
    }

    return display;
};

constexpr auto test_result_p2 =
R"(##..##..##..##..##..##..##..##..##..##..
###...###...###...###...###...###...###.
####....####....####....####....####....
#####.....#####.....#####.....#####.....
######......######......######......####
#######.......#######.......#######.....
)";

int main(int argc, char** argv)
{
    {
        auto const test_input = aoc::string_from_file("test_input.txt");
        assert(part1(test_input) == 13140);
        assert(part2(test_input) == test_result_p2);
    }

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    fmt::print("Part 1: {}\n", part1(input));
    fmt::print("\n{}\n", part2(input));
}
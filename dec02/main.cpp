
#include "../common.hpp"


/*auto score_p1 = [](int theirs, int mine) {
    return 1 + mine +
        (mine == theirs ? 3 :
         mine == ((theirs + 1) % 3) ? 6 :
         0);
};

auto score_p2 = [](int theirs, int mins) {
    switch (mine) {
    case 0: return 1 + ((theirs + 2) % 3);
    case 1: return 3 + 1 + theirs;
    case 2: return 6 + 1 + ((theirs + 1) % 3);
    default: return 0;
    }
};*/

constexpr auto score_table_p1 = std::array{
    3 + 1, // rock rock
    6 + 2, // rock paper
    0 + 3, // rock scissors
    0 + 1,// paper rock
    3 + 2,// paper paper
    6 + 3,// paper scissors
    6 + 1,// scissors rock
    0 + 2,// scissors paper
    3 + 3,// scissors scissors
};

constexpr auto score_table_p2 = std::array{
    3 + 0, // rock lose
    1 + 3, // rock draw
    2 + 6, // rock win
    1 + 0,// paper lose
    2 + 3,// paper draw
    3 + 6,// paper win
    2 + 0,// scissors lose
    3 + 3,// scissors draw
    1 + 6// scissors win
};

//constexpr auto score_table_p1 = std::array{4, 8, 3, 1, 5, 9, 7, 2, 6};
//constexpr auto score_table_p2 = std::array{3, 4, 8, 1, 5, 9, 2, 6, 7};

template <auto& ScoreTable>
auto calculate = [](std::string_view input) {
    return flux::split_string(input, '\n')
            .filter([](auto line) { return line.size() >= 3; })
            .map([](auto line) { return 3 * (line[0] - 'A') + line[2] - 'X'; })
            .map([](int i) { return ScoreTable[i]; })
            .sum();
};

auto part1 = calculate<score_table_p1>;
auto part2 = calculate<score_table_p2>;

constexpr auto test_data =
R"(A Y
B X
C Z
)";

int main(int argc, char** argv)
{
    static_assert(part1(test_data) == 15);
    static_assert(part2(test_data) == 12);

    if (argc < 2) {
        fmt::print(stderr, "No input");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    fmt::print("Part 1: {}\n", part1(input));
    fmt::print("Part 1: {}\n", part2(input));
}
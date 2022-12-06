
#include "../common.hpp"

class bitset32 {
    std::uint32_t bits_{};

public:
    constexpr void flip(std::uint32_t pos) { assert(pos < 32); bits_ ^= 1 << pos; }
    constexpr size_t count() const { return __builtin_popcount(bits_); }
};

template <std::size_t N>
auto calculate = [](std::string_view str) -> std::size_t {

    bitset32 bits;

    flux::take(str, N).for_each([&](char c) { bits.flip(c - 'a'); });

    return 1 + flux::iota(N, str.size()).for_each_while([&](std::size_t i) {
        bits.flip(str[i] - 'a');
        bits.flip(str[i - N] - 'a');
        return bits.count() != N;
    });
};

auto part1 = calculate<4>;
auto part2 = calculate<14>;


constexpr auto test_data1 = "mjqjpqmgbljsphdztnvjfqwrcgsmlb";
static_assert(part1(test_data1) == 7);
static_assert(part2(test_data1) == 19);

constexpr auto test_data2 = "bvwbjplbgvbhsrlpgdmjqwftvncz";
static_assert(part1(test_data2) == 5);
static_assert(part2(test_data2) == 23);

constexpr auto test_data3 = "nppdvjthqldpwncqszvftbrmjlhg";
static_assert(part1(test_data3) == 6);
static_assert(part2(test_data3) == 23);

constexpr auto test_data4 = "nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg";
static_assert(part1(test_data4) == 10);
static_assert(part2(test_data4) == 29);

constexpr auto test_data5 = "zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw";
static_assert(part1(test_data5) == 11);
static_assert(part2(test_data5) == 26);

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "Error, no input");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    fmt::print("Part 1: {}\n", part1(input));
    fmt::print("Part 2: {}\n", part2(input));
}

#include "../common.hpp"

auto part1 = [](std::string_view trees) {
    auto const y_max = flux::count(trees, '\n');
    auto const x_max = flux::distance_t(trees.find('\n'));

    auto height = [&](auto a, auto b) { return trees.at((x_max + 1) * b + a); };

    return flux::cartesian_product_with([&](auto x, auto y) {
                auto const h = height(x, y);
                return flux::ints(0    , x    ).all([&](auto i) { return height(i, y) < h; })
                    || flux::ints(x + 1, x_max).all([&](auto i) { return height(i, y) < h; })
                    || flux::ints(0    , y    ).all([&](auto j) { return height(x, j) < h; })
                    || flux::ints(y + 1, y_max).all([&](auto j) { return height(x, j) < h; });
            }, flux::ints(0, x_max), flux::ints(0, y_max))
            .count(true);
};

auto part2 = [](std::string_view trees) {
    auto const y_max = flux::count(trees, '\n');
    auto const x_max = flux::distance_t(trees.find('\n'));

    auto height = [&](auto a, auto b) { return trees.at((x_max + 1) * a + b); };

    return flux::cartesian_product_with([&](auto const x, auto const y) {
                auto const h = height(x, y);
                return (1 + flux::ints(1, x).reverse().take_while([&](auto i) { return height(i, y) < h; }).count())
                     * (1 + flux::ints(x + 1, x_max - 1).take_while([&](auto i) { return height(i, y) < h; }).count())
                     * (1 + flux::ints(1, y).reverse().take_while([&](auto j) { return height(x, j) < h; }).count())
                     * (1 + flux::ints(y + 1, y_max - 1).take_while([&](auto j) {  return height(x, j) < h; }).count());
            }, flux::ints(1, x_max - 1), flux::ints(1, y_max - 1))
            .max().value();
};

constexpr auto test_data =
R"(30373
25512
65332
33549
35390
)";

static_assert(part1(test_data) == 21);
static_assert(part2(test_data) == 8);

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    fmt::print("Part 1: {}\n", part1(input));
    fmt::print("part 2: {}\n", part2(input));
}
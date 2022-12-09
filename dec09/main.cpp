
#include "../common.hpp"

namespace {

struct position {
    int x = 0, y = 0;
    bool operator==(position const&) const = default;
    auto operator<=>(position const&) const = default;
};

auto abs = [](int i) { return i < 0 ? -i : i; };

auto sign = [](int i) { return i < 0 ? -1 : 1; };

auto update_knot = [](position const& prev, position& knot) {
    auto x_dist = prev.x - knot.x;
    auto y_dist = prev.y - knot.y;

    if (abs(x_dist) > 1 && y_dist == 0) {
        knot.x += sign(x_dist);
    } else if (abs(y_dist) > 1 && x_dist == 0) {
        knot.y += sign(y_dist);
    } else if (abs(x_dist) > 1 || abs(y_dist) > 1) {
        knot.x += sign(x_dist);
        knot.y += sign(y_dist);
    }
};

template <int RopeSize>
auto calculate = [](std::string_view input) {
    std::array<position, RopeSize> rope{};
    std::vector<position> positions{{0, 0}};

    flux::split_string(input, '\n')
        .filter([](auto sv) { return !sv.empty(); })
        .for_each([&](std::string_view line) {
            char const dir = line.at(0);
            int const steps = aoc::try_parse<int>(line.substr(2)).value();

            flux::ints(0, steps).for_each([&](auto) {
                auto& head = rope.front();
                switch (dir) {
                case 'U': ++head.y; break;
                case 'D': --head.y; break;
                case 'R': ++head.x; break;
                case 'L': --head.x; break;
                }
                for (std::size_t i = 1; i < RopeSize; i++) {
                    update_knot(rope[i - 1], rope[i]);
                }
                positions.push_back(rope.back());
            });
        });

    flux::sort(positions);
    // no unique algo in Flux yet :(
    auto [from, to] = std::ranges::unique(positions);
    return std::distance(positions.begin(), from);
};

auto part1 = calculate<2>;
auto part2 = calculate<10>;

constexpr auto test_data1 =
R"(R 4
U 4
L 3
D 1
R 4
D 1
L 5
R 2
)";

constexpr auto test_data2 =
R"(R 5
U 8
L 8
D 3
R 17
D 10
L 25
U 20)";

static_assert(part1(test_data1) == 13);
static_assert(part2(test_data1) == 1);
static_assert(part2(test_data2) == 36);

}

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
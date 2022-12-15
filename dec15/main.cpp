
#include "../common.hpp"

#include "../extern/ctre.hpp"

/*
 * Flux polyfills
 */

auto flat_map = []<flux::sequence Seq, typename Func>(Seq seq, Func func)
    -> flux::generator<flux::element_t<std::invoke_result_t<Func&, flux::element_t<Seq>>>>
{
    FLUX_FOR(auto&& elem, flux::map(std::move(seq), std::move(func))) {
        FLUX_FOR(auto&& inner, FLUX_FWD(elem)) {
            co_yield FLUX_FWD(inner);
        }
    }
};

auto front = []<typename Seq>(Seq&& seq) -> std::optional<flux::value_t<Seq>>
{
    auto cur = flux::first(seq);
    if (!flux::is_last(seq, cur)) {
        return std::optional<flux::value_t<Seq>>(std::in_place, flux::read_at(seq, cur));
    } else {
        return std::nullopt;
    }
};

/*
 * Solution code
 */

struct coord {
    int x = 0, y = 0;

    friend auto operator==(coord, coord) -> bool = default;
    friend auto operator<=>(coord, coord) = default;
};

auto distance = [](coord from, coord to) {
    return std::abs(from.x - to.x) + std::abs(from.y - to.y);
};

struct sensor {
    coord pos;
    coord beacon;
};

constexpr auto& regex = R"(Sensor at x=(-?\d+), y=(-?\d+): closest beacon is at x=(-?\d+), y=(-?\d+)\n?)";

auto to_int = [](std::string_view sv) { return aoc::try_parse<int>(sv).value(); };

auto parse_input = [](std::string_view input) -> std::vector<sensor>
{
    return flux::from(ctre::tokenize<regex>(input))
            .map([](auto match) {
                auto [_, x0, y0, x1, y1] = match;
                return sensor {
                    .pos = {.x = to_int(x0), .y = to_int(y0)},
                    .beacon = {.x = to_int(x1), .y = to_int(y1)}
                };
            })
            .to<std::vector>();
};

auto potential_beacon = [](auto const& sensors, coord pos) -> bool {
    auto within_min_distance = [&](auto& s) -> bool {
        auto [sensor, beacon] = s;
        return distance(sensor, pos) <= distance(sensor, beacon);
    };

    return flux::none(sensors, within_min_distance);
};

template <int Row>
auto part1 = [](auto const& sensors) {

    auto x_min = flux::ref(sensors)
                    .map([](auto s) { return s.pos.x - distance(s.pos, s.beacon); })
                    .min().value();

    auto x_max = flux::ref(sensors)
                    .map([](auto s) { return s.pos.x + distance(s.pos, s.beacon); })
                    .max().value();

    return flux::ints(x_min, x_max)
            .count_if([&](int x) { return !potential_beacon(sensors, {x, Row}); });
};

template <int Max>
auto in_bounds = [](coord c) { return c.x >= 0 && c.y <= Max && c.y >= 0 && c.x <= Max; };

auto border_generator = [](sensor s) -> flux::generator<coord> {
    auto [pos, beacon] = s;

    int dist = 1 + distance(pos, beacon);

    for (int d = 0; d <= 2*dist; d++) {
        co_yield {pos.x - dist + d, pos.y - d};
        co_yield {pos.x - dist + d, pos.y + d};
    }
};

template <int Max>
auto part2 = [](auto const& sensors) -> int64_t {
    return flux::ref(sensors)
                ._(flat_map, border_generator)
                .filter(in_bounds<Max>)
                .filter([&](coord c) { return potential_beacon(sensors, c); })
                .map([](coord c) { return c.x * 4'000'000LL + c.y; })
                ._(front)
                .value_or(-1);
};

constexpr auto test_data =
R"(Sensor at x=2, y=18: closest beacon is at x=-2, y=15
Sensor at x=9, y=16: closest beacon is at x=10, y=16
Sensor at x=13, y=2: closest beacon is at x=15, y=3
Sensor at x=12, y=14: closest beacon is at x=10, y=16
Sensor at x=10, y=20: closest beacon is at x=10, y=16
Sensor at x=14, y=17: closest beacon is at x=10, y=16
Sensor at x=8, y=7: closest beacon is at x=2, y=10
Sensor at x=2, y=0: closest beacon is at x=2, y=10
Sensor at x=0, y=11: closest beacon is at x=2, y=10
Sensor at x=20, y=14: closest beacon is at x=25, y=17
Sensor at x=17, y=20: closest beacon is at x=21, y=22
Sensor at x=16, y=7: closest beacon is at x=15, y=3
Sensor at x=14, y=3: closest beacon is at x=15, y=3
Sensor at x=20, y=1: closest beacon is at x=15, y=3
)";

auto test = [] {
    const auto sensors = parse_input(test_data);
    assert(part1<10>(sensors) == 26);
    assert(part2<20>(sensors) == 56000011);
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto input = aoc::string_from_file(argv[1]);

    auto const sensors = parse_input(input);

    fmt::print("Part 1: {}\n", part1<2'000'000>(sensors));
    fmt::print("Part 2: {}\n", part2<4'000'000>(sensors));
}
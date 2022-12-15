
#include "../common.hpp"


/*
 * Some things that should be in Flux but aren't (yet)
 */
auto pairwise = []<flux::multipass_sequence Seq>(Seq seq)
    -> flux::generator<std::pair<flux::element_t<Seq>, flux::element_t<Seq>>>
{
    auto cur0 = flux::first(seq);
    if (flux::is_last(seq, cur0)) {
        co_return;
    }

    auto cur1 = flux::next(seq, cur0);

    while (!flux::is_last(seq, cur1)) {
        co_yield std::pair(flux::read_at(seq, cur0),
                           flux::read_at(seq, cur1));

        cur0 = cur1;
        flux::inc(seq, cur1);
    }
};

auto flat_map = []<flux::sequence Seq, typename Func>(Seq seq, Func func)
    -> flux::generator<flux::element_t<std::invoke_result_t<Func&, flux::element_t<Seq>>>>
{
    FLUX_FOR(auto&& elem, flux::map(std::move(seq), std::move(func))) {
        FLUX_FOR(auto&& inner, FLUX_FWD(elem)) {
            co_yield FLUX_FWD(inner);
        }
    }
};

/*
 * The actual problem
 */
constexpr int x_max = 1000;
constexpr int y_max = 200;

struct pos_t {
    int x = 0, y = 0;

    friend bool operator==(pos_t, pos_t) = default;
    friend auto operator<=>(pos_t, pos_t) = default;
    friend constexpr pos_t operator+(pos_t lhs, pos_t rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }
};

enum class tile {
    empty, rock, sand
};

struct grid {
    std::vector<tile> tiles = std::vector<tile>(x_max * y_max, tile::empty);
    int floor = 0;

    constexpr auto operator[](pos_t p) const -> tile
    {
        if (p.y == floor) {
            return tile::rock;
        } else {
            return tiles.at((p.y * x_max) + p.x);
        }
    }

    constexpr void set(pos_t p, tile t)
    {
        tiles.at((p.y * x_max) + p.x) = t;
    }
};

auto parse_pos = [](std::string_view str) {
    auto parts = flux::split_string(str, ',')
        .map([](auto s) { return aoc::try_parse<int>(s).value(); });
    auto i = parts.first();
    return pos_t(parts[i], parts[parts.next(i)]);
};

auto extrapolate = [](auto pair) -> flux::generator<pos_t> {
    auto [from, to] = pair;

    if (from > to) {
        std::swap(from, to);
    }

    for (int i = from.x; i <= to.x; ++i) {
        for (int j = from.y; j <= to.y; ++j) {
            co_yield pos_t{i, j};
        }
    }
};

auto parse_input = [](std::string_view input) {
    grid g;

    flux::split_string(input, '\n')
        .filter([](auto line) { return !line.empty(); })
        ._(flat_map, [](auto line) {
            return flux::split_string(line, " -> ")
                .map(parse_pos)
                ._(pairwise)
                ._(flat_map, extrapolate);
        })
        .for_each([&g](pos_t p) {
            g.set(p, tile::rock);
            g.floor = std::max(g.floor, p.y);
        });

    g.floor += 2;

    return g;
};

constexpr auto sand_source = pos_t{500, 0};

auto drop_sand = [](grid const& cave) -> pos_t
{
    pos_t sand = sand_source;

    while (sand.y < cave.floor) {
        constexpr auto dirs = std::array<pos_t, 3>{
            pos_t{0, 1}, {-1, 1}, {1, 1}
        };

        auto seq = flux::map(dirs, [&](auto p) { return p + sand; });
        auto idx = seq.find(tile::empty, [&](pos_t p) { return cave[p]; });

        if (!seq.is_last(idx)) {
            sand = seq.read_at(idx);
        } else {
            break;
        }
    }

    return sand;
};

auto part1 = [](grid cave) {

    while (true) {
        pos_t sand = drop_sand(cave);
        if (sand.y == cave.floor - 1) {
            break;
        } else {
            cave.set(sand, tile::sand);
        }
    }

    return flux::count(cave.tiles, tile::sand);
};

auto part2 = [](grid cave) {
    while (true) {
        pos_t sand = drop_sand(cave);
        if (sand == sand_source) {
            break;
        } else {
            cave.set(sand, tile::sand);
        }
    }

    return 1 + flux::count(cave.tiles, tile::sand);
};

constexpr auto test_data =
R"(498,4 -> 498,6 -> 496,6
503,4 -> 502,4 -> 502,9 -> 494,9
)";

auto test = [] {
    auto cave = parse_input(test_data);
    return part1(cave) == 24
        && part2(cave) == 93;
};

int main(int argc, char** argv)
{
    assert(test());

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    auto const cave = parse_input(input);

    fmt::print("Part 1: {}\n", part1(cave));
    fmt::print("Part 2: {}\n", part2(cave));
}
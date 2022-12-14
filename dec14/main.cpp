
#include "../common.hpp"

#include "../extern/robin_hood.h"


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

auto flatten = []<flux::sequence Seq>(Seq seq)
    -> flux::generator<flux::element_t<flux::element_t<Seq>>>
{
    FLUX_FOR(auto&& elem, seq) {
        FLUX_FOR(auto&& inner, elem) {
            co_yield inner;
        }
    }
};

auto keys = [](flux::adaptable_sequence auto&& seq) {
    return flux::map(FLUX_FWD(seq), [](auto&& elem) -> decltype(auto) {
        return FLUX_FWD(elem).first;
    });
};

auto values = [](flux::adaptable_sequence auto&& seq) {
    return flux::map(FLUX_FWD(seq), [](auto&& elem) -> decltype(auto) {
        return FLUX_FWD(elem).second;
    });
};

/*
 * the actual problem solution
 */

struct pos_t {
    int x = 0, y = 0;

    friend bool operator==(pos_t, pos_t) = default;
    friend auto operator<=>(pos_t, pos_t) = default;

    friend constexpr pos_t operator+(pos_t lhs, pos_t rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y };
    }
};

template <>
struct std::hash<pos_t> {
    constexpr auto operator()(pos_t p) const -> std::uint64_t {
        return (uint64_t(p.x) << 32) | p.y;
    }
};

enum class tile_kind { rock, sand };

//using map_t = std::map<pos_t, tile_kind>;
//using map_t = std::unordered_map<pos_t, tile_kind>;
using map_t = robin_hood::unordered_map<pos_t, tile_kind>;


struct cave_t {
    map_t data;
    int floor;

    bool contains(pos_t pos) const
    {
        if (pos.y == floor) {
            return true;
        } else {
            return data.contains(pos);
        }
    }

    void emplace(pos_t pos, tile_kind k) {
        data.emplace(pos, k);
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

auto parse = [](std::string_view input) -> cave_t {
    auto map = flux::split_string(input, '\n')
                    .filter([](auto line) { return !line.empty(); })
                    .map([](auto line) {
                        return flux::split_string(line, " -> ")
                            .map(parse_pos)
                            ._(pairwise)
                            .map(extrapolate)
                            ._(flatten);
                    })
                    ._(flatten)
                    .map([](pos_t p) { return map_t::value_type(p, tile_kind::rock); })
                    .to<map_t>();

    auto floor = 2 + flux::ref(map)._(keys).max({}, &pos_t::y).value().y;

    return cave_t{std::move(map), floor};
};

constexpr auto sand_source = pos_t{500, 0};

auto drop_sand = [](cave_t const& cave) -> pos_t {

    pos_t sand = sand_source;

    while (sand.y < cave.floor) {
        if (!cave.contains(sand + pos_t{0, 1})) {
            sand = sand + pos_t{0, 1};
        } else if (!cave.contains(sand + pos_t{-1, 1})) {
            sand = sand + pos_t{-1, 1};
        } else if (!cave.contains(sand + pos_t{1, 1})) {
            sand = sand + pos_t{1, 1};
        } else {
            break;
        }
    }

    return sand;
};

auto part1 = [](cave_t cave) {
    while (true) {
        auto sand = drop_sand(cave);
        if (sand.y == cave.floor - 1) {
            break;
        } else {
            cave.emplace(sand, tile_kind::sand);
        }
    }

    return values(std::move(cave).data).count(tile_kind::sand);
};

auto part2 = [](cave_t cave) {
    while (true) {
        auto sand = drop_sand(cave);
        if (sand == sand_source) {
            break;
        } else {
            cave.emplace(sand, tile_kind::sand);
        }
    }

    return 1 + values(std::move(cave).data).count(tile_kind::sand);
};

constexpr auto test_data =
R"(498,4 -> 498,6 -> 496,6
503,4 -> 502,4 -> 502,9 -> 494,9
)";

auto test = [] {
    auto cave = parse(test_data);
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

    auto const cave = parse(input);

    fmt::print("Part 1: {}\n", part1(cave));
    fmt::print("Part 2: {}\n", part2(cave));
}
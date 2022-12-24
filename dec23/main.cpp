
#include "../common.hpp"

#include <set>

namespace {

struct vec2 {
    int x = 0;
    int y = 0;

    constexpr auto operator+=(vec2 rhs) -> vec2& {
        x += rhs.x; y += rhs.y;
        return *this;
    }

    friend auto operator==(vec2, vec2) -> bool = default;
    friend auto operator<=>(vec2, vec2) = default;

    friend constexpr auto operator+(vec2 lhs, vec2 rhs) -> vec2 {
        return lhs += rhs;
    }
};

auto parse_input = [](std::string_view input) -> std::vector<vec2> {
    int x = 0; int y = 0;
    std::vector<vec2> elves;
    for (char c : input) {
        switch (c) {
        case '#' : elves.push_back({x, y}); ++x; break;
        case '\n': ++y; x = 0; break;
        default: ++x;
        }
    }
    flux::sort(elves);
    return elves;
};

auto contains = [](auto const& elves, vec2 pos) -> bool {
    return std::ranges::binary_search(elves, pos);
};

auto get_neighbours = [](vec2 p) {
    return flux::from(std::array<vec2, 8>{
        vec2{p.x - 1, p.y - 1}, {p.x, p.y - 1}, {p.x + 1, p.y - 1},
            {p.x - 1, p.y    },                 {p.x + 1, p.y    },
            {p.x - 1, p.y + 1}, {p.x, p.y + 1}, {p.x + 1, p.y + 1}
    });
};

constexpr int indices_map[][3] = {
    /*north = */{0, 1, 2},
    /*south = */{5, 6, 7},
    /*west  = */{0, 3, 5},
    /*east  = */{2, 4, 7}
};

constexpr vec2 directions_map[] = {
    /*north = */{0 , -1},
    /*south = */{0 ,  1},
    /*west  = */{-1,  0},
    /*east  = */{1 , 0 }
};

auto unique_copy = [](std::vector<vec2> vec) {
    flux::sort(vec);
    std::vector<vec2> out;
    out.reserve(vec.size());

    auto seq = flux::ref(vec);
    auto idx = seq.first();

    while (!seq.is_last(idx)) {
        auto next = flux::slice(seq, seq.next(idx), flux::last).find_if([p1=seq[idx]](vec2 p2) { return p1 != p2; });
        if (next == seq.next(idx)) {
            out.push_back(seq[idx]);
        }
        idx = next;
    }

    return out;
};

auto round = [](std::vector<vec2> elves, int starting_dir) {

    std::vector<vec2> proposed_moves = flux::map(std::cref(elves), [&](vec2 elf) -> vec2 {
        std::array<bool, 8> occupied{};
        get_neighbours(elf)
            .map([&elves](vec2 p) { return contains(elves, p); })
            .output_to(occupied.begin());

        if (flux::any(occupied, std::identity{})) {
            for (int i = 0; i < 4; i++) {
                auto dir = (starting_dir + i) % 4;

                if (flux::none(indices_map[dir], [&](int i) { return occupied[i]; })) {
                    return  elf + directions_map[dir];
                }
            }
        }

        return elf;
    }).to<std::vector<vec2>>();

    std::vector<vec2> unq_moves = unique_copy(proposed_moves);

    flux::zip(std::ref(elves), std::cref(proposed_moves))
        .filter([&](auto p) { auto [_, move] = p; return contains(unq_moves, move); })
        .for_each([](auto p) { auto [elf, move] = p; elf = move; });

    flux::sort(elves);

    return elves;
};

auto part1 = [](std::vector<vec2> elves) {
    elves = flux::ints(0, 10).fold([](auto elves, int i) {
        return round(std::move(elves), i % 4);
    }, std::move(elves));

    auto [min_x, max_x] = flux::map(std::cref(elves), &vec2::x).minmax().value();
    auto [min_y, max_y] = flux::map(std::cref(elves), &vec2::y).minmax().value();

    return ((1 + max_x) - min_x) * ((1 + max_y) - min_y) - flux::size(elves);
};

auto part2 = [](std::vector<vec2> elves) {
    int i = 0;
    while (true) {
        auto new_elves = round(elves, i % 4);
        if (elves == new_elves) {
            break;
        }
        elves = std::move(new_elves);
        ++i;
    }

    return i + 1;
};

constexpr auto& test_input =
R"(....#..
..###.#
#...#.#
.#...##
#.###..
##.#.##
.#..#..
)";

auto test = []{
    auto elves = parse_input(test_input);
    return part1(elves) == 110
        && part2(elves) == 20;
};
static_assert(test());

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto elves = parse_input(aoc::string_from_file(argv[1]));
    fmt::print("Part 1: {}\n", part1(elves));
    fmt::print("Part 2: {}\n", part2(elves));
}
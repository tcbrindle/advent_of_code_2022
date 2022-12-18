
#include "../common.hpp"

#include "../extern/ctre.hpp"

struct vec3 {
    int x = 0, y = 0, z = 0;

    friend auto operator==(vec3 const&, vec3 const&) -> bool = default;
    friend auto operator<=>(vec3 const&, vec3 const&) = default;
};

using cubes_t = std::vector<vec3>;

auto contains = [](cubes_t const& cubes, vec3 const& pos) {
    return std::ranges::binary_search(cubes, pos);
};

constexpr auto& regex = R"((\d+),(\d+),(\d+))";

auto parse_input = [](std::string_view input)
{
    cubes_t cubes = flux::split_string(input, '\n')
            .filter([](auto line) { return !line.empty(); })
            .map([](auto line) {
                auto [_, a, b, c] = ctre::match<regex>(line);
                return vec3{
                    .x = aoc::try_parse<int>(a).value(),
                    .y = aoc::try_parse<int>(b).value(),
                    .z = aoc::try_parse<int>(c).value()
                };
            })
            .cache_last() // bugbugbug
            .to<cubes_t>();

    flux::sort(cubes);
    return cubes;
};

auto get_neighbours = [](vec3 pos) {
    return flux::from(std::array<vec3, 6>{
        vec3{pos.x - 1, pos.y,     pos.z},
            {pos.x + 1, pos.y,     pos.z},
            {pos.x,     pos.y - 1, pos.z},
            {pos.x,     pos.y + 1, pos.z},
            {pos.x,     pos.y,     pos.z - 1},
            {pos.x,     pos.y,     pos.z + 1}
    });;
};

auto int_rng = [](int max) { return flux::iota(0, max); };

auto part1 = [](cubes_t const& cubes) -> int {
    std::vector<int> faces(cubes.size(), 6);

    int_rng(cubes.size()).for_each([&](int idx) {
        flux::from(get_neighbours(cubes[idx]))
            .filter([&](vec3 n) { return contains(cubes, n); })
            .for_each([&](vec3) { --faces.at(idx); });
    });

    return flux::sum(faces);
};

auto find_bounds = [](cubes_t const& cubes) -> vec3 {
    return vec3 {
        .x = 2 + flux::max(cubes, {}, &vec3::x).value().x,
        .y = 2 + flux::max(cubes, {}, &vec3::y).value().y,
        .z = 2 + flux::max(cubes, {}, &vec3::z).value().z
    };
};

auto part2 = [](cubes_t const& cubes) -> int {
    auto const [x_max, y_max, z_max] = find_bounds(cubes);

    auto in_bounds = [&](vec3 p) {
        return p.x >= 0 && p.x < x_max && p.y >= 0 && p.y < y_max && p.z >= 0 && p.z < z_max;
    };

    // Starting at {0, 0, 0}, expand the "steam" in six directions,
    // ignoring out-of-bounds positions, positions we have already visited,
    // and places where cubes lie
    auto steam = std::vector<bool>(x_max * y_max * z_max, false);

    auto visited = [&](vec3 pos) -> decltype(auto) {
        return steam.at((pos.z * y_max * x_max) + (pos.y * x_max) + pos.x);
    };

    std::vector<vec3> queue{vec3{0, 0, 0}};

    while (!queue.empty()) {
        vec3 pos = queue.back();
        queue.pop_back();
        visited(pos) = true;

        get_neighbours(pos)
            .filter([&](vec3 n) {
                return in_bounds(n) && !visited(n) && !contains(cubes, n);
            })
            .for_each([&](vec3 n) { queue.push_back(n); });
    }

    // Now build another cube set of the empty spaces and do part 1 again
    cubes_t new_cubes = flux::cartesian_product(int_rng(x_max), int_rng(y_max), int_rng(z_max))
                        .map([](auto tuple) { auto [i, j, k] = tuple; return vec3{i, j, k}; })
                        .filter(std::not_fn(visited))
                        .to<cubes_t>();

    return part1(std::move(new_cubes));
};

constexpr auto test_data =
R"(2,2,2
1,2,2
3,2,2
2,1,2
2,3,2
2,2,1
2,2,3
2,2,4
2,2,6
1,2,5
3,2,5
2,1,5
2,3,5)";

auto test = [] {
    auto const test_cubes = parse_input(test_data);
    return part1(test_cubes) == 64;
    return part2(test_cubes) == 58;
};
static_assert(test());

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);
    auto cubes = parse_input(input);

    fmt::print("Part 1: {}\n", part1(cubes));
    fmt::print("Part 2: {}\n", part2(cubes));
}
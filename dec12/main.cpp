
#include "../common.hpp"

auto neighbour_indices = [](int idx, int x_max, int y_max) {
    int x = idx % x_max;
    int y = idx/x_max;

    return flux::filter(std::array{
        x > 0         ? idx - 1     : -1,
        x + 1 < x_max ? idx + 1     : -1,
        y > 0         ? idx - x_max : -1,
        y + 1 < y_max ? idx + x_max : -1
    }, [](int i) { return i > -1; });
};

auto dijkstra = [](std::string_view heights, int x_max, int y_max, int start_idx) -> std::vector<int>
{
    std::vector<bool> visited(heights.size(), false);
    std::vector<int> distances(heights.size(), INT_MAX);

    auto dist_at = [&](int idx) -> auto& { return distances.at(idx); };

    dist_at(start_idx) = 0;
    int cur_idx = start_idx;

    flux::for_each_while(flux::ints(0, heights.size()), [&](auto) {
        neighbour_indices(cur_idx, x_max, y_max)
            .filter([&](int n_idx) { return heights.at(cur_idx) - heights.at(n_idx) <= 1; })
            .filter([&](int n_idx) { return !visited.at(n_idx); })
            .for_each([&](int n_idx) { dist_at(n_idx) = std::min(dist_at(n_idx), 1 + dist_at(cur_idx)); });

        visited.at(cur_idx) = true;

        auto next_idx = flux::ints(0, distances.size())
                            .filter([&](auto idx) { return !visited.at(idx); })
                            .min({}, dist_at);

        if (next_idx.has_value() && dist_at(*next_idx) < INT_MAX) {
            cur_idx = *next_idx;
            return true;
        } else {
            return false;
        }
    });

    return distances;
};

auto read_input = [](std::string_view input) {

    auto x_max = flux::distance_t(input.find('\n'));
    auto y_max = flux::count(input, '\n');

    std::string heightmap(input);
    std::erase(heightmap, '\n');

    auto s_idx = heightmap.find('S');
    heightmap.at(s_idx) = 'a';

    auto e_idx = heightmap.find('E');
    heightmap.at(e_idx) = 'z';

    auto dists = dijkstra(heightmap, x_max, y_max, e_idx);

    return std::tuple(s_idx, std::move(heightmap), std::move(dists));
};

auto part1 = [](auto s_idx, auto const& dists) -> int {
    return dists.at(s_idx);
};

auto part2 = [](auto const& heights, auto const& dists) -> int {
    return flux::ints(0, heights.size())
            .filter([&](int idx) { return heights.at(idx) == 'a'; })
            .map([&](int idx) { return dists.at(idx); })
            .min().value();
};

constexpr auto& test_input =
R"(Sabqponm
abcryxxl
accszExk
acctuvwj
abdefghi
)";

auto test = [] {
    auto [start_idx, heights, dists] = read_input(test_input);
    return part1(start_idx, dists) == 31
        && part2(heights, dists) == 29;
};
static_assert(test());

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);
    auto [start_idx, heights, dists] = read_input(input);

    fmt::print("Part 1: {}\n", part1(start_idx, dists));
    fmt::print("Part 2: {}\n", part2(heights, dists));
}
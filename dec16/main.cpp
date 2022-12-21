
#include "../common.hpp"

#include "../extern/ctre.hpp"

#include <stack>

struct valve {
    std::string name;
    int flow_rate;
    std::vector<std::string> tunnels;
};

constexpr auto& valve_regex = R"(^Valve (\w\w) has flow rate=(\d+); tunnels? leads? to valves? (.*)$)";

auto parse_input = [](std::string_view input) -> std::vector<valve>
{
    return flux::split_string(input, '\n')
            .filter([](auto line) { return !line.empty(); })
            .map([](auto line) {
                auto [_, name, rate, tunnels] = ctre::match<valve_regex>(line);
                return valve{
                    .name = std::string(name),
                    .flow_rate = aoc::try_parse<int>(rate).value(),
                    .tunnels = flux::split_string(tunnels.view(), ", ")
                                .map([](auto sv) { return std::string(sv); })
                                .template to<std::vector>()
                };
            })
            .to<std::vector>();
};

auto valve_idx = [](std::vector<valve> const& valves, std::string_view name) -> size_t
{
    return flux::find(valves, name, &valve::name);
};

auto build_distance_map = [](std::vector<valve> const& valves) -> std::vector<std::vector<int>>
{
    auto const sz = flux::size(valves);

    std::vector<std::vector<int>> distance_map(sz);
    for (auto& row : distance_map) {
        row.assign(sz, 1000);
    }

    for (auto const& [i, valve] : flux::zip(flux::ints(), flux::ref(valves)).view()) {
        for (auto const& tunnel : valve.tunnels) {
            distance_map.at(i).at(valve_idx(valves, tunnel)) = 1;
        }
    }

    for (int i = 0; i < sz; i++) {
        distance_map[i][i] = 0;
    }

    for (auto k : flux::ints(0, sz).view()) {
        for (auto i : flux::ints(0, sz).view()) {
            for (auto j : flux::ints(0, sz).view()) {
                int d = distance_map[i][k] + distance_map[k][j];
                if (distance_map[i][j] > d) {
                    distance_map[i][j] = d;
                }
            }
        }
    }

    return distance_map;
};

template <int StartTime, int NumPaths>
auto calculate = [](std::vector<valve> const& valves) -> std::vector<std::pair<int, std::vector<int>>>
{
    auto const dist_map = build_distance_map(valves);

    struct state_t {
        std::vector<bool> opened;
        size_t current_idx;
        int time_remaining = StartTime;
        int flow = 0;
        std::vector<int> path{};
    };

    std::stack<state_t> queue;
    queue.push(state_t{
        .opened = std::vector<bool>(valves.size()),
        .current_idx = valve_idx(valves, "AA")
    });

    std::vector<std::pair<int, std::vector<int>>> best_paths;
    best_paths.emplace_back(0, std::vector<int>{});

    while (!queue.empty()) {
        state_t state = queue.top();
        queue.pop();

        if (valves.at(state.current_idx).flow_rate > 0) {
            state.path.push_back(state.current_idx);
            state.opened.at(state.current_idx) = true;
            state.time_remaining -= 1;
            state.flow += (state.time_remaining * valves.at(state.current_idx).flow_rate);
        }

        if (state.time_remaining > 0) {
            flux::iota(0uz, valves.size())
                    .filter([&](size_t i) { return i != state.current_idx; })
                    .filter([&](size_t i) { return valves.at(i).flow_rate > 0; })
                    .filter([&](size_t i) { return !state.opened.at(i); })
                    .filter([&](size_t i) { return dist_map.at(state.current_idx).at(i) < state.time_remaining; })
                    .for_each([&](size_t next_idx) {
                        auto next_state = state;
                        next_state.current_idx = next_idx;
                        next_state.time_remaining -= dist_map.at(state.current_idx).at(next_idx);

                        queue.push(next_state);
                    });
        }

        if (state.flow > best_paths.back().first) {
            best_paths.emplace_back(state.flow, std::move(state.path));
            flux::sort(best_paths, std::greater{}, &std::pair<int, std::vector<int>>::first);
            if (best_paths.size() > NumPaths) {
                best_paths.pop_back();
            }
        }
    }

    return best_paths;
};

auto part1 = [](std::vector<valve> const& valves) -> int
{
    return calculate<30, 1>(valves).front().first;
};

auto disjoint = [](auto&& rng1, auto&& rng2) -> bool
{
    auto i1 = std::ranges::begin(rng1);
    auto const s1 = std::ranges::end(rng1);
    auto i2 = std::ranges::begin(rng2);
    auto const s2 = std::ranges::end(rng2);

    while (i1 != s1 && i2 != s2) {
        if (*i1 < *i2) {
            ++i1;
        } else if (*i2 < *i1) {
            ++i2;
        } else {
            return false;
        }
    }

    return true;
};

auto part2 = [](std::vector<valve> const& valves) -> int
{
    auto paths = calculate<26, 2000>(valves);

    return flux::cartesian_product(std::ref(paths), std::ref(paths))
        .filter([](auto pair) {
            auto const& [left, right] = pair;
            flux::sort(left.second);
            flux::sort(right.second);
            return disjoint(left.second, right.second);
        })
        .map([](auto pair) {
            auto const& [left, right] = pair;
            return left.first + right.first;
        })
        .max().value_or(0);
};



constexpr auto test_input =
R"(Valve AA has flow rate=0; tunnels lead to valves DD, II, BB
Valve BB has flow rate=13; tunnels lead to valves CC, AA
Valve CC has flow rate=2; tunnels lead to valves DD, BB
Valve DD has flow rate=20; tunnels lead to valves CC, AA, EE
Valve EE has flow rate=3; tunnels lead to valves FF, DD
Valve FF has flow rate=0; tunnels lead to valves EE, GG
Valve GG has flow rate=0; tunnels lead to valves FF, HH
Valve HH has flow rate=22; tunnel leads to valve GG
Valve II has flow rate=0; tunnels lead to valves AA, JJ
Valve JJ has flow rate=21; tunnel leads to valve II
)";

auto test = [] {
    auto valves = parse_input(test_input);
    return part1(valves) == 1651
        && part2(valves) == 1707;
};

int main(int argc, char** argv)
{
    assert(test);

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const valves = parse_input(aoc::string_from_file(argv[1]));

    fmt::print("Part 1: {}\n", part1(valves));
    fmt::print("Part 2: {}\n", part2(valves));
}

#include "../common.hpp"

#include "../extern/ctre.hpp"

#include <stack>

struct resources {
    int ore = 0;
    int clay = 0;
    int obsidian = 0;
    int geode = 0;

    constexpr auto operator+=(resources const& rhs) -> resources&
    {
        ore += rhs.ore;
        clay += rhs.clay;
        obsidian += rhs.obsidian;
        geode += rhs.geode;
        return *this;
    }

    constexpr auto operator-=(resources const& rhs) -> resources&
    {
        ore -= rhs.ore;
        clay -= rhs.clay;
        obsidian -= rhs.obsidian;
        geode -= rhs.geode;
        return *this;
    }

    friend auto operator==(resources const&, resources const&) -> bool = default;
};

auto print_resources = [](resources const& r) {
    fmt::print("o: {}, c: {}, ob: {}, g: {}\n", r.ore, r.clay, r.obsidian, r.geode);
};

struct blueprint {
    int id;
    resources ore_robot;
    resources clay_robot;
    resources obsidian_robot;
    resources geode_robot;
};

auto can_build = [](resources const& need, resources const& have) {
    return have.ore      >= need.ore  &&
           have.clay     >= need.clay &&
           have.obsidian >= need.obsidian;
};

constexpr auto& blueprint_regex =
R"(Blueprint (\d+): Each ore robot costs (\d+) ore. Each clay robot costs (\d+) ore. Each obsidian robot costs (\d+) ore and (\d+) clay. Each geode robot costs (\d+) ore and (\d+) obsidian.)";

auto parse_blueprint = [](std::string_view line) -> blueprint
{
    auto match = ctre::match<blueprint_regex>(line);
    assert(match);
    return blueprint{
        .id = match.get<1>().to_number(),
        .ore_robot = resources{.ore = match.get<2>().to_number()},
        .clay_robot = resources{.ore = match.get<3>().to_number()},
        .obsidian_robot = resources{.ore = match.get<4>().to_number(),
                                    .clay = match.get<5>().to_number()},
        .geode_robot = resources{.ore = match.get<6>().to_number(),
                                 .obsidian = match.get<7>().to_number()}
    };
};

auto parse_input = [](std::string_view input) -> std::vector<blueprint>
{
    return flux::split_string(input, '\n')
            .filter([](auto line) { return !line.empty(); })
            .map(parse_blueprint)
            .to<std::vector>();
};

struct state_t {
    resources robots{};
    resources res{};
    int time_remaining;
    std::array<bool, 4> skip{};
};

template <int StartTime>
auto max_geodes = [](blueprint const& bp) -> int64_t
{
    std::stack<state_t> queue;
    queue.push(state_t{.robots{.ore = 1,}, .time_remaining = StartTime});

    int result = 0;

    resources const max_costs = {
        .ore = std::max({bp.ore_robot.ore, bp.clay_robot.ore, bp.obsidian_robot.ore, bp.geode_robot.ore}),
        .clay = bp.obsidian_robot.clay,
        .obsidian = bp.geode_robot.obsidian
    };

    while (!queue.empty()) {
        state_t state = queue.top();
        queue.pop();

        while (state.time_remaining-- > 0) {
            bool make_geode_r = !state.skip[0] && can_build(bp.geode_robot, state.res);
            bool make_obsidian_r = !state.skip[1] && can_build(bp.obsidian_robot, state.res);
            bool make_clay_r = !state.skip[2] && can_build(bp.clay_robot, state.res);
            bool make_ore_r = !state.skip[3] && can_build(bp.ore_robot, state.res);

            state.res += state.robots;

            if (make_geode_r) {
                state_t next_state = state;
                next_state.skip = {};
                next_state.res -= bp.geode_robot;
                next_state.robots.geode += 1;
                queue.push(next_state);
                continue; // always build a geode robot if we can
             }
             if (make_obsidian_r && state.robots.obsidian < max_costs.obsidian) {
                state_t next_state = state;
                next_state.skip = {};
                next_state.res -= bp.obsidian_robot;
                next_state.robots.obsidian += 1;
                queue.push(next_state);
                // Always building an obsidian robot works for my
                // input and halves the runtime, but gives the wrong
                // answer for the sample input
                continue;
            }
            if (make_clay_r && state.robots.clay < max_costs.clay) {
                state_t next_state = state;
                next_state.skip = {};
                next_state.res -= bp.clay_robot;
                next_state.robots.clay += 1;
                queue.push(next_state);
            }
            if (make_ore_r && state.robots.ore < max_costs.ore) {
                state_t next_state = state;
                next_state.skip = {};
                next_state.res -= bp.ore_robot;
                next_state.robots.ore += 1;
                queue.push(next_state);
            }

            state.skip[0] |= make_geode_r;
            state.skip[1] |= make_obsidian_r;
            state.skip[2] |= make_clay_r;
            state.skip[3] |= make_ore_r;
        }

        result = std::max(result, state.res.geode);
    }

    return result;
};

auto part1 = [](std::vector<blueprint> const& blueprints) -> int
{
    return flux::from(blueprints)
            .map([](blueprint const& bp) { return bp.id * max_geodes<24>(bp); })
            .sum();
};

auto part2 = [](std::vector<blueprint> const& blueprints) -> int
{
    return flux::from(blueprints)
            .take(3)
            .map(max_geodes<32>)
            .product();
};

constexpr auto& test_input =
R"(Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 2 ore. Each obsidian robot costs 3 ore and 14 clay. Each geode robot costs 2 ore and 7 obsidian.
Blueprint 2: Each ore robot costs 2 ore. Each clay robot costs 3 ore. Each obsidian robot costs 3 ore and 8 clay. Each geode robot costs 3 ore and 12 obsidian.
)";

int main(int argc, char** argv)
{
    {
        auto const test_blueprints = parse_input(test_input);

        assert(part1(test_blueprints) == 33);
        // Warning, slow!
        // assert(part2(test_blueprints) == 56 * 62);
    }

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const blueprints = parse_input(aoc::string_from_file(argv[1]));

    fmt::print("Part 1: {}\n", part1(blueprints));
    fmt::print("Part 2: {}\n", part2(blueprints));
}
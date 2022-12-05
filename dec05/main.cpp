
#include "../common.hpp"

#include "../extern/ctre.hpp"

#include <stack>

// Use string as base type for stacks so we get SSO
using str_stack = std::stack<char, std::string>;

auto parse_initial_stacks = [](std::string_view input) -> std::vector<str_stack>
{
    auto lines = flux::split(input, '\n').to<std::vector<std::string>>();
    flux::inplace_reverse(lines);

    auto num_stacks = flux::filter(std::ref(lines.at(0)), [](char c) { return c != ' '; })
                            .map([](char c) { return c - '0'; })
                            .max().value();

    std::vector<str_stack> stacks(num_stacks);

    flux::from(lines).drop(1).for_each([&](std::string_view line) {
        int stack_idx = 0;
        for (std::size_t i = 0; i < line.size(); i += 4) {
            auto crate = line.substr(i, 3);

            if (crate.at(1) != ' ') {
                stacks.at(stack_idx).push(crate.at(1));
            }

            ++stack_idx;
        }
    });

    return stacks;
};

struct instruction { int n, from, to; };

auto parse_instructions = [](std::string_view input)
{
    return flux::split_string(input, '\n')
            .filter([](auto line) { return line.size() > 0; })
            .map([](auto line) {
                auto [m, n, f, t] = ctre::match<"move (\\d+) from (\\d+) to (\\d+)">(line);
                return instruction{.n = aoc::try_parse<int>(n).value(),
                                   .from = aoc::try_parse<int>(f).value() - 1,
                                   .to = aoc::try_parse<int>(t).value() - 1};
            })
            .to<std::vector>();
};

auto parse_input = [](std::string_view input)
{
    auto blank = input.find("\n\n");
    return std::pair(parse_initial_stacks(input.substr(0, blank)),
                     parse_instructions(input.substr(blank+2)));
};

auto part1 = [](std::vector<str_stack> stacks,
                std::vector<instruction> const& instructions)
{
    for (auto [n, from, to] : instructions) {
        while (n-- > 0) {
            assert(!stacks.at(from).empty());
            char c = stacks.at(from).top();
            stacks.at(from).pop();
            stacks.at(to).push(c);
        }
    }

    return flux::fold(stacks, [](auto str, auto const& stack)  -> std::string {
        assert(!stack.empty());
        return std::move(str) + stack.top();
    }, std::string{});
};

auto part2 = [](std::vector<str_stack> stacks,
                std::vector<instruction> const& instructions)
{
    for (auto [n, from, to] : instructions) {
        str_stack temp;
        while (n-- > 0) {
            assert(!stacks.at(from).empty());
            char c = stacks.at(from).top();
            stacks.at(from).pop();
            temp.push(c);
        }
        while (!temp.empty()) {
            stacks.at(to).push(temp.top());
            temp.pop();
        }
    }

    return flux::fold(stacks, [](auto str, auto const& stack)  -> std::string {
        assert(!stack.empty());
        return std::move(str) + stack.top();
    }, std::string{});
};

constexpr auto test_data =
R"(    [D]
[N] [C]
[Z] [M] [P]
 1   2   3

move 1 from 2 to 1
move 3 from 1 to 3
move 2 from 2 to 1
move 1 from 1 to 2
)";

int main(int argc, char** argv)
{
    // No constexpr tests today :(
    {
        auto const [test_stacks, test_instructions] = parse_input(test_data);
        assert(part1(test_stacks, test_instructions) == "CMZ");
        assert(part2(test_stacks, test_instructions) == "MCD");
    }

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    auto const [initial_stacks, instructions] = parse_input(input);

    fmt::print("Part 1: {}\n", part1(initial_stacks, instructions));
    fmt::print("Part 2: {}\n", part2(initial_stacks, instructions));
}
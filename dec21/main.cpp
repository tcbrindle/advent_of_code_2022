
#include "../common.hpp"

struct operation {
    std::string lhs;
    std::string rhs;
    char op;
};

struct human {};

using job = std::variant<int64_t, operation, human>;

// Using a vector of pairs here rather than std::[unordered_]map
// only so we can be constexpr and use compile-time tests
using monkey_map = std::vector<std::pair<std::string, job>>;

auto parse_job = [](std::string_view str) -> job {
    auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

    if (is_digit(str.at(0))) {
        auto n = aoc::try_parse<int>(str).value();
        return job(n);
    } else {
        operation o{
            .lhs = std::string(str.substr(0, 4)),
            .rhs = std::string(str.substr(7)),
            .op = str.at(5)
        };
        return job(std::move(o));
    }
};

auto parse_input = [](std::string_view input) -> monkey_map {
    auto vec = flux::split_string(input, "\n")
                .filter([](auto line) { return !line.empty(); })
                .map([](auto line) {
                    return std::pair{std::string(line.substr(0, 4)),
                                     parse_job(line.substr(6))};
                })
                .cache_last()
                .to<monkey_map>();

    flux::sort(vec, {}, &std::pair<std::string, job>::first);
    return vec;
};

auto lookup = [](auto& monkeys, std::string const& name) -> auto&
{
    auto iter = std::ranges::lower_bound(monkeys, name, {}, &std::pair<std::string, job>::first);

    if (iter == monkeys.end() || iter->first != name) {
        throw std::runtime_error(fmt::format("No monkey named {}", name));
    }

    return iter->second;
};

constexpr
auto evaluate_rec(monkey_map const& monkeys, std::string const& name) -> std::optional<int64_t>
{
    job const& j = lookup(monkeys, name);

    if (j.index() == 0) {
        return std::get<0>(j);
    } else if (j.index() == 2) {
        return std::nullopt;
    }

    auto const& [lhs, rhs, op] = std::get<1>(j);

    auto left = evaluate_rec(monkeys, lhs);
    auto right = evaluate_rec(monkeys, rhs);

    if (left.has_value() && right.has_value()) {
        switch (op) {
        case '+': return *left + *right;
        case '-': return *left - *right;
        case '*': return *left * *right;
        case '/': return *left / *right;
        default: throw std::runtime_error("Unknown operation");
        }
    } else {
        return std::nullopt;
    }
}

auto part1 = [](monkey_map const& monkeys) -> int64_t
{
    return evaluate_rec(monkeys, "root").value();
};

constexpr
auto solve_rec(monkey_map& monkeys, std::string const& name, int64_t target) -> int64_t
{
    job const& j = lookup(monkeys, name);

    if (j.index() == 0) {
        return std::get<0>(j);
    } else if (j.index() == 2) {
        return target;
    }

    auto const& [lhs, rhs, op] = std::get<1>(j);

    auto left = evaluate_rec(monkeys, lhs);
    auto right = evaluate_rec(monkeys, rhs);

    if (!left.has_value()) {
        assert(right.has_value());
        switch (op) {
        case '+': return solve_rec(monkeys, lhs, target - *right);
        case '-': return solve_rec(monkeys, lhs, target + *right);
        case '*': return solve_rec(monkeys, lhs, target / *right);
        case '/': return solve_rec(monkeys, lhs, target * *right);
        default: throw std::runtime_error("Unknown operation");
        }
    } else {
        assert(left.has_value());
        switch (op) {
        case '+': return solve_rec(monkeys, rhs, target - *left);
        case '-': return solve_rec(monkeys, rhs, *left - target);
        case '*': return solve_rec(monkeys, rhs, target / *left);
        case '/': return solve_rec(monkeys, rhs, *left / target);
        default: throw std::runtime_error("Unknown operation");
        }
    }
}

auto part2 = [](monkey_map monkeys) -> int64_t
{
    std::get<1>(lookup(monkeys, "root")).op = '-';
    lookup(monkeys, "humn") = human{};

    return solve_rec(monkeys, "root", 0);
};

constexpr auto test_input =
R"(root: pppw + sjmn
dbpl: 5
cczh: sllz + lgvd
zczc: 2
ptdq: humn - dvpt
dvpt: 3
lfqf: 4
humn: 5
ljgn: 2
sjmn: drzm * dbpl
sllz: 4
pppw: cczh / lfqf
lgvd: ljgn * ptdq
drzm: hmdt - zczc
hmdt: 32
)";

auto test = [] {
    auto monkeys = parse_input(test_input);
    return part1(monkeys) == 152
        && part2(monkeys) == 301;
};
static_assert(test());

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const monkeys = parse_input(aoc::string_from_file(argv[1]));

    fmt::print("Part 1: {}\n", part1(monkeys));
    fmt::print("Part 2: {}\n", part2(monkeys));
}
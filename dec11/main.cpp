
#include "../common.hpp"

#include "../extern/ctre.hpp"

template <flux::distance_t N>
auto max_n = []<flux::multipass_sequence Seq>(Seq seq)
{
    assert(flux::count(seq) >= N);

    std::array<flux::value_t<Seq>, N> max_vals;

    flux::take(std::ref(seq), N).output_to(max_vals.begin());
    flux::sort(max_vals);

    std::move(seq)
        .drop(N)
        .for_each([&max_vals](auto&& elem) {
            if (elem > max_vals.front()) {
                max_vals.front() = FLUX_FWD(elem);
                flux::sort(max_vals);
            }
        });

    return flux::from(std::move(max_vals));
};

enum class operation {
    add, multiply
};

struct monkey {
    std::vector<int64_t> items;
    operation op;
    int arg;
    int divisor;
    int true_dest;
    int false_dest;
    int64_t count = 0;
};

constexpr auto& monkey_regex =
R"(Monkey \d+:
  Starting items: ((:?\d+, )*\d+)
  Operation: new = old (\+|\*) (old|\d+)
  Test: divisible by (\d+)
    If true: throw to monkey (\d+)
    If false: throw to monkey (\d+)\n?)";

auto parse_input = [](std::string_view input) -> std::vector<monkey> {
    return flux::from(ctre::range<monkey_regex>(input))
            .map([](auto match) {
                assert(match);
                auto [m, items_, _, op, arg, div, true_, false_] = std::move(match);

                auto items = flux::map(ctre::range<R"((\d+)(?:, )?)">(items_.view()),
                                      [](auto match) -> int64_t { return match.template get<1>().to_number(); })
                                .template to<std::vector<int64_t>>();

                return monkey{
                    .items = std::move(items),
                    .op = op == "+" ? operation::add : operation::multiply,
                    .arg = arg == "old" ? -1 : arg.to_number(),
                    .divisor = div.to_number(),
                    .true_dest = true_.to_number(),
                    .false_dest = false_.to_number()
                };
            })
            .to<std::vector<monkey>>();
};

auto print_monkey = [](monkey const& m) {
    fmt::print("  Items: {}\n", fmt::join(m.items, ", "));
    fmt::print("  Operation: {} {}\n", m.op == operation::add ? '+' : '*', m.arg);
    fmt::print("  Divisor: {}\n", m.divisor);
    fmt::print("  True: {}, false: {}\n", m.true_dest, m.false_dest);
};

template <bool Part1>
auto do_round = [](std::vector<monkey>& monkeys, int64_t lcm = 0) {

    for (auto& monkey : monkeys) {
        for (int64_t i : monkey.items) {
            ++monkey.count;

            // Monkey inspects item
            switch (monkey.op) {
            case operation::add: i += (monkey.arg >= 0) ? monkey.arg : i; break;
            case operation::multiply: i *= (monkey.arg >= 0) ? monkey.arg : i; break;
            }

            // Monkey puts item down
            if constexpr (Part1) {
                i /= 3;
            } else {
                i %= lcm;
            }

            // Throw to other monkey
            int dest = (i % monkey.divisor) == 0 ? monkey.true_dest : monkey.false_dest;
            monkeys.at(dest).items.push_back(i);
        }

        monkey.items.clear();
    }

};

auto part1 = [](std::vector<monkey> monkeys) -> int64_t {

    for (int i = 0; i < 20; i++) {
        do_round<true>(monkeys);
    }

    return flux::from(monkeys)
                .map(&monkey::count)
                ._(max_n<2>)
                .product();
};

auto part2 = [](std::vector<monkey> monkeys) -> int64_t {

    int64_t lcm = flux::ref(monkeys).map(&monkey::divisor).product();

    for (int i = 0; i < 10'000; i++) {
        do_round<false>(monkeys, lcm);
    }

    return flux::from(monkeys)
                .map(&monkey::count)
                ._(max_n<2>)
                .product();
};

constexpr auto& test_input =
R"(Monkey 0:
  Starting items: 79, 98
  Operation: new = old * 19
  Test: divisible by 23
    If true: throw to monkey 2
    If false: throw to monkey 3

Monkey 1:
  Starting items: 54, 65, 75, 74
  Operation: new = old + 6
  Test: divisible by 19
    If true: throw to monkey 2
    If false: throw to monkey 0

Monkey 2:
  Starting items: 79, 60, 97
  Operation: new = old * old
  Test: divisible by 13
    If true: throw to monkey 1
    If false: throw to monkey 3

Monkey 3:
  Starting items: 74
  Operation: new = old + 3
  Test: divisible by 17
    If true: throw to monkey 0
    If false: throw to monkey 1)";

int main(int argc, char** argv)
{
    {
        auto const test_monkeys = parse_input(test_input);
        assert(part1(test_monkeys) == 10605);
        assert(part2(test_monkeys) == 2713310158);
    }

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);
    auto const monkeys = parse_input(input);

    fmt::print("Part 1: {}\n", part1(monkeys));
    fmt::print("Part 2: {}\n", part2(monkeys));
}
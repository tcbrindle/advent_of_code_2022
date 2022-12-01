
#include "../common.hpp"

template <std::size_t N>
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

auto filter_deref = []<flux::sequence Seq>(Seq seq)
{
    return std::move(seq)
            .filter([](auto&& o) { return static_cast<bool>(o); })
            .map([](auto&& o) -> decltype(auto) { return *FLUX_FWD(o); });
};

template <int N>
auto solution = [](std::string_view input)
{
    return flux::split_string(input, "\n\n")
                .map([](std::string_view group) {
                    return flux::split_string(group, '\n')
                            .map(aoc::try_parse<int>)
                            ._(filter_deref)
                            .sum();
                })
                ._(max_n<N>)
                .sum();
};

auto part1 = solution<1>;
auto part2 = solution<3>;

constexpr auto test_input =
R"(1000
2000
3000

4000

5000
6000

7000
8000
9000

10000)";

int main(int argc, char** argv)
{
    static_assert(part1(test_input) == 24000);
    static_assert(part2(test_input) == 45000);

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    fmt::print("Part 1: {}\n", part1(input));
    fmt::print("Part 2: {}\n", part2(input));
}
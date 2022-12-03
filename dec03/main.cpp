
#include <utility>

#include "../common.hpp"

// This will eventually be upstreamed as chunk<N> (and done properly)
template <flux::multipass_sequence Seq>
struct chunk3 : flux::lens_base<chunk3<Seq>> {

    Seq base_;

    constexpr explicit chunk3(Seq&& seq) : base_(std::move(seq)) {}

    struct flux_sequence_iface {
        static constexpr auto first(auto& self) { return flux::first(self.base_); }

        static constexpr auto is_last(auto& self, auto cursor) -> bool
        {
            return flux::is_last(self.base_, cursor) ||
                   flux::is_last(self.base_, flux::inc(self.base_, cursor)) ||
                   flux::is_last(self.base_, flux::inc(self.base_, cursor));
        }

        static constexpr auto read_at(auto& self, auto const& cursor) -> decltype(auto)
        {
            auto c2 = flux::next(self.base_, cursor);
            auto c3 = flux::next(self.base_, c2);

            return std::tuple<flux::element_t<Seq>, flux::element_t<Seq>, flux::element_t<Seq>>(
                flux::read_at(self.base_, cursor), flux::read_at(self.base_, c2), flux::read_at(self.base_, c3)
            );
        }

        static constexpr auto inc(auto& self, auto& cursor) -> auto&
        {
            return cursor = flux::next(self.base_, cursor, 3);
        }
    };

};


auto get_priority = [](char c) -> int
{
    if (c >= 'a' && c <= 'z') {
        return 1 + c - 'a';
    } else if (c >= 'A' && c <= 'Z') {
        return 27 + c - 'A';
    } else {
        return 0;
    }
};

auto part1 = [](std::string_view input)
{
    return flux::split_string(input, '\n')
            .filter([](auto str) { return str.size() > 0; })
            .map([](std::string_view input) {
                auto first = flux::from(input.substr(0, input.size()/2));
                auto second = flux::from(input.substr(input.size()/2));

                auto cur = second.find_if([&](char c) { return first.contains(c); });
                return second[cur];
            })
            .map(get_priority)
            .sum();
};

auto part2 = [](std::string_view input)
{
    return chunk3(flux::split(input, '\n'))
            .map([](auto tuple) {
                auto [str1, str2, str3] = tuple;
                auto r = str1.find_if([&](char c) { return str2.contains(c) && str3.contains(c); });
                return str1[r];
            })
            .map(get_priority)
            .sum();
};

constexpr auto test_data =
R"(vJrwpWtwJgWrhcsFMMfFFhFp
jqHRNqRjqzjGDLGLrsFMfFZSrLrFZsSL
PmmdzqPrVvPwwTWBwg
wMqvLMZHhHMvwLHjbvcjnnSBnvTQFn
ttgJtRGJQctTZtZT
CrZsJsPPZsGzwwsLwLmpwMDw)";

int main(int argc, char** argv)
{
    static_assert(part1(test_data) == 157);
    static_assert(part2(test_data) == 70);
    part2(test_data);

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input = aoc::string_from_file(argv[1]);

    fmt::print("Part 1: {}\n", part1(input));
    fmt::print("Part 2: {}\n", part2(input));
}
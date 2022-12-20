
#include "../common.hpp"

#include <list>

template <int Loops, int64_t Key>
auto mix = [](flux::sequence auto const& values) -> int64_t {

    auto list = flux::from(values)
                  .map([](int i) { return i * Key; })
                  .template to<std::list<int64_t>>();

    auto const sz = flux::size(values);

    using iter_t = std::list<int64_t>::const_iterator;

    std::vector<iter_t> iters;
    for (auto i = list.cbegin(); i != list.cend(); ++i) {
        iters.push_back(i);
    }

    for (int i = 0; i < Loops; i++) {
        for (iter_t& iter : iters) {
            int64_t const value = *iter;

            int64_t places = value % (sz - 1);
            if (places < 0) {
                places += (sz - 1);
            }

            if (places == 0) {
                continue;
            }

            iter_t target = iter;
            for (int64_t k = 0; k < places; k++) {
                if (++target == list.cend()) {
                    target = list.cbegin();
                }
            }

            list.erase(iter);
            iter = list.insert(std::next(target), value);
        }
    }

    // Copy the list back to a vector to find the output
    auto res_vec = std::vector<int64_t>(list.cbegin(), list.cend());
    auto zero_idx = flux::find(res_vec, 0);

    return res_vec.at((zero_idx + 1000) % sz) +
           res_vec.at((zero_idx + 2000) % sz) +
           res_vec.at((zero_idx + 3000) % sz);
};

auto part1 = mix<1, 1>;
auto part2 = mix<10, 811589153>;

constexpr auto test_input = std::array{1, 2, -3, 3, -2, 0, 4};

int main(int argc, char** argv)
{
    {
        assert(part1(test_input) == 3;
        assert(part2(test_input) == 1623178306;
    }

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const values = aoc::vector_from_file<int>(argv[1]);
    fmt::print("Part 1: {}\n", part1(values));
    fmt::print("Part 2: {}\n", part2(values));
}
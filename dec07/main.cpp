
#include "../common.hpp"

using index_t = std::size_t;

auto const parse_input = [](std::string_view input) -> std::vector<int>
{
    std::vector<int> dir_sizes;
    std::vector<index_t> path;

    flux::split_string(input, "$ ").for_each([&](std::string_view line) {
        if (line.starts_with("cd ..")) {
            assert(!path.empty());
            path.pop_back();
        } else if (line.starts_with("cd")) {
            dir_sizes.push_back(0);
            path.push_back(dir_sizes.size() - 1);
        } else if (line.starts_with("ls")) {
            auto sz = flux::split_string(line, '\n')
                        .drop(1)
                        .filter([](auto sv) { return !sv.empty() && !sv.starts_with("dir"); })
                        .map([](auto sv) {
                            auto num = sv.substr(0, sv.find(' '));
                            return aoc::try_parse<int>(num).value();
                        })
                        .sum();

            flux::for_each(path, [&](auto idx) { dir_sizes.at(idx) += sz; });
        }
    });

    return dir_sizes;
};

auto const part1 = [](std::vector<int> const& sizes)
{
    return flux::from(sizes)
             .filter([](int s) { return s <= 100'000; })
             .sum();
};

auto const part2 = [](std::vector<int> const& sizes)
{
    auto const root_size = sizes.at(0);
    auto const unused = 70'000'000 - root_size;
    auto const must_free = 30'000'000 - unused;

    return flux::from(sizes)
            .filter([&](int s) { return s >= must_free; })
            .min().value();
};

constexpr auto& test_input =
R"($ cd /
$ ls
dir a
14848514 b.txt
8504156 c.dat
dir d
$ cd a
$ ls
dir e
29116 f
2557 g
62596 h.lst
$ cd e
$ ls
584 i
$ cd ..
$ cd ..
$ cd d
$ ls
4060174 j
8033020 d.log
5626152 d.ext
7214296 k)";

auto test = [] {
    auto const sizes = parse_input(test_input);
    return part1(sizes) == 95437 &&
           part2(sizes) == 24933642;
};
static_assert(test());

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const input_str = aoc::string_from_file(argv[1]);
    auto const dir_sizes = parse_input(input_str);
    fmt::print("Part 1: {}\n", part1(dir_sizes));
    fmt::print("Part 2: {}\n", part2(dir_sizes));
}
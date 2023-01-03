
#include "../common.hpp"

#include <cassert>

struct vec2 {
    int x = 0;
    int y = 0;

    constexpr auto operator+=(vec2 rhs) -> vec2& {
        x += rhs.x; y += rhs.y;
        return *this;
    }

    friend auto operator==(vec2, vec2) -> bool = default;

    friend constexpr auto operator+(vec2 lhs, vec2 rhs) -> vec2 {
        return lhs += rhs;
    }
};

using grid_t = std::vector<std::string>;

constexpr int go_left = -1;
constexpr int go_right = -2;

auto parse_instructions = [](std::string_view input) -> std::vector<int>
{
    auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

    std::vector<int> out;

    while (!input.empty()) {
        if (input.front() == 'L') {
            input.remove_prefix(1);
            out.push_back(go_left);
        } else if (input.front() == 'R') {
            input.remove_prefix(1);
            out.push_back(go_right);
        } else if (is_digit(input.front())) {
            auto nondigit = flux::find_if_not(input, is_digit);
            int dist = aoc::try_parse<int>(input.substr(0, nondigit)).value();
            input.remove_prefix(nondigit);
            out.push_back(dist);
        } else {
            // Got something we didn't understand, probably a trailing newline
            input.remove_prefix(1);
        }
    }

    return out;
};

auto parse_input = [](std::string_view input) -> std::pair<grid_t, std::vector<int>>
{
    auto nl = input.find("\n\n");

    grid_t grid = flux::split(input.substr(0, nl), '\n').to<grid_t>();
    // Make sure each line has the same length
    auto max_len = flux::map(std::ref(grid), flux::size).max().value();
    for (auto& str : grid) {
        if (flux::size(str) < max_len) {
            str.insert(str.end(), max_len - str.size(), ' ');
        }
    }

    return std::pair(std::move(grid),
                     parse_instructions(input.substr(nl+2)));
};

enum class direction {
    right = 0,
    down = 1,
    left = 2,
    up = 3
};

auto turn_left = [](direction& d) {
    d = direction{(static_cast<int>(d) + 3) % 4};
};

auto turn_right = [](direction& d) {
    d = direction{(static_cast<int>(d) + 1) % 4};
};

auto at = [](grid_t const& grid, vec2 pos) -> char {
    return grid.at(pos.y).at(pos.x);
};

struct state_t {
    vec2 pos;
    direction dir;
};

auto next_state_p1 = [](grid_t const& grid, state_t state) -> state_t
{
    constexpr vec2 offsets[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

    auto in_bounds = [&grid](vec2 p) {
        return p.y >= 0 && p.y < (int) grid.size() && p.x >= 0 && p.x < (int) grid.at(p.y).size();
    };

    auto [pos, dir] = state;

    vec2 offset = offsets[static_cast<int>(dir)];
    pos += offset;

    if (!in_bounds(pos) || at(grid, pos) == ' ') {
        using enum direction;
        switch (dir) {
        case right: pos.x = 0;                         break;
        case down:  pos.y = 0;                         break;
        case left:  pos.x = grid.at(pos.y).size() - 1; break;
        case up:    pos.y = grid.size() - 1;           break;
        }

        while (at(grid, pos) == ' ') {
            pos += offset;
        }
    }

    return {pos, dir};
};


/*
 My layout:
  AB
  C
 DE
 F
*/
constexpr int face_sz = 50;

template <int I, int J>
auto in = [](vec2 pos) {
    return pos.x >= I * face_sz &&
           pos.x < (I + 1) * face_sz &&
           pos.y >= J * face_sz &&
           pos.y < (J + 1) * face_sz;
};

auto in_A = in<1, 0>;
auto in_B = in<2, 0>;
auto in_C = in<1, 1>;
auto in_D = in<0, 2>;
auto in_E = in<1, 2>;
auto in_F = in<0, 3>;


auto next_state_p2 = [](grid_t const&, state_t state) -> state_t
{
    constexpr vec2 offsets[] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };

    auto [old_pos, dir] = state;

    vec2 offset = offsets[static_cast<int>(dir)];
    vec2 pos = old_pos + offset;

    if (in<1, -1>(pos) && in_A(old_pos)) { // up off face A, -> left of face F
        assert(dir == direction::up);
        pos = {0, pos.x + 2 * face_sz};
        dir = direction::right;
        assert(in_F(pos));
    } else if (in<0, 0>(pos) && in_A(old_pos)) { // left off face A -> left of face D
        assert(dir == direction::left);
        pos = {0, 3 * face_sz - pos.y - 1};
        dir = direction::right;
        assert(in_D(pos));
    } else if (in<2, -1>(pos) && in_B(old_pos)) { // up off face B, -> bottom of face F
        //fmt::print("  B -> F\n");
        assert(dir == direction::up);
        pos = {pos.x - 2 * face_sz, 4 * face_sz - 1};
        dir = direction::up; // ?!
        assert(in_F(pos));
    } else if (in<3, 0>(pos) && in_B(old_pos)) { // right off face B, -> right of face E
        assert(dir == direction::right);
        pos = {2 * face_sz - 1, 3 * face_sz - pos.y - 1};
        dir = direction::left;
        assert(in_E(pos));
    } else if (in<2, 1>(pos) && in_B(old_pos)) { // down off face B, -> right of face C
        assert(dir == direction::down);
        pos = {2 * face_sz - 1, pos.x - face_sz};
        dir = direction::left;
        assert(in_C(pos));
    } else if (in<0, 1>(pos) && in_C(old_pos)) { // left off face C, -> top of face D
        assert(dir == direction::left);
        pos = {pos.y - face_sz, 2 * face_sz};
        dir = direction::down;
        assert(in_D(pos));
    } else if (in<2, 1>(pos) && in_C(old_pos)) { // right off face C -> bottom of face B
        //fmt::print("  C -> B\n");
        assert(dir == direction::right);
        pos = {pos.y + face_sz, face_sz - 1};
        dir = direction::up;
        assert(in_B(pos));
    } else if (in<0, 1>(pos) && in_D(old_pos)) { // up off face D, -> left of face C
        assert(dir == direction::up);
        pos = {face_sz, face_sz + pos.x};
        dir = direction::right;
        assert(in_C(pos));
    } else if (in<-1, 2>(pos) && in_D(old_pos)) { // left off face D -> left of face A
        assert(dir == direction::left);
        pos = {face_sz, 3 * face_sz - pos.y - 1};
        dir = direction::right;
        assert(in_A(pos));
    } else if (in<2, 2>(pos) && in_E(old_pos)) { // right off face E, -> right of face B
        assert(dir == direction::right);
        pos = {3 * face_sz - 1, 3 * face_sz - pos.y - 1};
        dir = direction::left;
        assert(in_B(pos));
    } else if (in<1, 3>(pos) && in_E(old_pos)) { // down off face E, -> right of face F
        assert(dir == direction::down);
        pos = {face_sz - 1, pos.x + 2 * face_sz}; // ???
        dir = direction::left;
        assert(in_F(pos));
    } else if (in<1, 3>(pos) && in_F(old_pos)) { // right off face F, -> bottom of face E
        assert(dir == direction::right);
        pos = {pos.y - 2 * face_sz, 3 * face_sz - 2};
        dir = direction::up;
        assert(in_E(pos));
    } else if (in<-1, 3>(pos) && in_F(old_pos)) { // left off face F, -> top of face A
        assert(dir == direction::left);
        pos = {pos.y - 2 * face_sz, 0};
        dir = direction::down;
        assert(in_A(pos));
    } else if (in<0, 4>(pos) && in_F(old_pos)) { // down off face F, -> top of face B
        assert(dir == direction::down);
        pos = {pos.x + 2 * face_sz, 0};
        dir = direction::down; // !!
        assert(in_B(pos));
    }

    return {pos, dir};
};

auto test_stuff = [] {
    constexpr grid_t g{};

    auto test_all_dirs = [&](vec2 pos) {
        next_state_p2(g, {pos, direction::up});
        next_state_p2(g, {pos, direction::right});
        next_state_p2(g, {pos, direction::down});
        next_state_p2(g, {pos, direction::left});
    };

    auto test_all_corners = [&](vec2 top_left) {
        auto [x, y] = top_left;
        constexpr auto block = face_sz - 1;
        test_all_dirs({x, y});
        test_all_dirs({x + block, y});
        test_all_dirs({x, y + block});
        test_all_dirs({x + block, y + block});
    };

    // corners of A
    test_all_corners({face_sz, 0});

    // Corners of B
    test_all_corners({2 * face_sz, 0});

    // Corners of C
    test_all_corners({face_sz, face_sz});

    // Corners of D
    test_all_corners({0, 2 * face_sz});

    // Corners of E
    test_all_corners({face_sz, 2 * face_sz});

    // Corners of F
    test_all_corners({0, 3 * face_sz});

    return true;
};
static_assert(test_stuff());

template <auto& NextFn>
auto walk = [](grid_t const& grid, std::vector<int> instructions) -> int
{
    state_t state{
        .pos = {(int) grid.at(0).find('.'), 0},
        .dir = direction::right
    };

    state = flux::fold(instructions, [&grid](state_t s, int instr) {
        switch (instr) {
        case go_left: turn_left(s.dir); break;
        case go_right: turn_right(s.dir); break;
        default:
            while (instr-- > 0) {
                state_t next = NextFn(grid, s);
                if (at(grid, next.pos) == '#') {
                    break;
                }
                assert(at(grid, next.pos) == '.');
                s = next;
            }
        }
        return s;
    }, std::move(state));

    state.pos += {1, 1};

    return 1000 * state.pos.y
           + 4 * state.pos.x
           + static_cast<int>(state.dir);
};

auto part1 = walk<next_state_p1>;

auto part2 = walk<next_state_p2>;

constexpr auto test_input =
R"(        ...#
        .#..
        #...
        ....
...#.......#
........#...
..#....#....
..........#.
        ...#....
        .....#..
        .#......
        ......#.

10R5L5R10L4R5L5
)";

int main(int argc, char** argv)
{
    {
        auto [grid, instr] = parse_input(test_input);
        fmt::print("Part 1 test: {}\n", part1(grid, instr));
    }

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    auto const [grid, instr] = parse_input(aoc::string_from_file(argv[1]));
    fmt::print("Part 1: {}\n", part1(grid, instr));
    fmt::print("Part 2: {}\n", part2(grid, instr));
}
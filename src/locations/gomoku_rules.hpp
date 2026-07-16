// 五子棋纯规则状态机：棋盘、合法落子、胜负和电脑决策均可无窗口测试。
#pragma once

#include <array>

namespace pixel_town {

enum class GomokuCell { empty, player, computer };
enum class GomokuTurn { player, computer };
enum class GomokuState { playing, player_wins, computer_wins, draw };

struct GomokuMove {
    int row;
    int col;
};

using GomokuBoard = std::array<std::array<GomokuCell, 15>, 15>;

class GomokuGame {
public:
    static constexpr int kSize = 15;

    GomokuGame();

    explicit GomokuGame(GomokuBoard board, GomokuTurn turn);

    [[nodiscard]] GomokuCell cell(int row, int col) const;
    [[nodiscard]] GomokuTurn current_turn() const;
    [[nodiscard]] GomokuState state() const;
    [[nodiscard]] bool is_legal(int row, int col) const;

    bool play(int row, int col);

    [[nodiscard]] GomokuMove computer_move() const;
    bool computer_play();

private:
    GomokuBoard board_{};
    GomokuTurn turn_{GomokuTurn::player};
    GomokuState state_{GomokuState::playing};

    [[nodiscard]] bool in_bounds(int row, int col) const;
    [[nodiscard]] bool check_win(int row, int col, GomokuCell piece) const;
    [[nodiscard]] bool is_board_full() const;

    [[nodiscard]] int count_line(int row, int col, int dr, int dc, GomokuCell piece) const;
    [[nodiscard]] bool would_win_after_placing(int row, int col, GomokuCell piece) const;
    [[nodiscard]] GomokuMove find_immediate_win(GomokuCell piece) const;
    [[nodiscard]] int score_cell(int row, int col) const;
};

}  // namespace pixel_town

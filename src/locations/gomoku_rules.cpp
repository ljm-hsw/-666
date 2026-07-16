// 五子棋局内规则；不处理赌注、金钱或酒馆页面。
#include "locations/gomoku_rules.hpp"

namespace pixel_town {

GomokuGame::GomokuGame() {
    for (auto& row : board_) {
        row.fill(GomokuCell::empty);
    }
}

GomokuGame::GomokuGame(GomokuBoard board, GomokuTurn turn)
    : board_(board), turn_(turn), state_(GomokuState::playing) {}

GomokuCell GomokuGame::cell(int row, int col) const {
    if (!in_bounds(row, col)) return GomokuCell::empty;
    return board_[row][col];
}

GomokuTurn GomokuGame::current_turn() const { return turn_; }

GomokuState GomokuGame::state() const { return state_; }

bool GomokuGame::in_bounds(int row, int col) const {
    return row >= 0 && row < kSize && col >= 0 && col < kSize;
}

bool GomokuGame::is_legal(int row, int col) const {
    if (!in_bounds(row, col)) return false;
    if (board_[row][col] != GomokuCell::empty) return false;
    if (current_turn() != GomokuTurn::player) return false;
    if (state() != GomokuState::playing) return false;
    return true;
}

bool GomokuGame::play(int row, int col) {
    if (!is_legal(row, col)) return false;

    board_[row][col] = GomokuCell::player;

    if (check_win(row, col, GomokuCell::player)) {
        state_ = GomokuState::player_wins;
        return true;
    }

    if (is_board_full()) {
        state_ = GomokuState::draw;
        return true;
    }

    turn_ = GomokuTurn::computer;
    return true;
}

int GomokuGame::count_line(int row, int col, int dr, int dc, GomokuCell piece) const {
    int count = 1;

    for (int r = row + dr, c = col + dc; in_bounds(r, c) && board_[r][c] == piece;
         r += dr, c += dc) {
        ++count;
    }

    for (int r = row - dr, c = col - dc; in_bounds(r, c) && board_[r][c] == piece;
         r -= dr, c -= dc) {
        ++count;
    }

    return count;
}

bool GomokuGame::check_win(int row, int col, GomokuCell piece) const {
    constexpr int kDirs[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    for (int d = 0; d < 4; ++d) {
        if (count_line(row, col, kDirs[d][0], kDirs[d][1], piece) >= 5) return true;
    }
    return false;
}

bool GomokuGame::is_board_full() const {
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            if (board_[r][c] == GomokuCell::empty) return false;
        }
    }
    return true;
}

bool GomokuGame::would_win_after_placing(int row, int col, GomokuCell piece) const {
    constexpr int kDirs[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    for (int d = 0; d < 4; ++d) {
        if (count_line(row, col, kDirs[d][0], kDirs[d][1], piece) >= 5) return true;
    }
    return false;
}

GomokuMove GomokuGame::find_immediate_win(GomokuCell piece) const {
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            if (board_[r][c] == GomokuCell::empty && would_win_after_placing(r, c, piece)) {
                return {r, c};
            }
        }
    }
    return {-1, -1};
}

int GomokuGame::score_cell(int row, int col) const {
    constexpr int kWeights[] = {0, 10, 100, 1000, 10000, 100000};
    constexpr int kDirs[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    int total = 0;
    for (int d = 0; d < 4; ++d) {
        int comp_line = count_line(row, col, kDirs[d][0], kDirs[d][1], GomokuCell::computer);
        int player_line = count_line(row, col, kDirs[d][0], kDirs[d][1], GomokuCell::player);
        total += kWeights[comp_line] * 2 + kWeights[player_line];
    }
    return total;
}

GomokuMove GomokuGame::computer_move() const {
    if (current_turn() != GomokuTurn::computer) return {-1, -1};
    if (state() != GomokuState::playing) return {-1, -1};

    GomokuMove win_move = find_immediate_win(GomokuCell::computer);
    if (win_move.row != -1) return win_move;

    GomokuMove block_move = find_immediate_win(GomokuCell::player);
    if (block_move.row != -1) return block_move;

    GomokuMove best{-1, -1};
    int best_score = -1;
    int best_dist2 = -1;

    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            if (board_[r][c] != GomokuCell::empty) continue;

            if (best.row == -1) {
                best = {r, c};
                best_score = score_cell(r, c);
                best_dist2 = (r - 7) * (r - 7) + (c - 7) * (c - 7);
                continue;
            }

            const int s = score_cell(r, c);
            const int d2 = (r - 7) * (r - 7) + (c - 7) * (c - 7);

            bool replace = false;
            if (s > best_score) {
                replace = true;
            } else if (s == best_score) {
                if (d2 < best_dist2) {
                    replace = true;
                } else if (d2 == best_dist2) {
                    if (r < best.row) {
                        replace = true;
                    } else if (r == best.row && c < best.col) {
                        replace = true;
                    }
                }
            }

            if (replace) {
                best = {r, c};
                best_score = s;
                best_dist2 = d2;
            }
        }
    }

    return best;
}

bool GomokuGame::computer_play() {
    GomokuMove move = computer_move();
    if (move.row == -1) return false;

    board_[move.row][move.col] = GomokuCell::computer;

    if (check_win(move.row, move.col, GomokuCell::computer)) {
        state_ = GomokuState::computer_wins;
        return true;
    }

    if (is_board_full()) {
        state_ = GomokuState::draw;
        return true;
    }

    turn_ = GomokuTurn::player;
    return true;
}

}  // namespace pixel_town

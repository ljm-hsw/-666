#include <doctest/doctest.h>

#include "core/game_session.hpp"
#include "locations/gomoku_rules.hpp"
#include "core/tavern_rules.hpp"
#include "test_game_session_helpers.hpp"

namespace pixel_town {
namespace {

GomokuBoard empty_board() {
    GomokuBoard board{};
    for (auto& row : board) {
        row.fill(GomokuCell::empty);
    }
    return board;
}

TEST_CASE("legal move succeeds and switches turn") {
    GomokuGame game;
    CHECK(game.current_turn() == GomokuTurn::player);
    CHECK(game.state() == GomokuState::playing);

    bool ok = game.play(7, 7);
    CHECK(ok);
    CHECK(game.cell(7, 7) == GomokuCell::player);
    CHECK(game.current_turn() == GomokuTurn::computer);
    CHECK(game.state() == GomokuState::playing);
}

TEST_CASE("out of bounds move rejected and board unchanged") {
    GomokuGame game;

    CHECK_FALSE(game.play(-1, 0));
    CHECK_FALSE(game.play(0, -1));
    CHECK_FALSE(game.play(15, 0));
    CHECK_FALSE(game.play(0, 15));

    for (int r = 0; r < GomokuGame::kSize; ++r) {
        for (int c = 0; c < GomokuGame::kSize; ++c) {
            CHECK(game.cell(r, c) == GomokuCell::empty);
        }
    }
    CHECK(game.current_turn() == GomokuTurn::player);

    CHECK(game.cell(-1, 0) == GomokuCell::empty);
    CHECK(game.cell(0, 15) == GomokuCell::empty);
    CHECK(game.cell(20, 20) == GomokuCell::empty);
}

TEST_CASE("occupied cell move rejected") {
    GomokuGame game;
    REQUIRE(game.play(7, 7));
    CHECK_FALSE(game.is_legal(7, 7));
    CHECK_FALSE(game.play(7, 7));
    CHECK(game.cell(7, 7) == GomokuCell::player);
}

TEST_CASE("wrong turn move rejected and board unchanged") {
    GomokuGame game;
    REQUIRE(game.play(7, 7));
    CHECK(game.current_turn() == GomokuTurn::computer);

    CHECK_FALSE(game.is_legal(7, 8));
    CHECK_FALSE(game.play(7, 8));
    CHECK(game.cell(7, 8) == GomokuCell::empty);
    CHECK(game.current_turn() == GomokuTurn::computer);
}

TEST_CASE("move after game over rejected") {
    auto board = empty_board();
    board[0][0] = GomokuCell::player;
    board[0][1] = GomokuCell::player;
    board[0][2] = GomokuCell::player;
    board[0][3] = GomokuCell::player;
    GomokuGame game(board, GomokuTurn::player);
    REQUIRE(game.play(0, 4));
    CHECK(game.state() == GomokuState::player_wins);

    CHECK_FALSE(game.is_legal(7, 7));
    CHECK_FALSE(game.play(7, 7));
}

TEST_CASE("horizontal five-in-a-row wins") {
    auto board = empty_board();
    board[3][5] = GomokuCell::player;
    board[3][6] = GomokuCell::player;
    board[3][7] = GomokuCell::player;
    board[3][8] = GomokuCell::player;
    GomokuGame game(board, GomokuTurn::player);

    bool ok = game.play(3, 9);
    CHECK(ok);
    CHECK(game.state() == GomokuState::player_wins);
    CHECK(game.current_turn() == GomokuTurn::player);
}

TEST_CASE("vertical five-in-a-row wins for computer") {
    auto board = empty_board();
    board[4][6] = GomokuCell::computer;
    board[5][6] = GomokuCell::computer;
    board[6][6] = GomokuCell::computer;
    board[7][6] = GomokuCell::computer;
    GomokuGame game(board, GomokuTurn::computer);

    bool ok = game.computer_play();
    CHECK(ok);
    CHECK(game.state() == GomokuState::computer_wins);
    CHECK(game.cell(3, 6) == GomokuCell::computer);
}

TEST_CASE("diagonal-backslash five-in-a-row wins") {
    auto board = empty_board();
    board[2][2] = GomokuCell::player;
    board[3][3] = GomokuCell::player;
    board[4][4] = GomokuCell::player;
    board[5][5] = GomokuCell::player;
    GomokuGame game(board, GomokuTurn::player);

    bool ok = game.play(6, 6);
    CHECK(ok);
    CHECK(game.state() == GomokuState::player_wins);
}

TEST_CASE("diagonal-slash five-in-a-row wins") {
    auto board = empty_board();
    board[2][8]  = GomokuCell::player;
    board[3][7]  = GomokuCell::player;
    board[4][6]  = GomokuCell::player;
    board[5][5]  = GomokuCell::player;
    GomokuGame game(board, GomokuTurn::player);

    bool ok = game.play(6, 4);
    CHECK(ok);
    CHECK(game.state() == GomokuState::player_wins);
}

TEST_CASE("draw when board fills with no five-in-a-row") {
    GomokuBoard board{};
    for (int r = 0; r < GomokuGame::kSize; ++r) {
        for (int c = 0; c < GomokuGame::kSize; ++c) {
            int group = (r % 4 < 2) ? 0 : 1;
            board[r][c] = (c % 2 == group) ? GomokuCell::player : GomokuCell::computer;
        }
    }
    board[7][7] = GomokuCell::empty;

    GomokuGame game(board, GomokuTurn::player);
    REQUIRE(game.state() == GomokuState::playing);

    bool ok = game.play(7, 7);
    CHECK(ok);
    CHECK(game.state() == GomokuState::draw);
}

TEST_CASE("computer takes immediate winning move") {
    auto board = empty_board();
    board[10][2] = GomokuCell::computer;
    board[10][3] = GomokuCell::computer;
    board[10][4] = GomokuCell::computer;
    board[10][5] = GomokuCell::computer;
    GomokuGame game(board, GomokuTurn::computer);

    GomokuMove move = game.computer_move();
    CHECK(move.row != -1);
    CHECK(move.row == 10);
    CHECK((move.col == 1 || move.col == 6));
    CHECK(game.cell(move.row, move.col) == GomokuCell::empty);

    bool ok = game.computer_play();
    CHECK(ok);
    CHECK(game.state() == GomokuState::computer_wins);
}

TEST_CASE("computer blocks player immediate win") {
    auto board = empty_board();
    board[1][10] = GomokuCell::player;
    board[1][11] = GomokuCell::player;
    board[1][12] = GomokuCell::player;
    board[1][13] = GomokuCell::player;
    GomokuGame game(board, GomokuTurn::computer);

    GomokuMove move = game.computer_move();
    CHECK(move.row == 1);
    CHECK((move.col == 9 || move.col == 14));
    CHECK(game.cell(move.row, move.col) == GomokuCell::empty);
}

TEST_CASE("same board state produces same computer move") {
    auto board = empty_board();
    board[3][4] = GomokuCell::player;
    board[3][5] = GomokuCell::computer;
    board[4][4] = GomokuCell::computer;
    GomokuGame game1(board, GomokuTurn::computer);
    GomokuGame game2(board, GomokuTurn::computer);

    GomokuMove move1 = game1.computer_move();
    GomokuMove move2 = game2.computer_move();

    CHECK(move1.row == move2.row);
    CHECK(move1.col == move2.col);
}

TEST_CASE("computer_move returns sentinel when not computer turn") {
    GomokuGame game;
    CHECK(game.current_turn() == GomokuTurn::player);

    GomokuMove move = game.computer_move();
    CHECK(move.row == -1);
    CHECK(move.col == -1);
}

TEST_CASE("computer_move returns sentinel when game over") {
    auto board = empty_board();
    board[0][0] = GomokuCell::player;
    board[0][1] = GomokuCell::player;
    board[0][2] = GomokuCell::player;
    board[0][3] = GomokuCell::player;
    GomokuGame game(board, GomokuTurn::player);
    REQUIRE(game.play(0, 4));
    CHECK(game.state() == GomokuState::player_wins);

    GomokuMove move = game.computer_move();
    CHECK(move.row == -1);
    CHECK(move.col == -1);
}

TEST_CASE("computer_play returns false when not computer turn") {
    GomokuGame game;
    CHECK_FALSE(game.computer_play());
}

TEST_CASE("computer_play returns false when game over") {
    auto board = empty_board();
    board[0][0] = GomokuCell::player;
    board[0][1] = GomokuCell::player;
    board[0][2] = GomokuCell::player;
    board[0][3] = GomokuCell::player;
    GomokuGame game(board, GomokuTurn::player);
    REQUIRE(game.play(0, 4));

    CHECK_FALSE(game.computer_play());
}

TEST_CASE("completed gomoku game settles the active tavern location once") {
    GameSession session = GameSession::new_game(13);
    REQUIRE(session.enter_location(Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(test_support::completed_location_result(session)).accepted);
    REQUIRE(session.enter_location(Location::tavern));
    const int result_id = session.start_location();
    REQUIRE(result_id != 0);

    auto board = empty_board();
    board[7][3] = GomokuCell::player;
    board[7][4] = GomokuCell::player;
    board[7][5] = GomokuCell::player;
    board[7][6] = GomokuCell::player;
    GomokuGame game(board, GomokuTurn::player);
    REQUIRE(game.play(7, 7));
    REQUIRE(game.state() == GomokuState::player_wins);

    const TavernChallengeConfig config;
    const auto result = simulate_tavern_challenge(
        session.player(), config, ChallengeType::gomoku, BetTier::low,
        ChallengeOutcome::win, result_id);
    REQUIRE(session.apply_action_result(result).accepted);

    CHECK(session.phase() == GamePhase::day_summary);
    CHECK(session.tavern_wins() == 1);
    CHECK_FALSE(session.apply_action_result(result).accepted);
}

}  // namespace
}  // namespace pixel_town


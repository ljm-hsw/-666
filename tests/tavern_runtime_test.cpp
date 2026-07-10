#include <doctest/doctest.h>

#include <algorithm>

#include "app/tavern_runtime.hpp"
#include "app/tavern_layout.hpp"
#include "test_game_session_helpers.hpp"
#include "ui/ui_metrics.hpp"

namespace {

pixel_town::GameSession session_at_night_choice() {
    auto session = pixel_town::GameSession::new_game(20260710U);
    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(
                pixel_town::test_support::completed_location_result(session))
                .accepted);
    return session;
}

pixel_town::GameSession different_active_tavern_session() {
    auto session = pixel_town::GameSession::new_game(20260711U);
    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    REQUIRE(session.start_location() != 0);
    auto first_day = pixel_town::test_support::completed_location_result(session);
    first_day.delta.money = -45;
    REQUIRE(session.apply_action_result(first_day).accepted);
    REQUIRE(session.apply_action_result(session.home_rest_result()).accepted);
    REQUIRE(session.finish_day_summary());
    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(
                pixel_town::test_support::completed_location_result(session))
                .accepted);
    REQUIRE(session.enter_location(pixel_town::Location::tavern));
    REQUIRE(session.start_location() != 0);
    return session;
}

pixel_town::TavernFrameInput key_input(int digit = 0, bool space = false) {
    pixel_town::TavernFrameInput input;
    input.digit_pressed = digit;
    input.space_pressed = space;
    return input;
}

pixel_town::TavernFrameInput click_input(pixel_town::TavernRect bounds) {
    pixel_town::TavernFrameInput input;
    input.pointer = pixel_town::TavernCanvasPoint{
        (bounds.x + bounds.width * 0.5F) *
            pixel_town::ui::design_to_canvas_scale,
        (bounds.y + bounds.height * 0.5F) *
            pixel_town::ui::design_to_canvas_scale,
        true};
    input.primary_pressed = true;
    return input;
}

void start_gomoku(pixel_town::GameSession& session,
                  pixel_town::TavernRuntime& runtime) {
    REQUIRE(runtime.open(session).status == pixel_town::TavernOpenStatus::opened);
    REQUIRE(runtime.step(session, key_input(0, true)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(1)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(3)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(0, true)).status ==
            pixel_town::TavernStepStatus::changed);
}

}  // namespace

TEST_CASE("tavern runtime enters and starts one challenge through its public interface") {
    auto session = session_at_night_choice();
    pixel_town::TavernRuntime runtime;

    const auto opened = runtime.open(session);
    REQUIRE(opened.status == pixel_town::TavernOpenStatus::opened);
    CHECK(runtime.active());
    CHECK(session.phase() == pixel_town::GamePhase::night_location);
    CHECK(runtime.presentation().screen == pixel_town::TavernScreen::lobby);

    pixel_town::TavernFrameInput input;
    input.space_pressed = true;
    REQUIRE(runtime.step(session, input).status == pixel_town::TavernStepStatus::changed);
    CHECK(runtime.presentation().screen == pixel_town::TavernScreen::challenge_select);

    input = {};
    input.digit_pressed = 1;
    REQUIRE(runtime.step(session, input).status == pixel_town::TavernStepStatus::changed);
    input = {};
    input.digit_pressed = 3;
    REQUIRE(runtime.step(session, input).status == pixel_town::TavernStepStatus::changed);
    input = {};
    input.space_pressed = true;
    REQUIRE(runtime.step(session, input).status == pixel_town::TavernStepStatus::changed);

    const auto view = runtime.presentation();
    CHECK(view.screen == pixel_town::TavernScreen::gomoku);
    CHECK(view.selected_challenge == pixel_town::ChallengeType::gomoku);
    CHECK(view.selected_bet == pixel_town::BetTier::low);
    CHECK(view.gomoku.has_value());
    CHECK_FALSE(view.liars_dice.has_value());
    CHECK(session.location_started());
    CHECK(session.active_result_id() > 0);
}

TEST_CASE("tavern runtime returns to the night map before a challenge starts") {
    auto session = session_at_night_choice();
    pixel_town::TavernRuntime runtime;
    REQUIRE(runtime.open(session).status == pixel_town::TavernOpenStatus::opened);

    pixel_town::TavernFrameInput input;
    input.escape_pressed = true;
    const auto result = runtime.step(session, input);

    CHECK(result.status == pixel_town::TavernStepStatus::returned_to_map);
    CHECK_FALSE(runtime.active());
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.can_enter(pixel_town::Location::tavern).allowed);
}

TEST_CASE("tavern runtime keeps selection open when the bet is unaffordable") {
    auto session = session_at_night_choice();
    auto snapshot = session.snapshot();
    snapshot.player.money = 5;
    session = pixel_town::GameSession::from_snapshot(snapshot);
    pixel_town::TavernRuntime runtime;
    REQUIRE(runtime.open(session).status == pixel_town::TavernOpenStatus::opened);
    REQUIRE(runtime.step(session, key_input(0, true)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(5)).status ==
            pixel_town::TavernStepStatus::changed);

    const auto result = runtime.step(session, key_input(0, true));

    CHECK(result.status == pixel_town::TavernStepStatus::rejected);
    CHECK(runtime.active());
    CHECK(runtime.presentation().screen ==
          pixel_town::TavernScreen::challenge_select);
    CHECK_FALSE(session.location_started());
}

TEST_CASE("tavern runtime preserves a terminal challenge when core settlement rejects it") {
    auto session = session_at_night_choice();
    pixel_town::TavernRuntime runtime;
    start_gomoku(session, runtime);

    const auto layout = pixel_town::tavern_layout();
    int safety = 0;
    while (runtime.presentation().gomoku->state == pixel_town::GomokuState::playing &&
           safety++ < 300) {
        const auto view = runtime.presentation();
        pixel_town::TavernFrameInput input;
        if (view.gomoku->turn == pixel_town::GomokuTurn::computer) {
            input.elapsed_seconds = 1.0F;
        } else {
            bool found = false;
            for (int row = 0; row < pixel_town::GomokuGame::kSize && !found; ++row) {
                for (int col = 0; col < pixel_town::GomokuGame::kSize; ++col) {
                    if (view.gomoku->board[row][col] != pixel_town::GomokuCell::empty) {
                        continue;
                    }
                    input.pointer = pixel_town::TavernCanvasPoint{
                        (layout.gomoku_board_x +
                         static_cast<float>(col) * layout.gomoku_cell_size) *
                            pixel_town::ui::design_to_canvas_scale,
                        (layout.gomoku_board_y +
                         static_cast<float>(row) * layout.gomoku_cell_size) *
                            pixel_town::ui::design_to_canvas_scale,
                        true};
                    input.primary_pressed = true;
                    found = true;
                    break;
                }
            }
            REQUIRE(found);
        }
        (void)runtime.step(session, input);
    }

    REQUIRE(safety < 300);
    const auto terminal = runtime.presentation();
    REQUIRE(terminal.gomoku.has_value());
    REQUIRE(terminal.gomoku->state != pixel_town::GomokuState::playing);

    auto rejecting_session = different_active_tavern_session();
    REQUIRE(rejecting_session.active_result_id() != session.active_result_id());
    pixel_town::TavernFrameInput confirm;
    confirm.enter_pressed = true;
    const auto result = runtime.step(rejecting_session, confirm);

    CHECK(result.status == pixel_town::TavernStepStatus::rejected);
    CHECK(runtime.active());
    const auto after_rejection = runtime.presentation();
    CHECK(after_rejection.screen == pixel_town::TavernScreen::gomoku);
    REQUIRE(after_rejection.gomoku.has_value());
    CHECK(after_rejection.gomoku->state == terminal.gomoku->state);
    CHECK(after_rejection.feedback.find("行动结果不属于当前地点会话") !=
          std::string::npos);

    const auto retry = runtime.step(session, confirm);
    CHECK(retry.status == pixel_town::TavernStepStatus::settled);
    CHECK_FALSE(runtime.active());
    CHECK(session.phase() == pixel_town::GamePhase::day_summary);
}

TEST_CASE("tavern presentation never exposes unrevealed computer dice") {
    auto session = session_at_night_choice();
    pixel_town::TavernRuntime runtime;
    REQUIRE(runtime.open(session).status == pixel_town::TavernOpenStatus::opened);
    REQUIRE(runtime.step(session, key_input(0, true)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(2)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(3)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(0, true)).status ==
            pixel_town::TavernStepStatus::changed);

    const auto view = runtime.presentation();
    REQUIRE(view.liars_dice.has_value());
    CHECK_FALSE(view.gomoku.has_value());
    for (int index = 0; index < pixel_town::LiarsDiceGame::kDiceCount; ++index) {
        CHECK(view.liars_dice->player_dice[index].visible_face.has_value());
        CHECK_FALSE(
            view.liars_dice->computer_dice[index].visible_face.has_value());
    }
}

TEST_CASE("tavern runtime completes a deterministic liars dice night") {
    auto session = session_at_night_choice();
    pixel_town::TavernRuntime runtime;
    REQUIRE(runtime.open(session).status == pixel_town::TavernOpenStatus::opened);
    REQUIRE(runtime.step(session, key_input(0, true)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(2)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(3)).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.step(session, key_input(0, true)).status ==
            pixel_town::TavernStepStatus::changed);

    const auto layout = pixel_town::tavern_layout();
    int safety = 0;
    while (runtime.active() && safety++ < 100) {
        const auto view = runtime.presentation();
        REQUIRE(view.liars_dice.has_value());
        const auto& dice = *view.liars_dice;
        if (dice.round_over) {
            const int visible_player_dice = static_cast<int>(std::count_if(
                dice.player_dice.begin(), dice.player_dice.end(),
                [](const auto& die) { return die.active; }));
            const int visible_computer_dice = static_cast<int>(std::count_if(
                dice.computer_dice.begin(), dice.computer_dice.end(),
                [](const auto& die) { return die.active; }));
            CHECK(visible_player_dice ==
                  dice.player_dice_count +
                      (dice.round_loser == pixel_town::LiarsDiceParticipant::player
                           ? 1
                           : 0));
            CHECK(visible_computer_dice ==
                  dice.computer_dice_count +
                      (dice.round_loser == pixel_town::LiarsDiceParticipant::computer
                           ? 1
                           : 0));
            pixel_town::TavernFrameInput confirm;
            confirm.enter_pressed = true;
            (void)runtime.step(session, confirm);
        } else if (!dice.player_turn) {
            pixel_town::TavernFrameInput tick;
            tick.elapsed_seconds = 1.0F;
            (void)runtime.step(session, tick);
        } else if (dice.current_bid_count > 0) {
            (void)runtime.step(session, click_input(layout.challenge_button));
        } else {
            (void)runtime.step(session, click_input(layout.bid_button));
        }
    }

    REQUIRE(safety < 100);
    CHECK_FALSE(runtime.active());
    CHECK(session.phase() == pixel_town::GamePhase::day_summary);
    CHECK(session.tavern_wins() + session.tavern_losses() == 1);
}

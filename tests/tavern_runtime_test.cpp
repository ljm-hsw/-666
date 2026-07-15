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

pixel_town::GameSession returning_tavern_session(int day) {
    auto snapshot = pixel_town::GameSession::new_game(20260715U).snapshot();
    snapshot.day = day;
    snapshot.phase = pixel_town::GamePhase::night_choice;
    snapshot.day_action_done = true;
    snapshot.location_visits.tavern = 1;
    return pixel_town::GameSession::from_snapshot(snapshot);
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

void dismiss_opening_story(pixel_town::GameSession& session,
                           pixel_town::TavernRuntime& runtime) {
    REQUIRE(runtime.presentation().screen == pixel_town::TavernScreen::npc_dialog);
    pixel_town::TavernFrameInput skip;
    skip.escape_pressed = true;
    REQUIRE(runtime.step(session, skip).status ==
            pixel_town::TavernStepStatus::changed);
    REQUIRE(runtime.presentation().screen == pixel_town::TavernScreen::lobby);
}

void open_tavern(pixel_town::GameSession& session,
                 pixel_town::TavernRuntime& runtime) {
    REQUIRE(runtime.open(session).status == pixel_town::TavernOpenStatus::opened);
    dismiss_opening_story(session, runtime);
}

void start_gomoku(pixel_town::GameSession& session,
                  pixel_town::TavernRuntime& runtime) {
    open_tavern(session, runtime);
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

TEST_CASE("tavern opens its selected story before table hotspots") {
    auto session = session_at_night_choice();
    auto expected = session;
    REQUIRE(expected.enter_location(pixel_town::Location::tavern));
    pixel_town::TavernRuntime runtime;

    REQUIRE(runtime.open(session).status == pixel_town::TavernOpenStatus::opened);
    auto view = runtime.presentation();
    CHECK(view.screen == pixel_town::TavernScreen::npc_dialog);
    REQUIRE(view.dialogue.has_value());
    CHECK(view.dialogue->speaker == "酒保");
    CHECK(view.dialogue->total_lines == 3);
    CHECK(session.snapshot() == expected.snapshot());

    const auto blocked = runtime.step(
        session, click_input(pixel_town::tavern_layout().gomoku_hotspot));
    CHECK(blocked.status == pixel_town::TavernStepStatus::unchanged);
    CHECK(runtime.presentation().screen == pixel_town::TavernScreen::npc_dialog);
    CHECK_FALSE(session.location_started());

    pixel_town::TavernFrameInput skip;
    skip.escape_pressed = true;
    CHECK(runtime.step(session, skip).status == pixel_town::TavernStepStatus::changed);
    CHECK(runtime.presentation().screen == pixel_town::TavernScreen::lobby);
    CHECK(session.snapshot() == expected.snapshot());

    CHECK(runtime.step(session, key_input(0, true)).status ==
          pixel_town::TavernStepStatus::changed);
    CHECK(runtime.presentation().screen ==
          pixel_town::TavernScreen::challenge_select);
    CHECK_FALSE(session.location_started());
}

TEST_CASE("returning tavern replays the same day event before challenge selection") {
    auto session = returning_tavern_session(6);
    const auto boundary = session.snapshot();
    pixel_town::TavernRuntime runtime;

    REQUIRE(runtime.open(session).status == pixel_town::TavernOpenStatus::opened);
    const auto view = runtime.presentation();
    REQUIRE(view.dialogue.has_value());
    CHECK(view.dialogue->text.find("黑棋子") != std::string::npos);
    CHECK(session.location_visit_count(pixel_town::Location::tavern) == 1);
    CHECK_FALSE(session.location_started());

    auto restored = pixel_town::GameSession::from_snapshot(boundary);
    pixel_town::TavernRuntime restored_runtime;
    REQUIRE(restored_runtime.open(restored).status ==
            pixel_town::TavernOpenStatus::opened);
    REQUIRE(restored_runtime.presentation().dialogue.has_value());
    CHECK(restored_runtime.presentation().dialogue->text == view.dialogue->text);

    dismiss_opening_story(session, runtime);
    CHECK(runtime.step(session, click_input(pixel_town::tavern_layout().dice_hotspot))
              .status == pixel_town::TavernStepStatus::changed);
    CHECK(runtime.presentation().screen ==
          pixel_town::TavernScreen::challenge_select);
    CHECK_FALSE(session.location_started());
    CHECK(session.location_visit_count(pixel_town::Location::tavern) == 1);
}

TEST_CASE("tavern runtime enters and starts one challenge through its public interface") {
    auto session = session_at_night_choice();
    pixel_town::TavernRuntime runtime;

    open_tavern(session, runtime);
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
    open_tavern(session, runtime);

    pixel_town::TavernFrameInput input;
    input.escape_pressed = true;
    const auto result = runtime.step(session, input);

    CHECK(result.status == pixel_town::TavernStepStatus::returned_to_map);
    CHECK_FALSE(runtime.active());
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.can_enter(pixel_town::Location::tavern).allowed);
}

TEST_CASE("bartender dialogue is modal and leaves the game session unchanged") {
    auto session = session_at_night_choice();
    pixel_town::TavernRuntime runtime;
    open_tavern(session, runtime);
    const auto before_dialogue = session.snapshot();
    const auto layout = pixel_town::tavern_layout();

    REQUIRE(runtime.step(session, click_input(layout.npc_hotspot)).status ==
            pixel_town::TavernStepStatus::changed);
    auto view = runtime.presentation();
    CHECK(view.screen == pixel_town::TavernScreen::npc_dialog);
    REQUIRE(view.dialogue.has_value());
    CHECK(view.dialogue->speaker == "酒保");
    CHECK(view.dialogue->current_line == 1);
    CHECK(view.dialogue->total_lines == 3);
    const float animation_before = view.bartender_animation_seconds;

    pixel_town::TavernFrameInput advance;
    advance.enter_pressed = true;
    advance.space_pressed = true;
    REQUIRE(runtime.step(session, advance).status ==
            pixel_town::TavernStepStatus::changed);
    CHECK(runtime.presentation().dialogue->current_line == 2);

    auto blocked_click = click_input(layout.gomoku_hotspot);
    blocked_click.elapsed_seconds = 5.0F;
    CHECK(runtime.step(session, blocked_click).status ==
          pixel_town::TavernStepStatus::unchanged);
    view = runtime.presentation();
    REQUIRE(view.dialogue.has_value());
    CHECK(view.dialogue->current_line == 2);
    CHECK(view.bartender_animation_seconds == doctest::Approx(animation_before));

    REQUIRE(runtime.step(session, advance).status ==
            pixel_town::TavernStepStatus::changed);
    CHECK(runtime.presentation().dialogue->current_line == 3);
    REQUIRE(runtime.step(session, advance).status ==
            pixel_town::TavernStepStatus::changed);
    CHECK(runtime.presentation().screen == pixel_town::TavernScreen::lobby);
    CHECK_FALSE(runtime.presentation().dialogue.has_value());
    CHECK(session.snapshot() == before_dialogue);
}

TEST_CASE("tavern runtime keeps selection open when the bet is unaffordable") {
    auto session = session_at_night_choice();
    auto snapshot = session.snapshot();
    snapshot.player.money = 5;
    session = pixel_town::GameSession::from_snapshot(snapshot);
    pixel_town::TavernRuntime runtime;
    open_tavern(session, runtime);
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
    open_tavern(session, runtime);
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
    open_tavern(session, runtime);
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

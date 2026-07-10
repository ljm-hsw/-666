#include <doctest/doctest.h>

#include "locations/tavern_challenge_settlement.hpp"
#include "locations/gomoku_rules.hpp"
#include "locations/liars_dice_rules.hpp"
#include "test_game_session_helpers.hpp"

namespace {

pixel_town::GomokuGame terminal_player_win() {
    pixel_town::GomokuBoard board{};
    board[7][3] = pixel_town::GomokuCell::player;
    board[7][4] = pixel_town::GomokuCell::player;
    board[7][5] = pixel_town::GomokuCell::player;
    board[7][6] = pixel_town::GomokuCell::player;
    pixel_town::GomokuGame game{board, pixel_town::GomokuTurn::player};
    REQUIRE(game.play(7, 7));
    REQUIRE(game.state() == pixel_town::GomokuState::player_wins);
    return game;
}

}  // namespace

TEST_CASE("a tavern settlement refuses a challenge that is not terminal") {
    const pixel_town::TavernChallengeSettlement settlement{
        pixel_town::TavernChallengeConfig{}};
    const pixel_town::GomokuGame playing_game;

    const auto build = settlement.build(
        playing_game, pixel_town::PlayerState{}, pixel_town::BetTier::low, 1);

    CHECK_FALSE(build.accepted());
    CHECK(build.error == pixel_town::TavernSettlementError::not_terminal);
    CHECK(build.result.result_id == 0);

    const pixel_town::LiarsDiceGame playing_dice{20260710U};
    const auto dice_build = settlement.build(
        playing_dice, pixel_town::PlayerState{}, pixel_town::BetTier::low, 2);
    CHECK_FALSE(dice_build.accepted());
    CHECK(dice_build.error == pixel_town::TavernSettlementError::not_terminal);
    CHECK(dice_build.result.result_id == 0);
}

TEST_CASE("a tavern settlement builds one applicable night result") {
    auto session = pixel_town::GameSession::new_game(20260710U);
    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(
                pixel_town::test_support::completed_location_result(session))
                .accepted);
    REQUIRE(session.enter_location(pixel_town::Location::tavern));
    const int result_id = session.start_location();
    REQUIRE(result_id != 0);

    const pixel_town::TavernChallengeSettlement settlement{
        pixel_town::TavernChallengeConfig{}};
    const auto build = settlement.build(
        terminal_player_win(), session.player(), pixel_town::BetTier::low,
        result_id);

    REQUIRE(build.accepted());
    CHECK(build.result.result_id == result_id);
    CHECK(build.result.slot == pixel_town::ActionSlot::night);
    CHECK(build.result.location == pixel_town::Location::tavern);
    CHECK(build.result.outcome == pixel_town::ActionOutcome::completed);
    CHECK(build.result.tavern_win_delta == 1);
    CHECK(build.result.tavern_loss_delta == 0);
    CHECK(session.phase() == pixel_town::GamePhase::night_location);
    CHECK(session.tavern_wins() == 0);

    REQUIRE(session.apply_action_result(build.result).accepted);
    CHECK(session.phase() == pixel_town::GamePhase::day_summary);
    CHECK(session.tavern_wins() == 1);
    CHECK_FALSE(session.apply_action_result(build.result).accepted);
}

TEST_CASE("a tavern settlement rejects an unaffordable bet without a result") {
    pixel_town::PlayerState player;
    player.money = 9;
    const pixel_town::TavernChallengeSettlement settlement{
        pixel_town::TavernChallengeConfig{}};

    const auto build = settlement.build(
        terminal_player_win(), player, pixel_town::BetTier::low, 7);

    CHECK_FALSE(build.accepted());
    CHECK(build.error == pixel_town::TavernSettlementError::insufficient_funds);
    CHECK(build.result.result_id == 0);
    CHECK(build.message.find("金钱不足") != std::string::npos);
}

TEST_CASE("a tavern settlement rejects a non-positive result id") {
    const pixel_town::TavernChallengeSettlement settlement{
        pixel_town::TavernChallengeConfig{}};

    const auto build = settlement.build(
        terminal_player_win(), pixel_town::PlayerState{},
        pixel_town::BetTier::medium, 0);

    CHECK_FALSE(build.accepted());
    CHECK(build.error == pixel_town::TavernSettlementError::invalid_result_id);
    CHECK(build.result.result_id == 0);
}

TEST_CASE("a tavern settlement rejects malformed configuration and bet") {
    SUBCASE("negative bet configuration") {
        pixel_town::TavernChallengeConfig config;
        config.low_bet = -1;
        const pixel_town::TavernChallengeSettlement settlement{config};
        const auto build = settlement.build(
            terminal_player_win(), pixel_town::PlayerState{},
            pixel_town::BetTier::low, 1);
        CHECK(build.error == pixel_town::TavernSettlementError::invalid_config);
    }

    SUBCASE("unknown bet") {
        const pixel_town::TavernChallengeSettlement settlement{
            pixel_town::TavernChallengeConfig{}};
        const auto build = settlement.build(
            terminal_player_win(), pixel_town::PlayerState{},
            static_cast<pixel_town::BetTier>(99), 1);
        CHECK(build.error == pixel_town::TavernSettlementError::invalid_bet);
    }
}

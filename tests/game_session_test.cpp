#include <doctest/doctest.h>

#include "core/game_session.hpp"

namespace {

void complete_day_with_rest(pixel_town::GameSession& session, pixel_town::Location day_location) {
    REQUIRE(session.enter_location(day_location));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);
    REQUIRE(session.apply_action_result(session.home_rest_result()).accepted);
    REQUIRE(session.finish_day_summary());
}

}  // namespace

TEST_CASE("new game starts on the first day map") {
    const auto session = pixel_town::GameSession::new_game();

    CHECK(session.day() == 1);
    CHECK(session.phase() == pixel_town::GamePhase::day_choice);
    CHECK(session.player().money == 50);
    CHECK(session.player().stamina == 80);
    CHECK(session.player().reputation == 0);
    CHECK(session.player().knowledge == 0);
    CHECK(session.player().mood == 70);
}

TEST_CASE("only current phase locations are legal") {
    auto session = pixel_town::GameSession::new_game();

    CHECK(session.can_enter(pixel_town::Location::restaurant).allowed);
    CHECK(session.can_enter(pixel_town::Location::convenience_store).allowed);
    CHECK(session.can_enter(pixel_town::Location::library).allowed);
    CHECK_FALSE(session.can_enter(pixel_town::Location::home).allowed);
    CHECK_FALSE(session.can_enter(pixel_town::Location::tavern).allowed);

    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);

    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.can_enter(pixel_town::Location::home).allowed);
    CHECK(session.can_enter(pixel_town::Location::tavern).allowed);
    CHECK_FALSE(session.can_enter(pixel_town::Location::restaurant).allowed);
}

TEST_CASE("day action result applies once and moves to night choice") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::library));
    REQUIRE(session.start_location() != 0);

    const auto result = session.simulated_success_result();
    const auto before = session.player();
    const auto applied = session.apply_action_result(result);

    CHECK(applied.accepted);
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.player().money == before.money + 6);
    CHECK(session.player().stamina == before.stamina - 8);
    CHECK(session.player().reputation == before.reputation + 5);
    CHECK(session.player().knowledge == before.knowledge + 6);
    CHECK(session.player().mood == before.mood + 2);
    CHECK_FALSE(session.apply_action_result(result).accepted);
}

TEST_CASE("full one-day path reaches day two") {
    auto session = pixel_town::GameSession::new_game();

    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);

    const auto night_result = session.home_rest_result();
    REQUIRE(session.apply_action_result(night_result).accepted);
    CHECK(session.phase() == pixel_town::GamePhase::day_summary);
    CHECK(session.last_summary().find("热水") != std::string::npos);

    REQUIRE(session.finish_day_summary());
    CHECK(session.day() == 2);
    CHECK(session.phase() == pixel_town::GamePhase::day_choice);
    CHECK(session.can_enter(pixel_town::Location::library).allowed);
}

TEST_CASE("ten-day path ends with one placeholder ending") {
    auto session = pixel_town::GameSession::new_game(20260707);

    for (int expected_day = 1; expected_day <= 10; ++expected_day) {
        CHECK(session.day() == expected_day);
        REQUIRE(session.phase() == pixel_town::GamePhase::day_choice);
        complete_day_with_rest(session, pixel_town::Location::restaurant);
    }

    CHECK(session.day() == 10);
    CHECK(session.phase() == pixel_town::GamePhase::ending);
    CHECK(session.is_ended());
    CHECK(session.main_ending() == std::string{"平凡小镇新人"});
    CHECK(session.final_summary().find("十天不长") != std::string::npos);
    CHECK(session.final_summary().find("均衡体验") != std::string::npos);
    CHECK_FALSE(session.can_enter(pixel_town::Location::restaurant).allowed);
    CHECK_FALSE(session.finish_day_summary());
}

TEST_CASE("fixed seed produces repeatable daily context and results") {
    auto first = pixel_town::GameSession::new_game(42);
    auto second = pixel_town::GameSession::new_game(42);

    for (int day = 1; day <= 3; ++day) {
        const auto first_context = first.current_day_context();
        const auto second_context = second.current_day_context();
        CHECK(first_context.day == second_context.day);
        CHECK(first_context.seed == second_context.seed);
        CHECK(first_context.weather == second_context.weather);
        CHECK(first_context.event == second_context.event);

        REQUIRE(first.enter_location(pixel_town::Location::library));
        REQUIRE(second.enter_location(pixel_town::Location::library));
        REQUIRE(first.start_location() != 0);
        REQUIRE(second.start_location() != 0);
        CHECK(first.simulated_success_result().delta.money ==
              second.simulated_success_result().delta.money);
        CHECK(first.simulated_success_result().summary ==
              second.simulated_success_result().summary);

        REQUIRE(first.apply_action_result(first.simulated_success_result()).accepted);
        REQUIRE(second.apply_action_result(second.simulated_success_result()).accepted);
        REQUIRE(first.apply_action_result(first.home_rest_result()).accepted);
        REQUIRE(second.apply_action_result(second.home_rest_result()).accepted);
        REQUIRE(first.finish_day_summary());
        REQUIRE(second.finish_day_summary());
    }
}

TEST_CASE("day nine advances to day ten and day ten does not advance to eleven") {
    auto session = pixel_town::GameSession::new_game();

    for (int day = 1; day <= 8; ++day) {
        complete_day_with_rest(session, pixel_town::Location::restaurant);
    }
    CHECK(session.day() == 9);
    CHECK(session.phase() == pixel_town::GamePhase::day_choice);

    complete_day_with_rest(session, pixel_town::Location::restaurant);
    CHECK(session.day() == 10);
    CHECK(session.phase() == pixel_town::GamePhase::day_choice);

    complete_day_with_rest(session, pixel_town::Location::restaurant);
    CHECK(session.day() == 10);
    CHECK(session.phase() == pixel_town::GamePhase::ending);
    CHECK_FALSE(session.finish_day_summary());
}

TEST_CASE("attribute floor does not end the schedule early") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    const int result_id = session.start_location();
    REQUIRE(result_id != 0);

    pixel_town::ActionResult exhausting_result;
    exhausting_result.result_id = result_id;
    exhausting_result.slot = pixel_town::ActionSlot::day;
    exhausting_result.location = pixel_town::Location::restaurant;
    exhausting_result.outcome = pixel_town::ActionOutcome::completed;
    exhausting_result.delta = pixel_town::StatDelta{0, -999, 0, 0, -999};
    exhausting_result.summary = "压力测试：属性触底但不提前结束。";
    REQUIRE(session.apply_action_result(exhausting_result).accepted);

    CHECK(session.player().stamina == 0);
    CHECK(session.player().mood == 0);
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.can_enter(pixel_town::Location::home).allowed);
    REQUIRE(session.apply_action_result(session.home_rest_result()).accepted);
    REQUIRE(session.finish_day_summary());
    CHECK(session.day() == 2);
    CHECK(session.phase() == pixel_town::GamePhase::day_choice);
}

TEST_CASE("returning before start does not consume the day action") {
    auto session = pixel_town::GameSession::new_game();

    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    CHECK(session.phase() == pixel_town::GamePhase::day_location);
    REQUIRE(session.return_to_map());
    CHECK(session.phase() == pixel_town::GamePhase::day_choice);
    CHECK(session.can_enter(pixel_town::Location::library).allowed);

    REQUIRE(session.enter_location(pixel_town::Location::library));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
}

TEST_CASE("abandoning after start consumes the phase without rewards") {
    auto session = pixel_town::GameSession::new_game();
    const auto before = session.player();

    REQUIRE(session.enter_location(pixel_town::Location::convenience_store));
    REQUIRE(session.start_location() != 0);
    REQUIRE_FALSE(session.return_to_map());
    REQUIRE(session.apply_action_result(session.abandon_current_location()).accepted);

    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.player().money == before.money);
    CHECK(session.player().stamina == before.stamina);
    CHECK(session.player().reputation == before.reputation);
    CHECK(session.player().knowledge == before.knowledge);
    CHECK(session.player().mood == before.mood);
    CHECK_FALSE(session.can_enter(pixel_town::Location::restaurant).allowed);
}


#include "core/tavern_rules.hpp"

TEST_CASE("tavern is accessible at night_choice after day action") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::library));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.can_enter(pixel_town::Location::tavern).allowed);
    CHECK(session.can_enter(pixel_town::Location::home).allowed);
}

TEST_CASE("tavern apply action result transitions to day_summary and updates record") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);

    REQUIRE(session.enter_location(pixel_town::Location::tavern));
    REQUIRE(session.start_location() != 0);

    const int result_id = session.active_result_id();
    const auto& player = session.player();
    const pixel_town::TavernChallengeConfig config;
    const auto result = pixel_town::simulate_tavern_challenge(
        player, config, pixel_town::ChallengeType::gomoku,
        pixel_town::BetTier::low, pixel_town::ChallengeOutcome::win, result_id);

    const auto applied = session.apply_action_result(result);
    CHECK(applied.accepted);
    CHECK(session.phase() == pixel_town::GamePhase::day_summary);
    CHECK(session.tavern_wins() == 1);
    CHECK(session.tavern_losses() == 0);
    CHECK(session.last_summary().find("\u9152\u9986\u6311\u6218") != std::string::npos);
}

TEST_CASE("tavern and home are mutually exclusive per night") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::library));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);

    REQUIRE(session.enter_location(pixel_town::Location::tavern));
    REQUIRE(session.start_location() != 0);
    const int result_id = session.active_result_id();
    const auto& player = session.player();
    const pixel_town::TavernChallengeConfig config;
    const auto tavern_result = pixel_town::simulate_tavern_challenge(
        player, config, pixel_town::ChallengeType::gomoku,
        pixel_town::BetTier::medium, pixel_town::ChallengeOutcome::win, result_id);
    REQUIRE(session.apply_action_result(tavern_result).accepted);

    CHECK_FALSE(session.can_enter(pixel_town::Location::home).allowed);
    CHECK_FALSE(session.can_enter(pixel_town::Location::tavern).allowed);
}

TEST_CASE("home rest path is unchanged after tavern is added") {
    auto session = pixel_town::GameSession::new_game();
    const auto before = session.player();

    REQUIRE(session.enter_location(pixel_town::Location::library));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);

    REQUIRE(session.apply_action_result(session.home_rest_result()).accepted);
    CHECK(session.phase() == pixel_town::GamePhase::day_summary);
    CHECK(session.player().stamina == before.stamina - 8 + 15);
    CHECK(session.player().mood == before.mood + 2 + 5);
    CHECK(session.last_summary().find("热水") != std::string::npos);
    CHECK(session.tavern_wins() == 0);
    CHECK(session.tavern_losses() == 0);

    REQUIRE(session.finish_day_summary());
    CHECK(session.day() == 2);
}

TEST_CASE("bet exceeding money returns abandoned at rules layer") {
    pixel_town::PlayerState player;
    player.money = 5;
    const pixel_town::TavernChallengeConfig config;

    const auto result = pixel_town::simulate_tavern_challenge(
        player, config, pixel_town::ChallengeType::liars_dice,
        pixel_town::BetTier::medium, pixel_town::ChallengeOutcome::win, 1);

    CHECK(result.outcome == pixel_town::ActionOutcome::abandoned);
    CHECK(result.delta.money == 0);
    CHECK(result.summary.find("\u8d4c\u6ce8\u4e0d\u8db3") != std::string::npos);
}

TEST_CASE("ten days completing day work then tavern each night reaches ending") {
    auto session = pixel_town::GameSession::new_game(20260707);

    for (int expected_day = 1; expected_day <= 10; ++expected_day) {
        CHECK(session.day() == expected_day);
        REQUIRE(session.phase() == pixel_town::GamePhase::day_choice);
        REQUIRE(session.enter_location(pixel_town::Location::restaurant));
        REQUIRE(session.start_location() != 0);
        REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);

        REQUIRE(session.enter_location(pixel_town::Location::tavern));
        REQUIRE(session.start_location() != 0);
        const int result_id = session.active_result_id();
        const auto& player = session.player();
        const pixel_town::TavernChallengeConfig config;
        const auto tavern_result = pixel_town::simulate_tavern_challenge(
            player, config, pixel_town::ChallengeType::gomoku,
            pixel_town::BetTier::low, pixel_town::ChallengeOutcome::win, result_id);
        REQUIRE(session.apply_action_result(tavern_result).accepted);
        REQUIRE(session.finish_day_summary());
    }

    CHECK(session.phase() == pixel_town::GamePhase::ending);
    CHECK(session.is_ended());
    CHECK(session.player().money >= 0);
    CHECK(session.player().money <= 999);
    CHECK(session.player().stamina >= 0);
    CHECK(session.player().stamina <= 100);
    CHECK(session.player().mood >= 0);
    CHECK(session.player().mood <= 100);
    CHECK(session.tavern_wins() == 10);
    CHECK(session.tavern_losses() == 0);
}

#include <doctest/doctest.h>

#include "app/location_result_adapter.hpp"

TEST_CASE("library action result maps location result into core action result") {
    pixel_town::library::ActionResult library_result;
    library_result.completed = true;
    library_result.money_change = 3;
    library_result.stamina_change = -4;
    library_result.reputation_change = 5;
    library_result.knowledge_change = 6;
    library_result.mood_change = 7;
    library_result.summary = "图书馆工作完成";

    const auto result = pixel_town::library_action_result(
        library_result, 42, pixel_town::ActionSlot::day);

    CHECK(result.result_id == 42);
    CHECK(result.slot == pixel_town::ActionSlot::day);
    CHECK(result.location == pixel_town::Location::library);
    CHECK(result.outcome == pixel_town::ActionOutcome::completed);
    CHECK(result.delta.money == 3);
    CHECK(result.delta.stamina == -4);
    CHECK(result.delta.reputation == 5);
    CHECK(result.delta.knowledge == 6);
    CHECK(result.delta.mood == 7);
    CHECK(result.summary == "图书馆工作完成");
}

TEST_CASE("library give up maps to abandoned core result") {
    pixel_town::library::ActionResult library_result;
    library_result.gave_up = true;
    library_result.summary = "离开了图书馆";

    const auto result = pixel_town::library_action_result(
        library_result, 7, pixel_town::ActionSlot::day);

    CHECK(result.result_id == 7);
    CHECK(result.outcome == pixel_town::ActionOutcome::abandoned);
    CHECK(result.delta.money == 0);
}

TEST_CASE("library daily context is deterministic from session and visit count") {
    const auto session = pixel_town::GameSession::new_game(20260709);

    const auto first = pixel_town::make_library_daily_context(session, 1);
    const auto second = pixel_town::make_library_daily_context(session, 1);
    const auto next_visit = pixel_town::make_library_daily_context(session, 2);

    CHECK(first.day == 1);
    CHECK(first.current_knowledge == 0);
    CHECK(first.random_seed == second.random_seed);
    CHECK(first.random_seed != next_visit.random_seed);
}

TEST_CASE("tavern adapter checks bet affordability and uses active result id") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(session.simulated_success_result()).accepted);
    REQUIRE(session.enter_location(pixel_town::Location::tavern));
    REQUIRE(session.start_location() != 0);

    const pixel_town::TavernChallengeConfig config;
    CHECK(pixel_town::can_afford_tavern_bet(
        session.player(), pixel_town::BetTier::low, config));

    const auto result = pixel_town::tavern_action_result(
        session, pixel_town::ChallengeType::gomoku, pixel_town::BetTier::low,
        pixel_town::ChallengeOutcome::win, config);

    CHECK(result.result_id == session.active_result_id());
    CHECK(result.slot == pixel_town::ActionSlot::night);
    CHECK(result.location == pixel_town::Location::tavern);
    CHECK(result.tavern_win_delta == 1);
}

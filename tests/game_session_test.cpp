#include <doctest/doctest.h>

#include "core/game_session.hpp"

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
    CHECK_FALSE(session.can_enter(pixel_town::Location::restaurant).allowed);
    CHECK_FALSE(session.can_enter(pixel_town::Location::tavern).allowed);
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
    CHECK(session.last_summary().find("回家休息") != std::string::npos);

    REQUIRE(session.finish_day_summary());
    CHECK(session.day() == 2);
    CHECK(session.phase() == pixel_town::GamePhase::day_choice);
    CHECK(session.can_enter(pixel_town::Location::library).allowed);
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

#include <doctest/doctest.h>

#include "app/location_result_adapter.hpp"

TEST_CASE("library action result maps location result into core action result") {
    pixel_town::library::LibraryWorkResult library_result;
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
    pixel_town::library::LibraryWorkResult library_result;
    library_result.gave_up = true;
    library_result.summary = "离开了图书馆";

    const auto result = pixel_town::library_action_result(
        library_result, 7, pixel_town::ActionSlot::day);

    CHECK(result.result_id == 7);
    CHECK(result.outcome == pixel_town::ActionOutcome::abandoned);
    CHECK(result.delta.money == 0);
}

TEST_CASE("library organizing result uses the same core settlement contract") {
    pixel_town::library::LibraryWorkResult organizing_result;
    organizing_result.completed = true;
    organizing_result.money_change = 8;
    organizing_result.stamina_change = -15;
    organizing_result.reputation_change = 4;
    organizing_result.knowledge_change = 10;
    organizing_result.mood_change = 2;
    organizing_result.summary = "完成图书馆整理";

    const auto result = pixel_town::library_action_result(
        organizing_result, 43, pixel_town::ActionSlot::day);

    CHECK(result.result_id == 43);
    CHECK(result.location == pixel_town::Location::library);
    CHECK(result.outcome == pixel_town::ActionOutcome::completed);
    CHECK(result.delta.money == 8);
    CHECK(result.delta.stamina == -15);
    CHECK(result.delta.reputation == 4);
    CHECK(result.delta.knowledge == 10);
    CHECK(result.delta.mood == 2);
}

TEST_CASE("library organizing completion advances the day through the core session") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::library));
    const int result_id = session.start_location();
    REQUIRE(result_id != 0);

    pixel_town::library::LibraryWorkResult organizing_result;
    organizing_result.completed = true;
    organizing_result.knowledge_change = 10;
    organizing_result.reputation_change = 4;
    organizing_result.stamina_change = -15;
    organizing_result.summary = "完成图书馆整理";

    const auto applied = session.apply_action_result(
        pixel_town::library_action_result(
            organizing_result, result_id, pixel_town::ActionSlot::day));

    CHECK(applied.accepted);
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.player().knowledge == 10);
    CHECK(session.player().reputation == 4);
    CHECK(session.player().stamina == 65);
}

TEST_CASE("library action result keeps narrative echo in core summary") {
    pixel_town::library::LibraryWorkResult library_result;
    library_result.completed = true;
    library_result.summary = "图书馆工作完成";
    library_result.narrative_echo = "借书卡又多盖了一个章。";

    const auto result = pixel_town::library_action_result(
        library_result, 8, pixel_town::ActionSlot::day);

    CHECK(result.summary.find("图书馆工作完成") != std::string::npos);
    CHECK(result.summary.find("借书卡又多盖了一个章。") != std::string::npos);
}

TEST_CASE("library day action enters from map and returns to night choice") {
    auto session = pixel_town::GameSession::new_game();

    CHECK(session.can_enter(pixel_town::Location::library).allowed);
    REQUIRE(session.enter_location(pixel_town::Location::library));
    const int result_id = session.start_location();
    REQUIRE(result_id != 0);

    pixel_town::library::LibraryWorkResult library_result;
    library_result.completed = true;
    library_result.knowledge_change = 6;
    library_result.reputation_change = 4;
    library_result.stamina_change = -8;
    library_result.summary = "图书馆工作完成";
    library_result.narrative_echo = "泛黄书页被重新夹回借书卡盒子。";

    const auto applied = session.apply_action_result(
        pixel_town::library_action_result(library_result, result_id, pixel_town::ActionSlot::day));

    CHECK(applied.accepted);
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.player().knowledge == 6);
    CHECK(session.player().reputation == 4);
    CHECK(session.player().stamina == 72);
    CHECK(session.last_summary().find("泛黄书页") != std::string::npos);
    CHECK_FALSE(session.can_enter(pixel_town::Location::library).allowed);
}

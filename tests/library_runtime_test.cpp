#include <algorithm>

#include <doctest/doctest.h>

// 图书馆 Runtime 契约：模式互斥、语义 intent、完成结算和主动放弃。
#include "app/library_runtime.hpp"
#include "locations/library_data.hpp"

TEST_CASE("library runtime completes organizing through one public lifecycle") {
    auto session = pixel_town::GameSession::new_game(20260714U);
    REQUIRE(session.enter_location(pixel_town::Location::library));
    const auto loaded =
        pixel_town::library::load_library_data("assets/data/library_data.txt");
    REQUIRE(loaded.success);

    pixel_town::LibraryRuntime runtime;
    REQUIRE(runtime.open(session, loaded.data).status ==
            pixel_town::LibraryOpenStatus::opened);
    REQUIRE(runtime.step(
                session,
                {pixel_town::LibraryIntentType::select_organizing, {}})
                .status == pixel_town::LibraryStepStatus::changed);

    auto view = runtime.presentation();
    REQUIRE(view.organizing.has_value());
    REQUIRE(view.organizing->tasks.size() == 4);

    pixel_town::LibraryStepResult last_step;
    for (const auto& task : view.organizing->tasks) {
        REQUIRE(runtime.step(
                    session,
                    {pixel_town::LibraryIntentType::pick_up_book, task.id})
                    .status == pixel_town::LibraryStepStatus::changed);

        const auto shelf = std::find_if(
            view.organizing->shelves.begin(), view.organizing->shelves.end(),
            [&task](const auto& item) {
                return item.category_id == task.category_id;
            });
        REQUIRE(shelf != view.organizing->shelves.end());
        last_step = runtime.step(
            session,
            {pixel_town::LibraryIntentType::place_on_shelf, shelf->id});
    }

    CHECK(last_step.status == pixel_town::LibraryStepStatus::settled);
    CHECK_FALSE(runtime.active());
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.player().knowledge == 20);
    CHECK(session.player().reputation == 8);
    CHECK(session.player().stamina == 65);
}

TEST_CASE("library runtime completes reader consultation through the same lifecycle") {
    auto session = pixel_town::GameSession::new_game(20260714U);
    REQUIRE(session.enter_location(pixel_town::Location::library));
    const auto loaded =
        pixel_town::library::load_library_data("assets/data/library_data.txt");
    REQUIRE(loaded.success);

    pixel_town::LibraryRuntime runtime;
    REQUIRE(runtime.open(session, loaded.data).status ==
            pixel_town::LibraryOpenStatus::opened);
    REQUIRE(runtime.step(
                session,
                {pixel_town::LibraryIntentType::select_reader, {}})
                .status == pixel_town::LibraryStepStatus::changed);

    const auto introduction = runtime.presentation();
    REQUIRE(introduction.reader.has_value());
    CHECK(introduction.reader->welcome_message == "欢迎来到小镇图书馆");
    CHECK(introduction.reader->work_intro.find("读者会提出各种问题") == 0);

    while (true) {
        const auto view = runtime.presentation();
        REQUIRE(view.reader.has_value());
        if (!view.reader->current_question.has_value()) {
            break;
        }
        REQUIRE(runtime.step(
                    session,
                    {pixel_town::LibraryIntentType::answer_category,
                     view.reader->current_question->correct_category_id})
                    .status == pixel_town::LibraryStepStatus::changed);
    }

    const auto settled = runtime.step(
        session, {pixel_town::LibraryIntentType::finish_reader, {}});
    CHECK(settled.status == pixel_town::LibraryStepStatus::settled);
    CHECK_FALSE(runtime.active());
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.player().knowledge == 50);
    CHECK(session.player().reputation == 25);
    CHECK(session.player().money == 90);
    CHECK(session.player().stamina == 65);
}

TEST_CASE("abandoning reader consultation discards partial rewards") {
    auto session = pixel_town::GameSession::new_game(20260714U);
    const auto player_before = session.player();
    REQUIRE(session.enter_location(pixel_town::Location::library));
    const auto loaded =
        pixel_town::library::load_library_data("assets/data/library_data.txt");
    REQUIRE(loaded.success);

    pixel_town::LibraryRuntime runtime;
    REQUIRE(runtime.open(session, loaded.data).status ==
            pixel_town::LibraryOpenStatus::opened);
    REQUIRE(runtime.step(
                session,
                {pixel_town::LibraryIntentType::select_reader, {}})
                .status == pixel_town::LibraryStepStatus::changed);

    const auto view = runtime.presentation();
    REQUIRE(view.reader.has_value());
    REQUIRE(view.reader->current_question.has_value());
    REQUIRE(runtime.step(
                session,
                {pixel_town::LibraryIntentType::answer_category,
                 view.reader->current_question->correct_category_id})
                .status == pixel_town::LibraryStepStatus::changed);

    const auto abandoned = runtime.step(
        session, {pixel_town::LibraryIntentType::abandon, {}});

    CHECK(abandoned.status == pixel_town::LibraryStepStatus::settled);
    CHECK_FALSE(runtime.active());
    CHECK(session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(session.player().money == player_before.money);
    CHECK(session.player().stamina == player_before.stamina);
    CHECK(session.player().reputation == player_before.reputation);
    CHECK(session.player().knowledge == player_before.knowledge);
    CHECK(session.player().mood == player_before.mood);
}

TEST_CASE("organizing keeps the held book after a wrong shelf for runtime retry") {
    auto session = pixel_town::GameSession::new_game(20260714U);
    REQUIRE(session.enter_location(pixel_town::Location::library));
    const auto loaded =
        pixel_town::library::load_library_data("assets/data/library_data.txt");
    REQUIRE(loaded.success);

    pixel_town::LibraryRuntime runtime;
    REQUIRE(runtime.open(session, loaded.data).status ==
            pixel_town::LibraryOpenStatus::opened);
    REQUIRE(runtime.step(
                session,
                {pixel_town::LibraryIntentType::select_organizing, {}})
                .status == pixel_town::LibraryStepStatus::changed);

    const auto initial = runtime.presentation();
    REQUIRE(initial.organizing.has_value());
    REQUIRE_FALSE(initial.organizing->tasks.empty());
    const auto& task = initial.organizing->tasks.front();
    const auto correct = std::find_if(
        initial.organizing->shelves.begin(), initial.organizing->shelves.end(),
        [&task](const auto& shelf) {
            return shelf.category_id == task.category_id;
        });
    const auto wrong = std::find_if(
        initial.organizing->shelves.begin(), initial.organizing->shelves.end(),
        [&task](const auto& shelf) {
            return shelf.category_id != task.category_id;
        });
    REQUIRE(correct != initial.organizing->shelves.end());
    REQUIRE(wrong != initial.organizing->shelves.end());

    REQUIRE(runtime.step(
                session,
                {pixel_town::LibraryIntentType::pick_up_book, task.id})
                .status == pixel_town::LibraryStepStatus::changed);
    const auto wrong_step = runtime.step(
        session,
        {pixel_town::LibraryIntentType::place_on_shelf, wrong->id});

    CHECK(wrong_step.status == pixel_town::LibraryStepStatus::changed);
    const auto retry = runtime.presentation();
    REQUIRE(retry.organizing.has_value());
    CHECK(retry.organizing->state.held_book_id == task.id);
    CHECK(retry.organizing->state.completed_count == 0);

    CHECK(runtime.step(
              session,
              {pixel_town::LibraryIntentType::place_on_shelf, correct->id})
              .status == pixel_town::LibraryStepStatus::changed);
}

TEST_CASE("organizing explains empty pickup and shelf clicks") {
    auto session = pixel_town::GameSession::new_game(20260714U);
    REQUIRE(session.enter_location(pixel_town::Location::library));
    const auto loaded =
        pixel_town::library::load_library_data("assets/data/library_data.txt");
    REQUIRE(loaded.success);

    pixel_town::LibraryRuntime runtime;
    REQUIRE(runtime.open(session, loaded.data).status ==
            pixel_town::LibraryOpenStatus::opened);
    REQUIRE(runtime.step(
                session,
                {pixel_town::LibraryIntentType::select_organizing, {}})
                .status == pixel_town::LibraryStepStatus::changed);

    const auto empty_pickup = runtime.step(
        session, {pixel_town::LibraryIntentType::pick_up_book, {}});
    CHECK(empty_pickup.status == pixel_town::LibraryStepStatus::unchanged);
    CHECK(empty_pickup.notice ==
          "这里没有待整理的书，请点击带光圈的书本素材。");

    const auto view = runtime.presentation();
    REQUIRE(view.organizing.has_value());
    REQUIRE_FALSE(view.organizing->tasks.empty());
    REQUIRE(runtime.step(
                session,
                {pixel_town::LibraryIntentType::pick_up_book,
                 view.organizing->tasks.front().id})
                .status == pixel_town::LibraryStepStatus::changed);

    const auto empty_shelf = runtime.step(
        session, {pixel_town::LibraryIntentType::place_on_shelf, {}});
    CHECK(empty_shelf.status == pixel_town::LibraryStepStatus::unchanged);
    CHECK(empty_shelf.notice == "请点击书架高亮区域完成归位。");
}

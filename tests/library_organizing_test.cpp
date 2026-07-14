#include <doctest/doctest.h>

#include "locations/library_organizing.hpp"

TEST_CASE("library organizing lets the player shelve a scattered book correctly") {
    using namespace pixel_town::library;

    const std::vector<OrganizingShelf> shelves = {
        {"shelf_history", "history", "历史书架", 500, 80, 80, 90},
        {"shelf_science", "science", "科学书架", 620, 80, 80, 90},
    };
    const std::vector<OrganizingBookTask> tasks = {
        {"book_physics", "物理奥秘", "science", OrganizingBookSource::scattered,
         "", 280, 400},
    };

    LibraryOrganizingSession session(tasks, shelves, default_organizing_config());
    session.start(12345);

    const auto picked = session.pick_up("book_physics");
    REQUIRE(picked.status == OrganizingActionStatus::accepted);
    CHECK(session.state().held_book_id == "book_physics");

    const auto placed = session.place_on_shelf("shelf_science");
    CHECK(placed.status == OrganizingActionStatus::accepted);
    CHECK(session.state().completed_count == 1);
    CHECK(session.is_completed());
}

TEST_CASE("library organizing keeps a book in hand after a wrong shelf so the player can retry") {
    using namespace pixel_town::library;

    const std::vector<OrganizingShelf> shelves = {
        {"shelf_history", "history", "历史书架", 500, 80, 80, 90},
        {"shelf_science", "science", "科学书架", 620, 80, 80, 90},
    };
    const std::vector<OrganizingBookTask> tasks = {
        {"book_physics", "物理奥秘", "science", OrganizingBookSource::scattered,
         "", 280, 400},
    };
    LibraryOrganizingSession session(tasks, shelves, default_organizing_config());
    session.start(9);
    REQUIRE(session.pick_up("book_physics").status == OrganizingActionStatus::accepted);

    const auto wrong = session.place_on_shelf("shelf_history");
    CHECK(wrong.status == OrganizingActionStatus::wrong_shelf);
    CHECK(session.state().held_book_id == "book_physics");
    CHECK(session.state().completed_count == 0);
    CHECK_FALSE(session.is_completed());

    CHECK(session.place_on_shelf("shelf_science").status ==
          OrganizingActionStatus::accepted);
    CHECK(session.is_completed());
}

TEST_CASE("library organizing selects a deterministic task order from the daily seed") {
    using namespace pixel_town::library;

    std::vector<OrganizingBookTask> tasks;
    for (int index = 0; index < 8; ++index) {
        tasks.push_back({"book_" + std::to_string(index), "书" + std::to_string(index),
                         "history", OrganizingBookSource::scattered, "", 200 + index, 400});
    }
    OrganizingConfig config = default_organizing_config();
    config.tasks_per_session = 4;

    LibraryOrganizingSession first(tasks, {}, config);
    LibraryOrganizingSession repeated(tasks, {}, config);
    LibraryOrganizingSession another_day(tasks, {}, config);
    first.start(12345);
    repeated.start(12345);
    another_day.start(54321);

    const auto ids = [](const std::vector<OrganizingBookTask>& selected) {
        std::vector<std::string> result;
        for (const auto& task : selected) {
            result.push_back(task.id);
        }
        return result;
    };
    REQUIRE(first.tasks().size() == 4);
    CHECK(ids(first.tasks()) == ids(repeated.tasks()));
    CHECK(ids(first.tasks()) != ids(another_day.tasks()));
}

TEST_CASE("library organizing settles each completed task exactly once") {
    using namespace pixel_town::library;

    const std::vector<OrganizingShelf> shelves = {
        {"shelf_history", "history", "历史书架", 500, 80, 80, 90},
        {"shelf_science", "science", "科学书架", 620, 80, 80, 90},
    };
    const std::vector<OrganizingBookTask> tasks = {
        {"book_physics", "物理奥秘", "science", OrganizingBookSource::scattered,
         "", 280, 400},
        {"book_poetry", "唐诗宋词", "history", OrganizingBookSource::misplaced,
         "shelf_science", 660, 110},
    };
    OrganizingConfig config = default_organizing_config();
    config.tasks_per_session = 2;
    config.money_per_book = 4;
    config.knowledge_per_book = 5;
    config.reputation_per_book = 2;
    config.stamina_cost = 15;
    config.base_mood_change = 3;
    config.wrong_mood_penalty = -1;

    LibraryOrganizingSession session(tasks, shelves, config);
    session.start(77);
    for (const auto& task : session.tasks()) {
        REQUIRE(session.pick_up(task.id).status == OrganizingActionStatus::accepted);
        if (task.id == "book_physics") {
            CHECK(session.place_on_shelf("shelf_history").status ==
                  OrganizingActionStatus::wrong_shelf);
        }
        const std::string target = task.category_id == "science" ? "shelf_science"
                                                                  : "shelf_history";
        REQUIRE(session.place_on_shelf(target).status == OrganizingActionStatus::accepted);
    }

    const auto result = session.finish_session();
    CHECK(result.completed);
    CHECK(result.money_change == 8);
    CHECK(result.knowledge_change == 10);
    CHECK(result.reputation_change == 4);
    CHECK(result.stamina_change == -15);
    CHECK(result.mood_change == 2);
}

TEST_CASE("library organizing gives no partial rewards after the player abandons") {
    using namespace pixel_town::library;
    const std::vector<OrganizingShelf> shelves = {
        {"shelf_history", "history", "历史书架", 500, 80, 80, 90},
    };
    const std::vector<OrganizingBookTask> tasks = {
        {"book_a", "甲书", "history", OrganizingBookSource::scattered, "", 200, 400},
        {"book_b", "乙书", "history", OrganizingBookSource::scattered, "", 250, 400},
    };
    LibraryOrganizingSession session(tasks, shelves, default_organizing_config());
    session.start(2);
    REQUIRE(session.pick_up(session.tasks()[0].id).status ==
            OrganizingActionStatus::accepted);
    REQUIRE(session.place_on_shelf("shelf_history").status ==
            OrganizingActionStatus::accepted);

    session.give_up();
    const auto result = session.finish_session();

    CHECK(result.gave_up);
    CHECK_FALSE(result.completed);
    CHECK(result.money_change == 0);
    CHECK(result.knowledge_change == 0);
    CHECK(result.reputation_change == 0);
    CHECK(result.stamina_change == 0);
    CHECK(result.mood_change == 0);
}

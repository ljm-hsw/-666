#include <doctest/doctest.h>

#include "core/location_story.hpp"

TEST_CASE("unvisited location always selects its tutorial before daily events") {
    pixel_town::LocationStoryContext context;
    context.location = pixel_town::Location::convenience_store;
    context.day = 5;
    context.weather = "小雨";
    context.day_event = "便利店零食更受欢迎";
    context.completed_visits = 0;
    context.seed = 20260715U;

    const pixel_town::LocationStoryCatalog catalog;
    const auto selection = catalog.select(context);

    CHECK(selection.kind == pixel_town::LocationStoryEventKind::tutorial);
    CHECK(selection.id == "convenience_store_tutorial");
    REQUIRE_FALSE(selection.script.lines.empty());
    CHECK(selection.script.lines.front().speaker == "便利店店主");
}

TEST_CASE("returning location selects its day ten finale before familiar dialogue") {
    pixel_town::LocationStoryContext context;
    context.location = pixel_town::Location::library;
    context.day = 10;
    context.weather = "晴天";
    context.day_event = "小镇节奏平稳";
    context.completed_visits = 2;
    context.seed = 20260715U;

    const auto selection = pixel_town::LocationStoryCatalog{}.select(context);

    CHECK(selection.kind == pixel_town::LocationStoryEventKind::finale);
    CHECK(selection.id == "library_day_10_finale");
    REQUIRE_FALSE(selection.script.lines.empty());
    CHECK(selection.script.lines.front().speaker == "管理员");
}

TEST_CASE("returning location selects a deterministic daily event") {
    pixel_town::LocationStoryContext context;
    context.location = pixel_town::Location::restaurant;
    context.day = 2;
    context.weather = "晴天";
    context.day_event = "餐馆客流增加";
    context.completed_visits = 1;
    context.seed = 20260715U;

    const pixel_town::LocationStoryCatalog catalog;
    const auto first = catalog.select(context);
    const auto second = catalog.select(context);

    CHECK(first.kind == pixel_town::LocationStoryEventKind::daily);
    CHECK(first.id == "restaurant_day_2_daily");
    CHECK(first.id == second.id);
    REQUIRE_FALSE(first.script.lines.empty());
    CHECK(first.script.lines.front().speaker == "餐馆老板");
}

TEST_CASE("rainy store incident is deterministic and follows daily event priority") {
    pixel_town::LocationStoryContext context;
    context.location = pixel_town::Location::convenience_store;
    context.day = 5;
    context.weather = "小雨";
    context.day_event = "便利店零食更受欢迎";
    context.completed_visits = 3;
    context.seed = 20260715U;

    const pixel_town::LocationStoryCatalog catalog;
    const auto first = catalog.select(context);
    const auto second = catalog.select(context);

    CHECK(first.kind == pixel_town::LocationStoryEventKind::incident);
    CHECK(first.id == "convenience_store_day_5_umbrella_shortage");
    CHECK(first.id == second.id);
    REQUIRE_FALSE(first.script.lines.empty());
    CHECK(first.script.lines.front().speaker == "便利店店主");
}

TEST_CASE("each location has a planned returning-visit event") {
    const pixel_town::LocationStoryCatalog catalog;

    const auto select = [&catalog](pixel_town::Location location, int day,
                                   const char* weather) {
        return catalog.select({location, day, weather, "小镇节奏平稳", 1, 20260715U});
    };

    CHECK(select(pixel_town::Location::restaurant, 2, "晴天").kind ==
          pixel_town::LocationStoryEventKind::daily);
    CHECK(select(pixel_town::Location::convenience_store, 5, "小雨").kind ==
          pixel_town::LocationStoryEventKind::incident);
    CHECK(select(pixel_town::Location::library, 4, "多云").kind ==
          pixel_town::LocationStoryEventKind::daily);
    CHECK(select(pixel_town::Location::tavern, 6, "微风").kind ==
          pixel_town::LocationStoryEventKind::incident);
    CHECK(select(pixel_town::Location::home, 7, "晴天").kind ==
          pixel_town::LocationStoryEventKind::daily);
}

TEST_CASE("session-derived location story context is read only and reproducible") {
    const auto session = pixel_town::GameSession::new_game(20260715U);
    const auto before = session.snapshot();

    const auto context = pixel_town::location_story_context(session, pixel_town::Location::library);

    CHECK(context.location == pixel_town::Location::library);
    CHECK(context.day == 1);
    CHECK(context.completed_visits == 0);
    CHECK(context.weather == session.current_day_context().weather);
    CHECK(context.day_event == session.current_day_context().event);
    CHECK(context.seed == session.location_seed(pixel_town::Location::library, 0));
    (void)pixel_town::LocationStoryCatalog{}.select(context);
    CHECK(session.snapshot() == before);
}

TEST_CASE("location story catalog exposes its new event glyphs") {
    const std::string glyphs = pixel_town::LocationStoryCatalog{}.glyphs();

    CHECK(glyphs.find("雨伞一下少了不少") != std::string::npos);
    CHECK(glyphs.find("黑棋子卡在桌缝") != std::string::npos);
    CHECK(glyphs.find("晚上大家会看看") != std::string::npos);
}

TEST_CASE("returning restaurant selects its rainy day five incident") {
    const pixel_town::LocationStoryContext context{
        pixel_town::Location::restaurant, 5, "小雨", "餐馆客流增加", 1, 20260715U};

    const auto selection = pixel_town::LocationStoryCatalog{}.select(context);

    CHECK(selection.kind == pixel_town::LocationStoryEventKind::incident);
    CHECK(selection.id == "restaurant_day_5_rainy_guests");
    REQUIRE_FALSE(selection.script.lines.empty());
    CHECK(selection.script.lines.front().text.find("热汤") != std::string::npos);
}

TEST_CASE("each daytime location has three daily and two incident scripts") {
    const pixel_town::Location locations[] = {
        pixel_town::Location::restaurant,
        pixel_town::Location::convenience_store,
        pixel_town::Location::library,
    };
    const pixel_town::LocationStoryCatalog catalog;

    for (const pixel_town::Location location : locations) {
        for (const int day : {2, 3, 4}) {
            const auto selection = catalog.select(
                {location, day, "晴天", "小镇节奏平稳", 1, 20260715U});
            CHECK(selection.kind == pixel_town::LocationStoryEventKind::daily);
            CHECK_FALSE(selection.script.lines.empty());
        }
        for (const auto& context : {
                 pixel_town::LocationStoryContext{
                     location, 5, "小雨", "小镇节奏平稳", 1, 20260715U},
                 pixel_town::LocationStoryContext{
                     location, 8, "晴天", "小镇节奏平稳", 1, 20260715U},
             }) {
            const auto selection = catalog.select(context);
            CHECK(selection.kind == pixel_town::LocationStoryEventKind::incident);
            CHECK_FALSE(selection.script.lines.empty());
        }
    }
}

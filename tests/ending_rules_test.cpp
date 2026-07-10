#include <doctest/doctest.h>

#include "core/ending_rules.hpp"

namespace {

pixel_town::EndingConfig ending_test_config() {
    auto config = pixel_town::default_ending_config();
    config.failure_money_max = 10;
    config.failure_reputation_max = 10;
    config.star_money_min = 150;
    config.star_reputation_min = 60;
    config.star_knowledge_min = 50;
    config.star_tavern_wins_min = 3;
    config.money_route_min = 180;
    config.money_route_reputation_max = 40;
    config.reputation_route_min = 70;
    config.knowledge_route_min = 70;
    config.tavern_route_wins_min = 5;
    config.tavern_route_win_percent_min = 60;
    return config;
}

}  // namespace

TEST_CASE("failure ending has priority over otherwise strong single metrics") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 5;
    input.player.reputation = 5;
    input.player.knowledge = 100;
    input.tavern_wins = 10;

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.ending == pixel_town::MainEnding::business_failure);
    CHECK(result.growth_route == "重新起步路线");
    CHECK(result.reason.find("金钱") != std::string::npos);
    CHECK(result.reason.find("声望") != std::string::npos);
}

TEST_CASE("town star ending precedes all eligible single routes") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 200;
    input.player.reputation = 80;
    input.player.knowledge = 80;
    input.tavern_wins = 8;

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.ending == pixel_town::MainEnding::town_star);
    CHECK(result.growth_route == "均衡高成就路线");
    CHECK(result.reason.find("综合") != std::string::npos);
}

TEST_CASE("high money with low reputation selects the money route ending") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 200;
    input.player.reputation = 20;
    input.player.knowledge = 10;

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.ending == pixel_town::MainEnding::money_machine);
    CHECK(result.growth_route == "赚钱路线");
    CHECK(result.reason.find("金钱") != std::string::npos);
}

TEST_CASE("high reputation selects the popular resident ending") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 80;
    input.player.reputation = 80;
    input.player.knowledge = 10;

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.ending == pixel_town::MainEnding::popular_resident);
    CHECK(result.growth_route == "声望路线");
    CHECK(result.reason.find("声望") != std::string::npos);
}

TEST_CASE("high knowledge selects the library star ending") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 80;
    input.player.reputation = 20;
    input.player.knowledge = 80;

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.ending == pixel_town::MainEnding::library_star);
    CHECK(result.growth_route == "知识路线");
    CHECK(result.reason.find("知识") != std::string::npos);
}

TEST_CASE("strong tavern record selects the tavern legend ending") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 80;
    input.player.reputation = 20;
    input.player.knowledge = 20;
    input.tavern_wins = 6;
    input.tavern_losses = 2;

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.ending == pixel_town::MainEnding::tavern_legend);
    CHECK(result.growth_route == "娱乐路线");
    CHECK(result.reason.find("酒馆") != std::string::npos);
}

TEST_CASE("the most prominent normalized single route wins conflicts") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 80;
    input.player.reputation = 75;
    input.player.knowledge = 100;

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.ending == pixel_town::MainEnding::library_star);
    CHECK(result.reason.find("知识") != std::string::npos);
}

TEST_CASE("normalized single route ties use a stable documented order") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 80;
    input.player.reputation = config.reputation_route_min;
    input.player.knowledge = config.knowledge_route_min;

    const auto first = pixel_town::settle_ending(input, config);
    const auto second = pixel_town::settle_ending(input, config);

    REQUIRE(first.accepted);
    CHECK(first.ending == pixel_town::MainEnding::popular_resident);
    CHECK(second.ending == first.ending);
    CHECK(second.reason == first.reason);
}

TEST_CASE("ordinary ending is the fallback when no route threshold is met") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 60;
    input.player.reputation = 30;
    input.player.knowledge = 30;
    input.tavern_wins = 1;
    input.tavern_losses = 1;

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.ending == pixel_town::MainEnding::ordinary_newcomer);
    CHECK(result.growth_route == "均衡路线");
    CHECK(result.reason.find("普通区间") != std::string::npos);
}

TEST_CASE("inventory liquidation money participates in ending selection") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 176;
    input.player.reputation = 20;
    input.store_inventory = {{"umbrella", 2}};

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.inventory_cash == 6);
    CHECK(result.ending == pixel_town::MainEnding::money_machine);
}

TEST_CASE("ending settlement rejects invalid configuration and unknown inventory") {
    auto config = ending_test_config();
    config.inventory_liquidation_percent = 101;
    CHECK_FALSE(pixel_town::settle_ending({}, config).accepted);

    config = ending_test_config();
    pixel_town::EndingInput input;
    input.store_inventory = {{"unknown", 1}};
    const auto unknown = pixel_town::settle_ending(input, config);
    CHECK_FALSE(unknown.accepted);
    CHECK(unknown.message.find("无法清算") != std::string::npos);
}

TEST_CASE("inventory liquidation uses total configured cost with deterministic rounding") {
    const auto config = ending_test_config();
    pixel_town::EndingInput input;
    input.player.money = 50;
    input.store_inventory = {{"umbrella", 2}, {"soda", 3}};

    const auto result = pixel_town::settle_ending(input, config);

    REQUIRE(result.accepted);
    CHECK(result.inventory_cash == 10);
}

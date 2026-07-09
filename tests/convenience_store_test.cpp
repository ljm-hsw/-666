#include <doctest/doctest.h>

#include "locations/convenience_store.hpp"

TEST_CASE("store decision rejects purchases beyond available cash") {
    const auto config = pixel_town::store::default_store_config();
    pixel_town::store::PurchasePlan plan;
    plan.quantities = {{"umbrella", 20}};

    const auto validation = pixel_town::store::validate_purchase_plan(config, plan, 50);

    CHECK_FALSE(validation.allowed);
    CHECK(validation.message.find("现金") != std::string::npos);
}

TEST_CASE("store purchase plan costs each product independently") {
    const auto config = pixel_town::store::default_store_config();
    pixel_town::store::PurchasePlan plan;
    plan.quantities = {{"umbrella", 1}, {"soda", 3}, {"bento", 2}};

    const auto validation = pixel_town::store::validate_purchase_plan(config, plan, 50);

    REQUIRE(validation.allowed);
    CHECK(validation.purchase_cost == 1 * 6 + 3 * 3 + 2 * 5);
}

TEST_CASE("store demand is deterministic and price sensitive") {
    const auto config = pixel_town::store::default_store_config();
    pixel_town::store::StoreInventory inventory = {{"umbrella", 4}, {"soda", 4}};
    pixel_town::store::PurchasePlan plan;
    pixel_town::store::DailyStoreContext context;
    context.seed = 20260709;
    context.weather = "小雨";
    context.event = "便利店零食更受欢迎";

    auto low_prices = pixel_town::store::default_price_plan(config);
    auto high_prices = low_prices;
    high_prices.tiers["umbrella"] = pixel_town::store::PriceTier::high;
    low_prices.tiers["umbrella"] = pixel_town::store::PriceTier::low;

    const auto first = pixel_town::store::simulate_sales(
        config, inventory, plan, low_prices, context, 50);
    const auto second = pixel_town::store::simulate_sales(
        config, inventory, plan, low_prices, context, 50);
    const auto expensive = pixel_town::store::simulate_sales(
        config, inventory, plan, high_prices, context, 50);

    REQUIRE(first.accepted);
    REQUIRE(second.accepted);
    REQUIRE(expensive.accepted);
    CHECK(first.sold.at("umbrella") == second.sold.at("umbrella"));
    CHECK(first.sold.at("umbrella") >= expensive.sold.at("umbrella"));
}

TEST_CASE("store can sell existing inventory without new purchase") {
    const auto config = pixel_town::store::default_store_config();
    pixel_town::store::StoreInventory inventory = {{"umbrella", 3}};
    pixel_town::store::PurchasePlan no_purchase;
    const auto prices = pixel_town::store::default_price_plan(config);
    pixel_town::store::DailyStoreContext context;
    context.seed = 42;
    context.weather = "小雨";
    context.event = "小镇节奏平稳";

    const auto settlement = pixel_town::store::simulate_sales(
        config, inventory, no_purchase, prices, context, 50);

    REQUIRE(settlement.accepted);
    CHECK(settlement.purchase_cost == 0);
    CHECK(settlement.revenue >= 0);
    CHECK(settlement.sold.at("umbrella") <= 3);
    CHECK(settlement.remaining_inventory.at("umbrella") >= 0);
}

TEST_CASE("store weather and event affect matching products") {
    const auto config = pixel_town::store::default_store_config();
    pixel_town::store::StoreInventory inventory = {{"umbrella", 10}, {"soda", 10}};
    pixel_town::store::PurchasePlan no_purchase;
    const auto prices = pixel_town::store::default_price_plan(config);

    pixel_town::store::DailyStoreContext rainy;
    rainy.seed = 7;
    rainy.weather = "小雨";
    rainy.event = "小镇节奏平稳";
    pixel_town::store::DailyStoreContext snack_event = rainy;
    snack_event.weather = "晴天";
    snack_event.event = "便利店零食更受欢迎";

    const auto rainy_result = pixel_town::store::simulate_sales(
        config, inventory, no_purchase, prices, rainy, 50);
    const auto snack_result = pixel_town::store::simulate_sales(
        config, inventory, no_purchase, prices, snack_event, 50);

    REQUIRE(rainy_result.accepted);
    REQUIRE(snack_result.accepted);
    CHECK(rainy_result.sold.at("umbrella") >= snack_result.sold.at("umbrella"));
    CHECK(snack_result.sold.at("soda") >= rainy_result.sold.at("soda"));
}

TEST_CASE("store settlement preserves inventory and creates one action result") {
    const auto config = pixel_town::store::default_store_config();
    pixel_town::store::StoreInventory inventory = {{"umbrella", 1}, {"bento", 0}};
    pixel_town::store::PurchasePlan plan;
    plan.quantities = {{"umbrella", 2}, {"bento", 3}};
    const auto prices = pixel_town::store::default_price_plan(config);
    pixel_town::store::DailyStoreContext context;
    context.seed = 7;
    context.weather = "晴天";
    context.event = "小镇节奏平稳";

    const auto settlement = pixel_town::store::simulate_sales(
        config, inventory, plan, prices, context, 50);

    REQUIRE(settlement.accepted);
    CHECK(settlement.purchase_cost > 0);
    CHECK(settlement.revenue >= 0);
    CHECK(settlement.profit == settlement.revenue - settlement.purchase_cost);
    CHECK(settlement.remaining_inventory.at("umbrella") >= 0);

    const auto result = pixel_town::store::build_store_action_result(settlement, 12);
    CHECK(result.result_id == 12);
    CHECK(result.slot == pixel_town::ActionSlot::day);
    CHECK(result.location == pixel_town::Location::convenience_store);
    CHECK(result.delta.money == settlement.profit);
    CHECK(result.has_store_inventory_update);
    CHECK(result.summary.find("账本") != std::string::npos);
}

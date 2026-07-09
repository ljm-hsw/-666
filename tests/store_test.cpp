#include <doctest/doctest.h>

#include "locations/store/store.hpp"

using namespace pixel_town;

TEST_CASE("default store inventory is empty") {
    const auto inv = default_store_inventory();
    CHECK(inv.empty());
    CHECK(inv.total_stock() == 0);
}

TEST_CASE("add_stock increases quantity") {
    auto inv = default_store_inventory();
    inv.add_stock(1, 5);
    CHECK(inv.total_stock() == 5);
    CHECK(inv.find(1).quantity == 5);
    inv.add_stock(1, 3);
    CHECK(inv.find(1).quantity == 8);
    CHECK(inv.total_stock() == 8);
}

TEST_CASE("add_stock respects max limit") {
    auto inv = default_store_inventory();
    inv.add_stock(1, 60);
    CHECK(inv.find(1).quantity == max_stock_per_product);
}

TEST_CASE("can_afford_purchase with per-product decisions") {
    StoreDailyDecision d{};
    d.products[0].purchase_quantity = 3;
    d.products[1].purchase_quantity = 2;
    CHECK(can_afford_purchase(20, d));
    CHECK(can_afford_purchase(18, d));
    CHECK_FALSE(can_afford_purchase(17, d));
    CHECK_FALSE(can_afford_purchase(0, d));
}

TEST_CASE("total_purchase_cost calculates correctly") {
    StoreDailyDecision d{};
    d.products[0].purchase_quantity = 5;
    d.products[4].purchase_quantity = 10;
    CHECK(total_purchase_cost(d) == 40);
}

TEST_CASE("simulate_store_day with empty inventory and purchase") {
    const auto inv = default_store_inventory();
    StoreDailyDecision decision{};
    decision.products[0].purchase_quantity = 5;
    decision.products[0].tier = PriceTier::standard;
    const DayPrompt prompt{"Sunny", "Town is calm"};
    const auto result = simulate_store_day(inv, decision, prompt, 50, 12345);
    CHECK(result.total_cost > 0);
    CHECK(result.total_sold >= 0);
    CHECK(result.total_revenue >= 0);
    CHECK(result.profit == result.total_revenue - result.total_cost);
    CHECK_FALSE(result.summary.empty());
}

TEST_CASE("simulate_store_day is deterministic with same seed") {
    const auto inv = default_store_inventory();
    StoreDailyDecision decision{};
    decision.products[0].purchase_quantity = 5;
    decision.products[0].tier = PriceTier::standard;
    const DayPrompt prompt{"Sunny", "Town is calm"};
    const auto r1 = simulate_store_day(inv, decision, prompt, 50, 42);
    const auto r2 = simulate_store_day(inv, decision, prompt, 50, 42);
    CHECK(r1.total_cost == r2.total_cost);
    CHECK(r1.total_revenue == r2.total_revenue);
    CHECK(r1.total_sold == r2.total_sold);
    CHECK(r1.profit == r2.profit);
    CHECK(r1.summary == r2.summary);
}

TEST_CASE("low price tier increases demand") {
    const auto inv = default_store_inventory();
    StoreDailyDecision d_low{};
    d_low.products[0].purchase_quantity = 10;
    d_low.products[0].tier = PriceTier::low;
    StoreDailyDecision d_std{};
    d_std.products[0].purchase_quantity = 10;
    d_std.products[0].tier = PriceTier::standard;
    const DayPrompt prompt{"Sunny", "Town is calm"};
    const auto r_std = simulate_store_day(inv, d_std, prompt, 50, 100);
    const auto r_low = simulate_store_day(inv, d_low, prompt, 50, 100);
    CHECK(r_low.total_sold >= r_std.total_sold);
}

TEST_CASE("high price tier decreases demand") {
    const auto inv = default_store_inventory();
    StoreDailyDecision d_high{};
    d_high.products[0].purchase_quantity = 10;
    d_high.products[0].tier = PriceTier::high;
    StoreDailyDecision d_std{};
    d_std.products[0].purchase_quantity = 10;
    d_std.products[0].tier = PriceTier::standard;
    const DayPrompt prompt{"Sunny", "Town is calm"};
    const auto r_std = simulate_store_day(inv, d_std, prompt, 50, 100);
    const auto r_high = simulate_store_day(inv, d_high, prompt, 50, 100);
    CHECK(r_high.total_sold <= r_std.total_sold);
}

TEST_CASE("cannot afford purchase returns zero result") {
    const auto inv = default_store_inventory();
    StoreDailyDecision expensive{};
    expensive.products[0].purchase_quantity = 99;
    const DayPrompt prompt{"Sunny", "Town is calm"};
    const auto result = simulate_store_day(inv, expensive, prompt, 5, 42);
    CHECK(result.total_cost == 0);
    CHECK(result.total_sold == 0);
    CHECK(result.total_revenue == 0);
    CHECK(result.summary == "资金不足以支付本次进货。");
}

TEST_CASE("zero purchase uses existing inventory only") {
    auto inv = default_store_inventory();
    inv.add_stock(1, 5);
    StoreDailyDecision no_purchase{};
    no_purchase.products[0].tier = PriceTier::standard;
    const DayPrompt prompt{"Sunny", "Town is calm"};
    const auto result = simulate_store_day(inv, no_purchase, prompt, 50, 42);
    CHECK(result.total_cost == 0);
    CHECK(result.total_sold >= 0);
    CHECK(result.total_sold <= 5);
    CHECK(result.profit == result.total_revenue);
}

TEST_CASE("store_products catalog has 6 products") {
    CHECK(store_products.size() == 6);
    CHECK(store_products[0].id == 1);
    CHECK(store_products[5].id == 6);
}

TEST_CASE("weather affects product demand independently") {
    const auto inv = default_store_inventory();
    StoreDailyDecision decision{};
    decision.products[0].purchase_quantity = 20;
    decision.products[0].tier = PriceTier::standard;
    decision.products[3].purchase_quantity = 20;
    decision.products[3].tier = PriceTier::standard;

    const DayPrompt sunny{"晴天", "Town is calm"};
    const DayPrompt rainy{"小雨", "Town is calm"};

    const auto r_sunny = simulate_store_day(inv, decision, sunny, 200, 42);
    const auto r_rainy = simulate_store_day(inv, decision, rainy, 200, 42);

    CHECK(r_rainy.product_results[0].quantity_sold <= r_sunny.product_results[0].quantity_sold);
    CHECK(r_rainy.product_results[3].quantity_sold >= r_sunny.product_results[3].quantity_sold);
}

TEST_CASE("event affects matching products") {
    const auto inv = default_store_inventory();
    StoreDailyDecision decision{};
    decision.products[4].purchase_quantity = 20;
    decision.products[4].tier = PriceTier::standard;

    const DayPrompt normal{"晴天", "小镇节奏平稳"};
    const DayPrompt snack_event{"晴天", "便利店零食更受欢迎"};

    const auto r_normal = simulate_store_day(inv, decision, normal, 200, 42);
    const auto r_event = simulate_store_day(inv, decision, snack_event, 200, 42);

    CHECK(r_event.product_results[4].quantity_sold >= r_normal.product_results[4].quantity_sold);
}

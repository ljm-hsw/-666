#pragma once

#include <array>
#include <string>

namespace pixel_town {

enum class PriceTier {
    low,
    standard,
    high,
};

struct Product {
    int id{0};
    const char* display_name{""};
    int base_demand{10};
    int base_cost{3};
    int standard_price{5};
};

struct ProductStock {
    int product_id{0};
    int quantity{0};
    PriceTier tier{PriceTier::standard};

    [[nodiscard]] bool operator==(const ProductStock& other) const noexcept {
        return product_id == other.product_id && quantity == other.quantity && tier == other.tier;
    }
    [[nodiscard]] bool operator!=(const ProductStock& other) const noexcept {
        return !(*this == other);
    }
};

struct DayPrompt {
    std::string weather;
    std::string event;
};

struct ProductDecision {
    int purchase_quantity{0};
    PriceTier tier{PriceTier::standard};
};

struct StoreDailyDecision {
    std::array<ProductDecision, 6> products{};
};

struct ProductSaleResult {
    int product_id{0};
    int quantity_sold{0};
    int revenue{0};
    int remaining{0};
};

struct StoreSettlement {
    int total_cost{0};
    int total_revenue{0};
    int total_sold{0};
    int profit{0};
    int stamina_cost{12};
    int reputation_gain{2};
    int mood_change{-1};
    std::string summary;
    std::array<ProductSaleResult, 6> product_results{};
};

struct StoreInventory {
    std::array<ProductStock, 6> stocks{};

    [[nodiscard]] const ProductStock& find(int product_id) const;
    void add_stock(int product_id, int quantity);
    [[nodiscard]] int total_stock() const;
    [[nodiscard]] bool empty() const;
};

[[nodiscard]] int selling_price(const Product& product, PriceTier tier);
[[nodiscard]] int total_purchase_cost(const StoreDailyDecision& decision);
[[nodiscard]] StoreSettlement simulate_store_day(
    const StoreInventory& current_inventory,
    const StoreDailyDecision& decision,
    const DayPrompt& prompt,
    int player_money,
    unsigned int seed);
[[nodiscard]] bool can_afford_purchase(
    int player_money, const StoreDailyDecision& decision);
[[nodiscard]] StoreInventory default_store_inventory();
[[nodiscard]] StoreDailyDecision default_store_decision();

constexpr std::array<Product, 6> store_products{{
    Product{1, "冷饮", 12, 4, 6},
    Product{2, "热饮", 10, 3, 5},
    Product{3, "便当", 8, 5, 8},
    Product{4, "雨具", 5, 6, 10},
    Product{5, "零食", 15, 2, 4},
    Product{6, "杂志", 6, 3, 5},
}};

constexpr int max_stock_per_product = 10;

}  // namespace pixel_town

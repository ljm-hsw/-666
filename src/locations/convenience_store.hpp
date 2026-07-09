#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/game_session.hpp"

namespace pixel_town::store {

enum class PriceTier {
    low,
    standard,
    high,
};

struct ProductConfig {
    std::string id;
    std::string name;
    int unit_cost{0};
    int low_price{0};
    int standard_price{0};
    int high_price{0};
    int base_demand{0};
    int rainy_bonus{0};
    int snack_event_bonus{0};
};

struct StoreConfig {
    std::vector<ProductConfig> products;
    int stamina_cost{12};
    int base_reputation{1};
    int profit_reputation_step{20};
    int base_mood_change{-1};
    int max_stock_per_product{10};
};

using StoreInventory = std::map<std::string, int>;

struct PurchasePlan {
    std::map<std::string, int> quantities;
};

struct PricePlan {
    std::map<std::string, PriceTier> tiers;
};

struct DailyStoreContext {
    int day{1};
    unsigned int seed{0};
    std::string weather;
    std::string event;
};

struct ValidationResult {
    bool allowed{false};
    std::string message;
    int purchase_cost{0};
};

struct StoreSettlement {
    bool accepted{false};
    std::string message;
    std::map<std::string, int> purchased;
    std::map<std::string, int> sold;
    StoreInventory remaining_inventory;
    int purchase_cost{0};
    int revenue{0};
    int profit{0};
    std::string summary;
};

[[nodiscard]] StoreConfig default_store_config();
[[nodiscard]] PricePlan default_price_plan(const StoreConfig& config);
[[nodiscard]] const ProductConfig* find_product(const StoreConfig& config,
                                                const std::string& item_id);
[[nodiscard]] int price_for_tier(const ProductConfig& product, PriceTier tier);
[[nodiscard]] ValidationResult validate_purchase_plan(const StoreConfig& config,
                                                      const PurchasePlan& plan,
                                                      int available_cash);
[[nodiscard]] StoreSettlement simulate_sales(const StoreConfig& config,
                                             const StoreInventory& starting_inventory,
                                             const PurchasePlan& purchase_plan,
                                             const PricePlan& price_plan,
                                             const DailyStoreContext& context,
                                             int available_cash);
[[nodiscard]] ActionResult build_store_action_result(const StoreSettlement& settlement,
                                                     int result_id);

}  // namespace pixel_town::store

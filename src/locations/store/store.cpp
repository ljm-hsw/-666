#include "locations/store/store.hpp"

#include <cstdlib>
#include <numeric>
#include <sstream>

namespace pixel_town {
namespace {

unsigned int lcg_next(unsigned int& state) {
    state = state * 1103515245U + 12345U;
    return state;
}

float price_multiplier(PriceTier tier) {
    switch (tier) {
        case PriceTier::low:
            return 0.8F;
        case PriceTier::standard:
            return 1.0F;
        case PriceTier::high:
            return 1.3F;
    }
    return 1.0F;
}

float weather_modifier_for_product(int product_id, const std::string& weather) {
    const bool is_sunny = (weather == "Sunny" || weather == "Qingtian" || weather == "晴天");
    const bool is_rainy = (weather == "Rainy" || weather == "Xiaoyu" || weather == "小雨");

    switch (product_id) {
        case 1:
            if (is_sunny) return 1.3F;
            if (is_rainy) return 0.8F;
            return 1.0F;
        case 2:
            if (is_rainy) return 1.2F;
            if (is_sunny) return 0.9F;
            return 1.0F;
        case 3:
            if (is_sunny) return 1.1F;
            return 1.0F;
        case 4:
            if (is_rainy) return 3.0F;
            if (is_sunny) return 0.5F;
            return 1.0F;
        case 5:
            return 1.0F;
        case 6:
            if (is_rainy) return 1.15F;
            return 1.0F;
    }
    return 1.0F;
}

float event_modifier_for_product(int product_id, const std::string& event_text) {
    switch (product_id) {
        case 3:
            if (event_text.find("餐馆") != std::string::npos ||
                event_text.find("restaurant") != std::string::npos ||
                event_text.find("客流") != std::string::npos) {
                return 1.3F;
            }
            return 1.0F;
        case 5:
            if (event_text.find("零食") != std::string::npos ||
                event_text.find("Snack") != std::string::npos ||
                event_text.find("便利店") != std::string::npos ||
                event_text.find("store") != std::string::npos) {
                return 1.5F;
            }
            return 1.0F;
        case 6:
            if (event_text.find("图书馆") != std::string::npos ||
                event_text.find("library") != std::string::npos ||
                event_text.find("读者") != std::string::npos) {
                return 1.3F;
            }
            return 1.0F;
        case 1:
        case 2:
        case 4:
            return 1.0F;
    }
    return 1.0F;
}

float tier_demand_modifier(PriceTier tier) {
    switch (tier) {
        case PriceTier::low:
            return 1.3F;
        case PriceTier::standard:
            return 1.0F;
        case PriceTier::high:
            return 0.6F;
    }
    return 1.0F;
}

}  // namespace

int selling_price(const Product& product, PriceTier tier) {
    return static_cast<int>(product.standard_price * price_multiplier(tier));
}

const ProductStock& StoreInventory::find(int product_id) const {
    for (const auto& stock : stocks) {
        if (stock.product_id == product_id) {
            return stock;
        }
    }
    return stocks[0];
}

void StoreInventory::add_stock(int product_id, int quantity) {
    for (auto& stock : stocks) {
        if (stock.product_id == product_id) {
            stock.quantity += quantity;
            if (stock.quantity > max_stock_per_product) {
                stock.quantity = max_stock_per_product;
            }
            return;
        }
    }
}

int StoreInventory::total_stock() const {
    return std::accumulate(stocks.begin(), stocks.end(), 0,
                           [](int sum, const ProductStock& s) { return sum + s.quantity; });
}

bool StoreInventory::empty() const {
    return total_stock() == 0;
}

StoreInventory default_store_inventory() {
    return StoreInventory{{
        ProductStock{1, 0, PriceTier::standard},
        ProductStock{2, 0, PriceTier::standard},
        ProductStock{3, 0, PriceTier::standard},
        ProductStock{4, 0, PriceTier::standard},
        ProductStock{5, 0, PriceTier::standard},
        ProductStock{6, 0, PriceTier::standard},
    }};
}

StoreDailyDecision default_store_decision() {
    return StoreDailyDecision{};
}

int total_purchase_cost(const StoreDailyDecision& decision) {
    int cost = 0;
    for (std::size_t i = 0; i < store_products.size(); ++i) {
        cost += store_products[i].base_cost * decision.products[i].purchase_quantity;
    }
    return cost;
}

bool can_afford_purchase(int player_money, const StoreDailyDecision& decision) {
    return player_money >= total_purchase_cost(decision);
}

StoreSettlement simulate_store_day(
    const StoreInventory& current_inventory,
    const StoreDailyDecision& decision,
    const DayPrompt& prompt,
    int player_money,
    unsigned int seed) {

    StoreSettlement result{};

    const int cost = total_purchase_cost(decision);
    if (cost > player_money) {
        result.summary = "资金不足以支付本次进货。";
        return result;
    }
    result.total_cost = cost;

    StoreInventory new_inventory = current_inventory;
    for (std::size_t i = 0; i < store_products.size(); ++i) {
        new_inventory.stocks[i].quantity += decision.products[i].purchase_quantity;
        if (new_inventory.stocks[i].quantity > max_stock_per_product) {
            new_inventory.stocks[i].quantity = max_stock_per_product;
        }
        new_inventory.stocks[i].tier = decision.products[i].tier;
    }

    unsigned int rng = seed;
    int total_reputation = 0;

    for (std::size_t i = 0; i < store_products.size(); ++i) {
        const auto& product = store_products[i];
        auto& stock = new_inventory.stocks[i];

        result.product_results[i].product_id = product.id;
        result.product_results[i].remaining = stock.quantity;

        if (stock.quantity <= 0) {
            continue;
        }

        float demand = static_cast<float>(product.base_demand);
        demand *= weather_modifier_for_product(product.id, prompt.weather);
        demand *= event_modifier_for_product(product.id, prompt.event);
        demand *= tier_demand_modifier(decision.products[i].tier);

        const unsigned int raw = lcg_next(rng);
        const float jitter = 0.85F + static_cast<float>(raw % 31) / 100.0F;
        demand *= jitter;

        const int actual_demand = static_cast<int>(demand);
        const int sold = (actual_demand < stock.quantity) ? actual_demand : stock.quantity;

        const int price = selling_price(product, decision.products[i].tier);
        const int revenue = sold * price;

        result.product_results[i].quantity_sold = sold;
        result.product_results[i].revenue = revenue;
        result.product_results[i].remaining = stock.quantity - sold;

        result.total_revenue += revenue;
        result.total_sold += sold;
        stock.quantity -= sold;

        if (decision.products[i].tier == PriceTier::high && sold > 0) {
            total_reputation -= 1;
        } else if (decision.products[i].tier == PriceTier::low && sold > 0) {
            total_reputation += 1;
        }
    }

    result.profit = result.total_revenue - result.total_cost;

    result.reputation_gain = 2 + total_reputation;
    if (result.reputation_gain < -3) result.reputation_gain = -3;
    if (result.reputation_gain > 5) result.reputation_gain = 5;

    if (result.profit > 10) {
        result.mood_change = 3;
    } else if (result.profit > 0) {
        result.mood_change = 1;
    } else if (result.profit > -10) {
        result.mood_change = -2;
    } else {
        result.mood_change = -5;
    }

    result.stamina_cost = 12;

    std::ostringstream oss;
    if (result.total_sold > 0) {
        oss << "便利店营业结束：共售出 " << result.total_sold << " 件商品，"
            << "营收 " << result.total_revenue << "，"
            << "进货成本 " << result.total_cost << "，"
            << (result.profit >= 0 ? "盈利 " : "亏损 ") << result.profit << "。";
    } else {
        oss << "便利店营业结束：今日无销售。进货成本 " << result.total_cost << "，亏损 "
            << result.total_cost << "。";
    }
    result.summary = oss.str();

    return result;
}

}  // namespace pixel_town

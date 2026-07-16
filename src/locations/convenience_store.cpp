// 便利店需求、销售和库存结果计算；全局现金由 GameSession 统一应用。
#include "locations/convenience_store.hpp"

#include <algorithm>
#include <sstream>

namespace pixel_town::store {
namespace {

unsigned int mix(unsigned int value) {
    value += 0x9e3779b9U;
    value ^= value >> 16;
    value *= 0x85ebca6bU;
    value ^= value >> 13;
    value *= 0xc2b2ae35U;
    value ^= value >> 16;
    return value;
}

int tier_demand_modifier(PriceTier tier) {
    switch (tier) {
        case PriceTier::low:
            return 2;
        case PriceTier::standard:
            return 0;
        case PriceTier::high:
            return -2;
    }
    return 0;
}

bool is_rainy(const std::string& weather) {
    return weather.find("雨") != std::string::npos;
}

bool is_snack_event(const std::string& event) {
    return event.find("零食") != std::string::npos ||
           event.find("便利店") != std::string::npos;
}

int quantity_for(const StoreInventory& inventory, const std::string& item_id) {
    const auto found = inventory.find(item_id);
    return found == inventory.end() ? 0 : found->second;
}

PriceTier tier_for(const PricePlan& plan, const std::string& item_id) {
    const auto found = plan.tiers.find(item_id);
    return found == plan.tiers.end() ? PriceTier::standard : found->second;
}

int demand_for(const ProductConfig& product, PriceTier tier,
               const DailyStoreContext& context) {
    int demand = product.base_demand + tier_demand_modifier(tier);
    if (is_rainy(context.weather)) {
        demand += product.rainy_bonus;
    }
    if (is_snack_event(context.event)) {
        demand += product.snack_event_bonus;
    }
    const unsigned int noise =
        mix(context.seed ^ static_cast<unsigned int>(context.day * 131) ^
            mix(static_cast<unsigned int>(product.id.size() * 17)));
    demand += static_cast<int>(noise % 3U) - 1;
    return std::max(0, demand);
}

std::string build_store_summary(const StoreSettlement& settlement) {
    std::ostringstream output;
    output << "便利店经营完成：进货成本 " << settlement.purchase_cost
           << "，销售收入 " << settlement.revenue
           << "，利润 " << settlement.profit << "。";
    output << "雨伞、汽水和便当的数量都记进了账本。";
    if (settlement.profit > 0) {
        output << "店主在账本边角画了个小勾。";
    } else {
        output << "账本最后留了几处空白，明天还能再调价格。";
    }
    return output.str();
}

}  // namespace

StoreConfig default_store_config() {
    StoreConfig config;
    config.products = {
        {"umbrella", "雨伞", 6, 8, 10, 12, 2, 5, 0},
        {"soda", "汽水", 3, 5, 6, 8, 4, -1, 3},
        {"bento", "便当", 5, 7, 9, 11, 3, 1, 2},
        {"coffee", "咖啡", 4, 6, 8, 10, 2, 1, 1},
    };
    return config;
}

PricePlan default_price_plan(const StoreConfig& config) {
    PricePlan plan;
    for (const auto& product : config.products) {
        plan.tiers[product.id] = PriceTier::standard;
    }
    return plan;
}

const ProductConfig* find_product(const StoreConfig& config, const std::string& item_id) {
    for (const auto& product : config.products) {
        if (product.id == item_id) {
            return &product;
        }
    }
    return nullptr;
}

int price_for_tier(const ProductConfig& product, PriceTier tier) {
    switch (tier) {
        case PriceTier::low:
            return product.low_price;
        case PriceTier::standard:
            return product.standard_price;
        case PriceTier::high:
            return product.high_price;
    }
    return product.standard_price;
}

ValidationResult validate_purchase_plan(const StoreConfig& config,
                                        const StoreInventory& starting_inventory,
                                        const PurchasePlan& plan,
                                        int available_cash) {
    ValidationResult result;
    for (const auto& [item_id, quantity] : plan.quantities) {
        const auto* product = find_product(config, item_id);
        if (product == nullptr) {
            result.message = "未知商品：" + item_id;
            return result;
        }
        if (quantity < 0) {
            result.message = "进货数量不能为负数。";
            return result;
        }
        if (quantity_for(starting_inventory, item_id) + quantity >
            config.max_stock_per_product) {
            result.message = "进货后库存不能超过单品上限。";
            return result;
        }
        result.purchase_cost += product->unit_cost * quantity;
    }
    if (result.purchase_cost > available_cash) {
        result.message = "现金不足，不能完成这次进货。";
        return result;
    }
    result.allowed = true;
    return result;
}

StoreSettlement simulate_sales(const StoreConfig& config,
                               const StoreInventory& starting_inventory,
                               const PurchasePlan& purchase_plan,
                               const PricePlan& price_plan,
                               const DailyStoreContext& context,
                               int available_cash) {
    StoreSettlement settlement;
    const auto validation =
        validate_purchase_plan(config, starting_inventory, purchase_plan, available_cash);
    settlement.purchase_cost = validation.purchase_cost;
    if (!validation.allowed) {
        settlement.message = validation.message;
        return settlement;
    }

    settlement.remaining_inventory = starting_inventory;
    for (const auto& product : config.products) {
        const int purchased = quantity_for(purchase_plan.quantities, product.id);
        settlement.purchased[product.id] = purchased;
        settlement.remaining_inventory[product.id] =
            quantity_for(settlement.remaining_inventory, product.id) + purchased;
    }

    for (const auto& product : config.products) {
        const PriceTier tier = tier_for(price_plan, product.id);
        const int available = quantity_for(settlement.remaining_inventory, product.id);
        const int sold = std::min(available, demand_for(product, tier, context));
        settlement.sold[product.id] = sold;
        settlement.remaining_inventory[product.id] = available - sold;
        settlement.revenue += sold * price_for_tier(product, tier);
    }

    settlement.profit = settlement.revenue - settlement.purchase_cost;
    settlement.accepted = true;
    settlement.summary = build_store_summary(settlement);
    return settlement;
}

ActionResult build_store_action_result(const StoreConfig& config,
                                        const StoreSettlement& settlement,
                                        int result_id) {
    ActionResult result;
    result.result_id = result_id;
    result.slot = ActionSlot::day;
    result.location = Location::convenience_store;
    if (!settlement.accepted) {
        result.outcome = ActionOutcome::abandoned;
        result.summary = settlement.message;
        return result;
    }
    result.outcome = ActionOutcome::completed;
    result.delta.money = settlement.profit;
    result.delta.stamina = -config.stamina_cost;
    const int reputation_step = std::max(1, config.profit_reputation_step);
    result.delta.reputation =
        config.base_reputation + std::max(0, settlement.profit / reputation_step);
    result.delta.mood = config.base_mood_change;
    result.summary = settlement.summary;
    result.has_store_inventory_update = true;
    for (const auto& [item_id, quantity] : settlement.remaining_inventory) {
        result.store_inventory_after.push_back(StoreInventoryItem{item_id, quantity});
    }
    return result;
}

const char* convenience_store_glyphs() {
    return "0123456789-未知商品进货数量不能为负数进货后库存不能超过单品上限"
           "现金不足，不能完成这次进货便利店经营完成：进货成本销售收入利润。"
           "雨伞、汽水和便当的数量都记进了账本店主在账本边角画了个小勾"
           "账本最后留了几处空白，明天还能再调价格咖啡";
}

}  // namespace pixel_town::store

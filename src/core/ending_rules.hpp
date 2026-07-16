// 纯规则结局模块：读取最终状态和库存，返回唯一主结局及可解释依据。
#pragma once

#include <string>
#include <vector>

#include "core/game_session.hpp"

namespace pixel_town {

enum class MainEnding {
    town_star,
    money_machine,
    popular_resident,
    library_star,
    tavern_legend,
    ordinary_newcomer,
    business_failure,
};

struct InventoryLiquidationValue {
    std::string item_id;
    int unit_cost{0};
};

struct EndingConfig {
    int inventory_liquidation_percent{50};
    std::vector<InventoryLiquidationValue> inventory_values;
    int failure_money_max{20};
    int failure_reputation_max{10};
    int star_money_min{150};
    int star_reputation_min{60};
    int star_knowledge_min{50};
    int star_tavern_wins_min{3};
    int money_route_min{180};
    int money_route_reputation_max{40};
    int reputation_route_min{70};
    int knowledge_route_min{70};
    int tavern_route_wins_min{5};
    int tavern_route_win_percent_min{60};
};

struct EndingInput {
    PlayerState player;
    std::vector<StoreInventoryItem> store_inventory;
    int tavern_wins{0};
    int tavern_losses{0};
};

struct EndingSettlement {
    bool accepted{false};
    std::string message;
    int inventory_cash{0};
    MainEnding ending{MainEnding::ordinary_newcomer};
    std::string growth_route;
    std::string reason;
};

[[nodiscard]] EndingConfig default_ending_config();
[[nodiscard]] EndingSettlement settle_ending(const EndingInput& input,
                                             const EndingConfig& config);
[[nodiscard]] const char* main_ending_label(MainEnding ending);
[[nodiscard]] std::string ending_rules_glyphs();

}  // namespace pixel_town

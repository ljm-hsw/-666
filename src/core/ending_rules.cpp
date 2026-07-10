#include "core/ending_rules.hpp"

#include <array>
#include <limits>

namespace pixel_town {
namespace {

const InventoryLiquidationValue* find_value(
    const EndingConfig& config, const std::string& item_id) {
    for (const auto& value : config.inventory_values) {
        if (value.item_id == item_id) {
            return &value;
        }
    }
    return nullptr;
}

bool valid_config(const EndingConfig& config) {
    if (config.inventory_liquidation_percent < 0 ||
        config.inventory_liquidation_percent > 100 ||
        config.failure_money_max < 0 || config.failure_reputation_max < 0 ||
        config.star_money_min < 0 || config.star_reputation_min < 0 ||
        config.star_knowledge_min < 0 || config.star_tavern_wins_min < 0 ||
        config.money_route_min <= 0 || config.money_route_reputation_max < 0 ||
        config.reputation_route_min <= 0 || config.knowledge_route_min <= 0 ||
        config.tavern_route_wins_min <= 0 ||
        config.tavern_route_win_percent_min < 0 ||
        config.tavern_route_win_percent_min > 100) {
        return false;
    }
    for (std::size_t index = 0; index < config.inventory_values.size(); ++index) {
        const auto& value = config.inventory_values[index];
        if (value.item_id.empty() || value.unit_cost < 0) {
            return false;
        }
        for (std::size_t previous = 0; previous < index; ++previous) {
            if (config.inventory_values[previous].item_id == value.item_id) {
                return false;
            }
        }
    }
    return true;
}

struct RouteCandidate {
    bool eligible{false};
    double score{0.0};
    MainEnding ending{MainEnding::ordinary_newcomer};
    const char* growth_route{nullptr};
    const char* reason{nullptr};
};

}  // namespace

EndingConfig default_ending_config() {
    EndingConfig config;
    config.inventory_values = {
        {"umbrella", 6},
        {"soda", 3},
        {"bento", 5},
        {"coffee", 4},
    };
    return config;
}

const char* main_ending_label(MainEnding ending) {
    switch (ending) {
        case MainEnding::town_star:
            return "小镇明星经营者";
        case MainEnding::money_machine:
            return "冷酷赚钱机器";
        case MainEnding::popular_resident:
            return "受欢迎的普通居民";
        case MainEnding::library_star:
            return "图书馆之星";
        case MainEnding::tavern_legend:
            return "酒馆传说";
        case MainEnding::ordinary_newcomer:
            return "平凡小镇新人";
        case MainEnding::business_failure:
            return "经营失败者";
    }
    return "平凡小镇新人";
}

EndingSettlement settle_ending(const EndingInput& input,
                               const EndingConfig& config) {
    EndingSettlement settlement;
    if (!valid_config(config)) {
        settlement.message = "最终结局配置非法。";
        return settlement;
    }

    long long inventory_cost = 0;
    for (const auto& item : input.store_inventory) {
        const auto* value = find_value(config, item.item_id);
        if (value == nullptr || item.quantity < 0) {
            settlement.message = "最终库存包含无法清算的商品。";
            return settlement;
        }
        inventory_cost += static_cast<long long>(value->unit_cost) * item.quantity;
        if (inventory_cost > std::numeric_limits<int>::max()) {
            settlement.message = "最终库存清算金额超出范围。";
            return settlement;
        }
    }

    settlement.inventory_cash = static_cast<int>(
        inventory_cost * config.inventory_liquidation_percent / 100);
    const int final_money =
        input.player.money > 999 - settlement.inventory_cash
            ? 999
            : input.player.money + settlement.inventory_cash;
    if (final_money <= config.failure_money_max &&
        input.player.reputation <= config.failure_reputation_max) {
        settlement.ending = MainEnding::business_failure;
        settlement.growth_route = "重新起步路线";
        settlement.reason = "最终金钱和声望都较低。";
        settlement.accepted = true;
        return settlement;
    }
    if (final_money >= config.star_money_min &&
        input.player.reputation >= config.star_reputation_min &&
        input.player.knowledge >= config.star_knowledge_min &&
        input.tavern_wins >= config.star_tavern_wins_min) {
        settlement.ending = MainEnding::town_star;
        settlement.growth_route = "均衡高成就路线";
        settlement.reason = "金钱、声望、知识和酒馆表现都达到综合高成就标准。";
        settlement.accepted = true;
        return settlement;
    }
    const int tavern_games = input.tavern_wins + input.tavern_losses;
    const bool tavern_eligible =
        input.tavern_wins >= config.tavern_route_wins_min &&
        tavern_games > 0 &&
        input.tavern_wins * 100 >=
            tavern_games * config.tavern_route_win_percent_min;
    const std::array<RouteCandidate, 4> candidates{
        RouteCandidate{
            final_money >= config.money_route_min &&
                input.player.reputation <= config.money_route_reputation_max,
            static_cast<double>(final_money) / config.money_route_min,
            MainEnding::money_machine, "赚钱路线",
            "最终金钱突出，声望保持在较低区间。"},
        RouteCandidate{
            input.player.reputation >= config.reputation_route_min,
            static_cast<double>(input.player.reputation) /
                config.reputation_route_min,
            MainEnding::popular_resident, "声望路线",
            "最终声望是最突出的生活痕迹。"},
        RouteCandidate{
            input.player.knowledge >= config.knowledge_route_min,
            static_cast<double>(input.player.knowledge) /
                config.knowledge_route_min,
            MainEnding::library_star, "知识路线",
            "最终知识是最突出的生活痕迹。"},
        RouteCandidate{
            tavern_eligible,
            static_cast<double>(input.tavern_wins) /
                config.tavern_route_wins_min,
            MainEnding::tavern_legend, "娱乐路线",
            "酒馆胜场和胜率达到传说路线标准。"},
    };
    const RouteCandidate* selected = nullptr;
    for (const auto& candidate : candidates) {
        if (candidate.eligible &&
            (selected == nullptr || candidate.score > selected->score)) {
            selected = &candidate;
        }
    }
    if (selected != nullptr) {
        settlement.ending = selected->ending;
        settlement.growth_route = selected->growth_route;
        settlement.reason = selected->reason;
        settlement.accepted = true;
        return settlement;
    }
    settlement.ending = MainEnding::ordinary_newcomer;
    settlement.growth_route = "均衡路线";
    settlement.reason = "各项指标保持在普通区间。";
    settlement.accepted = true;
    return settlement;
}

std::string ending_rules_glyphs() {
    std::string glyphs;
    constexpr std::array endings{
        MainEnding::town_star,
        MainEnding::money_machine,
        MainEnding::popular_resident,
        MainEnding::library_star,
        MainEnding::tavern_legend,
        MainEnding::ordinary_newcomer,
        MainEnding::business_failure,
    };
    for (const MainEnding ending : endings) {
        glyphs += main_ending_label(ending);
    }
    glyphs += "重新起步路线均衡高成就路线赚钱路线声望路线知识路线娱乐路线均衡路线";
    glyphs += "最终金钱和声望都较低金钱声望知识和酒馆表现都达到综合高成就标准";
    glyphs += "最终金钱突出声望保持在较低区间最终声望是最突出的生活痕迹";
    glyphs += "最终知识是最突出的生活痕迹酒馆胜场和胜率达到传说路线标准";
    glyphs += "各项指标保持在普通区间最终结局配置非法库存清算金额超出范围";
    return glyphs;
}

}  // namespace pixel_town

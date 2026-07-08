#include "core/tavern_rules.hpp"

#include <string>

namespace pixel_town {

const char* challenge_type_label(ChallengeType type) {
    switch (type) {
        case ChallengeType::gomoku:
            return "五子棋";
        case ChallengeType::liars_dice:
            return "骗子骰子";
    }
    return "未知挑战";
}

const char* bet_tier_label(BetTier tier) {
    switch (tier) {
        case BetTier::low:
            return "低";
        case BetTier::medium:
            return "中";
        case BetTier::high:
            return "高";
    }
    return "未知";
}

const char* challenge_outcome_label(ChallengeOutcome outcome) {
    switch (outcome) {
        case ChallengeOutcome::win:
            return "获胜";
        case ChallengeOutcome::loss:
            return "失败";
        case ChallengeOutcome::draw:
            return "平局";
    }
    return "未知";
}

int bet_amount(const TavernChallengeConfig& config, BetTier tier) {
    switch (tier) {
        case BetTier::low:
            return config.low_bet;
        case BetTier::medium:
            return config.medium_bet;
        case BetTier::high:
            return config.high_bet;
    }
    return 0;
}

ActionResult simulate_tavern_challenge(
    const PlayerState& player,
    const TavernChallengeConfig& config,
    ChallengeType challenge,
    BetTier bet_tier,
    ChallengeOutcome outcome,
    int result_id) {
    const int bet = bet_amount(config, bet_tier);

    if (bet > player.money) {
        return {result_id,
                ActionSlot::night,
                Location::tavern,
                ActionOutcome::abandoned,
                {}, 0, 0,
                std::string{"赌注不足，当前金钱"} + std::to_string(player.money) +
                    "不足以支付" + bet_tier_label(bet_tier) + "赌注（" +
                    std::to_string(bet) + "金币）。"};
    }

    const char* type_str = challenge_type_label(challenge);
    const char* tier_str = bet_tier_label(bet_tier);

    int money_delta = 0;
    int mood_delta = 0;
    int win_delta = 0;
    int loss_delta = 0;
    std::string result_text;

    switch (outcome) {
        case ChallengeOutcome::win:
            money_delta = static_cast<int>(bet * config.net_win_factor);
            mood_delta = config.win_mood_delta;
            win_delta = 1;
            result_text = std::string{"获胜！赢得"} + std::to_string(money_delta) + "金币。";
            break;
        case ChallengeOutcome::loss:
            money_delta = static_cast<int>(bet * config.net_loss_factor);
            mood_delta = config.loss_mood_delta;
            loss_delta = 1;
            result_text = std::string{"失败，损失"} +
                          std::to_string(money_delta >= 0 ? money_delta : -money_delta) + "金币。";
            break;
        case ChallengeOutcome::draw:
            money_delta = static_cast<int>(bet * config.net_draw_factor);
            mood_delta = config.draw_mood_delta;
            result_text = "平局，赌注退还。";
            break;
    }

    const std::string summary = std::string{"酒馆挑战（"} + type_str + "/" + tier_str +
                                "赌注）：" + result_text;

    ActionResult result;
    result.result_id = result_id;
    result.slot = ActionSlot::night;
    result.location = Location::tavern;
    result.outcome = ActionOutcome::completed;
    result.delta = StatDelta{money_delta, 0, 0, 0, mood_delta};
    result.tavern_win_delta = win_delta;
    result.tavern_loss_delta = loss_delta;
    result.summary = summary;
    return result;
}

}  // namespace pixel_town

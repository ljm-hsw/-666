// 从五子棋/骰子终局推导胜负和赌注结果，不负责开始挑战或修改会话。
#include "locations/tavern_challenge_settlement.hpp"

#include <cmath>

#include "locations/gomoku_rules.hpp"
#include "locations/liars_dice_rules.hpp"

namespace pixel_town {
namespace {

bool valid_config(const TavernChallengeConfig& config) {
    return config.low_bet >= 0 && config.medium_bet >= 0 &&
           config.high_bet >= 0 && std::isfinite(config.net_win_factor) &&
           std::isfinite(config.net_loss_factor) &&
           std::isfinite(config.net_draw_factor);
}

bool valid_bet(BetTier bet) {
    return bet == BetTier::low || bet == BetTier::medium ||
           bet == BetTier::high;
}

}  // namespace

TavernChallengeSettlement::TavernChallengeSettlement(
    TavernChallengeConfig config)
    : config_(config) {}

TavernSettlementBuild TavernChallengeSettlement::build_terminal(
    const PlayerState& player, int result_id, ChallengeType challenge,
    BetTier bet, ChallengeOutcome outcome) const {
    TavernSettlementBuild build;
    if (!valid_config(config_)) {
        build.error = TavernSettlementError::invalid_config;
        build.message = "酒馆结算配置非法。";
        return build;
    }
    if (result_id <= 0) {
        build.error = TavernSettlementError::invalid_result_id;
        build.message = "酒馆行动结果编号非法。";
        return build;
    }
    if (!valid_bet(bet)) {
        build.error = TavernSettlementError::invalid_bet;
        build.message = "酒馆赌注档位非法。";
        return build;
    }
    if (bet_amount(config_, bet) > player.money) {
        build.error = TavernSettlementError::insufficient_funds;
        build.message = "金钱不足，无法支付所选酒馆赌注。";
        return build;
    }
    build.result = simulate_tavern_challenge(
        player, config_, challenge, bet, outcome, result_id);
    return build;
}

TavernSettlementBuild TavernChallengeSettlement::build(
    const GomokuGame& game, const PlayerState& player, BetTier bet,
    int result_id) const {
    if (game.state() == GomokuState::playing) {
        return {TavernSettlementError::not_terminal, {},
                "五子棋尚未结束，不能结算。"};
    }
    ChallengeOutcome outcome = ChallengeOutcome::draw;
    if (game.state() == GomokuState::player_wins) {
        outcome = ChallengeOutcome::win;
    } else if (game.state() == GomokuState::computer_wins) {
        outcome = ChallengeOutcome::loss;
    }
    return build_terminal(player, result_id, ChallengeType::gomoku, bet,
                          outcome);
}

TavernSettlementBuild TavernChallengeSettlement::build(
    const LiarsDiceGame& game, const PlayerState& player, BetTier bet,
    int result_id) const {
    if (!game.is_game_over()) {
        return {TavernSettlementError::not_terminal, {},
                "骗子骰子尚未结束，不能结算。"};
    }
    return build_terminal(player, result_id, ChallengeType::liars_dice, bet,
                          game.player_won() ? ChallengeOutcome::win
                                            : ChallengeOutcome::loss);
}

}  // namespace pixel_town

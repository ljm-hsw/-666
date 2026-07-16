// 酒馆共享领域契约：挑战类型、赌注和统一结算所需的数据结构。
#pragma once

#include <string>

#include "core/game_session.hpp"

namespace pixel_town {

enum class ChallengeType { gomoku, liars_dice };

enum class BetTier { low, medium, high };

enum class ChallengeOutcome { win, loss, draw };

struct TavernChallengeConfig {
    int low_bet{10};
    int medium_bet{25};
    int high_bet{50};
    double net_win_factor{2.0};
    double net_loss_factor{-1.0};
    double net_draw_factor{0.0};
    int win_mood_delta{8};
    int loss_mood_delta{-5};
    int draw_mood_delta{2};
};

[[nodiscard]] const char* challenge_type_label(ChallengeType type);
[[nodiscard]] const char* bet_tier_label(BetTier tier);
[[nodiscard]] const char* challenge_outcome_label(ChallengeOutcome outcome);
[[nodiscard]] int bet_amount(const TavernChallengeConfig& config, BetTier tier);

[[nodiscard]] ActionResult simulate_tavern_challenge(
    const PlayerState& player,
    const TavernChallengeConfig& config,
    ChallengeType challenge,
    BetTier bet_tier,
    ChallengeOutcome outcome,
    int result_id);

}  // namespace pixel_town

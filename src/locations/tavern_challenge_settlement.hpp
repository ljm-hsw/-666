#pragma once

#include <string>

#include "core/tavern_rules.hpp"

namespace pixel_town {

enum class TavernSettlementError {
    none,
    invalid_config,
    invalid_result_id,
    invalid_bet,
    insufficient_funds,
    not_terminal,
};

class GomokuGame;
class LiarsDiceGame;

struct TavernSettlementBuild {
    TavernSettlementError error{TavernSettlementError::none};
    ActionResult result{};
    std::string message;

    [[nodiscard]] bool accepted() const noexcept {
        return error == TavernSettlementError::none;
    }
};

class TavernChallengeSettlement {
public:
    explicit TavernChallengeSettlement(TavernChallengeConfig config);

    [[nodiscard]] TavernSettlementBuild build(const GomokuGame& game,
                                               const PlayerState& player,
                                               BetTier bet,
                                               int result_id) const;
    [[nodiscard]] TavernSettlementBuild build(const LiarsDiceGame& game,
                                               const PlayerState& player,
                                               BetTier bet,
                                               int result_id) const;

private:
    TavernChallengeConfig config_;

    [[nodiscard]] TavernSettlementBuild build_terminal(
        const PlayerState& player, int result_id, ChallengeType challenge,
        BetTier bet, ChallengeOutcome outcome) const;
};

}  // namespace pixel_town

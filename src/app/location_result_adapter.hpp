#pragma once

#include "core/game_session.hpp"
#include "core/tavern_rules.hpp"
#include "locations/library_rules.hpp"

namespace pixel_town {

[[nodiscard]] ActionSlot action_slot_for_phase(GamePhase phase);

[[nodiscard]] library::DailyContext make_library_daily_context(const GameSession& session,
                                                               int library_visits);

[[nodiscard]] ActionResult library_action_result(const library::ActionResult& result,
                                                 int result_id, ActionSlot slot);

[[nodiscard]] bool can_afford_tavern_bet(const PlayerState& player,
                                         BetTier bet_tier,
                                         const TavernChallengeConfig& config);

[[nodiscard]] ActionResult tavern_action_result(const GameSession& session,
                                                ChallengeType challenge,
                                                BetTier bet_tier,
                                                ChallengeOutcome outcome,
                                                const TavernChallengeConfig& config);

}  // namespace pixel_town

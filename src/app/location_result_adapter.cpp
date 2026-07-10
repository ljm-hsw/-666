#include "app/location_result_adapter.hpp"

#include <algorithm>

namespace pixel_town {

ActionSlot action_slot_for_phase(GamePhase phase) {
    return phase == GamePhase::day_location ? ActionSlot::day : ActionSlot::night;
}

library::DailyContext make_library_daily_context(const GameSession& session, int library_visits) {
    library::DailyContext context;
    context.day = session.day();
    context.random_seed = session.location_seed(
        Location::library, static_cast<unsigned int>(std::max(0, library_visits)));
    context.library_visits = library_visits;
    context.current_knowledge = session.player().knowledge;
    return context;
}

ActionResult library_action_result(const library::ActionResult& result,
                                   int result_id,
                                   ActionSlot slot) {
    ActionResult game_result;
    game_result.result_id = result_id;
    game_result.slot = slot;
    game_result.location = Location::library;
    game_result.outcome = result.gave_up ? ActionOutcome::abandoned : ActionOutcome::completed;
    game_result.delta.money = result.money_change;
    game_result.delta.stamina = result.stamina_change;
    game_result.delta.reputation = result.reputation_change;
    game_result.delta.knowledge = result.knowledge_change;
    game_result.delta.mood = result.mood_change;
    game_result.summary = result.summary;
    if (!result.narrative_echo.empty()) {
        if (!game_result.summary.empty()) {
            game_result.summary += "\n";
        }
        game_result.summary += result.narrative_echo;
    }
    return game_result;
}

bool can_afford_tavern_bet(const PlayerState& player,
                           BetTier bet_tier,
                           const TavernChallengeConfig& config) {
    return bet_amount(config, bet_tier) <= player.money;
}

ActionResult tavern_action_result(const GameSession& session,
                                  ChallengeType challenge,
                                  BetTier bet_tier,
                                  ChallengeOutcome outcome,
                                  const TavernChallengeConfig& config) {
    return simulate_tavern_challenge(
        session.player(), config, challenge, bet_tier, outcome, session.active_result_id());
}

}  // namespace pixel_town

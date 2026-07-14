#pragma once

#include "core/game_session.hpp"
#include "locations/library_organizing.hpp"
#include "locations/library_rules.hpp"

namespace pixel_town {

[[nodiscard]] ActionSlot action_slot_for_phase(GamePhase phase);

[[nodiscard]] library::DailyContext make_library_daily_context(const GameSession& session,
                                                               int library_visits);

[[nodiscard]] ActionResult library_action_result(const library::ActionResult& result,
                                                 int result_id, ActionSlot slot);
[[nodiscard]] ActionResult library_organizing_action_result(
    const library::OrganizingResult& result, int result_id, ActionSlot slot);

}  // namespace pixel_town

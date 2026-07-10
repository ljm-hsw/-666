#pragma once

#include "core/game_session.hpp"
#include "core/story_text.hpp"

namespace pixel_town::test_support {

inline ActionResult completed_location_result(const GameSession& session) {
    ActionResult result;
    result.result_id = session.active_result_id();
    result.slot = session.phase() == GamePhase::day_location ? ActionSlot::day
                                                             : ActionSlot::night;
    result.location = session.pending_location();
    result.outcome = ActionOutcome::completed;
    switch (result.location) {
        case Location::restaurant:
            result.delta = StatDelta{18, -18, 4, 0, -3};
            break;
        case Location::convenience_store:
            result.delta = StatDelta{14, -12, 2, 0, -1};
            break;
        case Location::library:
            result.delta = StatDelta{6, -8, 5, 6, 2};
            break;
        case Location::home:
        case Location::tavern:
            break;
    }
    result.summary = location_result_summary(result.location, ActionOutcome::completed);
    return result;
}

}  // namespace pixel_town::test_support

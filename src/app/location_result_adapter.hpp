#pragma once

#include "core/game_session.hpp"
#include "locations/library_work_result.hpp"

namespace pixel_town {

[[nodiscard]] ActionResult library_action_result(const library::LibraryWorkResult& result,
                                                 int result_id, ActionSlot slot);

}  // namespace pixel_town

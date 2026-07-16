// 结果适配边界：把地点专属结果映射成核心 GameSession 能接受的 ActionResult。
#pragma once

#include "core/game_session.hpp"
#include "locations/library_work_result.hpp"

namespace pixel_town {

[[nodiscard]] ActionResult library_action_result(const library::LibraryWorkResult& result,
                                                 int result_id, ActionSlot slot);

}  // namespace pixel_town

// 保持地点规则与核心会话模型解耦；这里只做字段映射，不做额外结算。
#include "app/location_result_adapter.hpp"

namespace pixel_town {

ActionResult library_action_result(const library::LibraryWorkResult& result,
                                   int result_id,
                                   ActionSlot slot) {
    // 窄适配器只复制地点结果，不重新计算奖励或修改会话。
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

}  // namespace pixel_town

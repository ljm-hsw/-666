// 规则结果到可展示叙事文本的纯函数接口；UI 不直接持有剧情分支。
#pragma once

#include <string>

#include "core/game_session.hpp"

namespace pixel_town {

enum class MainEnding;

[[nodiscard]] const char* opening_story();
[[nodiscard]] const char* daily_prompt(int day);
[[nodiscard]] const char* location_result_summary(Location location, ActionOutcome outcome);
[[nodiscard]] const char* day_closing_summary(int day);
[[nodiscard]] const char* council_opening();
[[nodiscard]] const char* ending_narrative(MainEnding ending);
[[nodiscard]] std::string story_text_glyphs();

}  // namespace pixel_town

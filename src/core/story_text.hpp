#pragma once

#include <string>

#include "core/game_session.hpp"

namespace pixel_town {

[[nodiscard]] const char* opening_story();
[[nodiscard]] const char* daily_prompt(int day);
[[nodiscard]] const char* location_result_summary(Location location, ActionOutcome outcome);
[[nodiscard]] const char* day_closing_summary(int day);
[[nodiscard]] const char* council_opening();
[[nodiscard]] const char* placeholder_ending_text();
[[nodiscard]] std::string story_text_glyphs();

}  // namespace pixel_town

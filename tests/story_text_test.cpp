#include <doctest/doctest.h>

#include <string>

#include "core/story_text.hpp"

TEST_CASE("story text provides ten stable daily prompts") {
    CHECK(std::string{pixel_town::daily_prompt(1)}.find("镇长把地图") != std::string::npos);
    CHECK(std::string{pixel_town::daily_prompt(10)}.find("账本和几张便签") !=
          std::string::npos);
    CHECK(std::string{pixel_town::daily_prompt(0)} == pixel_town::daily_prompt(1));
    CHECK(std::string{pixel_town::daily_prompt(11)} == pixel_town::daily_prompt(1));
}

TEST_CASE("story text maps placeholder location summaries by location") {
    CHECK(std::string{pixel_town::location_result_summary(
              pixel_town::Location::restaurant, pixel_town::ActionOutcome::completed)}
              .find("最后一碗汤") != std::string::npos);
    CHECK(std::string{pixel_town::location_result_summary(
              pixel_town::Location::convenience_store, pixel_town::ActionOutcome::completed)}
              .find("账本边角") != std::string::npos);
    CHECK(std::string{pixel_town::location_result_summary(
              pixel_town::Location::library, pixel_town::ActionOutcome::completed)}
              .find("借书卡") != std::string::npos);
    CHECK(std::string{pixel_town::location_result_summary(
              pixel_town::Location::home, pixel_town::ActionOutcome::abandoned)}
              .find("提前离开") != std::string::npos);
}

TEST_CASE("story text exposes council and placeholder ending copy for final pages") {
    CHECK(std::string{pixel_town::opening_story()}.find("先住十天") != std::string::npos);
    CHECK(std::string{pixel_town::council_opening()}.find("十天不长") != std::string::npos);
    CHECK(std::string{pixel_town::placeholder_ending_text()}.find("纸角") !=
          std::string::npos);
    CHECK(pixel_town::story_text_glyphs().find("酒馆昨晚的棋盘") != std::string::npos);
}

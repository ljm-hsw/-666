#include <doctest/doctest.h>

#include "app/dialogue_runtime.hpp"
#include "app/tavern_runtime.hpp"
#include "core/story_dialogue.hpp"

TEST_CASE("bartender dialogue plays through the shared runtime") {
    const pixel_town::StoryDialogueCatalog catalog;
    const auto* script =
        catalog.find(pixel_town::DialogueTrigger::tavern_bartender_intro);
    REQUIRE(script != nullptr);
    REQUIRE(script->lines.size() == 3);

    pixel_town::DialogueRuntime runtime;
    REQUIRE(runtime.open(*script));
    REQUIRE(runtime.active());

    auto view = runtime.presentation();
    CHECK(view.speaker == "酒保");
    CHECK_FALSE(view.text.empty());
    CHECK(view.current_line == 1);
    CHECK(view.total_lines == 3);

    pixel_town::DialogueFrameInput input;
    input.advance_pressed = true;
    CHECK(runtime.step(input) == pixel_town::DialogueStepStatus::advanced);
    view = runtime.presentation();
    CHECK(view.speaker == "主角");
    CHECK(view.current_line == 2);

    CHECK(runtime.step(input) == pixel_town::DialogueStepStatus::advanced);
    CHECK(runtime.presentation().current_line == 3);
    CHECK(runtime.step(input) == pixel_town::DialogueStepStatus::closed);
    CHECK_FALSE(runtime.active());
}

TEST_CASE("dialogue glyph manifest covers scripts and shared view labels") {
    const pixel_town::StoryDialogueCatalog catalog;
    const auto* script =
        catalog.find(pixel_town::DialogueTrigger::tavern_bartender_intro);
    REQUIRE(script != nullptr);

    const std::string glyphs = std::string{pixel_town::tavern_ui_glyphs()};
    for (const auto& line : script->lines) {
        CHECK(glyphs.find(line.speaker) != std::string::npos);
        CHECK(glyphs.find(line.text) != std::string::npos);
    }
    CHECK(glyphs.find("下一句") != std::string::npos);
    CHECK(glyphs.find("关闭") != std::string::npos);
    CHECK(glyphs.find("点击继续") != std::string::npos);
    CHECK(glyphs.find("对话暂时不可用") != std::string::npos);
}

TEST_CASE("dialogue can be skipped without applying another line") {
    const pixel_town::StoryDialogueCatalog catalog;
    const auto* script =
        catalog.find(pixel_town::DialogueTrigger::tavern_bartender_intro);
    REQUIRE(script != nullptr);
    pixel_town::DialogueRuntime runtime;
    REQUIRE(runtime.open(*script));

    pixel_town::DialogueFrameInput input;
    input.skip_pressed = true;
    CHECK(runtime.step(input) == pixel_town::DialogueStepStatus::closed);
    CHECK_FALSE(runtime.active());
    CHECK_FALSE(runtime.presentation().active);
    CHECK(runtime.step(input) == pixel_town::DialogueStepStatus::unchanged);
}

TEST_CASE("library administrator dialogue is available from the shared catalog") {
    const pixel_town::StoryDialogueCatalog catalog;
    const auto* script = catalog.find(
        pixel_town::DialogueTrigger::library_administrator_intro);

    REQUIRE(script != nullptr);
    REQUIRE(script->lines.size() == 3);
    CHECK(script->lines.front().speaker == "管理员");
    CHECK(script->lines.back().text.find("整理") != std::string::npos);
    const std::string glyphs = catalog.glyphs();
    for (const auto& line : script->lines) {
        CHECK(glyphs.find(line.speaker) != std::string::npos);
        CHECK(glyphs.find(line.text) != std::string::npos);
    }
}

TEST_CASE("restaurant owner dialogue extends the existing main story") {
    const pixel_town::StoryDialogueCatalog catalog;
    const auto* script =
        catalog.find(pixel_town::DialogueTrigger::restaurant_owner_intro);

    REQUIRE(script != nullptr);
    REQUIRE(script->lines.size() == 3);
    CHECK(script->lines.front().speaker == "餐馆老板");
    CHECK(script->lines[1].speaker == "主角");
    CHECK(script->lines.front().text.find("围裙") != std::string::npos);
    CHECK(script->lines.back().text.find("订单") != std::string::npos);

    const std::string glyphs = catalog.glyphs();
    for (const auto& line : script->lines) {
        CHECK(glyphs.find(line.speaker) != std::string::npos);
        CHECK(glyphs.find(line.text) != std::string::npos);
    }
}

TEST_CASE("convenience store owner dialogue extends the existing main story") {
    const pixel_town::StoryDialogueCatalog catalog;
    const auto* script =
        catalog.find(pixel_town::DialogueTrigger::convenience_store_owner_intro);

    REQUIRE(script != nullptr);
    REQUIRE(script->lines.size() == 3);
    CHECK(script->lines.front().speaker == "便利店店主");
    CHECK(script->lines[1].speaker == "主角");
    CHECK(script->lines.front().text.find("天气") != std::string::npos);
    CHECK(script->lines[1].text.find("库存") != std::string::npos);
    CHECK(script->lines.back().text.find("账本") != std::string::npos);

    const std::string glyphs = catalog.glyphs();
    for (const auto& line : script->lines) {
        CHECK(glyphs.find(line.speaker) != std::string::npos);
        CHECK(glyphs.find(line.text) != std::string::npos);
    }
}

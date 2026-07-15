#include <doctest/doctest.h>

#include "app/npc_lobby_runtime.hpp"

TEST_CASE("fixed NPC lobby requests its activity only after dialogue closes") {
    pixel_town::NpcLobbyRuntime lobby;
    REQUIRE(lobby.open(pixel_town::DialogueTrigger::restaurant_owner_intro));

    pixel_town::NpcLobbyInput input;
    input.interaction_activated = true;
    CHECK(lobby.step(input).status ==
          pixel_town::NpcLobbyStepStatus::dialogue_opened);
    REQUIRE(lobby.presentation().dialogue.has_value());
    CHECK(lobby.presentation().dialogue->speaker == "餐馆老板");

    input = {};
    input.dialogue.advance_pressed = true;
    CHECK(lobby.step(input).status == pixel_town::NpcLobbyStepStatus::changed);
    CHECK(lobby.step(input).status == pixel_town::NpcLobbyStepStatus::changed);
    CHECK(lobby.step(input).status ==
          pixel_town::NpcLobbyStepStatus::activity_requested);
    CHECK_FALSE(lobby.active());
}

TEST_CASE("fixed NPC lobby blocks scene inputs while dialogue is active") {
    pixel_town::NpcLobbyRuntime lobby;
    REQUIRE(lobby.open(pixel_town::DialogueTrigger::restaurant_owner_intro));

    pixel_town::NpcLobbyInput input;
    input.interaction_activated = true;
    REQUIRE(lobby.step(input).status ==
            pixel_town::NpcLobbyStepStatus::dialogue_opened);
    const auto dialogue_before = lobby.presentation().dialogue;
    REQUIRE(dialogue_before.has_value());

    input = {};
    input.elapsed_seconds = 0.2F;
    input.interaction_activated = true;
    input.back_pressed = true;
    CHECK(lobby.step(input).status == pixel_town::NpcLobbyStepStatus::unchanged);
    REQUIRE(lobby.presentation().dialogue.has_value());
    CHECK(lobby.presentation().dialogue->current_line ==
          dialogue_before->current_line);
    CHECK(lobby.presentation().npc_animation_seconds == doctest::Approx(0.2F));

    input = {};
    input.dialogue.skip_pressed = true;
    CHECK(lobby.step(input).status ==
          pixel_town::NpcLobbyStepStatus::activity_requested);
}

TEST_CASE("fixed NPC lobby can return without requesting its activity") {
    pixel_town::NpcLobbyRuntime lobby;
    REQUIRE(lobby.open(pixel_town::DialogueTrigger::restaurant_owner_intro));

    pixel_town::NpcLobbyInput input;
    input.back_pressed = true;
    const auto result = lobby.step(input);

    CHECK(result.status == pixel_town::NpcLobbyStepStatus::closed);
    CHECK_FALSE(lobby.active());
    CHECK(result.notice.find("未消耗") != std::string::npos);
}

TEST_CASE("fixed NPC lobby can play a selected location story script") {
    pixel_town::NpcLobbyRuntime lobby;
    const pixel_town::DialogueScript script{
        pixel_town::DialogueTrigger::restaurant_owner_intro,
        {{"餐馆老板", "新菜单刚换好，先把桌号听清。"},
         {"主角", "我会先确认桌号，再开始上菜。"}}};
    REQUIRE(lobby.open(script));

    pixel_town::NpcLobbyInput input;
    input.interaction_activated = true;
    CHECK(lobby.step(input).status ==
          pixel_town::NpcLobbyStepStatus::dialogue_opened);
    REQUIRE(lobby.presentation().dialogue.has_value());
    CHECK(lobby.presentation().dialogue->text.find("新菜单") !=
          std::string::npos);

    input = {};
    input.interaction_activated = true;
    input.back_pressed = true;
    CHECK(lobby.step(input).status == pixel_town::NpcLobbyStepStatus::unchanged);
    CHECK(lobby.presentation().dialogue->current_line == 1);
}

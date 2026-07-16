#include <doctest/doctest.h>

#include <cmath>

#include "core/scene_navigation.hpp"

namespace {

void move(pixel_town::SceneNavigationRuntime& navigation, float seconds,
          bool left, bool right, bool up, bool down) {
    pixel_town::SceneNavigationInput input;
    input.elapsed_seconds = seconds;
    input.move_left = left;
    input.move_right = right;
    input.move_up = up;
    input.move_down = down;
    const auto result = navigation.step(input);
    REQUIRE((result.status == pixel_town::SceneNavigationStepStatus::moved ||
             result.status == pixel_town::SceneNavigationStepStatus::blocked));
}

}  // namespace

TEST_CASE("indoor navigation moves the protagonist from explicit input") {
    pixel_town::SceneNavigationRuntime navigation;
    REQUIRE(navigation.open(pixel_town::Location::library));

    const auto before = navigation.presentation();
    pixel_town::SceneNavigationInput input;
    input.elapsed_seconds = 0.25F;
    input.move_right = true;

    const auto result = navigation.step(input);
    const auto after = navigation.presentation();

    CHECK(result.status == pixel_town::SceneNavigationStepStatus::moved);
    CHECK(after.player_position.x > before.player_position.x);
    CHECK(after.player_position.y == doctest::Approx(before.player_position.y));
}

TEST_CASE("diagonal indoor movement is normalized") {
    pixel_town::SceneNavigationRuntime navigation;
    REQUIRE(navigation.open(pixel_town::Location::library));
    const auto before = navigation.presentation().player_position;

    pixel_town::SceneNavigationInput input;
    input.elapsed_seconds = 0.25F;
    input.move_right = true;
    input.move_up = true;
    REQUIRE(navigation.step(input).status ==
            pixel_town::SceneNavigationStepStatus::moved);

    const auto after = navigation.presentation().player_position;
    const float distance = std::sqrt((after.x - before.x) * (after.x - before.x) +
                                     (after.y - before.y) * (after.y - before.y));
    CHECK(distance == doctest::Approx(40.0F));
}

TEST_CASE("library interaction activates only after approaching the administrator") {
    pixel_town::SceneNavigationRuntime navigation;
    REQUIRE(navigation.open(pixel_town::Location::library));

    REQUIRE(navigation.presentation().nearby_interaction.has_value());
    CHECK(*navigation.presentation().nearby_interaction ==
          pixel_town::SceneInteractionKind::exit);

    pixel_town::SceneNavigationInput move;
    move.elapsed_seconds = 0.4F;
    move.move_right = true;
    REQUIRE(navigation.step(move).status ==
            pixel_town::SceneNavigationStepStatus::moved);
    move = {};
    move.elapsed_seconds = 2.0F;
    move.move_up = true;
    REQUIRE(navigation.step(move).status ==
            pixel_town::SceneNavigationStepStatus::moved);

    REQUIRE(navigation.presentation().nearby_interaction.has_value());
    CHECK(*navigation.presentation().nearby_interaction ==
          pixel_town::SceneInteractionKind::npc);
    pixel_town::SceneNavigationInput interact;
    interact.interact_pressed = true;
    const auto activated = navigation.step(interact);
    CHECK(activated.status ==
          pixel_town::SceneNavigationStepStatus::interaction_activated);
    REQUIRE(activated.interaction.has_value());
    CHECK(*activated.interaction == pixel_town::SceneInteractionKind::npc);
}

TEST_CASE("tavern interaction chooses the closest table") {
    pixel_town::SceneNavigationRuntime navigation;
    REQUIRE(navigation.open(pixel_town::Location::tavern));

    pixel_town::SceneNavigationInput move;
    move.elapsed_seconds = 0.275F;
    move.move_up = true;
    REQUIRE(navigation.step(move).status ==
            pixel_town::SceneNavigationStepStatus::moved);
    move = {};
    move.elapsed_seconds = 0.425F;
    move.move_right = true;
    REQUIRE(navigation.step(move).status ==
            pixel_town::SceneNavigationStepStatus::moved);
    move = {};
    move.elapsed_seconds = 0.1F;
    move.move_up = true;
    REQUIRE(navigation.step(move).status ==
            pixel_town::SceneNavigationStepStatus::moved);

    REQUIRE(navigation.presentation().nearby_interaction.has_value());
    CHECK(*navigation.presentation().nearby_interaction ==
          pixel_town::SceneInteractionKind::secondary_activity);
}

TEST_CASE("restaurant store and home targets are reachable from their spawns") {
    pixel_town::SceneNavigationRuntime navigation;

    REQUIRE(navigation.open(pixel_town::Location::restaurant));
    move(navigation, 0.2F, false, true, false, false);
    move(navigation, 1.3F, false, false, true, false);
    move(navigation, 0.15F, true, false, false, false);
    REQUIRE(navigation.presentation().nearby_interaction.has_value());
    CHECK(*navigation.presentation().nearby_interaction ==
          pixel_town::SceneInteractionKind::npc);

    REQUIRE(navigation.open(pixel_town::Location::convenience_store));
    move(navigation, 0.4F, false, false, true, false);
    move(navigation, 1.0F, false, true, false, false);
    REQUIRE(navigation.presentation().nearby_interaction.has_value());
    CHECK(*navigation.presentation().nearby_interaction ==
          pixel_town::SceneInteractionKind::npc);

    REQUIRE(navigation.open(pixel_town::Location::home));
    move(navigation, 0.125F, true, false, false, false);
    move(navigation, 2.0F, false, false, true, false);
    REQUIRE(navigation.presentation().nearby_interaction.has_value());
    CHECK(*navigation.presentation().nearby_interaction ==
          pixel_town::SceneInteractionKind::primary_activity);
}

TEST_CASE("closing and reopening navigation restores the scene spawn") {
    pixel_town::SceneNavigationRuntime navigation;
    const auto* layout = pixel_town::find_indoor_scene_layout(
        pixel_town::Location::library);
    REQUIRE(layout != nullptr);
    REQUIRE(navigation.open(pixel_town::Location::library));
    move(navigation, 0.25F, false, true, false, false);
    CHECK(navigation.presentation().player_position.x !=
          doctest::Approx(layout->player_spawn.x));

    navigation.close();
    CHECK_FALSE(navigation.presentation().active);
    REQUIRE(navigation.open(pixel_town::Location::library));
    const auto reopened = navigation.presentation();
    CHECK(reopened.player_position.x == doctest::Approx(layout->player_spawn.x));
    CHECK(reopened.player_position.y == doctest::Approx(layout->player_spawn.y));
    CHECK(reopened.animation_seconds == doctest::Approx(0.0F));
}

TEST_CASE("indoor navigation exposes every runtime notice to the font glyph list") {
    const std::string glyphs = pixel_town::scene_navigation_glyphs();

    CHECK(glyphs.find("前方被家具或墙体挡住了") != std::string::npos);
    CHECK(glyphs.find("附近没有可以互动的目标") != std::string::npos);
    CHECK(glyphs.find("室内导航收到非法帧时间") != std::string::npos);
}

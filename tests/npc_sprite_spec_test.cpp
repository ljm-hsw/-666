#include <doctest/doctest.h>

#include <array>

#include "app/npc_sprite_spec.hpp"

TEST_CASE("fixed character sprite sheets share one four-frame layout") {
    using pixel_town::NpcSpriteKind;
    constexpr std::array kinds{
        NpcSpriteKind::salesclerk,
        NpcSpriteKind::bartender,
        NpcSpriteKind::restaurant_chef,
        NpcSpriteKind::librarian,
        NpcSpriteKind::protagonist,
        NpcSpriteKind::mayor,
    };

    for (const auto kind : kinds) {
        const auto& spec = pixel_town::npc_sprite_spec(kind);
        CHECK(spec.frame_width == 64);
        CHECK(spec.frame_height == 96);
        CHECK(spec.frame_count == 4);
        CHECK(spec.frame_duration_seconds > 0.0F);
        REQUIRE(spec.runtime_path != nullptr);
        CHECK(spec.runtime_path[0] != '\0');
    }
}

TEST_CASE("fixed NPC animation frame wraps deterministically") {
    const auto& spec =
        pixel_town::npc_sprite_spec(pixel_town::NpcSpriteKind::bartender);

    CHECK(pixel_town::npc_sprite_frame(spec, 0.0F) == 0);
    CHECK(pixel_town::npc_sprite_frame(spec, spec.frame_duration_seconds) == 1);
    CHECK(pixel_town::npc_sprite_frame(
              spec, spec.frame_duration_seconds * spec.frame_count) == 0);
    CHECK(pixel_town::npc_sprite_frame(spec, -1.0F) == 0);
}

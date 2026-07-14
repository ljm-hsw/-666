#include <doctest/doctest.h>

#include <array>

#include "core/scene_collision.hpp"

TEST_CASE("indoor art diagnostic layouts are valid") {
    const std::array locations{
        pixel_town::Location::restaurant,
        pixel_town::Location::convenience_store,
        pixel_town::Location::home,
        pixel_town::Location::library,
        pixel_town::Location::tavern,
    };

    for (const auto location : locations) {
        const auto* layout = pixel_town::find_indoor_scene_layout(location);
        REQUIRE(layout != nullptr);
        CHECK(layout->canvas_width == 960);
        CHECK(layout->canvas_height == 540);
        CHECK(pixel_town::validate_indoor_scene_layout(*layout).empty());
        CHECK_FALSE(layout->static_colliders.empty());
    }
}

TEST_CASE("scene art diagnostics use strict overlap and permit edge contact") {
    const pixel_town::SceneRect obstacle{100.0F, 100.0F, 40.0F, 30.0F};

    CHECK(pixel_town::scene_rects_overlap(
        obstacle, pixel_town::SceneRect{120.0F, 110.0F, 20.0F, 20.0F}));
    CHECK_FALSE(pixel_town::scene_rects_overlap(
        obstacle, pixel_town::SceneRect{140.0F, 110.0F, 20.0F, 20.0F}));
    CHECK_FALSE(pixel_town::scene_rects_overlap(
        obstacle, pixel_town::SceneRect{80.0F, 110.0F, 20.0F, 20.0F}));
}

TEST_CASE("door review regions do not overlap diagnostic furniture") {
    const std::array locations{
        pixel_town::Location::restaurant,
        pixel_town::Location::convenience_store,
        pixel_town::Location::home,
        pixel_town::Location::library,
        pixel_town::Location::tavern,
    };
    for (const auto location : locations) {
        const auto* layout = pixel_town::find_indoor_scene_layout(location);
        REQUIRE(layout != nullptr);
        for (const auto& collider : layout->static_colliders) {
            CAPTURE(collider.id);
            CHECK_FALSE(pixel_town::scene_rects_overlap(
                layout->exit_trigger, collider.bounds));
        }
    }
}

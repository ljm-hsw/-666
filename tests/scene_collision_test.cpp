#include <doctest/doctest.h>

#include <array>

// 场景布局契约：可行走边界、家具碰撞体和出口不被封堵。
#include "core/scene_collision.hpp"

TEST_CASE("indoor navigation layouts are valid and have free player spawns") {
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
        CHECK(pixel_town::scene_position_is_free(
            *layout, layout->player_spawn,
            pixel_town::SceneSize{24.0F, 24.0F}));
        CHECK_FALSE(layout->static_colliders.empty());
    }
}

TEST_CASE("scene collision uses strict overlap and permits edge contact") {
    const pixel_town::SceneRect obstacle{100.0F, 100.0F, 40.0F, 30.0F};

    CHECK(pixel_town::scene_rects_overlap(
        obstacle, pixel_town::SceneRect{120.0F, 110.0F, 20.0F, 20.0F}));
    CHECK_FALSE(pixel_town::scene_rects_overlap(
        obstacle, pixel_town::SceneRect{140.0F, 110.0F, 20.0F, 20.0F}));
    CHECK_FALSE(pixel_town::scene_rects_overlap(
        obstacle, pixel_town::SceneRect{80.0F, 110.0F, 20.0F, 20.0F}));
}

TEST_CASE("axis separated movement stops at furniture and slides along it") {
    const auto* layout = pixel_town::find_indoor_scene_layout(
        pixel_town::Location::library);
    REQUIRE(layout != nullptr);
    const pixel_town::SceneSize player_size{24.0F, 24.0F};
    const pixel_town::ScenePoint start{360.0F, 330.0F};
    REQUIRE(pixel_town::scene_position_is_free(*layout, start, player_size));

    const auto blocked = pixel_town::move_with_scene_collisions(
        *layout, start, pixel_town::ScenePoint{50.0F, 0.0F}, player_size);
    CHECK(blocked.x == doctest::Approx(368.0F));
    CHECK(blocked.y == doctest::Approx(start.y));

    const auto sliding = pixel_town::move_with_scene_collisions(
        *layout, start, pixel_town::ScenePoint{50.0F, -30.0F}, player_size);
    CHECK(sliding.x == doctest::Approx(368.0F));
    CHECK(sliding.y == doctest::Approx(300.0F));
    CHECK(pixel_town::scene_position_is_free(*layout, sliding, player_size));
}

TEST_CASE("movement remains inside every scene walkable boundary") {
    const std::array locations{
        pixel_town::Location::restaurant,
        pixel_town::Location::convenience_store,
        pixel_town::Location::home,
        pixel_town::Location::library,
        pixel_town::Location::tavern,
    };
    const pixel_town::SceneSize player_size{24.0F, 24.0F};
    for (const auto location : locations) {
        const auto* layout = pixel_town::find_indoor_scene_layout(location);
        REQUIRE(layout != nullptr);
        const auto moved = pixel_town::move_with_scene_collisions(
            *layout, layout->player_spawn,
            pixel_town::ScenePoint{5000.0F, 5000.0F}, player_size);
        CHECK(pixel_town::scene_position_is_free(*layout, moved, player_size));
    }
}

TEST_CASE("door corridors do not overlap furniture") {
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

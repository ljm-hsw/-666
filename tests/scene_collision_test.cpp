#include <doctest/doctest.h>

#include <array>

#include "core/scene_collision.hpp"

namespace {

constexpr pixel_town::SceneSize player_size{24.0F, 24.0F};

}  // namespace

TEST_CASE("approved indoor scene layouts are valid and have free player spawns") {
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
            *layout, layout->player_spawn, player_size));
        CHECK_FALSE(layout->static_colliders.empty());
    }
}

TEST_CASE("every supplied indoor composite exposes named furniture colliders") {
    const auto* store = pixel_town::find_indoor_scene_layout(
        pixel_town::Location::convenience_store);
    const auto* tavern = pixel_town::find_indoor_scene_layout(
        pixel_town::Location::tavern);

    REQUIRE(store != nullptr);
    REQUIRE(tavern != nullptr);
    CHECK(store->static_colliders.size() >= 10);
    CHECK(tavern->static_colliders.size() >= 10);
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

TEST_CASE("axis separated movement stops at furniture and still slides along it") {
    const auto* layout = pixel_town::find_indoor_scene_layout(pixel_town::Location::library);
    REQUIRE(layout != nullptr);

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

TEST_CASE("movement is clamped to walkable bounds") {
    const auto* layout = pixel_town::find_indoor_scene_layout(pixel_town::Location::home);
    REQUIRE(layout != nullptr);

    const pixel_town::ScenePoint start{500.0F, 400.0F};
    REQUIRE(pixel_town::scene_position_is_free(*layout, start, player_size));
    const auto moved = pixel_town::move_with_scene_collisions(
        *layout, start, pixel_town::ScenePoint{1000.0F, 0.0F}, player_size);

    CHECK(moved.x <= layout->walkable_bounds.x + layout->walkable_bounds.width -
                         player_size.width * 0.5F);
    CHECK(pixel_town::scene_position_is_free(*layout, moved, player_size));
}

TEST_CASE("door corridors remain free and lead into exit triggers") {
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
        CHECK(pixel_town::scene_rect_contains(
            layout->exit_trigger, layout->player_spawn));
        CHECK(pixel_town::scene_position_is_free(
            *layout, layout->player_spawn, player_size));
    }
}

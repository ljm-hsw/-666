#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "core/display_config.hpp"

TEST_CASE("default display presents the logical canvas at one-times scale") {
    const auto display = pixel_town::default_display_config();

    CHECK(display.logical_width == 960);
    CHECK(display.logical_height == 540);
    CHECK(display.window_width == 960);
    CHECK(display.window_height == 540);
}

TEST_CASE("viewport uses integer scale with letterbox offsets") {
    const auto display = pixel_town::DisplayConfig{960, 540, 1920, 1080};

    const auto viewport = pixel_town::integer_scaled_viewport(display, 2560, 1440);

    CHECK(viewport.scale == 2);
    CHECK(viewport.x == 320);
    CHECK(viewport.y == 180);
    CHECK(viewport.width == 1920);
    CHECK(viewport.height == 1080);
}

TEST_CASE("screen input maps to logical canvas and rejects black bars") {
    const auto display = pixel_town::DisplayConfig{960, 540, 960, 540};
    const auto viewport = pixel_town::integer_scaled_viewport(display, 2560, 1440);

    const auto point = pixel_town::screen_to_logical_point(viewport, 1280, 720);

    REQUIRE(point.has_value());
    CHECK(point->x == 480);
    CHECK(point->y == 270);
    CHECK_FALSE(pixel_town::screen_to_logical_point(viewport, 100, 720).has_value());
    CHECK_FALSE(pixel_town::screen_to_logical_point(viewport, 2450, 720).has_value());
}

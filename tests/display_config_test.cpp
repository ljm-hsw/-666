#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "core/display_config.hpp"
#include "ui/scene_viewport.hpp"

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

TEST_CASE("indoor scene viewport starts below the status header and keeps 16:9") {
    const auto viewport = pixel_town::ui::indoor_scene_viewport();

    CHECK(viewport.y == pixel_town::ui::scene_header_height);
    CHECK(viewport.y + viewport.height == pixel_town::ui::canvas_height);
    CHECK(viewport.width / viewport.height ==
          doctest::Approx(16.0F / 9.0F));
    CHECK(viewport.x > 0.0F);
}

TEST_CASE("indoor scene coordinates round trip through the fitted viewport") {
    const pixel_town::ui::SceneViewportPoint design_point{320.0F, 180.0F};
    const auto canvas_point =
        pixel_town::ui::scene_design_to_viewport(design_point);
    const auto restored =
        pixel_town::ui::viewport_to_scene_design(canvas_point);

    CHECK(canvas_point.x == doctest::Approx(480.0F));
    CHECK(canvas_point.y == doctest::Approx(300.0F));
    CHECK(restored.x == doctest::Approx(design_point.x));
    CHECK(restored.y == doctest::Approx(design_point.y));
}

TEST_CASE("scene rectangles can reuse design-space UI hit testing") {
    const pixel_town::ui::SceneViewportRect full_design_scene{
        0.0F, 0.0F, static_cast<float>(pixel_town::ui::design_width),
        static_cast<float>(pixel_town::ui::design_height)};
    const auto screen_design =
        pixel_town::ui::scene_design_to_screen_design(full_design_scene);

    CHECK(screen_design.x * pixel_town::ui::design_to_canvas_scale ==
          doctest::Approx(pixel_town::ui::scene_viewport_x));
    CHECK(screen_design.y * pixel_town::ui::design_to_canvas_scale ==
          doctest::Approx(pixel_town::ui::scene_header_height));
    CHECK(screen_design.width * pixel_town::ui::design_to_canvas_scale ==
          doctest::Approx(pixel_town::ui::scene_viewport_width));
    CHECK(screen_design.height * pixel_town::ui::design_to_canvas_scale ==
          doctest::Approx(pixel_town::ui::scene_viewport_height));
}

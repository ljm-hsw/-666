#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "core/display_config.hpp"

TEST_CASE("default display presents a two-times pixel canvas") {
    const auto display = pixel_town::default_display_config();

    CHECK(display.logical_width == 640);
    CHECK(display.logical_height == 360);
    CHECK(display.window_width == 1280);
    CHECK(display.window_height == 720);
}

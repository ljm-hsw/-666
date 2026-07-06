#pragma once

namespace pixel_town {

struct DisplayConfig {
    int logical_width;
    int logical_height;
    int window_width;
    int window_height;
};

[[nodiscard]] constexpr DisplayConfig default_display_config() noexcept {
    return DisplayConfig{640, 360, 1280, 720};
}

}  // namespace pixel_town

#pragma once

namespace pixel_town {

struct DisplayConfig {
    int logical_width;
    int logical_height;
    int window_width;
    int window_height;
};

[[nodiscard]] constexpr DisplayConfig default_display_config() noexcept {
    return DisplayConfig{960, 540, 960, 540};
}

}  // namespace pixel_town

// 逻辑画布和窗口尺寸契约；保证 raylib 窗口缩放不改变游戏坐标。
#pragma once

#include <algorithm>
#include <optional>

namespace pixel_town {

struct DisplayConfig {
    int logical_width;
    int logical_height;
    int window_width;
    int window_height;
};

struct IntegerViewport {
    int x;
    int y;
    int width;
    int height;
    int scale;
};

struct LogicalPoint {
    int x;
    int y;
};

[[nodiscard]] constexpr DisplayConfig default_display_config() noexcept {
    return DisplayConfig{960, 540, 960, 540};
}

[[nodiscard]] constexpr IntegerViewport integer_scaled_viewport(
    DisplayConfig display, int screen_width, int screen_height) noexcept {
    const int width_scale = screen_width / display.logical_width;
    const int height_scale = screen_height / display.logical_height;
    const int scale = std::max(1, std::min(width_scale, height_scale));
    const int viewport_width = display.logical_width * scale;
    const int viewport_height = display.logical_height * scale;
    return IntegerViewport{(screen_width - viewport_width) / 2,
                           (screen_height - viewport_height) / 2,
                           viewport_width,
                           viewport_height,
                           scale};
}

[[nodiscard]] inline std::optional<LogicalPoint> screen_to_logical_point(
    IntegerViewport viewport, int screen_x, int screen_y) {
    if (screen_x < viewport.x || screen_y < viewport.y ||
        screen_x >= viewport.x + viewport.width || screen_y >= viewport.y + viewport.height) {
        return std::nullopt;
    }
    return LogicalPoint{(screen_x - viewport.x) / viewport.scale,
                        (screen_y - viewport.y) / viewport.scale};
}

}  // namespace pixel_town

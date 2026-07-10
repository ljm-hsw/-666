#pragma once

namespace pixel_town::ui {

inline constexpr int design_width = 640;
inline constexpr int design_height = 360;
inline constexpr int canvas_width = 960;
inline constexpr int canvas_height = 540;
inline constexpr float design_to_canvas_scale = 1.5F;

static_assert(static_cast<int>(design_width * design_to_canvas_scale) == canvas_width);
static_assert(static_cast<int>(design_height * design_to_canvas_scale) == canvas_height);

}  // namespace pixel_town::ui

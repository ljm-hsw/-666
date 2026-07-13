#pragma once

#include "ui/ui_metrics.hpp"

namespace pixel_town::ui {

struct SceneViewportPoint {
    float x{0.0F};
    float y{0.0F};
};

struct SceneViewportRect {
    float x{0.0F};
    float y{0.0F};
    float width{0.0F};
    float height{0.0F};
};

inline constexpr float scene_header_height = 60.0F;
inline constexpr float scene_viewport_height =
    static_cast<float>(canvas_height) - scene_header_height;
inline constexpr float scene_viewport_scale =
    scene_viewport_height / static_cast<float>(canvas_height);
inline constexpr float scene_viewport_width =
    static_cast<float>(canvas_width) * scene_viewport_scale;
inline constexpr float scene_viewport_x =
    (static_cast<float>(canvas_width) - scene_viewport_width) * 0.5F;

[[nodiscard]] constexpr SceneViewportRect indoor_scene_viewport() {
    return SceneViewportRect{scene_viewport_x, scene_header_height,
                             scene_viewport_width, scene_viewport_height};
}

[[nodiscard]] constexpr SceneViewportPoint scene_canvas_to_viewport(
    SceneViewportPoint point) {
    return SceneViewportPoint{scene_viewport_x + point.x * scene_viewport_scale,
                              scene_header_height + point.y * scene_viewport_scale};
}

[[nodiscard]] constexpr SceneViewportRect scene_canvas_to_viewport(
    SceneViewportRect rect) {
    const SceneViewportPoint origin =
        scene_canvas_to_viewport(SceneViewportPoint{rect.x, rect.y});
    return SceneViewportRect{origin.x, origin.y, rect.width * scene_viewport_scale,
                             rect.height * scene_viewport_scale};
}

[[nodiscard]] constexpr SceneViewportPoint scene_design_to_viewport(
    SceneViewportPoint point) {
    return scene_canvas_to_viewport(SceneViewportPoint{
        point.x * design_to_canvas_scale, point.y * design_to_canvas_scale});
}

[[nodiscard]] constexpr SceneViewportRect scene_design_to_viewport(
    SceneViewportRect rect) {
    return scene_canvas_to_viewport(SceneViewportRect{
        rect.x * design_to_canvas_scale, rect.y * design_to_canvas_scale,
        rect.width * design_to_canvas_scale, rect.height * design_to_canvas_scale});
}

[[nodiscard]] constexpr SceneViewportPoint viewport_to_scene_design(
    SceneViewportPoint point) {
    return SceneViewportPoint{
        (point.x - scene_viewport_x) / scene_viewport_scale /
            design_to_canvas_scale,
        (point.y - scene_header_height) / scene_viewport_scale /
            design_to_canvas_scale};
}

[[nodiscard]] constexpr SceneViewportPoint viewport_to_scene_canvas(
    SceneViewportPoint point) {
    return SceneViewportPoint{(point.x - scene_viewport_x) / scene_viewport_scale,
                              (point.y - scene_header_height) /
                                  scene_viewport_scale};
}

[[nodiscard]] constexpr SceneViewportRect scene_design_to_screen_design(
    SceneViewportRect rect) {
    const SceneViewportRect viewport_rect = scene_design_to_viewport(rect);
    return SceneViewportRect{viewport_rect.x / design_to_canvas_scale,
                             viewport_rect.y / design_to_canvas_scale,
                             viewport_rect.width / design_to_canvas_scale,
                             viewport_rect.height / design_to_canvas_scale};
}

}  // namespace pixel_town::ui

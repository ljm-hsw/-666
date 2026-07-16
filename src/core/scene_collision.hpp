// raylib-free 静态碰撞布局：描述可行走区域、家具碰撞体和出口触发区。
#pragma once

#include <string>
#include <vector>

#include "core/game_session.hpp"

namespace pixel_town {

struct ScenePoint {
    float x{0.0F};
    float y{0.0F};
};

struct SceneSize {
    float width{0.0F};
    float height{0.0F};
};

struct SceneRect {
    float x{0.0F};
    float y{0.0F};
    float width{0.0F};
    float height{0.0F};
};

struct SceneCollider {
    std::string id;
    SceneRect bounds;
};

struct IndoorSceneLayout {
    Location location{Location::home};
    int canvas_width{960};
    int canvas_height{540};
    SceneRect walkable_bounds;
    ScenePoint player_spawn;
    SceneRect exit_trigger;
    std::vector<SceneCollider> static_colliders;
};

[[nodiscard]] const IndoorSceneLayout* find_indoor_scene_layout(Location location);
[[nodiscard]] bool scene_rects_overlap(SceneRect left, SceneRect right) noexcept;
[[nodiscard]] bool scene_rect_contains(SceneRect rect, ScenePoint point) noexcept;
[[nodiscard]] SceneRect scene_actor_bounds(ScenePoint center,
                                           SceneSize size) noexcept;
[[nodiscard]] bool scene_position_is_free(const IndoorSceneLayout& layout,
                                          ScenePoint center,
                                          SceneSize size) noexcept;
[[nodiscard]] ScenePoint move_with_scene_collisions(
    const IndoorSceneLayout& layout, ScenePoint current, ScenePoint delta,
    SceneSize size) noexcept;
[[nodiscard]] std::vector<std::string> validate_indoor_scene_layout(
    const IndoorSceneLayout& layout);

}  // namespace pixel_town

// 主角移动、朝向、邻近互动和出口检测；只依赖静态场景碰撞布局。
#include "core/scene_navigation.hpp"

#include <cmath>
#include <limits>
#include <vector>

namespace pixel_town {
namespace {

constexpr SceneSize player_collision_size{24.0F, 24.0F};
constexpr float player_speed = 160.0F;
constexpr const char* navigation_closed_notice = "室内导航尚未打开。";
constexpr const char* invalid_frame_time_notice = "室内导航收到非法帧时间。";
constexpr const char* interaction_unavailable_notice =
    "附近没有可以互动的目标。";
constexpr const char* movement_blocked_notice = "前方被家具或墙体挡住了。";

struct InteractionTarget {
    SceneInteractionKind kind{SceneInteractionKind::npc};
    SceneRect bounds;
    float radius{0.0F};
};

std::vector<InteractionTarget> interaction_targets(Location location) {
    switch (location) {
        case Location::restaurant:
            return {{SceneInteractionKind::npc,
                     SceneRect{108.0F, 117.0F, 192.0F, 150.0F}, 52.0F}};
        case Location::convenience_store:
            return {{SceneInteractionKind::npc,
                     SceneRect{630.0F, 162.0F, 195.0F, 150.0F}, 100.0F}};
        case Location::library:
            return {{SceneInteractionKind::npc,
                     SceneRect{72.0F, 93.0F, 240.0F, 150.0F}, 52.0F}};
        case Location::tavern:
            return {
                {SceneInteractionKind::npc,
                 SceneRect{63.0F, 132.0F, 99.0F, 123.0F}, 56.0F},
                {SceneInteractionKind::primary_activity,
                 SceneRect{48.0F, 309.0F, 168.0F, 102.0F}, 42.0F},
                {SceneInteractionKind::secondary_activity,
                 SceneRect{234.0F, 306.0F, 195.0F, 102.0F}, 42.0F},
            };
        case Location::home:
            return {{SceneInteractionKind::primary_activity,
                     SceneRect{16.0F, 62.0F, 184.0F, 158.0F}, 48.0F}};
    }
    return {};
}

float distance_to_rect(ScenePoint point, SceneRect rect) noexcept {
    const float nearest_x =
        std::max(rect.x, std::min(point.x, rect.x + rect.width));
    const float nearest_y =
        std::max(rect.y, std::min(point.y, rect.y + rect.height));
    const float dx = point.x - nearest_x;
    const float dy = point.y - nearest_y;
    return std::sqrt(dx * dx + dy * dy);
}

std::optional<SceneInteractionKind> nearby_interaction(
    const IndoorSceneLayout& layout, ScenePoint player_position) {
    std::optional<SceneInteractionKind> nearest;
    float nearest_distance = std::numeric_limits<float>::max();
    for (const InteractionTarget& target : interaction_targets(layout.location)) {
        const float distance = distance_to_rect(player_position, target.bounds);
        if (distance <= target.radius && distance < nearest_distance) {
            nearest = target.kind;
            nearest_distance = distance;
        }
    }
    if (nearest.has_value()) {
        return nearest;
    }
    if (scene_rect_contains(layout.exit_trigger, player_position)) {
        return SceneInteractionKind::exit;
    }
    return std::nullopt;
}

bool same_position(ScenePoint left, ScenePoint right) noexcept {
    return left.x == right.x && left.y == right.y;
}

}  // namespace

std::string scene_navigation_glyphs() {
    return std::string{navigation_closed_notice} + invalid_frame_time_notice +
           interaction_unavailable_notice + movement_blocked_notice;
}

bool SceneNavigationRuntime::open(Location location) {
    const IndoorSceneLayout* layout = find_indoor_scene_layout(location);
    if (layout == nullptr ||
        !scene_position_is_free(*layout, layout->player_spawn,
                                player_collision_size)) {
        return false;
    }
    layout_ = layout;
    player_position_ = layout->player_spawn;
    facing_ = SceneFacing::up;
    moving_ = false;
    animation_seconds_ = 0.0F;
    return true;
}

void SceneNavigationRuntime::close() noexcept {
    layout_ = nullptr;
    moving_ = false;
}

SceneNavigationStepResult SceneNavigationRuntime::step(
    const SceneNavigationInput& input) {
    if (layout_ == nullptr) {
        return {SceneNavigationStepStatus::rejected, std::nullopt,
                navigation_closed_notice};
    }
    if (!std::isfinite(input.elapsed_seconds) || input.elapsed_seconds < 0.0F) {
        return {SceneNavigationStepStatus::rejected, std::nullopt,
                invalid_frame_time_notice};
    }

    if (input.interact_pressed) {
        const auto interaction = nearby_interaction(*layout_, player_position_);
        if (!interaction.has_value()) {
            return {SceneNavigationStepStatus::interaction_unavailable,
                    std::nullopt, interaction_unavailable_notice};
        }
        return {SceneNavigationStepStatus::interaction_activated, interaction,
                {}};
    }

    float axis_x = 0.0F;
    float axis_y = 0.0F;
    if (input.move_left != input.move_right) {
        axis_x = input.move_left ? -1.0F : 1.0F;
        facing_ = input.move_left ? SceneFacing::left : SceneFacing::right;
    }
    if (input.move_up != input.move_down) {
        axis_y = input.move_up ? -1.0F : 1.0F;
        facing_ = input.move_up ? SceneFacing::up : SceneFacing::down;
    }
    if (axis_x == 0.0F && axis_y == 0.0F) {
        moving_ = false;
        return {SceneNavigationStepStatus::unchanged, std::nullopt, {}};
    }

    if (axis_x != 0.0F && axis_y != 0.0F) {
        constexpr float inverse_sqrt_two = 0.70710678118F;
        axis_x *= inverse_sqrt_two;
        axis_y *= inverse_sqrt_two;
    }

    const ScenePoint delta{axis_x * player_speed * input.elapsed_seconds,
                           axis_y * player_speed * input.elapsed_seconds};
    const ScenePoint next = move_with_scene_collisions(
        *layout_, player_position_, delta, player_collision_size);
    moving_ = !same_position(next, player_position_);
    player_position_ = next;
    if (!moving_) {
        return {SceneNavigationStepStatus::blocked, std::nullopt,
                movement_blocked_notice};
    }
    animation_seconds_ += input.elapsed_seconds;
    return {SceneNavigationStepStatus::moved, std::nullopt, {}};
}

SceneNavigationPresentation SceneNavigationRuntime::presentation() const noexcept {
    SceneNavigationPresentation view;
    view.active = layout_ != nullptr;
    if (layout_ != nullptr) {
        view.location = layout_->location;
    }
    view.player_position = player_position_;
    view.facing = facing_;
    view.moving = moving_;
    view.animation_seconds = animation_seconds_;
    if (layout_ != nullptr) {
        view.nearby_interaction =
            nearby_interaction(*layout_, player_position_);
    }
    return view;
}

}  // namespace pixel_town

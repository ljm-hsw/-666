// raylib-free 主角导航 Runtime。
//
// 输入被限定为显式轴向移动和互动意图；Runtime 使用场景布局进行速度、斜向
// 归一化、轴分离碰撞、出口和 NPC 邻近检测，最终只返回 presentation。它不拥有
// 地点规则、NPC 对话正文、玩家属性或存档状态，因此可以被 app 层和无窗口测试复用。
#pragma once

#include <optional>
#include <string>

#include "core/scene_collision.hpp"

namespace pixel_town {

enum class SceneFacing {
    down,
    left,
    right,
    up,
};

enum class SceneInteractionKind {
    npc,
    primary_activity,
    secondary_activity,
    exit,
};

struct SceneNavigationInput {
    float elapsed_seconds{0.0F};
    bool move_left{false};
    bool move_right{false};
    bool move_up{false};
    bool move_down{false};
    bool interact_pressed{false};
};

enum class SceneNavigationStepStatus {
    unchanged,
    moved,
    blocked,
    interaction_activated,
    interaction_unavailable,
    rejected,
};

struct SceneNavigationStepResult {
    SceneNavigationStepStatus status{SceneNavigationStepStatus::unchanged};
    std::optional<SceneInteractionKind> interaction;
    std::string notice;
};

struct SceneNavigationPresentation {
    bool active{false};
    Location location{Location::home};
    ScenePoint player_position;
    SceneFacing facing{SceneFacing::up};
    bool moving{false};
    float animation_seconds{0.0F};
    std::optional<SceneInteractionKind> nearby_interaction;
};

[[nodiscard]] std::string scene_navigation_glyphs();

class SceneNavigationRuntime {
public:
    [[nodiscard]] bool open(Location location);
    void close() noexcept;
    [[nodiscard]] SceneNavigationStepResult step(
        const SceneNavigationInput& input);
    [[nodiscard]] SceneNavigationPresentation presentation() const noexcept;
    [[nodiscard]] bool active() const noexcept { return layout_ != nullptr; }

private:
    const IndoorSceneLayout* layout_{nullptr};
    ScenePoint player_position_;
    SceneFacing facing_{SceneFacing::up};
    bool moving_{false};
    float animation_seconds_{0.0F};
};

}  // namespace pixel_town

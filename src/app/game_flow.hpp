// 应用流程协调器。
//
// GameAppState 是页面层的组合状态，GameSession 是其中唯一的持久游戏状态。
// update_game_flow 负责输入路由和页面切换，draw_game_flow 只读取状态绘制；
// 两者都不应重新实现餐馆、便利店、图书馆或酒馆的领域规则。
#pragma once

#include <memory>
#include <optional>
#include <string>

#include <raylib.h>

#include "app/location_runtime.hpp"
#include "app/scene_visual_assets.hpp"
#include "core/game_session.hpp"

namespace pixel_town {

struct GameAppState {
    bool has_session{false};
    GameSession session{GameSession::new_game()};
    bool save_present{false};
    bool resume_available{false};
    GameSession resume_session{GameSession::new_game()};
    bool confirm_new_game_overwrite{false};
    std::optional<Location> location_lobby;
    bool home_preview_open{false};
    bool collision_debug_visible{false};
    std::string notice{"点击新游戏开始第一天。"};
    LocationRuntimeState locations;
};

[[nodiscard]] const char* game_flow_glyphs();
void update_game_flow(GameAppState& state, Vector2 logical_mouse);
void draw_game_flow(const Font& font, const Texture2D& title_background,
                    const Texture2D& town_marker,
                    const Texture2D& kenney_tiles, const Texture2D& generated_full_map_scene,
                    const Texture2D& generated_map_background,
                    const Texture2D& generated_buildings,
                    const SceneVisualAssets& scene_assets, const GameAppState& state,
                    bool audio_enabled, bool paused, Vector2 logical_mouse);

}  // namespace pixel_town

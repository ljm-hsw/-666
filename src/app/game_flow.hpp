#pragma once

#include <string>

#include <raylib.h>

#include "core/game_session.hpp"

namespace pixel_town {

struct GameAppState {
    bool has_session{false};
    GameSession session{GameSession::new_game()};
    std::string notice{"点击“新游戏”开始第一天。"};
};

[[nodiscard]] const char* game_flow_glyphs();
void update_game_flow(GameAppState& state, Vector2 logical_mouse);
void draw_game_flow(const Font& font, const Texture2D& town_marker,
                    const Texture2D& kenney_tiles, const Texture2D& generated_map_background,
                    const Texture2D& generated_buildings, const GameAppState& state,
                    bool audio_enabled, Vector2 logical_mouse);

}  // namespace pixel_town

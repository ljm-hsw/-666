#pragma once

#include <memory>
#include <string>

#include <raylib.h>

#include "core/game_session.hpp"
#include "core/tavern_rules.hpp"
#include "locations/library_data.hpp"
#include "locations/library_rules.hpp"
#include "locations/library_scene.hpp"
#include "locations/library_ui.hpp"
#include "locations/restaurant.hpp"

namespace pixel_town {

struct GameAppState {
    bool has_session{false};
    GameSession session{GameSession::new_game()};
    bool save_present{false};
    bool resume_available{false};
    GameSession resume_session{GameSession::new_game()};
    bool confirm_new_game_overwrite{false};
    std::string notice{"点击新游戏开始第一天。"};
    std::unique_ptr<RestaurantSession> restaurant;
    float restaurant_timer{0.0F};
    ChallengeType tavern_challenge{ChallengeType::gomoku};
    BetTier tavern_bet{BetTier::medium};
    bool in_library{false};
    library::LibraryData library_data;
    std::unique_ptr<library::LibraryRuleEngine> library_engine;
    library::LibraryScene library_scene;
    library::ui::LibraryUIState library_ui_state;
    int library_visits{0};
};

[[nodiscard]] const char* game_flow_glyphs();
void update_game_flow(GameAppState& state, Vector2 logical_mouse);
void draw_game_flow(const Font& font, const Texture2D& title_background,
                    const Texture2D& town_marker,
                    const Texture2D& kenney_tiles, const Texture2D& generated_full_map_scene,
                    const Texture2D& generated_map_background,
                    const Texture2D& generated_buildings, const GameAppState& state,
                    bool audio_enabled, bool paused, Vector2 logical_mouse);

}  // namespace pixel_town

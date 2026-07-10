#pragma once

#include <string>

#include <raylib.h>

#include "core/gomoku_rules.hpp"
#include "core/liars_dice_rules.hpp"
#include "core/tavern_rules.hpp"

namespace pixel_town {

struct GameAppState;

enum class TavernUiScreen {
    lobby,
    game_select,
    npc_dialog,
    gomoku,
    liars_dice,
};

struct TavernUiLayout {
    // ---- lobby hotspots (design coords 640×360) ----
    Rectangle npc_button;
    Rectangle gomoku_table;
    Rectangle dice_table;
    Rectangle game_select_button;
    Rectangle back_button;

    // ---- game_select overlay ----
    Rectangle overlay_panel;
    Rectangle gomoku_card;
    Rectangle dice_card;
    Rectangle bet_low_btn;
    Rectangle bet_med_btn;
    Rectangle bet_high_btn;
    Rectangle start_btn;
    Rectangle cancel_btn;

    // ---- npc ----
    Rectangle npc_sprite;

    // ---- npc_dialog ----
    Rectangle dialog_panel;
    Rectangle dialog_close_btn;

    // ---- gomoku board ----
    float gomoku_board_x;
    float gomoku_board_y;
    float gomoku_cell_size;
    float gomoku_board_pixels;
    float gomoku_info_x;
    float gomoku_info_y;
    Rectangle gomoku_play_abandon_btn;
    Rectangle gomoku_confirm_btn;

    // ---- liars_dice ----
    Rectangle ld_panel;
    Rectangle ld_player_dice[5];
    Rectangle ld_computer_dice[5];
    Rectangle ld_bid_label;
    Rectangle ld_count_down;
    Rectangle ld_count_val;
    Rectangle ld_count_up;
    Rectangle ld_face_down;
    Rectangle ld_face_val;
    Rectangle ld_face_up;
    Rectangle ld_bid_btn;
    Rectangle ld_challenge_btn;
    Rectangle ld_abandon_btn;
    Rectangle ld_result_panel;
    Rectangle ld_confirm_btn;
};

TavernUiLayout build_tavern_layout();

struct TavernUiAssets {
    Texture2D lobby_bg{};
    Texture2D bartender_idle{};
    bool loaded{false};

    void load();
    void unload();
    bool has_lobby_bg() const;
    bool has_bartender() const;
};

struct TavernUiState {
    TavernUiScreen screen{TavernUiScreen::lobby};
    ChallengeType selected_challenge{ChallengeType::gomoku};
    BetTier selected_bet{BetTier::medium};
    GomokuGame gomoku_game{};
    float gomoku_computer_timer{0.0f};
    LiarsDiceGame liars_dice_game{0u};
    int ld_ui_count{1};
    int ld_ui_face{3};
    float ld_computer_timer{0.0f};
    std::string ld_feedback;
    float npc_idle_timer{0.0f};
};

namespace tavern_resources {
    constexpr const char* lobby_bg = "assets/textures/ui/tavern/tavern_lobby.png";
    constexpr const char* bartender_idle =
        "assets/textures/ui/tavern/npc/bartender_idle_sheet.png";
}

// Draw
void draw_tavern_lobby(const Font& font, const GameAppState& state, Vector2 mouse);
void draw_tavern_game_select(const Font& font, const TavernUiState& ui, Vector2 mouse);
void draw_tavern_npc_dialog(const Font& font, const TavernUiState& ui, Vector2 mouse);
void draw_tavern_gomoku(const Font& font, const TavernUiState& ui, Vector2 mouse);
void draw_tavern_liars_dice(const Font& font, const TavernUiState& ui, Vector2 mouse);

// Update
void update_tavern_lobby(GameAppState& state, Vector2 logical_mouse);
void update_tavern_game_select(GameAppState& state, Vector2 logical_mouse);
void update_tavern_npc_dialog(GameAppState& state, Vector2 logical_mouse);
void update_tavern_gomoku(GameAppState& state, Vector2 logical_mouse);
void update_tavern_liars_dice(GameAppState& state, Vector2 logical_mouse);

}  // namespace pixel_town

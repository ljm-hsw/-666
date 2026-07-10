#pragma once

#include <string>

#include <raylib.h>

#include "core/game_session.hpp"
#include "core/tavern_rules.hpp"
#include "locations/gomoku_rules.hpp"
#include "locations/liars_dice_rules.hpp"

namespace pixel_town {

enum class TavernScreen {
    lobby,
    challenge_select,
    npc_dialog,
    gomoku,
    liars_dice,
};

struct TavernLayout {
    Rectangle npc_hotspot;
    Rectangle gomoku_hotspot;
    Rectangle dice_hotspot;
    Rectangle select_button;
    Rectangle back_button;

    Rectangle overlay_panel;
    Rectangle gomoku_card;
    Rectangle dice_card;
    Rectangle low_bet_button;
    Rectangle medium_bet_button;
    Rectangle high_bet_button;
    Rectangle start_button;
    Rectangle cancel_button;

    Rectangle dialog_panel;
    Rectangle dialog_close_button;

    float gomoku_board_x{0.0F};
    float gomoku_board_y{0.0F};
    float gomoku_cell_size{0.0F};
    Rectangle gomoku_confirm_button;
    Rectangle gomoku_abandon_button;

    Rectangle dice_panel;
    Rectangle player_dice[5];
    Rectangle computer_dice[5];
    Rectangle count_down;
    Rectangle count_value;
    Rectangle count_up;
    Rectangle face_down;
    Rectangle face_value;
    Rectangle face_up;
    Rectangle bid_button;
    Rectangle challenge_button;
    Rectangle dice_abandon_button;
    Rectangle round_result_panel;
    Rectangle round_confirm_button;
};

[[nodiscard]] TavernLayout tavern_layout();

struct TavernRuntimeState {
    TavernScreen screen{TavernScreen::lobby};
    ChallengeType selected_challenge{ChallengeType::gomoku};
    BetTier selected_bet{BetTier::medium};
    GomokuGame gomoku{};
    LiarsDiceGame liars_dice{0U};
    float computer_timer{0.0F};
    int bid_count{1};
    int bid_face{1};
    std::string feedback;
    float npc_animation_timer{0.0F};
    Texture2D lobby_background{};
    Texture2D bartender_sheet{};
    bool assets_attempted{false};
};

void prepare_tavern_runtime(TavernRuntimeState& runtime);
void ensure_tavern_assets_loaded(TavernRuntimeState& runtime);
void unload_tavern_assets(TavernRuntimeState& runtime);

[[nodiscard]] bool update_tavern_runtime(GameSession& session,
                                         TavernRuntimeState& runtime,
                                         std::string& notice,
                                         Vector2 logical_mouse);

[[nodiscard]] const char* tavern_ui_glyphs();

}  // namespace pixel_town

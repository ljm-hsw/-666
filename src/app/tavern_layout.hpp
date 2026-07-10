#pragma once

namespace pixel_town {

struct TavernRect {
    float x{0.0F};
    float y{0.0F};
    float width{0.0F};
    float height{0.0F};
};

struct TavernLayout {
    TavernRect npc_hotspot;
    TavernRect gomoku_hotspot;
    TavernRect dice_hotspot;
    TavernRect select_button;
    TavernRect back_button;
    TavernRect overlay_panel;
    TavernRect gomoku_card;
    TavernRect dice_card;
    TavernRect low_bet_button;
    TavernRect medium_bet_button;
    TavernRect high_bet_button;
    TavernRect start_button;
    TavernRect cancel_button;
    TavernRect dialog_panel;
    TavernRect dialog_close_button;
    float gomoku_board_x{0.0F};
    float gomoku_board_y{0.0F};
    float gomoku_cell_size{0.0F};
    TavernRect gomoku_confirm_button;
    TavernRect gomoku_abandon_button;
    TavernRect dice_panel;
    TavernRect player_dice[5];
    TavernRect computer_dice[5];
    TavernRect count_down;
    TavernRect count_value;
    TavernRect count_up;
    TavernRect face_down;
    TavernRect face_value;
    TavernRect face_up;
    TavernRect bid_button;
    TavernRect challenge_button;
    TavernRect dice_abandon_button;
    TavernRect round_result_panel;
    TavernRect round_confirm_button;
};

[[nodiscard]] TavernLayout tavern_layout();

}  // namespace pixel_town

#include "app/tavern_layout.hpp"

namespace pixel_town {

TavernLayout tavern_layout() {
    TavernLayout layout{};
    layout.npc_hotspot = TavernRect{42, 88, 66, 82};
    layout.gomoku_hotspot = TavernRect{32, 206, 112, 68};
    layout.dice_hotspot = TavernRect{156, 204, 130, 68};
    layout.select_button = TavernRect{470, 310, 92, 32};
    layout.back_button = TavernRect{570, 310, 58, 32};
    layout.overlay_panel = TavernRect{108, 52, 424, 286};
    layout.gomoku_card = TavernRect{132, 86, 180, 90};
    layout.dice_card = TavernRect{328, 86, 180, 90};
    layout.low_bet_button = TavernRect{154, 210, 96, 30};
    layout.medium_bet_button = TavernRect{272, 210, 96, 30};
    layout.high_bet_button = TavernRect{390, 210, 96, 30};
    layout.start_button = TavernRect{154, 270, 154, 38};
    layout.cancel_button = TavernRect{332, 270, 154, 38};
    layout.dialog_panel = TavernRect{76, 188, 488, 158};
    layout.dialog_close_button = TavernRect{458, 308, 86, 28};
    layout.gomoku_board_x = 78.0F;
    layout.gomoku_board_y = 66.0F;
    layout.gomoku_cell_size = 18.0F;
    layout.gomoku_confirm_button = TavernRect{420, 238, 140, 36};
    layout.gomoku_abandon_button = TavernRect{420, 286, 140, 34};
    layout.dice_panel = TavernRect{52, 64, 536, 282};
    for (int index = 0; index < 5; ++index) {
        layout.computer_dice[index] =
            TavernRect{84.0F + static_cast<float>(index) * 48.0F, 126, 38, 38};
        layout.player_dice[index] =
            TavernRect{84.0F + static_cast<float>(index) * 48.0F, 270, 38, 38};
    }
    layout.count_down = TavernRect{372, 154, 28, 28};
    layout.count_value = TavernRect{404, 154, 42, 28};
    layout.count_up = TavernRect{450, 154, 28, 28};
    layout.face_down = TavernRect{372, 190, 28, 28};
    layout.face_value = TavernRect{404, 190, 42, 28};
    layout.face_up = TavernRect{450, 190, 28, 28};
    layout.bid_button = TavernRect{372, 234, 92, 34};
    layout.challenge_button = TavernRect{474, 234, 92, 34};
    layout.dice_abandon_button = TavernRect{474, 286, 92, 32};
    layout.round_result_panel = TavernRect{150, 118, 340, 190};
    layout.round_confirm_button = TavernRect{270, 266, 100, 30};
    return layout;
}

}  // namespace pixel_town

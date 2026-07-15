#include "app/tavern_view.hpp"

#include <cstddef>
#include <string>

#include "app/tavern_layout.hpp"
#include "app/npc_sprite_spec.hpp"
#include "app/npc_sprite_view.hpp"
#include "app/ui_primitives.hpp"
#include "ui/scene_viewport.hpp"
#include "ui/ui_metrics.hpp"

namespace pixel_town {
namespace {

constexpr Color tavern_dark{42, 31, 28, 235};
constexpr Color tavern_warm{112, 65, 45, 240};
constexpr Color muted_text{112, 103, 91, 255};
constexpr const char* lobby_background_path =
    "assets/textures/ui/tavern/tavern_lobby.png";

Rectangle to_rectangle(TavernRect bounds) {
    return Rectangle{bounds.x, bounds.y, bounds.width, bounds.height};
}

void panel(Rectangle bounds, Color fill, Color border = ink) {
    pixel_town::panel(bounds, fill, border);
}

void panel(TavernRect bounds, Color fill, Color border = ink) {
    pixel_town::panel(to_rectangle(bounds), fill, border);
}

bool hovered(TavernRect bounds, Vector2 mouse) {
    return pixel_town::hovered(to_rectangle(bounds), mouse);
}

void centered_text(const Font& font, const char* value, Rectangle bounds,
                   float size, Color color = ink) {
    pixel_town::centered_text(font, value, bounds, size, color);
}

void centered_text(const Font& font, const char* value, TavernRect bounds,
                   float size, Color color = ink) {
    pixel_town::centered_text(font, value, to_rectangle(bounds), size, color);
}

void draw_dim_overlay() {
    DrawRectangle(0, 0, ui::canvas_width, ui::canvas_height, Color{10, 8, 6, 170});
}

Camera2D indoor_scene_camera() {
    const auto viewport = ui::indoor_scene_viewport();
    Camera2D camera{};
    camera.offset = Vector2{viewport.x, viewport.y};
    camera.target = Vector2{0.0F, 0.0F};
    camera.rotation = 0.0F;
    camera.zoom = ui::scene_viewport_scale;
    return camera;
}

Vector2 scene_canvas_mouse(Vector2 mouse) {
    const auto transformed =
        ui::viewport_to_scene_canvas({mouse.x, mouse.y});
    return Vector2{transformed.x, transformed.y};
}

void draw_lobby_fallback() {
    DrawRectangle(0, 0, ui::canvas_width, ui::canvas_height,
                  Color{54, 38, 32, 255});
    DrawRectangleRec(scaled_rect(Rectangle{0, 54, 640, 306}), Color{92, 58, 41, 255});
    DrawRectangleRec(scaled_rect(Rectangle{0, 224, 640, 136}), Color{126, 84, 52, 255});
    for (int index = 0; index < 8; ++index) {
        DrawRectangleRec(scaled_rect(Rectangle{18.0F + index * 82.0F, 72, 62, 16}),
                         Color{58, 37, 31, 255});
    }
    DrawRectangleRec(scaled_rect(Rectangle{38, 112, 288, 42}),
                     Color{79, 48, 36, 255});
    DrawRectangleLinesEx(scaled_rect(Rectangle{38, 112, 288, 42}), 3.0F,
                         Color{196, 139, 80, 255});
}

void draw_bartender(const TavernPresentation& presentation,
                    const TavernVisualAssets& assets) {
    const Rectangle destination = scaled_rect(Rectangle{50, 91, 50, 76});
    if (assets.bartender_sheet.id == 0) {
        DrawCircleV(Vector2{destination.x + destination.width * 0.5F,
                            destination.y + destination.height * 0.23F},
                    destination.width * 0.18F, Color{222, 188, 146, 255});
        DrawRectangleRec(Rectangle{destination.x + destination.width * 0.25F,
                                   destination.y + destination.height * 0.37F,
                                   destination.width * 0.5F,
                                   destination.height * 0.55F},
                         Color{55, 49, 45, 255});
        return;
    }
    draw_npc_sprite(assets.bartender_sheet, NpcSpriteKind::bartender,
                    presentation.bartender_animation_seconds, destination);
}

void draw_labeled_hotspot(const Font& font, TavernRect bounds, const char* label,
                          Vector2 mouse) {
    const bool is_hovered = hovered(bounds, mouse);
    const Rectangle label_bounds{bounds.x + 4.0F, bounds.y + bounds.height - 26.0F,
                                 bounds.width - 8.0F, 24.0F};
    panel(label_bounds, is_hovered ? Color{250, 238, 203, 248} : tavern_dark,
          is_hovered ? gold : Color{219, 181, 119, 220});
    centered_text(font, label, label_bounds, 13, is_hovered ? ink : RAYWHITE);
}

void draw_lobby(const Font& font, const TavernPresentation& presentation,
                const TavernVisualAssets& assets, Vector2 mouse) {
    ClearBackground(Color{34, 39, 40, 255});
    const Vector2 transformed_mouse = scene_canvas_mouse(mouse);
    BeginMode2D(indoor_scene_camera());
    if (assets.lobby_background.id != 0) {
        const Rectangle source{0.0F, 0.0F,
                               static_cast<float>(assets.lobby_background.width),
                               static_cast<float>(assets.lobby_background.height)};
        DrawTexturePro(assets.lobby_background, source,
                       scaled_rect(Rectangle{0, 0, 640, 360}), Vector2{0, 0}, 0.0F,
                       WHITE);
    } else {
        draw_lobby_fallback();
    }

    draw_bartender(presentation, assets);
    const TavernLayout layout = tavern_layout();
    draw_labeled_hotspot(font, layout.npc_hotspot, "和酒保交谈", transformed_mouse);
    draw_labeled_hotspot(font, layout.gomoku_hotspot, "五子棋桌", transformed_mouse);
    draw_labeled_hotspot(font, layout.dice_hotspot, "骗子骰子桌", transformed_mouse);
    EndMode2D();

    panel(layout.select_button, hovered(layout.select_button, mouse) ? paper : tavern_warm,
          Color{219, 181, 119, 255});
    centered_text(font, "玩法选择", layout.select_button, 13, RAYWHITE);
    panel(layout.back_button, hovered(layout.back_button, mouse) ? paper : tavern_dark,
          Color{219, 181, 119, 255});
    centered_text(font, "返回", layout.back_button, 13, RAYWHITE);

    if (presentation.challenge_started) {
        panel(Rectangle{452, 274, 176, 30}, Color{150, 69, 59, 245});
        centered_text(font, "未完成挑战：Esc 放弃", Rectangle{452, 274, 176, 30},
                      12, RAYWHITE);
    }
}

void draw_selection(const Font& font, const TavernPresentation& presentation,
                    Vector2 mouse) {
    draw_dim_overlay();
    const TavernLayout layout = tavern_layout();
    panel(layout.overlay_panel, cream);
    text(font, "选择今晚的挑战", 132, 64, 22, red);
    text(font, "一晚只能完成一局，结算后直接进入每日总结。", 132, 184, 13,
         muted_text);

    const auto draw_card = [&](TavernRect bounds, ChallengeType challenge,
                               const char* title, const char* description) {
        const bool selected = presentation.selected_challenge == challenge;
        panel(bounds, selected ? Color{255, 239, 194, 255} : paper,
              selected ? gold : Color{157, 143, 124, 255});
        text(font, title, bounds.x + 14, bounds.y + 14, 18, selected ? red : ink);
        text(font, description, bounds.x + 14, bounds.y + 48, 12, muted_text);
    };
    draw_card(layout.gomoku_card, ChallengeType::gomoku, "五子棋",
              "黑子先手 · 连成五子获胜");
    draw_card(layout.dice_card, ChallengeType::liars_dice, "骗子骰子",
              "提高叫点 · 判断何时质疑");

    text(font, "选择赌注", 132, 212, 14, ink);
    const auto draw_bet = [&](TavernRect bounds, BetTier tier) {
        const bool selected = presentation.selected_bet == tier;
        const std::size_t index = tier == BetTier::low ? 0U
                                  : tier == BetTier::medium ? 1U
                                                           : 2U;
        const std::string label = std::string{bet_tier_label(tier)} + " " +
                                  std::to_string(presentation.bet_amounts[index]) +
                                  "金币";
        panel(bounds, selected ? gold : Color{226, 216, 193, 255},
              selected ? ink : Color{157, 143, 124, 255});
        centered_text(font, label.c_str(), bounds, 13, ink);
    };
    draw_bet(layout.low_bet_button, BetTier::low);
    draw_bet(layout.medium_bet_button, BetTier::medium);
    draw_bet(layout.high_bet_button, BetTier::high);
    if (!presentation.feedback.empty()) {
        text(font, presentation.feedback, 154, 248, 12, red);
    }

    panel(layout.start_button, hovered(layout.start_button, mouse) ? paper : green);
    centered_text(font, "开始挑战", layout.start_button, 15, RAYWHITE);
    panel(layout.cancel_button, hovered(layout.cancel_button, mouse) ? paper
                                                                     : Color{215, 204, 180, 255});
    centered_text(font, "返回大厅", layout.cancel_button, 15, ink);
}

void draw_dialog_character(const TavernVisualAssets& assets,
                           const DialoguePresentation& dialogue) {
    const Rectangle slot{92, 208, 54, 92};
    panel(slot, Color{239, 220, 182, 255}, Color{157, 111, 72, 255});
    const Rectangle destination = scaled_rect(Rectangle{94, 214, 50, 78});
    if (dialogue.speaker == "酒保" && assets.bartender_sheet.id != 0) {
        // Keep the dialogue portrait fixed even while the lobby NPC continues
        // its ambient idle loop behind the modal overlay.
        draw_npc_sprite(assets.bartender_sheet, NpcSpriteKind::bartender,
                        0.0F, destination);
        return;
    }
    if (dialogue.speaker == "主角" && assets.protagonist_sheet.id != 0) {
        draw_npc_sprite(assets.protagonist_sheet, NpcSpriteKind::protagonist,
                        0.0F, destination);
        return;
    }
    const Color coat = dialogue.speaker == "主角" ? green : tavern_dark;
    DrawCircleV(Vector2{destination.x + destination.width * 0.5F,
                        destination.y + destination.height * 0.24F},
                destination.width * 0.18F, Color{222, 188, 146, 255});
    DrawRectangleRec(Rectangle{destination.x + destination.width * 0.24F,
                               destination.y + destination.height * 0.4F,
                               destination.width * 0.52F,
                               destination.height * 0.48F},
                     coat);
}

void draw_npc_dialog(const Font& font, const TavernPresentation& presentation,
                     const TavernVisualAssets& assets, Vector2 mouse) {
    draw_dim_overlay();
    const TavernLayout layout = tavern_layout();
    panel(layout.dialog_panel, cream);
    if (!presentation.dialogue.has_value()) {
        text(font, "对话暂时不可用。", 160, 236, 14, red);
        return;
    }
    const DialoguePresentation& dialogue = *presentation.dialogue;
    draw_dialog_character(assets, dialogue);
    text(font, dialogue.speaker, 160, 202, 18, red);
    const std::string progress = std::to_string(dialogue.current_line) + " / " +
                                 std::to_string(dialogue.total_lines);
    text(font, progress, 486, 204, 12, muted_text);
    const auto lines = wrap_text_lines(dialogue.text, 24, 3);
    for (std::size_t index = 0; index < lines.size(); ++index) {
        text(font, lines[index], 160, 236 + static_cast<float>(index) * 22.0F, 14,
             ink);
    }
    text(font, "Enter / Space / 点击继续 · Esc 跳过", 160, 316, 12,
         muted_text);
    panel(layout.dialog_close_button,
          hovered(layout.dialog_close_button, mouse) ? paper
                                                      : Color{215, 204, 180, 255});
    const char* button_label = dialogue.current_line == dialogue.total_lines
                                   ? "关闭"
                                   : "下一句";
    centered_text(font, button_label, layout.dialog_close_button, 13, ink);
}

float board_point(int index, float origin, float cell_size) {
    return origin + static_cast<float>(index) * cell_size;
}

void draw_gomoku(const Font& font, const TavernPresentation& presentation,
                 Vector2 mouse) {
    ClearBackground(Color{56, 43, 38, 255});
    if (!presentation.gomoku.has_value()) {
        return;
    }
    const TavernGomokuPresentation& game = *presentation.gomoku;
    const TavernLayout layout = tavern_layout();
    const float board_pixels = 14.0F * layout.gomoku_cell_size;
    const Rectangle board_background{layout.gomoku_board_x - 14.0F,
                                     layout.gomoku_board_y - 14.0F,
                                     board_pixels + 28.0F, board_pixels + 28.0F};
    panel(board_background, Color{222, 184, 112, 255});

    for (int index = 0; index < GomokuGame::kSize; ++index) {
        const float position_x = board_point(index, layout.gomoku_board_x,
                                             layout.gomoku_cell_size);
        const float position_y = board_point(index, layout.gomoku_board_y,
                                             layout.gomoku_cell_size);
        DrawLineEx(scaled_point(Vector2{layout.gomoku_board_x, position_y}),
                   scaled_point(Vector2{layout.gomoku_board_x + board_pixels, position_y}),
                   1.5F, Color{60, 55, 45, 255});
        DrawLineEx(scaled_point(Vector2{position_x, layout.gomoku_board_y}),
                   scaled_point(Vector2{position_x, layout.gomoku_board_y + board_pixels}),
                   1.5F, Color{60, 55, 45, 255});
    }

    for (int row = 0; row < GomokuGame::kSize; ++row) {
        for (int col = 0; col < GomokuGame::kSize; ++col) {
            const GomokuCell cell = game.board[row][col];
            if (cell == GomokuCell::empty) {
                continue;
            }
            const Vector2 center = scaled_point(Vector2{
                board_point(col, layout.gomoku_board_x, layout.gomoku_cell_size),
                board_point(row, layout.gomoku_board_y, layout.gomoku_cell_size)});
            const float radius = scaled(layout.gomoku_cell_size * 0.43F);
            DrawCircleV(center, radius,
                        cell == GomokuCell::player ? Color{28, 28, 30, 255}
                                                   : Color{239, 234, 220, 255});
            DrawCircleLinesV(center, radius, Color{35, 35, 35, 255});
        }
    }

    panel(Rectangle{392, 66, 196, 266}, cream);
    text(font, "五子棋", 414, 82, 22, red);
    const std::string bet = std::string{bet_tier_label(presentation.selected_bet)} +
                            "赌注 · " +
                            std::to_string(presentation.selected_bet_amount) +
                            "金币";
    text(font, bet, 414, 118, 14, ink);
    text(font, "你执黑子，点击交叉点落子。", 414, 148, 12, muted_text);
    text(font, "先连成五子的一方获胜。", 414, 174, 12, muted_text);

    const char* status = "你的回合";
    if (game.state == GomokuState::player_wins) {
        status = "你赢了！";
    } else if (game.state == GomokuState::computer_wins) {
        status = "电脑获胜。";
    } else if (game.state == GomokuState::draw) {
        status = "棋盘已满，平局。";
    } else if (game.turn == GomokuTurn::computer) {
        status = "电脑思考中……";
    }
    text(font, status, 414, 206, 17,
         game.state == GomokuState::playing ? ink : red);

    if (game.state == GomokuState::playing) {
        panel(layout.gomoku_abandon_button,
              hovered(layout.gomoku_abandon_button, mouse) ? paper : red);
        centered_text(font, "主动放弃 (Esc)", layout.gomoku_abandon_button, 14,
                      RAYWHITE);
    } else {
        panel(layout.gomoku_confirm_button,
              hovered(layout.gomoku_confirm_button, mouse) ? paper : green);
        centered_text(font, "确认结算", layout.gomoku_confirm_button, 14, RAYWHITE);
        if (!presentation.feedback.empty()) {
            text(font, presentation.feedback, 414, 286, 11, red);
        }
    }
}

void draw_die(const Font& font, TavernRect bounds, const char* value, bool active,
              bool hidden) {
    panel(bounds, active ? (hidden ? tavern_dark : paper)
                         : Color{194, 188, 176, 255},
          active ? ink : Color{145, 139, 128, 255});
    centered_text(font, active ? value : "×", bounds, 17,
                  hidden ? RAYWHITE : ink);
}

void draw_round_result(const Font& font, const TavernLiarsDicePresentation& game,
                       Vector2 mouse) {
    draw_dim_overlay();
    const TavernLayout layout = tavern_layout();
    panel(layout.round_result_panel, cream);
    text(font, "揭晓这一轮", 174, 132, 20, red);
    const std::string bid = "叫点 " + std::to_string(game.current_bid_count) + " 个 " +
                            std::to_string(game.current_bid_face) + "，实际 " +
                            std::to_string(game.actual_count) + " 个。";
    text(font, bid, 174, 170, 14, ink);
    text(font, game.bid_was_valid ? "叫点成立。" : "叫点不成立。", 174, 198,
         15, game.bid_was_valid ? green : red);
    const bool player_lost = game.round_loser == LiarsDiceParticipant::player;
    const char* outcome = game.game_over
                              ? (game.player_won ? "你赢下整场比赛。" : "你输掉了整场比赛。")
                              : (player_lost ? "你失去一枚骰子。"
                                             : "电脑失去一枚骰子。");
    text(font, outcome, 174, 226, 15, player_lost ? red : green);
    const std::string remaining = "剩余骰子：你 " +
                                  std::to_string(game.player_dice_count) +
                                  " / 电脑 " +
                                  std::to_string(game.computer_dice_count);
    text(font, remaining, 174, 250, 13, muted_text);
    panel(layout.round_confirm_button,
          hovered(layout.round_confirm_button, mouse) ? paper : green);
    centered_text(font, game.game_over ? "确认结算" : "下一轮",
                  layout.round_confirm_button, 13, RAYWHITE);
}

void draw_liars_dice(const Font& font, const TavernPresentation& presentation,
                     Vector2 mouse) {
    ClearBackground(Color{54, 38, 33, 255});
    if (!presentation.liars_dice.has_value()) {
        return;
    }
    const TavernLayout layout = tavern_layout();
    const TavernLiarsDicePresentation& game = *presentation.liars_dice;
    panel(layout.dice_panel, cream);
    text(font, "骗子骰子", 74, 76, 22, red);
    text(font, "一点可代替其他点数；叫一点时只计算一点。", 226, 82, 12,
         muted_text);

    text(font, "电脑骰子", 74, 104, 14, ink);
    for (int index = 0; index < LiarsDiceGame::kDiceCount; ++index) {
        const TavernDiePresentation& die = game.computer_dice[index];
        const bool hidden = die.active && !die.visible_face.has_value();
        const std::string value = hidden ? "?" : std::to_string(die.visible_face.value_or(0));
        draw_die(font, layout.computer_dice[index], value.c_str(), die.active, hidden);
    }

    text(font, "你的骰子", 74, 248, 14, ink);
    for (int index = 0; index < LiarsDiceGame::kDiceCount; ++index) {
        const TavernDiePresentation& die = game.player_dice[index];
        const std::string value = std::to_string(die.visible_face.value_or(0));
        draw_die(font, layout.player_dice[index], value.c_str(), die.active, false);
    }

    const std::string current_bid = game.current_bid_count == 0
                                        ? "当前：尚无叫点"
                                        : "当前：" + std::to_string(game.current_bid_count) +
                                              " 个 " + std::to_string(game.current_bid_face);
    text(font, current_bid, 366, 118, 15, gold);
    text(font, game.player_turn ? "轮到你行动" : "电脑思考中……", 366, 140,
         13, ink);

    text(font, "数量", 326, 160, 13, ink);
    panel(layout.count_down, hovered(layout.count_down, mouse) ? paper
                                                               : Color{225, 215, 194, 255});
    centered_text(font, "-", layout.count_down, 16, ink);
    panel(layout.count_value, paper);
    centered_text(font, std::to_string(game.proposed_bid_count).c_str(), layout.count_value,
                  16, ink);
    panel(layout.count_up, hovered(layout.count_up, mouse) ? paper
                                                           : Color{225, 215, 194, 255});
    centered_text(font, "+", layout.count_up, 16, ink);

    text(font, "点数", 326, 196, 13, ink);
    panel(layout.face_down, hovered(layout.face_down, mouse) ? paper
                                                             : Color{225, 215, 194, 255});
    centered_text(font, "-", layout.face_down, 16, ink);
    panel(layout.face_value, paper);
    centered_text(font, std::to_string(game.proposed_bid_face).c_str(), layout.face_value,
                  16, ink);
    panel(layout.face_up, hovered(layout.face_up, mouse) ? paper
                                                         : Color{225, 215, 194, 255});
    centered_text(font, "+", layout.face_up, 16, ink);

    panel(layout.bid_button, hovered(layout.bid_button, mouse) ? paper : green);
    centered_text(font, "叫点", layout.bid_button, 14, RAYWHITE);
    const bool can_challenge = game.current_bid_count > 0;
    panel(layout.challenge_button,
          can_challenge ? (hovered(layout.challenge_button, mouse) ? paper : red)
                        : Color{180, 175, 165, 255});
    centered_text(font, "质疑", layout.challenge_button, 14,
                  can_challenge ? RAYWHITE : muted_text);
    panel(layout.dice_abandon_button,
          hovered(layout.dice_abandon_button, mouse) ? paper
                                                      : Color{211, 202, 184, 255});
    centered_text(font, "放弃", layout.dice_abandon_button, 13, ink);
    if (!presentation.feedback.empty()) {
        text(font, presentation.feedback, 74, 316, 12, red);
    }

    if (game.round_over) {
        draw_round_result(font, game, mouse);
    }
}

}  // namespace

void ensure_tavern_assets_loaded(TavernVisualAssets& assets) {
    if (assets.attempted) {
        return;
    }
    assets.attempted = true;
    if (assets.lobby_background.id == 0) {
        assets.lobby_background = LoadTexture(lobby_background_path);
    }
    if (assets.bartender_sheet.id == 0) {
        assets.bartender_sheet =
            LoadTexture(npc_sprite_spec(NpcSpriteKind::bartender).runtime_path);
    }
    if (assets.protagonist_sheet.id == 0) {
        assets.protagonist_sheet =
            LoadTexture(npc_sprite_spec(NpcSpriteKind::protagonist).runtime_path);
    }
    if (assets.lobby_background.id != 0) {
        SetTextureFilter(assets.lobby_background, TEXTURE_FILTER_POINT);
    }
    if (assets.bartender_sheet.id != 0) {
        SetTextureFilter(assets.bartender_sheet, TEXTURE_FILTER_POINT);
    }
    if (assets.protagonist_sheet.id != 0) {
        SetTextureFilter(assets.protagonist_sheet, TEXTURE_FILTER_POINT);
    }
}

void unload_tavern_assets(TavernVisualAssets& assets) {
    if (assets.lobby_background.id != 0) {
        UnloadTexture(assets.lobby_background);
        assets.lobby_background = {};
    }
    if (assets.bartender_sheet.id != 0) {
        UnloadTexture(assets.bartender_sheet);
        assets.bartender_sheet = {};
    }
    if (assets.protagonist_sheet.id != 0) {
        UnloadTexture(assets.protagonist_sheet);
        assets.protagonist_sheet = {};
    }
    assets.attempted = false;
}

void draw_tavern_view(const Font& font, const TavernPresentation& presentation,
                      const TavernVisualAssets& assets, Vector2 logical_mouse) {
    switch (presentation.screen) {
        case TavernScreen::lobby:
            draw_lobby(font, presentation, assets, logical_mouse);
            break;
        case TavernScreen::challenge_select:
            draw_lobby(font, presentation, assets, logical_mouse);
            draw_selection(font, presentation, logical_mouse);
            break;
        case TavernScreen::npc_dialog:
            draw_lobby(font, presentation, assets, logical_mouse);
            draw_npc_dialog(font, presentation, assets, logical_mouse);
            break;
        case TavernScreen::gomoku:
            draw_gomoku(font, presentation, logical_mouse);
            break;
        case TavernScreen::liars_dice:
            draw_liars_dice(font, presentation, logical_mouse);
            break;
    }
}

}  // namespace pixel_town

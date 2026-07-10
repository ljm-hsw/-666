#include "app/tavern_ui.hpp"

#include <cmath>
#include <string>

#include "app/game_flow.hpp"

namespace pixel_town {
namespace {

constexpr float kScale = 1.5F;
constexpr Color ink{45, 52, 54, 255};
constexpr Color paper{250, 238, 203, 255};
constexpr Color cream{255, 248, 226, 255};
constexpr Color green{82, 137, 92, 255};
constexpr Color red{183, 83, 72, 255};
constexpr Color gold{224, 169, 74, 255};
constexpr Color shadow{39, 48, 53, 120};

constexpr int kBartenderFrames = 6;
constexpr int kBartenderFrameW = 64;
constexpr int kBartenderFrameH = 64;
constexpr float kBartenderFrameDuration = 0.15F;

float scaled(float value) { return std::round(value * kScale); }

Vector2 scaled_point(Vector2 value) {
    return Vector2{scaled(value.x), scaled(value.y)};
}

Rectangle scaled_rect(Rectangle value) {
    return Rectangle{scaled(value.x), scaled(value.y), scaled(value.width), scaled(value.height)};
}

float scaled_font_size(float design_size) {
    if (design_size <= 20.0F) return 24.0F;
    if (design_size <= 30.0F) return 36.0F;
    return 48.0F;
}

void draw_text(const Font& font, const char* value, float x, float y, float size,
               Color color = ink) {
    DrawTextEx(font, value, Vector2{scaled(x), scaled(y)}, scaled_font_size(size), 1.0F, color);
}

void draw_text(const Font& font, const std::string& value, float x, float y, float size,
               Color color = ink) {
    draw_text(font, value.c_str(), x, y, size, color);
}

void draw_centered_text(const Font& font, const char* value, Rectangle bounds, float size,
                        Color color = ink) {
    const Rectangle sb = scaled_rect(bounds);
    const float font_size = scaled_font_size(size);
    const Vector2 measured = MeasureTextEx(font, value, font_size, 1.0F);
    DrawTextEx(font, value,
               Vector2{sb.x + (sb.width - measured.x) / 2.0F,
                       sb.y + (sb.height - measured.y) / 2.0F},
               font_size, 1.0F, color);
}

void draw_panel(Rectangle bounds, Color fill, Color border = ink) {
    const Rectangle sb = scaled_rect(bounds);
    DrawRectangleRec(Rectangle{sb.x + 4, sb.y + 4, sb.width, sb.height}, shadow);
    DrawRectangleRec(sb, fill);
    DrawRectangleLinesEx(sb, 3.0F, border);
}

bool is_clicked(Rectangle bounds, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, scaled_rect(bounds)) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool is_hovered(Rectangle bounds, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, scaled_rect(bounds));
}

int gomoku_col_from_mouse(float design_mouse_x, float board_x, float cell_size) {
    return static_cast<int>((design_mouse_x - board_x + cell_size * 0.5F) / cell_size);
}

int gomoku_row_from_mouse(float design_mouse_y, float board_y, float cell_size) {
    return static_cast<int>((design_mouse_y - board_y + cell_size * 0.5F) / cell_size);
}

float gomoku_intersection_x(int col, float board_x, float cell_size) {
    return board_x + static_cast<float>(col) * cell_size;
}

float gomoku_intersection_y(int row, float board_y, float cell_size) {
    return board_y + static_cast<float>(row) * cell_size;
}

void draw_lobby_background_placeholder() {
    DrawRectangleRec(scaled_rect(Rectangle{12, 58, 616, 290}), Color{40, 32, 28, 255});
    for (int i = 0; i < 8; ++i) {
        const float lx = 22 + static_cast<float>(i) * 76;
        DrawRectangleRec(scaled_rect(Rectangle{lx, 294, 62, 14}), Color{55, 44, 38, 255});
    }
    const float cx = 540;
    const float cy = 68;
    DrawRectangleRec(scaled_rect(Rectangle{cx, cy, 14, 40}), Color{180, 155, 110, 255});
    DrawCircleV(scaled_point(Vector2{cx + 7, cy - 4}), scaled(10), Color{255, 200, 80, 200});
    DrawCircleV(scaled_point(Vector2{cx + 7, cy - 8}), scaled(6), Color{255, 230, 140, 230});
}

void draw_npc_fallback(Rectangle sprite_rect) {
    const Rectangle sr = scaled_rect(sprite_rect);
    const float cx = sr.x + sr.width * 0.5F;
    const float cy = sr.y + sr.height * 0.18F;
    const float head_r = sr.width * 0.22F;
    DrawCircleV(Vector2{cx, cy}, head_r, Color{220, 190, 150, 255});
    DrawCircleLinesV(Vector2{cx, cy}, head_r, ink);
    DrawCircleV(Vector2{cx - head_r * 0.35F, cy - head_r * 0.1F}, 2.5F * kScale, ink);
    DrawCircleV(Vector2{cx + head_r * 0.35F, cy - head_r * 0.1F}, 2.5F * kScale, ink);
    const float body_w = sr.width * 0.56F;
    const float body_h = sr.height * 0.62F;
    const float body_x = cx - body_w * 0.5F;
    const float body_y = cy + head_r;
    DrawRectangleRec(Rectangle{body_x + 3, body_y + 3, body_w, body_h}, shadow);
    DrawRectangleRec(Rectangle{body_x, body_y, body_w, body_h}, Color{60, 52, 45, 255});
    DrawRectangleLinesEx(Rectangle{body_x, body_y, body_w, body_h}, 2.5F * kScale, ink);
    const float apron_w = body_w * 0.76F;
    const float apron_h = body_h * 0.55F;
    DrawRectangleRec(Rectangle{body_x + (body_w - apron_w) * 0.5F, body_y + body_h * 0.32F,
                               apron_w, apron_h},
                     Color{245, 238, 220, 255});
}

void draw_overlay_dim() {
    DrawRectangle(0, 0, 960, 540, Color{10, 8, 6, 160});
}

}  // namespace

// ---- layout ----

TavernUiLayout build_tavern_layout() {
    TavernUiLayout L{};

    L.npc_sprite           = Rectangle{42, 146, 50, 56};
    L.npc_button           = Rectangle{36, 140, 62, 68};
    L.gomoku_table         = Rectangle{160, 228, 100, 64};
    L.dice_table           = Rectangle{340, 228, 100, 64};
    L.game_select_button   = Rectangle{520, 280, 100, 34};
    L.back_button          = Rectangle{550, 312, 70, 30};

    L.overlay_panel = Rectangle{120, 50, 400, 270};
    L.gomoku_card   = Rectangle{140, 72, 360, 55};
    L.dice_card     = Rectangle{140, 138, 360, 55};

    L.bet_low_btn  = Rectangle{140, 210, 72, 26};
    L.bet_med_btn  = Rectangle{222, 210, 72, 26};
    L.bet_high_btn = Rectangle{304, 210, 72, 26};

    L.start_btn  = Rectangle{140, 255, 170, 36};
    L.cancel_btn = Rectangle{330, 255, 170, 36};

    L.dialog_panel     = Rectangle{120, 240, 400, 100};
    L.dialog_close_btn = Rectangle{440, 248, 60, 28};

    L.gomoku_board_x      = 102.0F;
    L.gomoku_board_y      = 66.0F;
    L.gomoku_cell_size    = 18.0F;
    L.gomoku_board_pixels = 14.0F * 18.0F;
    L.gomoku_info_x       = 392.0F;
    L.gomoku_info_y       = 68.0F;
    L.gomoku_play_abandon_btn = Rectangle{392, 232, 130, 34};
    L.gomoku_confirm_btn      = Rectangle{392, 212, 130, 34};

    L.ld_panel = Rectangle{70, 70, 500, 280};
    for (int i = 0; i < 5; ++i) {
        L.ld_player_dice[i] = Rectangle{100.0F + i * 48.0F, 248.0F, 38.0F, 38.0F};
        L.ld_computer_dice[i] = Rectangle{100.0F + i * 48.0F, 106.0F, 38.0F, 38.0F};
    }
    L.ld_bid_label     = Rectangle{340, 96, 200, 24};
    L.ld_count_down    = Rectangle{340, 148, 28, 28};
    L.ld_count_val     = Rectangle{372, 148, 36, 28};
    L.ld_count_up      = Rectangle{412, 148, 28, 28};
    L.ld_face_down     = Rectangle{340, 184, 28, 28};
    L.ld_face_val      = Rectangle{372, 184, 36, 28};
    L.ld_face_up       = Rectangle{412, 184, 28, 28};
    L.ld_bid_btn       = Rectangle{340, 230, 90, 36};
    L.ld_challenge_btn = Rectangle{440, 230, 90, 36};
    L.ld_abandon_btn   = Rectangle{340, 276, 90, 34};
    L.ld_result_panel  = Rectangle{160, 140, 320, 160};
    L.ld_confirm_btn   = Rectangle{280, 260, 80, 30};

    return L;
}

// ---- assets ----

void TavernUiAssets::load() {
    if (loaded) return;
    loaded = true;
    lobby_bg = LoadTexture(tavern_resources::lobby_bg);
    if (lobby_bg.id != 0) {
        SetTextureFilter(lobby_bg, TEXTURE_FILTER_POINT);
    }
    bartender_idle = LoadTexture(tavern_resources::bartender_idle);
    if (bartender_idle.id != 0) {
        SetTextureFilter(bartender_idle, TEXTURE_FILTER_POINT);
    }
}

void TavernUiAssets::unload() {
    if (lobby_bg.id != 0) {
        UnloadTexture(lobby_bg);
        lobby_bg = {};
    }
    if (bartender_idle.id != 0) {
        UnloadTexture(bartender_idle);
        bartender_idle = {};
    }
    loaded = false;
}

bool TavernUiAssets::has_lobby_bg() const {
    return lobby_bg.id != 0;
}

bool TavernUiAssets::has_bartender() const {
    return bartender_idle.id != 0;
}

// ---- draw: lobby ----

void draw_tavern_lobby(const Font& font, const GameAppState& state, Vector2 mouse) {
    const auto& assets = state.tavern_assets;
    const auto L = build_tavern_layout();
    const auto& ui = state.tavern_ui;

    if (assets.has_lobby_bg()) {
        const Rectangle source{0, 1, 480, 270};
        const Rectangle dest{0, 0, 960, 540};
        DrawTexturePro(assets.lobby_bg, source, dest, Vector2{0, 0}, 0.0F, WHITE);
    } else {
        draw_lobby_background_placeholder();
    }

    // ---- NPC idle animation ----
    if (assets.has_bartender()) {
        const int frame =
            static_cast<int>(ui.npc_idle_timer / kBartenderFrameDuration) % kBartenderFrames;
        const Rectangle src{static_cast<float>(frame * kBartenderFrameW), 0,
                            static_cast<float>(kBartenderFrameW),
                            static_cast<float>(kBartenderFrameH)};
        DrawTexturePro(assets.bartender_idle, src, scaled_rect(L.npc_sprite),
                       Vector2{0, 0}, 0.0F, WHITE);
    } else {
        draw_npc_fallback(L.npc_sprite);
    }

    // ---- hotspot highlight on hover ----
    constexpr bool kDebugHotspots = false;
    const auto draw_hotspot_hint = [&](Rectangle r, const char* label) {
        if (!is_hovered(r, mouse)) return;
        const Rectangle sr = scaled_rect(r);
        if (kDebugHotspots) {
            DrawRectangleRec(sr, Color{255, 255, 200, 45});
        }
        DrawRectangleLinesEx(sr, 1.5F, Color{255, 255, 255, 70});
        draw_text(font, label, r.x, r.y - 13, 11, Color{255, 245, 210, 220});
    };
    draw_hotspot_hint(L.npc_button, "酒保");
    draw_hotspot_hint(L.gomoku_table, "五子棋");
    draw_hotspot_hint(L.dice_table, "骗子骰子");

    // ---- back button ----
    const bool hover_back = is_hovered(L.back_button, mouse);
    draw_panel(L.back_button, hover_back ? paper : Color{50, 42, 36, 220});
    draw_centered_text(font, "返回", L.back_button, 14,
                       hover_back ? ink : Color{220, 210, 190, 255});

    // ---- generic game select fallback button ----
    if (!state.session.location_started()) {
        const bool hover_gs = is_hovered(L.game_select_button, mouse);
        draw_panel(L.game_select_button, hover_gs ? paper : Color{50, 42, 36, 220});
        draw_centered_text(font, "玩法选择", L.game_select_button, 13,
                           hover_gs ? ink : Color{220, 210, 190, 255});
    }

    // A started location can only reach the lobby after an interrupted round.
    if (state.session.location_started()) {
        const Rectangle abandon_btn{480, 318, 100, 30};
        draw_text(font, "本局尚未结算", 478, 296, 12, Color{255, 238, 205, 255});
        const bool hover_a = is_hovered(abandon_btn, mouse);
        draw_panel(abandon_btn, hover_a ? paper : red);
        draw_centered_text(font, "结束本局", abandon_btn, 13, RAYWHITE);
    }
}

// ---- draw: game_select overlay ----

void draw_tavern_game_select(const Font& font, const TavernUiState& ui, Vector2 mouse) {
    draw_overlay_dim();
    const auto L = build_tavern_layout();

    draw_panel(L.overlay_panel, cream, ink);
    draw_text(font, "选择挑战", L.overlay_panel.x + 16, L.overlay_panel.y + 10, 22, red);

    const auto draw_card = [&](Rectangle r, ChallengeType type, const char* title,
                                const char* subtitle, const char* icon) {
        const bool sel = ui.selected_challenge == type;
        const bool hover = is_hovered(r, mouse);
        const Color fill = sel ? Color{255, 248, 226, 255}
                               : (hover ? Color{240, 234, 214, 255} : Color{250, 245, 235, 255});
        draw_panel(r, fill, sel ? gold : Color{160, 155, 140, 255});
        const float icon_w = 38;
        DrawRectangleRec(scaled_rect(Rectangle{r.x + 8, r.y + 8, icon_w, r.height - 16}),
                         Color{55, 65, 58, 255});
        draw_centered_text(font, icon,
                           Rectangle{r.x + 8, r.y + 8, icon_w, r.height - 16}, 16, RAYWHITE);
        draw_text(font, title, r.x + 54, r.y + 8, 18, ink);
        draw_text(font, subtitle, r.x + 54, r.y + 30, 12, Color{100, 95, 85, 255});
    };
    draw_card(L.gomoku_card, ChallengeType::gomoku, "五子棋", "策略对弈 · 与酒保对局", "棋");
    draw_card(L.dice_card, ChallengeType::liars_dice, "骗子骰子", "运气博弈 · 看穿谎言", "骰");

    draw_text(font, "赌注", L.bet_low_btn.x, L.bet_low_btn.y - 16, 14, ink);
    const auto draw_bet = [&](Rectangle r, BetTier tier) {
        const bool sel = ui.selected_bet == tier;
        const bool hover = is_hovered(r, mouse);
        draw_panel(r, sel ? gold : (hover ? paper : Color{230, 225, 212, 255}),
                   sel ? ink : Color{160, 155, 140, 255});
        const TavernChallengeConfig config;
        draw_centered_text(font,
                           (std::string{bet_tier_label(tier)} + " " +
                            std::to_string(bet_amount(config, tier)) + "G").c_str(),
                           r, 12, sel ? ink : Color{80, 75, 65, 255});
    };
    draw_bet(L.bet_low_btn, BetTier::low);
    draw_bet(L.bet_med_btn, BetTier::medium);
    draw_bet(L.bet_high_btn, BetTier::high);

    {
        const bool hover_s = is_hovered(L.start_btn, mouse);
        draw_panel(L.start_btn, hover_s ? paper : green);
        draw_centered_text(font, "开始挑战", L.start_btn, 16, RAYWHITE);
    }
    {
        const bool hover_c = is_hovered(L.cancel_btn, mouse);
        draw_panel(L.cancel_btn, hover_c ? paper : Color{211, 202, 174, 255});
        draw_centered_text(font, "返回大厅", L.cancel_btn, 16, ink);
    }
}

// ---- draw: npc_dialog ----

void draw_tavern_npc_dialog(const Font& font, const TavernUiState& ui, Vector2 mouse) {
    (void)ui;
    draw_overlay_dim();
    const auto L = build_tavern_layout();

    draw_panel(L.dialog_panel, cream, ink);
    draw_text(font, "酒保", L.dialog_panel.x + 16, L.dialog_panel.y + 10, 20, red);
    draw_text(font, "欢迎来到像素小镇酒馆！想试试手气吗？", L.dialog_panel.x + 16,
              L.dialog_panel.y + 46, 15, ink);
    draw_text(font, "左边是五子棋桌，右边是骗子骰子。选好玩法后告诉我。",
              L.dialog_panel.x + 16, L.dialog_panel.y + 68, 13, Color{80, 75, 65, 255});

    const bool hover_close = is_hovered(L.dialog_close_btn, mouse);
    draw_panel(L.dialog_close_btn, hover_close ? paper : Color{211, 202, 174, 255});
    draw_centered_text(font, "关闭", L.dialog_close_btn, 13, ink);
    draw_text(font, "点击关闭或 Esc", L.dialog_panel.x + 16, L.dialog_panel.y + 84, 11,
              Color{140, 135, 125, 255});
}

// ---- draw: gomoku (unchanged) ----

void draw_tavern_gomoku(const Font& font, const TavernUiState& tavern_ui, Vector2 mouse) {
    const auto L = build_tavern_layout();
    const auto& game = tavern_ui.gomoku_game;
    const float bx = L.gomoku_board_x;
    const float by = L.gomoku_board_y;
    const float cs = L.gomoku_cell_size;
    const float bp = L.gomoku_board_pixels;

    const float dmx = mouse.x / kScale;
    const float dmy = mouse.y / kScale;

    const Rectangle board_bg{bx - 14, by - 14, bp + 28, bp + 28};
    DrawRectangleRec(scaled_rect(Rectangle{board_bg.x + 4, board_bg.y + 4, board_bg.width,
                                           board_bg.height}),
                     shadow);
    DrawRectangleRec(scaled_rect(board_bg), Color{222, 184, 112, 255});
    DrawRectangleLinesEx(scaled_rect(board_bg), 3.0F, ink);

    for (int i = 0; i < GomokuGame::kSize; ++i) {
        const float pos_x = gomoku_intersection_x(i, bx, cs);
        const float pos_y = gomoku_intersection_y(i, by, cs);
        DrawLineEx(scaled_point(Vector2{bx, pos_y}),
                   scaled_point(Vector2{bx + bp, pos_y}), 1.5F, Color{60, 55, 45, 255});
        DrawLineEx(scaled_point(Vector2{pos_x, by}),
                   scaled_point(Vector2{pos_x, by + bp}), 1.5F, Color{60, 55, 45, 255});
    }

    constexpr int kStarPoints[][2] = {{3, 3},  {3, 7},  {3, 11}, {7, 3}, {7, 7},
                                       {7, 11}, {11, 3}, {11, 7}, {11, 11}};
    for (auto [sr, sc] : kStarPoints) {
        const float sx = gomoku_intersection_x(sc, bx, cs);
        const float sy = gomoku_intersection_y(sr, by, cs);
        DrawCircleV(scaled_point(Vector2{sx, sy}), scaled(3.0F), Color{60, 55, 45, 255});
    }

    for (int r = 0; r < GomokuGame::kSize; ++r) {
        for (int c = 0; c < GomokuGame::kSize; ++c) {
            const auto cell = game.cell(r, c);
            if (cell == GomokuCell::empty) continue;
            const float cx = gomoku_intersection_x(c, bx, cs);
            const float cy = gomoku_intersection_y(r, by, cs);
            const float radius = cs * 0.44F;
            if (cell == GomokuCell::player) {
                DrawCircleV(scaled_point(Vector2{cx, cy}), scaled(radius), Color{30, 30, 30, 255});
            } else {
                DrawCircleV(scaled_point(Vector2{cx, cy}), scaled(radius), Color{238, 232, 218, 255});
                DrawCircleLinesV(scaled_point(Vector2{cx, cy}), scaled(radius), Color{30, 30, 30, 255});
            }
        }
    }

    if (game.current_turn() == GomokuTurn::player && game.state() == GomokuState::playing) {
        const int hover_col = gomoku_col_from_mouse(dmx, bx, cs);
        const int hover_row = gomoku_row_from_mouse(dmy, by, cs);
        if (game.is_legal(hover_row, hover_col)) {
            const float hx = gomoku_intersection_x(hover_col, bx, cs);
            const float hy = gomoku_intersection_y(hover_row, by, cs);
            DrawCircleV(scaled_point(Vector2{hx, hy}), scaled(cs * 0.38F), Color{0, 0, 0, 100});
        }
    }

    const float ix = L.gomoku_info_x;
    const float iy = L.gomoku_info_y;
    const Rectangle info_panel{ix - 10, iy - 10, 172, 234};
    DrawRectangleRec(scaled_rect(Rectangle{info_panel.x + 3, info_panel.y + 3, info_panel.width,
                                           info_panel.height}), shadow);
    DrawRectangleRec(scaled_rect(info_panel), cream);
    DrawRectangleLinesEx(scaled_rect(info_panel), 2.5F, ink);

    draw_text(font, "五子棋", ix, iy, 22, red);
    const TavernChallengeConfig config;
    const std::string bet_text = std::string{bet_tier_label(tavern_ui.selected_bet)} + "赌注（" +
                                 std::to_string(bet_amount(config, tavern_ui.selected_bet)) + "金币）";
    draw_text(font, bet_text, ix, iy + 34, 14, ink);

    const char* status_text = "";
    if (game.state() == GomokuState::player_wins) {
        status_text = "你赢了！";
    } else if (game.state() == GomokuState::computer_wins) {
        status_text = "你输了。";
    } else if (game.state() == GomokuState::draw) {
        status_text = "平局。";
    } else if (game.current_turn() == GomokuTurn::player) {
        status_text = "你的回合";
    } else {
        status_text = "电脑思考中...";
    }
    draw_text(font, status_text, ix, iy + 70, 16,
              game.state() != GomokuState::playing ? red : ink);
    if (game.current_turn() == GomokuTurn::player && game.state() == GomokuState::playing) {
        draw_text(font, "点击棋盘落子", ix, iy + 94, 14, Color{78, 78, 72, 255});
    }

    if (game.state() != GomokuState::playing) {
        const bool hover_confirm = is_hovered(L.gomoku_confirm_btn, mouse);
        draw_panel(L.gomoku_confirm_btn, hover_confirm ? paper : green);
        draw_centered_text(font, "确认结算", L.gomoku_confirm_btn, 14, RAYWHITE);
        draw_text(font, "Enter / 点击确认", ix, iy + 184, 12, Color{100, 95, 85, 255});
    } else {
        const bool hover_abandon = is_hovered(L.gomoku_play_abandon_btn, mouse);
        draw_panel(L.gomoku_play_abandon_btn, hover_abandon ? paper : red);
        draw_centered_text(font, "主动放弃 (Esc)", L.gomoku_play_abandon_btn, 14, RAYWHITE);
    }
}

// ============================================================
//  screen updates
// ============================================================

void update_tavern_lobby(GameAppState& state, Vector2 logical_mouse) {
    auto& ui = state.tavern_ui;
    const auto L = build_tavern_layout();

    state.tavern_assets.load();
    ui.npc_idle_timer += GetFrameTime();

    if (!state.session.location_started()) {
        if (is_clicked(L.npc_button, logical_mouse)) {
            ui.screen = TavernUiScreen::npc_dialog;
            return;
        }
        if (is_clicked(L.gomoku_table, logical_mouse)) {
            ui.selected_challenge = ChallengeType::gomoku;
            ui.screen = TavernUiScreen::game_select;
            return;
        }
        if (is_clicked(L.dice_table, logical_mouse)) {
            ui.selected_challenge = ChallengeType::liars_dice;
            ui.screen = TavernUiScreen::game_select;
            return;
        }
        if (is_clicked(L.game_select_button, logical_mouse)) {
            ui.screen = TavernUiScreen::game_select;
            return;
        }
        if (is_clicked(L.back_button, logical_mouse) || IsKeyPressed(KEY_ESCAPE)) {
            if (state.session.return_to_map()) {
                state.notice = "已返回地图：阶段未消耗。";
            }
            return;
        }
        return;
    }

    // A restored/interrupted round cannot be awarded without a game result.
    const Rectangle abandon_btn{480, 318, 100, 30};

    if (is_clicked(abandon_btn, logical_mouse) || IsKeyPressed(KEY_ESCAPE)) {
        const auto applied =
            state.session.apply_action_result(state.session.abandon_current_location());
        state.notice = applied.message;
    }
}

void update_tavern_game_select(GameAppState& state, Vector2 logical_mouse) {
    auto& ui = state.tavern_ui;
    const auto L = build_tavern_layout();

    if (IsKeyPressed(KEY_ESCAPE)) {
        ui.screen = TavernUiScreen::lobby;
        return;
    }
    if (is_clicked(L.gomoku_card, logical_mouse)) {
        ui.selected_challenge = ChallengeType::gomoku;
    }
    if (is_clicked(L.dice_card, logical_mouse)) {
        ui.selected_challenge = ChallengeType::liars_dice;
    }
    if (is_clicked(L.bet_low_btn, logical_mouse)) {
        ui.selected_bet = BetTier::low;
    }
    if (is_clicked(L.bet_med_btn, logical_mouse)) {
        ui.selected_bet = BetTier::medium;
    }
    if (is_clicked(L.bet_high_btn, logical_mouse)) {
        ui.selected_bet = BetTier::high;
    }
    if (is_clicked(L.cancel_btn, logical_mouse)) {
        ui.screen = TavernUiScreen::lobby;
        return;
    }
    if (is_clicked(L.start_btn, logical_mouse) || IsKeyPressed(KEY_SPACE)) {
        const TavernChallengeConfig config;
        const int bet = bet_amount(config, ui.selected_bet);
        if (bet > state.session.player().money) {
            state.notice = "金钱不足，无法选择该赌注档位。";
            return;
        }
        if (state.session.start_location() != 0) {
            if (ui.selected_challenge == ChallengeType::gomoku) {
                ui.screen = TavernUiScreen::gomoku;
                ui.gomoku_game = GomokuGame{};
                ui.gomoku_computer_timer = 0.0f;
                state.notice = "五子棋开始：点击棋盘落子。";
            } else {
                ui.screen = TavernUiScreen::liars_dice;
                const auto snapshot = state.session.snapshot();
                ui.liars_dice_game = LiarsDiceGame{make_liars_dice_seed(
                    snapshot.seed, state.session.day(), state.session.active_result_id())};
                ui.ld_ui_count = 1;
                ui.ld_ui_face = 3;
                ui.ld_computer_timer = 0.0f;
                ui.ld_feedback.clear();
                state.notice = "骗子骰子开始。";
            }
        }
    }

    // keyboard shortcuts
    if (IsKeyPressed(KEY_ONE)) ui.selected_challenge = ChallengeType::gomoku;
    if (IsKeyPressed(KEY_TWO)) ui.selected_challenge = ChallengeType::liars_dice;
    if (IsKeyPressed(KEY_THREE)) ui.selected_bet = BetTier::low;
    if (IsKeyPressed(KEY_FOUR)) ui.selected_bet = BetTier::medium;
    if (IsKeyPressed(KEY_FIVE)) ui.selected_bet = BetTier::high;
}

void update_tavern_npc_dialog(GameAppState& state, Vector2 logical_mouse) {
    const auto L = build_tavern_layout();
    if (is_clicked(L.dialog_close_btn, logical_mouse) || IsKeyPressed(KEY_ESCAPE)) {
        state.tavern_ui.screen = TavernUiScreen::lobby;
    }
}

void update_tavern_gomoku(GameAppState& state, Vector2 logical_mouse) {
    const auto L = build_tavern_layout();
    auto& ui = state.tavern_ui;
    auto& game = ui.gomoku_game;

    const float dmx = logical_mouse.x / kScale;
    const float dmy = logical_mouse.y / kScale;

    if (game.state() != GomokuState::playing) {
        if (is_clicked(L.gomoku_confirm_btn, logical_mouse) || IsKeyPressed(KEY_ENTER) ||
            IsKeyPressed(KEY_SPACE)) {
            ChallengeOutcome outcome = ChallengeOutcome::draw;
            if (game.state() == GomokuState::player_wins)
                outcome = ChallengeOutcome::win;
            else if (game.state() == GomokuState::computer_wins)
                outcome = ChallengeOutcome::loss;
            const TavernChallengeConfig config;
            const auto result = simulate_tavern_challenge(
                state.session.player(), config, ui.selected_challenge, ui.selected_bet, outcome,
                state.session.active_result_id());
            if (result.result_id != 0) {
                const auto applied = state.session.apply_action_result(result);
                state.notice = applied.message;
            }
            ui.screen = TavernUiScreen::lobby;
        }
        return;
    }

    if (game.current_turn() == GomokuTurn::computer) {
        ui.gomoku_computer_timer += GetFrameTime();
        if (ui.gomoku_computer_timer >= 0.35F) {
            game.computer_play();
            ui.gomoku_computer_timer = 0.0F;
        }
        if (is_clicked(L.gomoku_play_abandon_btn, logical_mouse) ||
            IsKeyPressed(KEY_ESCAPE)) {
            const auto applied =
                state.session.apply_action_result(state.session.abandon_current_location());
            state.notice = applied.message;
            ui.screen = TavernUiScreen::lobby;
        }
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const int col = gomoku_col_from_mouse(dmx, L.gomoku_board_x, L.gomoku_cell_size);
        const int row = gomoku_row_from_mouse(dmy, L.gomoku_board_y, L.gomoku_cell_size);
        if (game.is_legal(row, col)) {
            game.play(row, col);
        }
    }

    if (is_clicked(L.gomoku_play_abandon_btn, logical_mouse) || IsKeyPressed(KEY_ESCAPE)) {
        const auto applied =
            state.session.apply_action_result(state.session.abandon_current_location());
        state.notice = applied.message;
        ui.screen = TavernUiScreen::lobby;
    }
}

// ---- draw: liars_dice ----

void draw_tavern_liars_dice(const Font& font, const TavernUiState& ui, Vector2 mouse) {
    const auto L = build_tavern_layout();
    const auto& game = ui.liars_dice_game;

    draw_panel(L.ld_panel, cream, ink);
    draw_text(font, "骗子骰子", L.ld_panel.x + 16, L.ld_panel.y + 10, 22, red);

    // computer dice
    draw_text(font, "电脑骰子", 86, 82, 14, ink);
    for (int i = 0; i < LiarsDiceGame::kDiceCount; ++i) {
        const auto r = L.ld_computer_dice[i];
        if (i >= game.computer_dice_count()) {
            draw_panel(r, Color{205, 198, 184, 255}, Color{160, 152, 140, 255});
            draw_centered_text(font, "X", r, 14, Color{145, 137, 126, 255});
            continue;
        }
        draw_panel(r, Color{55, 48, 42, 255}, Color{100, 92, 82, 255});
        if (game.dice_revealed()) {
            draw_centered_text(font,
                               std::to_string(game.computer_dice()[i]).c_str(),
                               r, 16, RAYWHITE);
        } else {
            draw_centered_text(font, "?", r, 16, Color{180, 170, 150, 255});
        }
    }

    // current bid
    if (game.bid_count() > 0) {
        const std::string bid_str = "当前叫点: " + std::to_string(game.bid_count()) + " 个 " +
                                    std::to_string(game.bid_face());
        draw_text(font, bid_str, L.ld_bid_label.x, L.ld_bid_label.y, 16, gold);
    } else {
        draw_text(font, "尚无叫点", L.ld_bid_label.x, L.ld_bid_label.y, 16, Color{120, 112, 100, 255});
    }

    // player dice
    draw_text(font, "你的骰子", 86, 226, 14, ink);
    for (int i = 0; i < LiarsDiceGame::kDiceCount; ++i) {
        const auto r = L.ld_player_dice[i];
        if (i >= game.player_dice_count()) {
            draw_panel(r, Color{205, 198, 184, 255}, Color{160, 152, 140, 255});
            draw_centered_text(font, "X", r, 14, Color{145, 137, 126, 255});
            continue;
        }
        draw_panel(r, Color{240, 234, 218, 255}, ink);
        draw_centered_text(font, std::to_string(game.player_dice()[i]).c_str(), r, 18, ink);
    }

    if (game.is_round_over()) {
        draw_overlay_dim();
        draw_panel(L.ld_result_panel, cream, ink);
        const bool player_challenged = game.who_challenged() == ChallengeInitiator::player;
        const char* who_str = player_challenged ? "你质疑了电脑" : "电脑质疑了你";
        draw_text(font, who_str, L.ld_result_panel.x + 20, L.ld_result_panel.y + 16,
                  14, Color{80, 75, 65, 255});
        const std::string bid_info = "叫点: " + std::to_string(game.bid_count()) + " 个 " +
                                     std::to_string(game.bid_face());
        draw_text(font, bid_info, L.ld_result_panel.x + 20, L.ld_result_panel.y + 42, 15, ink);
        const std::string actual_info =
            "实际: " + std::to_string(game.actual_count()) + " 个 " + std::to_string(game.bid_face());
        draw_text(font, actual_info, L.ld_result_panel.x + 20, L.ld_result_panel.y + 64, 15, ink);
        const char* valid_str = game.bid_was_valid() ? "叫点成立" : "叫点不成立";
        draw_text(font, valid_str, L.ld_result_panel.x + 20, L.ld_result_panel.y + 78, 14,
                  game.bid_was_valid() ? green : red);
        const bool player_lost_round =
            game.round_loser() == LiarsDiceParticipant::player;
        const char* round_text = game.is_game_over()
                                     ? (game.player_won() ? "你赢下整场" : "本场结束")
                                     : (player_lost_round ? "你失去一枚骰子"
                                                          : "电脑失去一枚骰子");
        draw_text(font, round_text, L.ld_result_panel.x + 150, L.ld_result_panel.y + 78,
                  13, game.player_won() || !player_lost_round ? green : red);
        const std::string dice_left =
            "剩余骰子  你 " + std::to_string(game.player_dice_count()) + " · 电脑 " +
            std::to_string(game.computer_dice_count());
        draw_text(font, dice_left, L.ld_result_panel.x + 20, L.ld_result_panel.y + 100, 13, ink);
        const bool hover_confirm = is_hovered(L.ld_confirm_btn, mouse);
        draw_panel(L.ld_confirm_btn, hover_confirm ? paper : green);
        draw_centered_text(font, game.is_game_over() ? "确认结算" : "下一轮",
                           L.ld_confirm_btn, 13, RAYWHITE);
        return;
    }

    {
        const bool hover_abandon = is_hovered(L.ld_abandon_btn, mouse);
        draw_panel(L.ld_abandon_btn,
                   hover_abandon ? paper : Color{211, 202, 174, 255});
        draw_centered_text(font, "放弃", L.ld_abandon_btn, 14, ink);
    }

    if (!game.is_player_turn()) {
        draw_text(font, "电脑思考中...", L.ld_panel.x + 180, L.ld_panel.y + 200, 16,
                  Color{120, 112, 100, 255});
        return;
    }

    if (!ui.ld_feedback.empty()) {
        draw_text(font, ui.ld_feedback, L.ld_panel.x + 270, L.ld_panel.y + 250, 11, red);
    }
    draw_text(font, "一点可代替其他点数；叫一点时只数一点。", L.ld_panel.x + 16,
              L.ld_panel.y + 232, 10, Color{100, 95, 85, 255});

    // bid count selector
    draw_text(font, "数量", L.ld_count_down.x - 38, L.ld_count_down.y + 4, 13, ink);
    {
        const bool hd = is_hovered(L.ld_count_down, mouse);
        draw_panel(L.ld_count_down, hd ? paper : Color{230, 222, 210, 255});
        draw_centered_text(font, "-", L.ld_count_down, 16, ink);
    }
    draw_panel(L.ld_count_val, Color{245, 240, 230, 255});
    draw_centered_text(font, std::to_string(ui.ld_ui_count).c_str(), L.ld_count_val, 16, ink);
    {
        const bool hu = is_hovered(L.ld_count_up, mouse);
        draw_panel(L.ld_count_up, hu ? paper : Color{230, 222, 210, 255});
        draw_centered_text(font, "+", L.ld_count_up, 16, ink);
    }

    // bid face selector
    draw_text(font, "点数", L.ld_face_down.x - 38, L.ld_face_down.y + 4, 13, ink);
    {
        const bool hd = is_hovered(L.ld_face_down, mouse);
        draw_panel(L.ld_face_down, hd ? paper : Color{230, 222, 210, 255});
        draw_centered_text(font, "-", L.ld_face_down, 16, ink);
    }
    draw_panel(L.ld_face_val, Color{245, 240, 230, 255});
    draw_centered_text(font, std::to_string(ui.ld_ui_face).c_str(), L.ld_face_val, 16, ink);
    {
        const bool hu = is_hovered(L.ld_face_up, mouse);
        draw_panel(L.ld_face_up, hu ? paper : Color{230, 222, 210, 255});
        draw_centered_text(font, "+", L.ld_face_up, 16, ink);
    }

    // action buttons
    {
        const bool hb = is_hovered(L.ld_bid_btn, mouse);
        draw_panel(L.ld_bid_btn, hb ? paper : green);
        draw_centered_text(font, "叫点", L.ld_bid_btn, 16, RAYWHITE);
    }
    {
        const bool hc = is_hovered(L.ld_challenge_btn, mouse);
        const bool can_challenge = game.bid_count() > 0;
        draw_panel(L.ld_challenge_btn, can_challenge ? (hc ? paper : red)
                                                       : Color{180, 175, 168, 255});
        draw_centered_text(font, "质疑", L.ld_challenge_btn, 16,
                           can_challenge ? RAYWHITE : Color{140, 135, 128, 255});
    }
}

// ---- update: liars_dice ----

void update_tavern_liars_dice(GameAppState& state, Vector2 logical_mouse) {
    auto& ui = state.tavern_ui;
    auto& game = ui.liars_dice_game;
    const auto L = build_tavern_layout();

    if (game.is_round_over()) {
        if (is_clicked(L.ld_confirm_btn, logical_mouse) || IsKeyPressed(KEY_SPACE) ||
            IsKeyPressed(KEY_ENTER)) {
            if (game.is_game_over()) {
                ChallengeOutcome outcome =
                    game.player_won() ? ChallengeOutcome::win : ChallengeOutcome::loss;
                const TavernChallengeConfig config;
                const auto result = simulate_tavern_challenge(
                    state.session.player(), config, ChallengeType::liars_dice, ui.selected_bet,
                    outcome, state.session.active_result_id());
                if (result.result_id != 0) {
                    const auto applied = state.session.apply_action_result(result);
                    state.notice = applied.message;
                }
                ui.screen = TavernUiScreen::lobby;
            } else if (game.start_next_round()) {
                ui.ld_ui_count = 1;
                ui.ld_ui_face = 1;
                ui.ld_computer_timer = 0.0F;
                ui.ld_feedback.clear();
            }
        }
        return;
    }

    if (!game.is_player_turn()) {
        ui.ld_computer_timer += GetFrameTime();
        if (ui.ld_computer_timer >= 0.5F) {
            game.computer_act();
            ui.ld_computer_timer = 0.0F;
            if (const auto next_bid = game.minimum_legal_bid(); next_bid.has_value()) {
                ui.ld_ui_count = next_bid->count;
                ui.ld_ui_face = next_bid->face;
                ui.ld_feedback.clear();
            }
        }
        if (is_clicked(L.ld_abandon_btn, logical_mouse) || IsKeyPressed(KEY_ESCAPE)) {
            const auto applied =
                state.session.apply_action_result(state.session.abandon_current_location());
            state.notice = applied.message;
            ui.screen = TavernUiScreen::lobby;
        }
        return;
    }

    if (is_clicked(L.ld_count_down, logical_mouse) && ui.ld_ui_count > 1) {
        --ui.ld_ui_count;
        ui.ld_feedback.clear();
    }
    const int total_dice = game.player_dice_count() + game.computer_dice_count();
    if (is_clicked(L.ld_count_up, logical_mouse) && ui.ld_ui_count < total_dice) {
        ++ui.ld_ui_count;
        ui.ld_feedback.clear();
    }
    if (is_clicked(L.ld_face_down, logical_mouse) && ui.ld_ui_face > 1) {
        --ui.ld_ui_face;
        ui.ld_feedback.clear();
    }
    if (is_clicked(L.ld_face_up, logical_mouse) && ui.ld_ui_face < 6) {
        ++ui.ld_ui_face;
        ui.ld_feedback.clear();
    }

    if (is_clicked(L.ld_bid_btn, logical_mouse)) {
        if (game.player_bid(ui.ld_ui_count, ui.ld_ui_face)) {
            ui.ld_feedback.clear();
        } else if (const auto next_bid = game.minimum_legal_bid(); next_bid.has_value()) {
            ui.ld_ui_count = next_bid->count;
            ui.ld_ui_face = next_bid->face;
            ui.ld_feedback = "已调到最小合法叫点，请确认。";
        } else {
            ui.ld_feedback = "已无法加价，请选择质疑。";
        }
    }
    if (is_clicked(L.ld_challenge_btn, logical_mouse) && game.bid_count() > 0) {
        game.player_challenge();
    }
    if (is_clicked(L.ld_abandon_btn, logical_mouse) || IsKeyPressed(KEY_ESCAPE)) {
        const auto applied =
            state.session.apply_action_result(state.session.abandon_current_location());
        state.notice = applied.message;
        ui.screen = TavernUiScreen::lobby;
    }
}

}  // namespace pixel_town

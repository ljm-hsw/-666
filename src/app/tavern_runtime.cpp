#include "app/tavern_runtime.hpp"

#include <algorithm>

#include "app/location_result_adapter.hpp"
#include "app/ui_primitives.hpp"
#include "ui/ui_metrics.hpp"

namespace pixel_town {
namespace {

constexpr const char* lobby_background_path =
    "assets/textures/ui/tavern/tavern_lobby.png";
constexpr const char* bartender_sheet_path =
    "assets/textures/ui/tavern/npc/bartender_idle_sheet.png";

Vector2 design_mouse(Vector2 logical_mouse) {
    return Vector2{logical_mouse.x / ui::design_to_canvas_scale,
                   logical_mouse.y / ui::design_to_canvas_scale};
}

int board_index(float mouse_position, float origin, float cell_size) {
    return static_cast<int>((mouse_position - origin + cell_size * 0.5F) / cell_size);
}

void settle_tavern(GameSession& session, TavernRuntimeState& runtime,
                   ChallengeOutcome outcome, std::string& notice) {
    const auto result = tavern_action_result(session, runtime.selected_challenge,
                                             runtime.selected_bet, outcome,
                                             TavernChallengeConfig{});
    const auto applied = session.apply_action_result(result);
    notice = applied.message;
    runtime.screen = TavernScreen::lobby;
}

void abandon_tavern(GameSession& session, TavernRuntimeState& runtime,
                    std::string& notice) {
    const auto applied = session.apply_action_result(session.abandon_current_location());
    notice = applied.message;
    runtime.screen = TavernScreen::lobby;
}

bool update_lobby(GameSession& session, TavernRuntimeState& runtime,
                  std::string& notice, Vector2 mouse) {
    const TavernLayout layout = tavern_layout();
    runtime.npc_animation_timer += GetFrameTime();

    if (session.location_started()) {
        if (clicked(layout.back_button, mouse) || IsKeyPressed(KEY_ESCAPE)) {
            abandon_tavern(session, runtime, notice);
        }
        return true;
    }
    if (clicked(layout.npc_hotspot, mouse)) {
        runtime.screen = TavernScreen::npc_dialog;
        return true;
    }
    if (clicked(layout.gomoku_hotspot, mouse)) {
        runtime.selected_challenge = ChallengeType::gomoku;
        runtime.screen = TavernScreen::challenge_select;
        return true;
    }
    if (clicked(layout.dice_hotspot, mouse)) {
        runtime.selected_challenge = ChallengeType::liars_dice;
        runtime.screen = TavernScreen::challenge_select;
        return true;
    }
    if (clicked(layout.select_button, mouse) || IsKeyPressed(KEY_SPACE)) {
        runtime.screen = TavernScreen::challenge_select;
        return true;
    }
    if (clicked(layout.back_button, mouse) || IsKeyPressed(KEY_ESCAPE)) {
        if (session.return_to_map()) {
            notice = "已返回地图：阶段未消耗。";
            prepare_tavern_runtime(runtime);
        }
        return true;
    }
    return true;
}

bool update_selection(GameSession& session, TavernRuntimeState& runtime,
                      std::string& notice, Vector2 mouse) {
    const TavernLayout layout = tavern_layout();
    if (clicked(layout.gomoku_card, mouse) || IsKeyPressed(KEY_ONE)) {
        runtime.selected_challenge = ChallengeType::gomoku;
    }
    if (clicked(layout.dice_card, mouse) || IsKeyPressed(KEY_TWO)) {
        runtime.selected_challenge = ChallengeType::liars_dice;
    }
    if (clicked(layout.low_bet_button, mouse) || IsKeyPressed(KEY_THREE)) {
        runtime.selected_bet = BetTier::low;
        runtime.feedback.clear();
    }
    if (clicked(layout.medium_bet_button, mouse) || IsKeyPressed(KEY_FOUR)) {
        runtime.selected_bet = BetTier::medium;
        runtime.feedback.clear();
    }
    if (clicked(layout.high_bet_button, mouse) || IsKeyPressed(KEY_FIVE)) {
        runtime.selected_bet = BetTier::high;
        runtime.feedback.clear();
    }
    if (clicked(layout.cancel_button, mouse) || IsKeyPressed(KEY_ESCAPE)) {
        runtime.screen = TavernScreen::lobby;
        return true;
    }
    if (!clicked(layout.start_button, mouse) && !IsKeyPressed(KEY_SPACE)) {
        return true;
    }

    const TavernChallengeConfig config;
    if (!can_afford_tavern_bet(session.player(), runtime.selected_bet, config)) {
        notice = "金钱不足，无法选择该赌注档位。";
        runtime.feedback = notice;
        return true;
    }
    if (session.start_location() == 0) {
        notice = "酒馆挑战启动失败，请返回地图后重试。";
        runtime.feedback = notice;
        return true;
    }

    runtime.computer_timer = 0.0F;
    runtime.feedback.clear();
    if (runtime.selected_challenge == ChallengeType::gomoku) {
        runtime.gomoku = GomokuGame{};
        runtime.screen = TavernScreen::gomoku;
        notice = "五子棋开始：点击棋盘交叉点落下黑子。";
    } else {
        runtime.liars_dice = LiarsDiceGame{session.location_seed(
            Location::tavern, static_cast<unsigned int>(session.active_result_id()))};
        runtime.bid_count = 1;
        runtime.bid_face = 1;
        runtime.screen = TavernScreen::liars_dice;
        notice = "骗子骰子开始：提高叫点或质疑电脑。";
    }
    return true;
}

bool update_gomoku(GameSession& session, TavernRuntimeState& runtime,
                   std::string& notice, Vector2 mouse) {
    const TavernLayout layout = tavern_layout();
    auto& game = runtime.gomoku;

    if (game.state() != GomokuState::playing) {
        if (clicked(layout.gomoku_confirm_button, mouse) || IsKeyPressed(KEY_ENTER) ||
            IsKeyPressed(KEY_SPACE)) {
            ChallengeOutcome outcome = ChallengeOutcome::draw;
            if (game.state() == GomokuState::player_wins) {
                outcome = ChallengeOutcome::win;
            } else if (game.state() == GomokuState::computer_wins) {
                outcome = ChallengeOutcome::loss;
            }
            settle_tavern(session, runtime, outcome, notice);
        }
        return true;
    }

    if (clicked(layout.gomoku_abandon_button, mouse) || IsKeyPressed(KEY_ESCAPE)) {
        abandon_tavern(session, runtime, notice);
        return true;
    }

    if (game.current_turn() == GomokuTurn::computer) {
        runtime.computer_timer += GetFrameTime();
        if (runtime.computer_timer >= 0.35F) {
            (void)game.computer_play();
            runtime.computer_timer = 0.0F;
        }
        return true;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const Vector2 mouse_design = design_mouse(mouse);
        const int col = board_index(mouse_design.x, layout.gomoku_board_x,
                                    layout.gomoku_cell_size);
        const int row = board_index(mouse_design.y, layout.gomoku_board_y,
                                    layout.gomoku_cell_size);
        (void)game.play(row, col);
    }
    return true;
}

bool update_liars_dice(GameSession& session, TavernRuntimeState& runtime,
                       std::string& notice, Vector2 mouse) {
    const TavernLayout layout = tavern_layout();
    auto& game = runtime.liars_dice;

    if (game.is_round_over()) {
        if (clicked(layout.round_confirm_button, mouse) || IsKeyPressed(KEY_ENTER) ||
            IsKeyPressed(KEY_SPACE)) {
            if (game.is_game_over()) {
                settle_tavern(session, runtime,
                              game.player_won() ? ChallengeOutcome::win
                                                : ChallengeOutcome::loss,
                              notice);
            } else if (game.start_next_round()) {
                runtime.computer_timer = 0.0F;
                runtime.feedback.clear();
                if (const auto bid = game.minimum_legal_bid(); bid.has_value()) {
                    runtime.bid_count = bid->count;
                    runtime.bid_face = bid->face;
                }
            }
        }
        return true;
    }

    if (clicked(layout.dice_abandon_button, mouse) || IsKeyPressed(KEY_ESCAPE)) {
        abandon_tavern(session, runtime, notice);
        return true;
    }

    if (!game.is_player_turn()) {
        runtime.computer_timer += GetFrameTime();
        if (runtime.computer_timer >= 0.5F) {
            (void)game.computer_act();
            runtime.computer_timer = 0.0F;
            if (const auto bid = game.minimum_legal_bid(); bid.has_value()) {
                runtime.bid_count = bid->count;
                runtime.bid_face = bid->face;
            }
        }
        return true;
    }

    const int total_dice = game.player_dice_count() + game.computer_dice_count();
    if (clicked(layout.count_down, mouse)) {
        runtime.bid_count = std::max(1, runtime.bid_count - 1);
        runtime.feedback.clear();
    }
    if (clicked(layout.count_up, mouse)) {
        runtime.bid_count = std::min(total_dice, runtime.bid_count + 1);
        runtime.feedback.clear();
    }
    if (clicked(layout.face_down, mouse)) {
        runtime.bid_face = std::max(1, runtime.bid_face - 1);
        runtime.feedback.clear();
    }
    if (clicked(layout.face_up, mouse)) {
        runtime.bid_face = std::min(6, runtime.bid_face + 1);
        runtime.feedback.clear();
    }
    if (clicked(layout.bid_button, mouse)) {
        if (game.player_bid(runtime.bid_count, runtime.bid_face)) {
            runtime.feedback.clear();
        } else if (const auto bid = game.minimum_legal_bid(); bid.has_value()) {
            runtime.bid_count = bid->count;
            runtime.bid_face = bid->face;
            runtime.feedback = "已调整为最小合法叫点，请再次确认。";
        } else {
            runtime.feedback = "已经无法继续加价，请选择质疑。";
        }
    }
    if (clicked(layout.challenge_button, mouse) && game.bid_count() > 0) {
        (void)game.player_challenge();
    }
    return true;
}

}  // namespace

TavernLayout tavern_layout() {
    TavernLayout layout{};
    layout.npc_hotspot = Rectangle{42, 88, 66, 82};
    layout.gomoku_hotspot = Rectangle{70, 292, 168, 48};
    layout.dice_hotspot = Rectangle{390, 198, 198, 112};
    layout.select_button = Rectangle{470, 310, 92, 32};
    layout.back_button = Rectangle{570, 310, 58, 32};

    layout.overlay_panel = Rectangle{108, 52, 424, 286};
    layout.gomoku_card = Rectangle{132, 86, 180, 90};
    layout.dice_card = Rectangle{328, 86, 180, 90};
    layout.low_bet_button = Rectangle{154, 210, 96, 30};
    layout.medium_bet_button = Rectangle{272, 210, 96, 30};
    layout.high_bet_button = Rectangle{390, 210, 96, 30};
    layout.start_button = Rectangle{154, 270, 154, 38};
    layout.cancel_button = Rectangle{332, 270, 154, 38};

    layout.dialog_panel = Rectangle{100, 224, 440, 122};
    layout.dialog_close_button = Rectangle{456, 306, 64, 28};

    layout.gomoku_board_x = 78.0F;
    layout.gomoku_board_y = 66.0F;
    layout.gomoku_cell_size = 18.0F;
    layout.gomoku_confirm_button = Rectangle{420, 238, 140, 36};
    layout.gomoku_abandon_button = Rectangle{420, 286, 140, 34};

    layout.dice_panel = Rectangle{52, 64, 536, 282};
    for (int index = 0; index < 5; ++index) {
        layout.computer_dice[index] =
            Rectangle{84.0F + static_cast<float>(index) * 48.0F, 126, 38, 38};
        layout.player_dice[index] =
            Rectangle{84.0F + static_cast<float>(index) * 48.0F, 270, 38, 38};
    }
    layout.count_down = Rectangle{372, 154, 28, 28};
    layout.count_value = Rectangle{404, 154, 42, 28};
    layout.count_up = Rectangle{450, 154, 28, 28};
    layout.face_down = Rectangle{372, 190, 28, 28};
    layout.face_value = Rectangle{404, 190, 42, 28};
    layout.face_up = Rectangle{450, 190, 28, 28};
    layout.bid_button = Rectangle{372, 234, 92, 34};
    layout.challenge_button = Rectangle{474, 234, 92, 34};
    layout.dice_abandon_button = Rectangle{474, 286, 92, 32};
    layout.round_result_panel = Rectangle{150, 118, 340, 190};
    layout.round_confirm_button = Rectangle{270, 266, 100, 30};
    return layout;
}

void prepare_tavern_runtime(TavernRuntimeState& runtime) {
    runtime.screen = TavernScreen::lobby;
    runtime.selected_challenge = ChallengeType::gomoku;
    runtime.selected_bet = BetTier::medium;
    runtime.gomoku = GomokuGame{};
    runtime.liars_dice = LiarsDiceGame{0U};
    runtime.computer_timer = 0.0F;
    runtime.bid_count = 1;
    runtime.bid_face = 1;
    runtime.feedback.clear();
    runtime.npc_animation_timer = 0.0F;
}

void ensure_tavern_assets_loaded(TavernRuntimeState& runtime) {
    if (runtime.assets_attempted) {
        return;
    }
    runtime.assets_attempted = true;
    runtime.lobby_background = LoadTexture(lobby_background_path);
    runtime.bartender_sheet = LoadTexture(bartender_sheet_path);
    if (runtime.lobby_background.id != 0) {
        SetTextureFilter(runtime.lobby_background, TEXTURE_FILTER_POINT);
    }
    if (runtime.bartender_sheet.id != 0) {
        SetTextureFilter(runtime.bartender_sheet, TEXTURE_FILTER_POINT);
    }
}

void unload_tavern_assets(TavernRuntimeState& runtime) {
    if (runtime.lobby_background.id != 0) {
        UnloadTexture(runtime.lobby_background);
        runtime.lobby_background = {};
    }
    if (runtime.bartender_sheet.id != 0) {
        UnloadTexture(runtime.bartender_sheet);
        runtime.bartender_sheet = {};
    }
    runtime.assets_attempted = false;
}

bool update_tavern_runtime(GameSession& session, TavernRuntimeState& runtime,
                           std::string& notice, Vector2 logical_mouse) {
    ensure_tavern_assets_loaded(runtime);
    switch (runtime.screen) {
        case TavernScreen::lobby:
            return update_lobby(session, runtime, notice, logical_mouse);
        case TavernScreen::challenge_select:
            return update_selection(session, runtime, notice, logical_mouse);
        case TavernScreen::npc_dialog:
            if (clicked(tavern_layout().dialog_close_button, logical_mouse) ||
                IsKeyPressed(KEY_ESCAPE)) {
                runtime.screen = TavernScreen::lobby;
            }
            return true;
        case TavernScreen::gomoku:
            return update_gomoku(session, runtime, notice, logical_mouse);
        case TavernScreen::liars_dice:
            return update_liars_dice(session, runtime, notice, logical_mouse);
    }
    return true;
}

const char* tavern_ui_glyphs() {
    return "酒馆酒保五子棋骗子骰子玩法选择返回和酒保交谈五子棋桌骗子骰子桌"
           "未完成挑战Esc放弃选择今晚的挑战一晚只能完成一局结算后直接进入每日总结"
           "黑子先手连成五子获胜提高叫点判断何时质疑选择赌注低中高金币开始挑战返回大厅"
           "欢迎来到像素小镇酒馆今晚想坐在哪张桌边左边是五子棋右边是骗子骰子"
           "选好玩法和赌注再开始关闭你执黑子点击交叉点落子先连成五子的一方获胜"
           "你的回合你赢了电脑获胜棋盘已满平局电脑思考中主动放弃确认结算"
           "揭晓这一轮个实际叫点成立不成立你赢下整场比赛你输掉了整场比赛"
           "你失去一枚骰子电脑失去一枚骰子剩余骰子下一轮一点可代替其他点数"
           "叫一点时只计算一点电脑骰子你的骰子当前尚无叫点轮到你行动数量点数"
           "已返回地图阶段未消耗金钱不足无法选择该赌注档位酒馆挑战启动失败请返回地图后重试"
           "五子棋开始骗子骰子开始提高叫点或质疑电脑已调整为最小合法叫点请再次确认"
           "已经无法继续加价请选择质疑×……?!():";
}

}  // namespace pixel_town

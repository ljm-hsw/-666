#include "app/tavern_runtime.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include "app/tavern_layout.hpp"
#include "ui/ui_metrics.hpp"

namespace pixel_town {
namespace {

bool clicked(const TavernRect& bounds, const TavernFrameInput& input) {
    if (!input.primary_pressed || !input.pointer.valid) {
        return false;
    }
    const float x = input.pointer.x / ui::design_to_canvas_scale;
    const float y = input.pointer.y / ui::design_to_canvas_scale;
    return x >= bounds.x && x <= bounds.x + bounds.width &&
           y >= bounds.y && y <= bounds.y + bounds.height;
}

int board_index(float canvas_position, float origin, float cell_size) {
    const float design_position = canvas_position / ui::design_to_canvas_scale;
    return static_cast<int>((design_position - origin + cell_size * 0.5F) /
                            cell_size);
}

TavernStepResult step_result(TavernStepStatus status,
                             std::optional<std::string> notice = std::nullopt) {
    return TavernStepResult{status, std::move(notice)};
}

}  // namespace

TavernRuntime::TavernRuntime()
    : settlement_(config_) {}

void TavernRuntime::reset() {
    active_ = false;
    screen_ = TavernScreen::lobby;
    selected_challenge_ = ChallengeType::gomoku;
    selected_bet_ = BetTier::medium;
    gomoku_ = GomokuGame{};
    liars_dice_ = LiarsDiceGame{0U};
    computer_timer_ = 0.0F;
    bid_count_ = 1;
    bid_face_ = 1;
    feedback_.clear();
    npc_animation_timer_ = 0.0F;
    active_result_id_ = 0;
    player_at_start_ = PlayerState{};
    pending_settlement_.reset();
}

TavernOpenResult TavernRuntime::open(GameSession& session) {
    if (active_) {
        return {TavernOpenStatus::already_active, "酒馆会话已经打开。"};
    }
    const auto permission = session.can_enter(Location::tavern);
    if (!permission.allowed) {
        return {TavernOpenStatus::denied, permission.reason};
    }
    if (!session.enter_location(Location::tavern)) {
        return {TavernOpenStatus::denied, "酒馆入口状态发生变化，请重试。"};
    }
    reset();
    active_ = true;
    return {TavernOpenStatus::opened, "已进入酒馆，选择挑战和赌注。"};
}

TavernStepResult TavernRuntime::step(GameSession& session,
                                     const TavernFrameInput& input) {
    if (!active_) {
        return step_result(TavernStepStatus::rejected, "酒馆会话尚未打开。");
    }
    if (session.phase() != GamePhase::night_location ||
        !session.has_pending_location() ||
        session.pending_location() != Location::tavern) {
        return step_result(TavernStepStatus::rejected,
                           "当前游戏阶段与酒馆会话不一致。");
    }
    if (!input.updates_enabled) {
        return step_result(TavernStepStatus::unchanged);
    }

    float elapsed = input.elapsed_seconds;
    std::optional<std::string> frame_notice;
    if (!std::isfinite(elapsed) || elapsed < 0.0F) {
        elapsed = 0.0F;
        frame_notice = "忽略了非法的酒馆帧时间。";
    }

    const TavernLayout layout = tavern_layout();
    if (screen_ == TavernScreen::lobby) {
        npc_animation_timer_ += elapsed;
        if (session.location_started()) {
            if (clicked(layout.back_button, input) || input.escape_pressed) {
                const auto applied =
                    session.apply_action_result(session.abandon_current_location());
                feedback_ = applied.message;
                if (!applied.accepted) {
                    return step_result(TavernStepStatus::rejected, applied.message);
                }
                active_ = false;
                return step_result(TavernStepStatus::settled, applied.message);
            }
            return step_result(TavernStepStatus::unchanged, frame_notice);
        }
        if (clicked(layout.npc_hotspot, input)) {
            screen_ = TavernScreen::npc_dialog;
            return step_result(TavernStepStatus::changed, frame_notice);
        }
        if (clicked(layout.gomoku_hotspot, input)) {
            selected_challenge_ = ChallengeType::gomoku;
            screen_ = TavernScreen::challenge_select;
            return step_result(TavernStepStatus::changed, frame_notice);
        }
        if (clicked(layout.dice_hotspot, input)) {
            selected_challenge_ = ChallengeType::liars_dice;
            screen_ = TavernScreen::challenge_select;
            return step_result(TavernStepStatus::changed, frame_notice);
        }
        if (clicked(layout.select_button, input) || input.space_pressed) {
            screen_ = TavernScreen::challenge_select;
            return step_result(TavernStepStatus::changed, frame_notice);
        }
        if (clicked(layout.back_button, input) || input.escape_pressed) {
            if (!session.return_to_map()) {
                feedback_ = "当前无法返回地图。";
                return step_result(TavernStepStatus::rejected, feedback_);
            }
            active_ = false;
            return step_result(TavernStepStatus::returned_to_map,
                               "已返回地图：阶段未消耗。");
        }
        return step_result(TavernStepStatus::unchanged, frame_notice);
    }

    if (screen_ == TavernScreen::npc_dialog) {
        if (clicked(layout.dialog_close_button, input) || input.escape_pressed) {
            screen_ = TavernScreen::lobby;
            return step_result(TavernStepStatus::changed, frame_notice);
        }
        return step_result(TavernStepStatus::unchanged, frame_notice);
    }

    if (screen_ == TavernScreen::challenge_select) {
        bool changed = false;
        if (clicked(layout.gomoku_card, input) || input.digit_pressed == 1) {
            selected_challenge_ = ChallengeType::gomoku;
            changed = true;
        } else if (clicked(layout.dice_card, input) || input.digit_pressed == 2) {
            selected_challenge_ = ChallengeType::liars_dice;
            changed = true;
        } else if (clicked(layout.low_bet_button, input) || input.digit_pressed == 3) {
            selected_bet_ = BetTier::low;
            feedback_.clear();
            changed = true;
        } else if (clicked(layout.medium_bet_button, input) || input.digit_pressed == 4) {
            selected_bet_ = BetTier::medium;
            feedback_.clear();
            changed = true;
        } else if (clicked(layout.high_bet_button, input) || input.digit_pressed == 5) {
            selected_bet_ = BetTier::high;
            feedback_.clear();
            changed = true;
        }
        if (clicked(layout.cancel_button, input) || input.escape_pressed) {
            screen_ = TavernScreen::lobby;
            return step_result(TavernStepStatus::changed, frame_notice);
        }
        if (!clicked(layout.start_button, input) && !input.space_pressed) {
            return step_result(changed ? TavernStepStatus::changed
                                       : TavernStepStatus::unchanged,
                               frame_notice);
        }
        if (bet_amount(config_, selected_bet_) > session.player().money) {
            feedback_ = "金钱不足，无法选择该赌注档位。";
            return step_result(TavernStepStatus::rejected, feedback_);
        }
        player_at_start_ = session.player();
        active_result_id_ = session.start_location();
        if (active_result_id_ == 0) {
            feedback_ = "酒馆挑战启动失败，请返回地图后重试。";
            return step_result(TavernStepStatus::rejected, feedback_);
        }
        computer_timer_ = 0.0F;
        pending_settlement_.reset();
        feedback_.clear();
        if (selected_challenge_ == ChallengeType::gomoku) {
            gomoku_ = GomokuGame{};
            screen_ = TavernScreen::gomoku;
            return step_result(TavernStepStatus::changed,
                               "五子棋开始：点击棋盘交叉点落下黑子。");
        }
        liars_dice_ = LiarsDiceGame{session.location_seed(
            Location::tavern, static_cast<unsigned int>(active_result_id_))};
        bid_count_ = 1;
        bid_face_ = 1;
        screen_ = TavernScreen::liars_dice;
        return step_result(TavernStepStatus::changed,
                           "骗子骰子开始：提高叫点或质疑电脑。");
    }

    if (screen_ == TavernScreen::gomoku) {
        if (gomoku_.state() != GomokuState::playing) {
            if (!clicked(layout.gomoku_confirm_button, input) &&
                !input.enter_pressed && !input.space_pressed) {
                return step_result(TavernStepStatus::unchanged, frame_notice);
            }
            if (!pending_settlement_.has_value()) {
                const auto build = settlement_.build(
                    gomoku_, player_at_start_, selected_bet_, active_result_id_);
                if (!build.accepted()) {
                    feedback_ = build.message;
                    return step_result(TavernStepStatus::rejected, build.message);
                }
                pending_settlement_ = build.result;
            }
            const auto applied =
                session.apply_action_result(*pending_settlement_);
            feedback_ = applied.message;
            if (!applied.accepted) {
                return step_result(TavernStepStatus::rejected, applied.message);
            }
            active_ = false;
            return step_result(TavernStepStatus::settled, applied.message);
        }
        if (clicked(layout.gomoku_abandon_button, input) || input.escape_pressed) {
            const auto applied =
                session.apply_action_result(session.abandon_current_location());
            feedback_ = applied.message;
            if (!applied.accepted) {
                return step_result(TavernStepStatus::rejected, applied.message);
            }
            active_ = false;
            return step_result(TavernStepStatus::settled, applied.message);
        }
        if (gomoku_.current_turn() == GomokuTurn::computer) {
            computer_timer_ += elapsed;
            if (computer_timer_ >= 0.35F) {
                (void)gomoku_.computer_play();
                computer_timer_ = 0.0F;
                return step_result(TavernStepStatus::changed, frame_notice);
            }
            return step_result(TavernStepStatus::unchanged, frame_notice);
        }
        if (input.primary_pressed && input.pointer.valid) {
            const int col = board_index(input.pointer.x, layout.gomoku_board_x,
                                        layout.gomoku_cell_size);
            const int row = board_index(input.pointer.y, layout.gomoku_board_y,
                                        layout.gomoku_cell_size);
            if (gomoku_.play(row, col)) {
                return step_result(TavernStepStatus::changed, frame_notice);
            }
        }
        return step_result(TavernStepStatus::unchanged, frame_notice);
    }

    if (liars_dice_.is_round_over()) {
        if (!clicked(layout.round_confirm_button, input) &&
            !input.enter_pressed && !input.space_pressed) {
            return step_result(TavernStepStatus::unchanged, frame_notice);
        }
        if (liars_dice_.is_game_over()) {
            if (!pending_settlement_.has_value()) {
                const auto build = settlement_.build(
                    liars_dice_, player_at_start_, selected_bet_, active_result_id_);
                if (!build.accepted()) {
                    feedback_ = build.message;
                    return step_result(TavernStepStatus::rejected, build.message);
                }
                pending_settlement_ = build.result;
            }
            const auto applied =
                session.apply_action_result(*pending_settlement_);
            feedback_ = applied.message;
            if (!applied.accepted) {
                return step_result(TavernStepStatus::rejected, applied.message);
            }
            active_ = false;
            return step_result(TavernStepStatus::settled, applied.message);
        }
        if (liars_dice_.start_next_round()) {
            computer_timer_ = 0.0F;
            feedback_.clear();
            if (const auto bid = liars_dice_.minimum_legal_bid(); bid.has_value()) {
                bid_count_ = bid->count;
                bid_face_ = bid->face;
            }
            return step_result(TavernStepStatus::changed, frame_notice);
        }
        return step_result(TavernStepStatus::rejected,
                           "骗子骰子下一轮启动失败。");
    }
    if (clicked(layout.dice_abandon_button, input) || input.escape_pressed) {
        const auto applied =
            session.apply_action_result(session.abandon_current_location());
        feedback_ = applied.message;
        if (!applied.accepted) {
            return step_result(TavernStepStatus::rejected, applied.message);
        }
        active_ = false;
        return step_result(TavernStepStatus::settled, applied.message);
    }
    if (!liars_dice_.is_player_turn()) {
        computer_timer_ += elapsed;
        if (computer_timer_ >= 0.5F) {
            (void)liars_dice_.computer_act();
            computer_timer_ = 0.0F;
            if (const auto bid = liars_dice_.minimum_legal_bid(); bid.has_value()) {
                bid_count_ = bid->count;
                bid_face_ = bid->face;
            }
            return step_result(TavernStepStatus::changed, frame_notice);
        }
        return step_result(TavernStepStatus::unchanged, frame_notice);
    }

    const int total_dice =
        liars_dice_.player_dice_count() + liars_dice_.computer_dice_count();
    bool changed = false;
    if (clicked(layout.count_down, input)) {
        bid_count_ = std::max(1, bid_count_ - 1);
        feedback_.clear();
        changed = true;
    } else if (clicked(layout.count_up, input)) {
        bid_count_ = std::min(total_dice, bid_count_ + 1);
        feedback_.clear();
        changed = true;
    } else if (clicked(layout.face_down, input)) {
        bid_face_ = std::max(1, bid_face_ - 1);
        feedback_.clear();
        changed = true;
    } else if (clicked(layout.face_up, input)) {
        bid_face_ = std::min(6, bid_face_ + 1);
        feedback_.clear();
        changed = true;
    } else if (clicked(layout.bid_button, input)) {
        if (liars_dice_.player_bid(bid_count_, bid_face_)) {
            feedback_.clear();
        } else if (const auto bid = liars_dice_.minimum_legal_bid(); bid.has_value()) {
            bid_count_ = bid->count;
            bid_face_ = bid->face;
            feedback_ = "已调整为最小合法叫点，请再次确认。";
        } else {
            feedback_ = "已经无法继续加价，请选择质疑。";
        }
        changed = true;
    } else if (clicked(layout.challenge_button, input) &&
               liars_dice_.bid_count() > 0) {
        changed = liars_dice_.player_challenge();
    }
    return step_result(changed ? TavernStepStatus::changed
                               : TavernStepStatus::unchanged,
                       frame_notice);
}

TavernPresentation TavernRuntime::presentation() const {
    TavernPresentation view;
    view.screen = screen_;
    view.selected_challenge = selected_challenge_;
    view.selected_bet = selected_bet_;
    view.selected_bet_amount = bet_amount(config_, selected_bet_);
    view.bet_amounts = {bet_amount(config_, BetTier::low),
                        bet_amount(config_, BetTier::medium),
                        bet_amount(config_, BetTier::high)};
    view.bartender_animation_seconds = npc_animation_timer_;
    view.feedback = feedback_;
    view.challenge_started = active_result_id_ > 0;

    if (screen_ == TavernScreen::gomoku) {
        TavernGomokuPresentation gomoku_view;
        for (int row = 0; row < GomokuGame::kSize; ++row) {
            for (int col = 0; col < GomokuGame::kSize; ++col) {
                gomoku_view.board[row][col] = gomoku_.cell(row, col);
            }
        }
        gomoku_view.turn = gomoku_.current_turn();
        gomoku_view.state = gomoku_.state();
        view.gomoku = gomoku_view;
    } else if (screen_ == TavernScreen::liars_dice) {
        TavernLiarsDicePresentation dice_view;
        dice_view.player_dice_count = liars_dice_.player_dice_count();
        dice_view.computer_dice_count = liars_dice_.computer_dice_count();
        const int revealed_player_dice_count =
            dice_view.player_dice_count +
            (liars_dice_.is_round_over() &&
                     liars_dice_.round_loser() == LiarsDiceParticipant::player
                 ? 1
                 : 0);
        const int revealed_computer_dice_count =
            dice_view.computer_dice_count +
            (liars_dice_.is_round_over() &&
                     liars_dice_.round_loser() == LiarsDiceParticipant::computer
                 ? 1
                 : 0);
        for (int index = 0; index < LiarsDiceGame::kDiceCount; ++index) {
            dice_view.player_dice[index].active =
                index < revealed_player_dice_count;
            if (dice_view.player_dice[index].active) {
                dice_view.player_dice[index].visible_face =
                    liars_dice_.player_dice()[index];
            }
            dice_view.computer_dice[index].active =
                index < revealed_computer_dice_count;
            if (dice_view.computer_dice[index].active &&
                liars_dice_.dice_revealed()) {
                dice_view.computer_dice[index].visible_face =
                    liars_dice_.computer_dice()[index];
            }
        }
        dice_view.current_bid_count = liars_dice_.bid_count();
        dice_view.current_bid_face = liars_dice_.bid_face();
        dice_view.proposed_bid_count = bid_count_;
        dice_view.proposed_bid_face = bid_face_;
        dice_view.player_turn = liars_dice_.is_player_turn();
        dice_view.round_over = liars_dice_.is_round_over();
        dice_view.game_over = liars_dice_.is_game_over();
        dice_view.player_won = liars_dice_.player_won();
        dice_view.actual_count = liars_dice_.actual_count();
        dice_view.bid_was_valid = liars_dice_.bid_was_valid();
        dice_view.round_loser = liars_dice_.round_loser();
        view.liars_dice = dice_view;
    }
    return view;
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

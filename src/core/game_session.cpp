#include "core/game_session.hpp"

#include <algorithm>
#include <array>

namespace pixel_town {
namespace {

int clamp_value(int value, int lower, int upper) {
    return std::max(lower, std::min(value, upper));
}

bool is_day_work(Location location) {
    return location == Location::restaurant || location == Location::convenience_store ||
           location == Location::library;
}

StatDelta day_work_delta(Location location) {
    switch (location) {
        case Location::restaurant:
            return StatDelta{18, -18, 4, 0, -3};
        case Location::convenience_store:
            return StatDelta{14, -12, 2, 0, -1};
        case Location::library:
            return StatDelta{6, -8, 5, 6, 2};
        case Location::home:
        case Location::tavern:
            break;
    }
    return {};
}

DayContext make_day_context(int day, unsigned int seed) {
    constexpr std::array<const char*, 4> weather{"晴天", "多云", "小雨", "微风"};
    constexpr std::array<const char*, 4> event{"餐馆客流增加", "便利店零食更受欢迎",
                                               "图书馆读者变多", "小镇节奏平稳"};
    const std::size_t weather_index = static_cast<std::size_t>((seed + day * 3U) % weather.size());
    const std::size_t event_index = static_cast<std::size_t>((seed / 7U + day * 5U) % event.size());
    return DayContext{day, seed, weather[weather_index], event[event_index]};
}

std::string completed_summary(Location location) {
    switch (location) {
        case Location::restaurant:
            return "餐馆模拟工作完成：服务了午餐客流，获得金钱与声望。";
        case Location::convenience_store:
            return "便利店模拟经营完成：完成一次进货与销售结算。";
        case Location::library:
            return "图书馆模拟工作完成：帮助读者找书并提升知识。";
        case Location::home:
            return "回家休息：恢复体力并结束今天。";
        case Location::tavern:
            break;
    }
    return "行动完成。";
}

}  // namespace

const char* phase_label(GamePhase phase) {
    switch (phase) {
        case GamePhase::day_choice:
            return "白天选择";
        case GamePhase::day_location:
            return "白天地点";
        case GamePhase::night_choice:
            return "夜晚选择";
        case GamePhase::night_location:
            return "夜晚地点";
        case GamePhase::day_summary:
            return "每日总结";
        case GamePhase::ending:
            return "最终结局";
    }
    return "未知阶段";
}

const char* location_label(Location location) {
    switch (location) {
        case Location::home:
            return "家";
        case Location::restaurant:
            return "餐馆";
        case Location::convenience_store:
            return "便利店";
        case Location::library:
            return "图书馆";
        case Location::tavern:
            return "酒馆";
    }
    return "未知地点";
}

GameSession GameSession::new_game(unsigned int seed) {
    GameSession session;
    session.seed_ = seed;
    return session;
}

DayContext GameSession::current_day_context() const {
    return make_day_context(day_, seed_);
}

Location GameSession::pending_location() const noexcept {
    return pending_location_;
}

ActionPermission GameSession::can_enter(Location location) const {
    if (phase_ == GamePhase::day_choice) {
        if (day_action_done_) {
            return {false, "今天的白天行动已经完成。"};
        }
        if (is_day_work(location)) {
            return {true, ""};
        }
        if (location == Location::home) {
            return {false, "现在是白天，回家休息只能在夜晚选择。"};
        }
        return {false, "酒馆夜晚开放，当前阶段不能进入。"};
    }

    if (phase_ == GamePhase::night_choice) {
        if (night_action_done_) {
            return {false, "今晚的行动已经完成。"};
        }
        if (location == Location::home) {
            return {true, ""};
        }
        if (location == Location::tavern) {
            return {false, "酒馆玩法将在后续 issue 接入；本切片先开放回家休息。"};
        }
        return {false, "白天工作已经结束，夜晚不能再进入该地点。"};
    }

    if (phase_ == GamePhase::ending) {
        return {false, "十日经营计划已经结束，不能继续选择地点。"};
    }

    return {false, "当前正在处理另一个阶段，不能选择新地点。"};
}

bool GameSession::enter_location(Location location) {
    if (!can_enter(location).allowed) {
        return false;
    }
    pending_location_ = location;
    location_started_ = false;
    active_result_id_ = 0;
    phase_ = phase_ == GamePhase::day_choice ? GamePhase::day_location : GamePhase::night_location;
    return true;
}

bool GameSession::return_to_map() {
    if (!has_pending_location() || location_started_) {
        return false;
    }
    phase_ = phase_ == GamePhase::day_location ? GamePhase::day_choice : GamePhase::night_choice;
    clear_pending_location();
    return true;
}

int GameSession::start_location() {
    if (!has_pending_location() || location_started_) {
        return 0;
    }
    location_started_ = true;
    active_result_id_ = next_result_id_++;
    return active_result_id_;
}

ActionResult GameSession::simulated_success_result() const {
    if (!has_pending_location() || !location_started_) {
        return {};
    }
    const ActionSlot slot =
        phase_ == GamePhase::day_location ? ActionSlot::day : ActionSlot::night;
    return ActionResult{active_result_id_, slot, pending_location_, ActionOutcome::completed,
                        day_work_delta(pending_location_), completed_summary(pending_location_)};
}

ActionResult GameSession::abandon_current_location() const {
    if (!has_pending_location() || !location_started_) {
        return {};
    }
    const ActionSlot slot =
        phase_ == GamePhase::day_location ? ActionSlot::day : ActionSlot::night;
    return ActionResult{active_result_id_, slot, pending_location_, ActionOutcome::abandoned,
                        {}, std::string{location_label(pending_location_)} + "已主动放弃：阶段已消耗，本次无收益。"};
}

ActionResult GameSession::home_rest_result() {
    if (phase_ != GamePhase::night_choice || !can_enter(Location::home).allowed) {
        return {};
    }
    pending_location_ = Location::home;
    location_started_ = true;
    active_result_id_ = next_result_id_++;
    phase_ = GamePhase::night_location;
    return ActionResult{active_result_id_, ActionSlot::night, Location::home,
                        ActionOutcome::completed, StatDelta{0, 15, 0, 0, 5},
                        completed_summary(Location::home)};
}

ApplyResult GameSession::apply_action_result(const ActionResult& result) {
    if (result.result_id == 0 || result.result_id != active_result_id_) {
        return {false, "行动结果不属于当前地点会话。"};
    }
    if (result_was_applied(result.result_id)) {
        return {false, "行动结果已经应用，不能重复提交。"};
    }
    if (!has_pending_location() || !location_started_) {
        return {false, "当前没有已开始的地点会话。"};
    }
    if (result.location != pending_location_) {
        return {false, "行动结果地点与当前地点不一致。"};
    }

    if (phase_ == GamePhase::day_location) {
        if (result.slot != ActionSlot::day || day_action_done_) {
            return {false, "当前不能应用白天行动结果。"};
        }
        apply_delta(result.delta);
        day_action_done_ = true;
        applied_result_ids_.push_back(result.result_id);
        last_summary_ = result.summary;
        clear_pending_location();
        phase_ = GamePhase::night_choice;
        return {true, "白天行动已结算。"};
    }

    if (phase_ == GamePhase::night_location) {
        if (result.slot != ActionSlot::night || night_action_done_) {
            return {false, "当前不能应用夜晚行动结果。"};
        }
        apply_delta(result.delta);
        night_action_done_ = true;
        applied_result_ids_.push_back(result.result_id);
        last_summary_ = result.summary;
        clear_pending_location();
        phase_ = GamePhase::day_summary;
        return {true, "夜晚行动已结算。"};
    }

    return {false, "当前阶段不能应用行动结果。"};
}

bool GameSession::finish_day_summary() {
    if (phase_ != GamePhase::day_summary) {
        return false;
    }
    if (day_ < 10) {
        ++day_;
        day_action_done_ = false;
        night_action_done_ = false;
        clear_pending_location();
        last_summary_.clear();
        phase_ = GamePhase::day_choice;
    } else {
        create_placeholder_ending();
        clear_pending_location();
        phase_ = GamePhase::ending;
        return true;
    }
    return true;
}

GameSessionSnapshot GameSession::snapshot() const {
    return GameSessionSnapshot{day_,
                               seed_,
                               next_result_id_,
                               active_result_id_,
                               phase_,
                               player_,
                               has_pending_location(),
                               has_pending_location() ? pending_location_ : Location::home,
                               location_started_,
                               day_action_done_,
                               night_action_done_,
                               last_summary_,
                               main_ending_,
                               final_summary_,
                               applied_result_ids_};
}

GameSession GameSession::from_snapshot(const GameSessionSnapshot& snapshot) {
    GameSession session;
    session.day_ = snapshot.day;
    session.seed_ = snapshot.seed;
    session.next_result_id_ = snapshot.next_result_id;
    session.active_result_id_ = snapshot.active_result_id;
    session.phase_ = snapshot.phase;
    session.player_ = snapshot.player;
    session.pending_location_ =
        snapshot.has_pending_location ? snapshot.pending_location : GameSession::none_;
    session.location_started_ = snapshot.location_started;
    session.day_action_done_ = snapshot.day_action_done;
    session.night_action_done_ = snapshot.night_action_done;
    session.last_summary_ = snapshot.last_summary;
    session.main_ending_ = snapshot.main_ending;
    session.final_summary_ = snapshot.final_summary;
    session.applied_result_ids_ = snapshot.applied_result_ids;
    return session;
}

bool GameSession::result_was_applied(int result_id) const {
    return std::find(applied_result_ids_.begin(), applied_result_ids_.end(), result_id) !=
           applied_result_ids_.end();
}

void GameSession::clear_pending_location() {
    pending_location_ = none_;
    location_started_ = false;
    active_result_id_ = 0;
}

void GameSession::apply_delta(const StatDelta& delta) {
    player_.money = clamp_value(player_.money + delta.money, 0, 999);
    player_.stamina = clamp_value(player_.stamina + delta.stamina, 0, 100);
    player_.reputation = clamp_value(player_.reputation + delta.reputation, 0, 100);
    player_.knowledge = clamp_value(player_.knowledge + delta.knowledge, 0, 100);
    player_.mood = clamp_value(player_.mood + delta.mood, 0, 100);
}

void GameSession::create_placeholder_ending() {
    main_ending_ = "平凡小镇新人";
    final_summary_ = "最终状态：金钱 " + std::to_string(player_.money) + "，体力 " +
                     std::to_string(player_.stamina) + "，声望 " +
                     std::to_string(player_.reputation) + "，知识 " +
                     std::to_string(player_.knowledge) + "，心情 " +
                     std::to_string(player_.mood) + "。成长路线摘要：均衡体验小镇生活。";
}

}  // namespace pixel_town

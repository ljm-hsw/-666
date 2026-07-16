// GameSession 的状态转移、结果幂等校验、每日结算与最终结局收口。
#include "core/game_session.hpp"

#include <algorithm>
#include <array>

#include "core/ending_rules.hpp"
#include "core/story_text.hpp"

namespace pixel_town {
namespace {

int clamp_value(int value, int lower, int upper) {
    return std::max(lower, std::min(value, upper));
}

unsigned int mix_seed(unsigned int value) {
    value ^= value >> 16U;
    value *= 0x7feb352dU;
    value ^= value >> 15U;
    value *= 0x846ca68bU;
    value ^= value >> 16U;
    return value;
}

unsigned int derive_seed(unsigned int root_seed, int day, unsigned int stream,
                         unsigned int session_index) {
    unsigned int value = root_seed ^ 0x9e3779b9U;
    value ^= static_cast<unsigned int>(day) * 0x85ebca6bU;
    value ^= stream * 0xc2b2ae35U;
    value ^= session_index * 0x27d4eb2fU;
    return mix_seed(value);
}

bool is_day_work(Location location) {
    return location == Location::restaurant || location == Location::convenience_store ||
           location == Location::library;
}

DayContext make_day_context(int day, unsigned int seed) {
    constexpr std::array<const char*, 4> weather{"晴天", "多云", "小雨", "微风"};
    constexpr std::array<const char*, 4> event{"餐馆客流增加", "便利店零食更受欢迎",
                                               "图书馆读者变多", "小镇节奏平稳"};
    const unsigned int day_seed = derive_seed(seed, day, 0U, 0U);
    const std::size_t weather_index = static_cast<std::size_t>(day_seed % weather.size());
    const std::size_t event_index =
        static_cast<std::size_t>((day_seed / 7U) % event.size());
    return DayContext{day, day_seed, weather[weather_index], event[event_index]};
}

bool has_nonzero_delta(const StatDelta& delta) {
    return delta.money != 0 || delta.stamina != 0 || delta.reputation != 0 ||
           delta.knowledge != 0 || delta.mood != 0;
}

bool has_valid_store_inventory(const std::vector<StoreInventoryItem>& inventory) {
    for (std::size_t index = 0; index < inventory.size(); ++index) {
        if (inventory[index].item_id.empty() || inventory[index].quantity < 0) {
            return false;
        }
        for (std::size_t previous = 0; previous < index; ++previous) {
            if (inventory[previous].item_id == inventory[index].item_id) {
                return false;
            }
        }
    }
    return true;
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

bool operator==(const GameSessionSnapshot& left, const GameSessionSnapshot& right) {
    return left.day == right.day && left.seed == right.seed &&
           left.next_result_id == right.next_result_id &&
           left.active_result_id == right.active_result_id && left.phase == right.phase &&
           left.player.money == right.player.money &&
           left.player.stamina == right.player.stamina &&
           left.player.reputation == right.player.reputation &&
           left.player.knowledge == right.player.knowledge &&
           left.player.mood == right.player.mood &&
           left.has_pending_location == right.has_pending_location &&
           left.pending_location == right.pending_location &&
           left.location_started == right.location_started &&
           left.day_action_done == right.day_action_done &&
           left.night_action_done == right.night_action_done &&
           left.last_summary == right.last_summary && left.main_ending == right.main_ending &&
           left.final_summary == right.final_summary &&
           left.applied_result_ids == right.applied_result_ids &&
           left.store_inventory == right.store_inventory &&
           left.tavern_wins == right.tavern_wins &&
           left.tavern_losses == right.tavern_losses &&
           left.location_visits == right.location_visits;
}

bool operator==(const LocationVisitCounts& left, const LocationVisitCounts& right) {
    return left.home == right.home && left.restaurant == right.restaurant &&
           left.convenience_store == right.convenience_store &&
           left.library == right.library && left.tavern == right.tavern;
}

GameSession GameSession::new_game(unsigned int seed) {
    GameSession session;
    session.seed_ = seed;
    return session;
}

DayContext GameSession::current_day_context() const {
    return make_day_context(day_, seed_);
}

unsigned int GameSession::location_seed(Location location,
                                        unsigned int session_index) const noexcept {
    const unsigned int stream = static_cast<unsigned int>(location) + 1U;
    return derive_seed(seed_, day_, stream, session_index);
}

int GameSession::location_visit_count(Location location) const noexcept {
    switch (location) {
        case Location::home:
            return location_visits_.home;
        case Location::restaurant:
            return location_visits_.restaurant;
        case Location::convenience_store:
            return location_visits_.convenience_store;
        case Location::library:
            return location_visits_.library;
        case Location::tavern:
            return location_visits_.tavern;
    }
    return 0;
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
            return {true, ""};
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

ActionResult GameSession::abandon_current_location() const {
    if (!has_pending_location() || !location_started_) {
        return {};
    }
    const ActionSlot slot =
        phase_ == GamePhase::day_location ? ActionSlot::day : ActionSlot::night;
    ActionResult result;
    result.result_id = active_result_id_;
    result.slot = slot;
    result.location = pending_location_;
    result.outcome = ActionOutcome::abandoned;
    result.summary = location_result_summary(pending_location_, ActionOutcome::abandoned);
    return result;
}

ActionResult GameSession::home_rest_result() {
    if (phase_ != GamePhase::night_choice || !can_enter(Location::home).allowed) {
        return {};
    }
    pending_location_ = Location::home;
    location_started_ = true;
    active_result_id_ = next_result_id_++;
    phase_ = GamePhase::night_location;
    ActionResult result;
    result.result_id = active_result_id_;
    result.slot = ActionSlot::night;
    result.location = Location::home;
    result.outcome = ActionOutcome::completed;
    result.delta = StatDelta{0, 15, 0, 0, 5};
    result.summary = location_result_summary(Location::home, ActionOutcome::completed);
    return result;
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
    if (result.outcome == ActionOutcome::abandoned &&
        (has_nonzero_delta(result.delta) || result.has_store_inventory_update ||
         result.tavern_win_delta != 0 || result.tavern_loss_delta != 0)) {
        return {false, "主动放弃结果不能携带收益或地点状态变化。"};
    }
    if (result.has_store_inventory_update) {
        if (result.location != Location::convenience_store ||
            !has_valid_store_inventory(result.store_inventory_after)) {
            return {false, "店铺库存更新不属于当前便利店行动或库存数据非法。"};
        }
    } else if (!result.store_inventory_after.empty()) {
        return {false, "行动结果携带了未声明的店铺库存。"};
    }
    if (result.tavern_win_delta < 0 || result.tavern_loss_delta < 0 ||
        result.tavern_win_delta + result.tavern_loss_delta > 1) {
        return {false, "酒馆战绩变化非法。"};
    }
    if ((result.tavern_win_delta != 0 || result.tavern_loss_delta != 0) &&
        result.location != Location::tavern) {
        return {false, "酒馆战绩只能由酒馆行动修改。"};
    }

    if (phase_ == GamePhase::day_location) {
        if (result.slot != ActionSlot::day || day_action_done_) {
            return {false, "当前不能应用白天行动结果。"};
        }
        apply_delta(result.delta);
        if (result.has_store_inventory_update) {
            store_inventory_ = result.store_inventory_after;
        }
        day_action_done_ = true;
        applied_result_ids_.push_back(result.result_id);
        last_summary_ = result.summary;
        tavern_wins_ += result.tavern_win_delta;
        tavern_losses_ += result.tavern_loss_delta;
        if (result.outcome == ActionOutcome::completed) {
            record_completed_visit(result.location);
        }
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
        tavern_wins_ += result.tavern_win_delta;
        tavern_losses_ += result.tavern_loss_delta;
        if (result.outcome == ActionOutcome::completed) {
            record_completed_visit(result.location);
        }
        clear_pending_location();
        phase_ = GamePhase::day_summary;
        return {true, "夜晚行动已结算。"};
    }

    return {false, "当前阶段不能应用行动结果。"};
}

bool GameSession::finish_day_summary() {
    return finish_day_summary(default_ending_config());
}

bool GameSession::finish_day_summary(const EndingConfig& config) {
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
        if (!create_final_ending(config)) {
            return false;
        }
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
                               applied_result_ids_,
                               store_inventory_,
                               tavern_wins_,
                               tavern_losses_,
                               location_visits_};
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
    session.store_inventory_ = snapshot.store_inventory;
    session.tavern_wins_ = snapshot.tavern_wins;
    session.tavern_losses_ = snapshot.tavern_losses;
    session.location_visits_ = snapshot.location_visits;
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

void GameSession::record_completed_visit(Location location) {
    switch (location) {
        case Location::home:
            ++location_visits_.home;
            return;
        case Location::restaurant:
            ++location_visits_.restaurant;
            return;
        case Location::convenience_store:
            ++location_visits_.convenience_store;
            return;
        case Location::library:
            ++location_visits_.library;
            return;
        case Location::tavern:
            ++location_visits_.tavern;
            return;
    }
}

bool GameSession::create_final_ending(const EndingConfig& config) {
    const auto settlement = settle_ending(
        EndingInput{player_, store_inventory_, tavern_wins_, tavern_losses_},
        config);
    if (!settlement.accepted) {
        return false;
    }
    player_.money = clamp_value(player_.money + settlement.inventory_cash, 0, 999);
    store_inventory_.clear();
    main_ending_ = main_ending_label(settlement.ending);
    final_summary_ = std::string{council_opening()} + "\n" +
                     ending_narrative(settlement.ending) + "\n库存清算 " +
                     std::to_string(settlement.inventory_cash) +
                     " 金币 · 成长路线：" + settlement.growth_route +
                     "。\n判定依据：" + settlement.reason;
    return true;
}

}  // namespace pixel_town

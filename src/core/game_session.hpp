// 核心会话状态机。
//
// GameSession 是应用层和地点规则之间的状态边界：地点模块只能生成
// ActionResult，不能直接修改玩家属性、日期、库存或酒馆战绩。应用层通过
// enter/start/apply/finish 这组窄接口推进流程，存档层只读取这里生成的快照。
// 该类型不依赖 raylib，因此可以在无窗口测试中验证完整的十日阶段闭环。
#pragma once

#include <string>
#include <vector>

namespace pixel_town {

struct EndingConfig;

enum class GamePhase {
    day_choice,
    day_location,
    night_choice,
    night_location,
    day_summary,
    ending,
};

enum class Location {
    home,
    restaurant,
    convenience_store,
    library,
    tavern,
};

struct LocationVisitCounts {
    int home{0};
    int restaurant{0};
    int convenience_store{0};
    int library{0};
    int tavern{0};
};

[[nodiscard]] bool operator==(const LocationVisitCounts& left,
                              const LocationVisitCounts& right);
[[nodiscard]] inline bool operator!=(const LocationVisitCounts& left,
                                     const LocationVisitCounts& right) {
    return !(left == right);
}

enum class ActionSlot {
    day,
    night,
};

enum class ActionOutcome {
    completed,
    abandoned,
};

struct PlayerState {
    int money{50};
    int stamina{80};
    int reputation{0};
    int knowledge{0};
    int mood{70};
};

struct DayContext {
    int day{1};
    unsigned int seed{0};
    std::string weather;
    std::string event;
};

struct StatDelta {
    int money{0};
    int stamina{0};
    int reputation{0};
    int knowledge{0};
    int mood{0};
};

struct StoreInventoryItem {
    std::string item_id;
    int quantity{0};
};

[[nodiscard]] inline bool operator==(const StoreInventoryItem& left,
                                     const StoreInventoryItem& right) {
    return left.item_id == right.item_id && left.quantity == right.quantity;
}

struct ActionResult {
    // 地点一次完整行动的不可变提交单元。result_id 必须等于当前地点会话的
    // active_result_id；GameSession 用它阻止重复应用和跨地点提交。
    int result_id{0};
    ActionSlot slot{ActionSlot::day};
    Location location{Location::restaurant};
    ActionOutcome outcome{ActionOutcome::completed};
    StatDelta delta{};
    int tavern_win_delta{0};
    int tavern_loss_delta{0};
    std::string summary;
    bool has_store_inventory_update{false};
    std::vector<StoreInventoryItem> store_inventory_after;
};

struct ActionPermission {
    bool allowed{false};
    std::string reason;
};

struct ApplyResult {
    bool accepted{false};
    std::string message;
};

struct GameSessionSnapshot {
    // 持久化边界。这里保存“阶段边界事实”，不保存 raylib 资源、按钮状态、
    // NPC 动画计时或地点 Runtime 的临时对象；恢复后由应用层重新打开展示 Runtime。
    int day{1};
    unsigned int seed{20260707};
    int next_result_id{1};
    int active_result_id{0};
    GamePhase phase{GamePhase::day_choice};
    PlayerState player{};
    bool has_pending_location{false};
    Location pending_location{Location::home};
    bool location_started{false};
    bool day_action_done{false};
    bool night_action_done{false};
    std::string last_summary;
    std::string main_ending;
    std::string final_summary;
    std::vector<int> applied_result_ids;
    std::vector<StoreInventoryItem> store_inventory;
    int tavern_wins{0};
    int tavern_losses{0};
    LocationVisitCounts location_visits{};
};

[[nodiscard]] bool operator==(const GameSessionSnapshot& left,
                              const GameSessionSnapshot& right);
[[nodiscard]] inline bool operator!=(const GameSessionSnapshot& left,
                                     const GameSessionSnapshot& right) {
    return !(left == right);
}

[[nodiscard]] const char* phase_label(GamePhase phase);
[[nodiscard]] const char* location_label(Location location);

class GameSession {
public:
    [[nodiscard]] static GameSession new_game(unsigned int seed = 20260707);

    [[nodiscard]] int day() const noexcept { return day_; }
    [[nodiscard]] GamePhase phase() const noexcept { return phase_; }
    [[nodiscard]] const PlayerState& player() const noexcept { return player_; }
    [[nodiscard]] DayContext current_day_context() const;
    [[nodiscard]] unsigned int location_seed(Location location,
                                             unsigned int session_index = 0) const noexcept;
    [[nodiscard]] const std::string& last_summary() const noexcept { return last_summary_; }
    [[nodiscard]] const std::string& main_ending() const noexcept { return main_ending_; }
    [[nodiscard]] const std::string& final_summary() const noexcept { return final_summary_; }
    [[nodiscard]] bool is_ended() const noexcept { return phase_ == GamePhase::ending; }
    [[nodiscard]] bool has_pending_location() const noexcept { return pending_location_ != none_; }
    [[nodiscard]] bool location_started() const noexcept { return location_started_; }
    [[nodiscard]] Location pending_location() const noexcept;
    [[nodiscard]] int active_result_id() const noexcept { return active_result_id_; }
    [[nodiscard]] int tavern_wins() const noexcept { return tavern_wins_; }
    [[nodiscard]] int tavern_losses() const noexcept { return tavern_losses_; }
    [[nodiscard]] int location_visit_count(Location location) const noexcept;
    [[nodiscard]] const std::vector<StoreInventoryItem>& store_inventory() const noexcept {
        return store_inventory_;
    }

    // 入口检查只回答“当前阶段能否进入”，不产生副作用。
    [[nodiscard]] ActionPermission can_enter(Location location) const;
    // enter_location 只建立待处理地点；返回地图不会消耗行动。
    [[nodiscard]] bool enter_location(Location location);
    [[nodiscard]] bool return_to_map();
    // start_location 分配唯一结果 ID，标志地点从准备页进入实际行动。
    [[nodiscard]] int start_location();
    [[nodiscard]] ActionResult abandon_current_location() const;
    [[nodiscard]] ActionResult home_rest_result();
    // 唯一的全局写入口：校验结果身份、阶段、地点专属字段后，应用属性变化并
    // 清理 pending location。地点 Runtime 不应绕过此函数直接写玩家状态。
    [[nodiscard]] ApplyResult apply_action_result(const ActionResult& result);
    // 每日总结完成后进入下一日；第十日则调用结局模块并转入 ending。
    [[nodiscard]] bool finish_day_summary();
    [[nodiscard]] bool finish_day_summary(const EndingConfig& config);
    [[nodiscard]] GameSessionSnapshot snapshot() const;
    [[nodiscard]] static GameSession from_snapshot(const GameSessionSnapshot& snapshot);

private:
    static constexpr Location none_{static_cast<Location>(-1)};

    int day_{1};
    unsigned int seed_{20260707};
    int next_result_id_{1};
    int active_result_id_{0};
    GamePhase phase_{GamePhase::day_choice};
    PlayerState player_{};
    Location pending_location_{none_};
    bool location_started_{false};
    bool day_action_done_{false};
    bool night_action_done_{false};
    std::string last_summary_;
    std::string main_ending_;
    std::string final_summary_;
    std::vector<int> applied_result_ids_;
    std::vector<StoreInventoryItem> store_inventory_;
    int tavern_wins_{0};
    int tavern_losses_{0};
    LocationVisitCounts location_visits_{};

    [[nodiscard]] bool result_was_applied(int result_id) const;
    void clear_pending_location();
    void apply_delta(const StatDelta& delta);
    void record_completed_visit(Location location);
    [[nodiscard]] bool create_final_ending(const EndingConfig& config);
};

}  // namespace pixel_town

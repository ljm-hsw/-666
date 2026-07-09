#pragma once

#include <string>
#include <vector>

namespace pixel_town {

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
};

[[nodiscard]] const char* phase_label(GamePhase phase);
[[nodiscard]] const char* location_label(Location location);

class GameSession {
public:
    [[nodiscard]] static GameSession new_game(unsigned int seed = 20260707);

    [[nodiscard]] int day() const noexcept { return day_; }
    [[nodiscard]] GamePhase phase() const noexcept { return phase_; }
    [[nodiscard]] const PlayerState& player() const noexcept { return player_; }
    [[nodiscard]] DayContext current_day_context() const;
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
    [[nodiscard]] const std::vector<StoreInventoryItem>& store_inventory() const noexcept {
        return store_inventory_;
    }

    [[nodiscard]] ActionPermission can_enter(Location location) const;
    [[nodiscard]] bool enter_location(Location location);
    [[nodiscard]] bool return_to_map();
    [[nodiscard]] int start_location();
    [[nodiscard]] ActionResult simulated_success_result() const;
    [[nodiscard]] ActionResult abandon_current_location() const;
    [[nodiscard]] ActionResult home_rest_result();
    [[nodiscard]] ApplyResult apply_action_result(const ActionResult& result);
    [[nodiscard]] bool finish_day_summary();
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

    [[nodiscard]] bool result_was_applied(int result_id) const;
    void clear_pending_location();
    void apply_delta(const StatDelta& delta);
    void create_placeholder_ending();
};

}  // namespace pixel_town

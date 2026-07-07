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

struct StatDelta {
    int money{0};
    int stamina{0};
    int reputation{0};
    int knowledge{0};
    int mood{0};
};

struct ActionResult {
    int result_id{0};
    ActionSlot slot{ActionSlot::day};
    Location location{Location::restaurant};
    ActionOutcome outcome{ActionOutcome::completed};
    StatDelta delta{};
    std::string summary;
};

struct ActionPermission {
    bool allowed{false};
    std::string reason;
};

struct ApplyResult {
    bool accepted{false};
    std::string message;
};

[[nodiscard]] const char* phase_label(GamePhase phase);
[[nodiscard]] const char* location_label(Location location);

class GameSession {
public:
    [[nodiscard]] static GameSession new_game();

    [[nodiscard]] int day() const noexcept { return day_; }
    [[nodiscard]] GamePhase phase() const noexcept { return phase_; }
    [[nodiscard]] const PlayerState& player() const noexcept { return player_; }
    [[nodiscard]] const std::string& last_summary() const noexcept { return last_summary_; }
    [[nodiscard]] bool has_pending_location() const noexcept { return pending_location_ != none_; }
    [[nodiscard]] bool location_started() const noexcept { return location_started_; }
    [[nodiscard]] Location pending_location() const noexcept;

    [[nodiscard]] ActionPermission can_enter(Location location) const;
    [[nodiscard]] bool enter_location(Location location);
    [[nodiscard]] bool return_to_map();
    [[nodiscard]] int start_location();
    [[nodiscard]] ActionResult simulated_success_result() const;
    [[nodiscard]] ActionResult abandon_current_location() const;
    [[nodiscard]] ActionResult home_rest_result();
    [[nodiscard]] ApplyResult apply_action_result(const ActionResult& result);
    [[nodiscard]] bool finish_day_summary();

private:
    static constexpr Location none_{static_cast<Location>(-1)};

    int day_{1};
    int next_result_id_{1};
    int active_result_id_{0};
    GamePhase phase_{GamePhase::day_choice};
    PlayerState player_{};
    Location pending_location_{none_};
    bool location_started_{false};
    bool day_action_done_{false};
    bool night_action_done_{false};
    std::string last_summary_;
    std::vector<int> applied_result_ids_;

    [[nodiscard]] bool result_was_applied(int result_id) const;
    void clear_pending_location();
    void apply_delta(const StatDelta& delta);
};

}  // namespace pixel_town

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/game_session.hpp"

namespace pixel_town {

// ---- Menu ----

enum class Dish : int { fried_rice, noodles, soup, dumplings, salad, dish_count };

[[nodiscard]] const char* dish_label(Dish dish);
[[nodiscard]] int dish_count();

// ---- Order ----

struct CustomerOrder {
    Dish required_dish{Dish::fried_rice};
    int wait_turns{3};  // remaining turns before timeout
};

// ---- Balance config ----

struct RestaurantConfig {
    int total_orders{6};
    int base_wait_turns{3};     // base patience per order
    int bonus_wait_turns{1};    // extra turns added by difficulty scaling

    int base_tip{5};            // money per correct serve
    int stamina_cost{18};       // total stamina spent
    int reputation_per_correct{1};
    int reputation_penalty{1};  // lost per wrong/timeout
    int mood_penalty_wrong{2};
    int mood_penalty_timeout{3};
    int mood_bonus_perfect{5};  // bonus if 100% correct
};

// ---- Service stats (accumulated during session) ----

struct ServiceStats {
    int correct{0};
    int wrong{0};
    int timeout{0};

    [[nodiscard]] int total_handled() const noexcept { return correct + wrong + timeout; }
    [[nodiscard]] double accuracy() const noexcept {
        const int handled = total_handled();
        return handled == 0 ? 0.0 : static_cast<double>(correct) / handled;
    }
};

// ---- Session state ----

enum class RestaurantPhase {
    showing_instructions,
    waiting_for_order,
    order_feedback,     // brief feedback after serve (correct/wrong)
    finished,
};

class RestaurantSession {
public:
    explicit RestaurantSession(unsigned int seed,
                               const RestaurantConfig& config = RestaurantConfig{});

    // Accessors
    [[nodiscard]] RestaurantPhase phase() const noexcept { return phase_; }
    [[nodiscard]] const CustomerOrder* current_order() const noexcept;
    [[nodiscard]] const ServiceStats& stats() const noexcept { return stats_; }
    [[nodiscard]] const RestaurantConfig& config() const noexcept { return config_; }
    [[nodiscard]] int orders_remaining() const noexcept;
    [[nodiscard]] int time_remaining() const noexcept { return time_remaining_; }
    [[nodiscard]] bool last_serve_correct() const noexcept { return last_serve_correct_; }
    [[nodiscard]] unsigned int seed() const noexcept { return seed_; }

    // Actions (all return false if invalid in current phase)
    [[nodiscard]] bool skip_instructions();            // -> waiting_for_order
    [[nodiscard]] bool view_instructions();            // -> showing_instructions
    [[nodiscard]] bool serve_dish(Dish dish);          // check correct/wrong
    [[nodiscard]] bool advance_time();                 // decrement wait, handle timeout
    [[nodiscard]] bool finish_session();               // -> finished, only if no more orders

    // Generate the final action result (call once when finished)
    [[nodiscard]] ActionResult build_result(int result_id) const;

private:
    unsigned int seed_;
    RestaurantConfig config_;
    RestaurantPhase phase_{RestaurantPhase::showing_instructions};
    std::vector<CustomerOrder> orders_;
    int current_order_index_{0};
    int time_remaining_{0};
    ServiceStats stats_;
    bool last_serve_correct_{false};

    void generate_orders();
    void advance_to_next_order_or_finish();
};

// ---- Standalone helper (for testing without session) ----

[[nodiscard]] ActionResult compute_restaurant_result(
    const ServiceStats& stats,
    const RestaurantConfig& config,
    int result_id);

}  // namespace pixel_town

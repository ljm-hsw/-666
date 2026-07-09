#include "locations/restaurant.hpp"

#include <string>

namespace pixel_town {

// ---- Menu labels ----

const char* dish_label(Dish dish) {
    switch (dish) {
        case Dish::fried_rice:  return "炒饭";
        case Dish::noodles:     return "面条";
        case Dish::soup:        return "汤";
        case Dish::dumplings:   return "饺子";
        case Dish::salad:       return "沙拉";
        default:                return "未知菜品";
    }
}

int dish_count() {
    return static_cast<int>(Dish::dish_count);
}

// ---- Deterministic seed helper ----

namespace {

unsigned int hash_mix(unsigned int x) {
    x += 0x9e3779b9U;
    x ^= x >> 15;
    x *= 0x85ebca6bU;
    x ^= x >> 13;
    x *= 0xc2b2ae35U;
    x ^= x >> 16;
    return x;
}

unsigned int dish_from_pair(unsigned int seed, int index) {
    // Combine seed and index so consecutive orders get different dishes
    return hash_mix(seed ^ hash_mix(static_cast<unsigned int>(index) * 2654435761U));
}

}  // namespace

// ---- RestaurantSession ----

RestaurantSession::RestaurantSession(unsigned int seed, const RestaurantConfig& config)
    : seed_(seed), config_(config) {
    generate_orders();
}

void RestaurantSession::generate_orders() {
    orders_.clear();
    orders_.reserve(config_.total_orders);
    for (int i = 0; i < config_.total_orders; ++i) {
        unsigned int raw = dish_from_pair(seed_, i);
        Dish d = static_cast<Dish>(static_cast<int>(raw % static_cast<unsigned int>(dish_count())));
        int wait = config_.base_wait_turns + (config_.bonus_wait_turns * i / config_.total_orders);
        orders_.push_back(CustomerOrder{d, wait});
    }
    current_order_index_ = 0;
    if (!orders_.empty()) {
        time_remaining_ = orders_[0].wait_turns;
    }
}



const CustomerOrder* RestaurantSession::current_order() const noexcept {
    if (current_order_index_ >= 0 && current_order_index_ < static_cast<int>(orders_.size())) {
        return &orders_[static_cast<std::size_t>(current_order_index_)];
    }
    return nullptr;
}

int RestaurantSession::orders_remaining() const noexcept {
    const int remaining = static_cast<int>(orders_.size()) - current_order_index_;
    return remaining > 0 ? remaining : 0;
}

bool RestaurantSession::skip_instructions() {
    if (phase_ == RestaurantPhase::showing_instructions) {
        phase_ = RestaurantPhase::waiting_for_order;
        return true;
    }
    return false;
}

bool RestaurantSession::view_instructions() {
    if (phase_ == RestaurantPhase::finished) {
        return false;
    }
    phase_ = RestaurantPhase::showing_instructions;
    return true;
}

bool RestaurantSession::serve_dish(Dish dish) {
    if (phase_ != RestaurantPhase::waiting_for_order) {
        return false;
    }
    const auto* order = current_order();
    if (!order) {
        return false;
    }

    if (dish == order->required_dish) {
        // Correct!
        ++stats_.correct;
        last_serve_correct_ = true;
        last_feedback_ = RestaurantFeedback::correct;
    } else {
        // Wrong dish - counts as an error, time penalty
        ++stats_.wrong;
        last_serve_correct_ = false;
        last_feedback_ = RestaurantFeedback::wrong;
    }

    phase_ = RestaurantPhase::order_feedback;
    return true;
}

bool RestaurantSession::advance_time() {
    if (phase_ == RestaurantPhase::order_feedback) {
        // After feedback, move to next order
        advance_to_next_order_or_finish();
        return true;
    }

    if (phase_ == RestaurantPhase::waiting_for_order) {
        // Tick down the timer
        --time_remaining_;
        if (time_remaining_ <= 0) {
            // Customer timed out
            ++stats_.timeout;
            last_serve_correct_ = false;
            last_feedback_ = RestaurantFeedback::timeout;
            phase_ = RestaurantPhase::order_feedback;
        }
        return true;
    }

    return false;
}

bool RestaurantSession::finish_session() {
    if (phase_ == RestaurantPhase::finished) {
        return true;  // already finished
    }
    if (orders_remaining() > 0 && phase_ != RestaurantPhase::showing_instructions) {
        return false;  // still have orders to handle
    }
    phase_ = RestaurantPhase::finished;
    return true;
}

void RestaurantSession::advance_to_next_order_or_finish() {
    ++current_order_index_;
    if (current_order_index_ >= static_cast<int>(orders_.size())) {
        phase_ = RestaurantPhase::finished;
    } else {
        time_remaining_ = orders_[static_cast<std::size_t>(current_order_index_)].wait_turns;
        last_feedback_ = RestaurantFeedback::none;
        phase_ = RestaurantPhase::waiting_for_order;
    }
}

ActionResult RestaurantSession::build_result(int result_id) const {
    return compute_restaurant_result(stats_, config_, result_id);
}

// ---- Standalone result computation ----

ActionResult compute_restaurant_result(
    const ServiceStats& stats,
    const RestaurantConfig& config,
    int result_id) {

    const int total = stats.total_handled();
    if (total == 0) {
        ActionResult result;
        result.result_id = result_id;
        result.slot = ActionSlot::day;
        result.location = Location::restaurant;
        result.outcome = ActionOutcome::abandoned;
        result.summary = "餐馆工作已放弃：未服务任何顾客。";
        return result;
    }

    const double acc = stats.accuracy();

    // Money: base_tip * correct - small penalty for wrong/timeout
    const int money_delta = config.base_tip * stats.correct
                          - (config.reputation_penalty * (stats.wrong + stats.timeout));

    // Stamina: always costs the full amount
    const int stamina_delta = -config.stamina_cost;

    // Reputation: gain per correct, lose per wrong/timeout
    int reputation_delta = config.reputation_per_correct * stats.correct
                         - config.reputation_penalty * (stats.wrong + stats.timeout);

    // Mood: penalty for wrong and timeout, bonus if perfect
    int mood_delta = -config.mood_penalty_wrong * stats.wrong
                   - config.mood_penalty_timeout * stats.timeout;
    if (acc >= 1.0 && total >= config.total_orders) {
        mood_delta += config.mood_bonus_perfect;
    }

    // Summary
    const std::string summary =
        "餐馆工作完成：服务了 " + std::to_string(total) + " 位顾客，" +
        "正确 " + std::to_string(stats.correct) + "，" +
        "错单 " + std::to_string(stats.wrong) + "，" +
        "超时 " + std::to_string(stats.timeout) + "。" +
        "正确率 " + std::to_string(static_cast<int>(acc * 100)) + "%。";

    ActionResult result;
    result.result_id = result_id;
    result.slot = ActionSlot::day;
    result.location = Location::restaurant;
    result.outcome = ActionOutcome::completed;
    result.delta = StatDelta{money_delta, stamina_delta, reputation_delta, 0, mood_delta};
    result.summary = summary;
    return result;
}

}  // namespace pixel_town

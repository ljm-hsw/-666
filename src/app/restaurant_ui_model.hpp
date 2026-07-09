#pragma once

#include <string>
#include <vector>

#include "locations/restaurant.hpp"

namespace pixel_town {

struct RestaurantOrderTicket {
    std::string order_progress;
    std::string dish_name;
    std::string time_label;
    float time_ratio{0.0F};
    bool time_warning{false};
};

struct RestaurantCompletionSummary {
    std::string stats_line;
    std::string accuracy_line;
    std::string delta_line;
};

[[nodiscard]] std::vector<std::string> restaurant_tutorial_lines();
[[nodiscard]] RestaurantOrderTicket restaurant_order_ticket(const RestaurantSession& session);
[[nodiscard]] std::string restaurant_feedback_text(const RestaurantSession& session);
[[nodiscard]] RestaurantCompletionSummary restaurant_completion_summary(
    const RestaurantSession& session,
    int preview_result_id);
[[nodiscard]] std::string signed_stat(int value);

}  // namespace pixel_town

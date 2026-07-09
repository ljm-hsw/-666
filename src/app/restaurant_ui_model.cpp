#include "app/restaurant_ui_model.hpp"

#include <algorithm>
#include <cmath>

namespace pixel_town {
namespace {

int handled_before_current(const RestaurantSession& session) {
    const int handled = session.stats().total_handled();
    return std::min(handled + 1, session.config().total_orders);
}

int accuracy_percent(const ServiceStats& stats) {
    return static_cast<int>(std::round(stats.accuracy() * 100.0));
}

}  // namespace

std::vector<std::string> restaurant_tutorial_lines() {
    return {
        "目标：在午餐高峰中按订单给顾客上对菜品。",
        "操作：按 1-5 或用鼠标点击菜品按钮上菜。",
        "收益：正确上菜会获得金钱和声望，完美服务会提升心情。",
        "风险：等待归零会超时，错单和超时会降低表现。",
        "结束：服务完全部顾客后点击结算；主动放弃会消耗白天且无收益。",
    };
}

RestaurantOrderTicket restaurant_order_ticket(const RestaurantSession& session) {
    RestaurantOrderTicket ticket;
    ticket.order_progress = "第 " + std::to_string(handled_before_current(session)) + " / " +
                            std::to_string(session.config().total_orders) + " 单";
    if (const auto* order = session.current_order()) {
        ticket.dish_name = dish_label(order->required_dish);
        ticket.time_label = "剩余 " + std::to_string(session.time_remaining()) + " 秒";
        const float total = static_cast<float>(std::max(1, order->wait_turns));
        ticket.time_ratio =
            std::max(0.0F, std::min(1.0F, static_cast<float>(session.time_remaining()) / total));
        ticket.time_warning = session.time_remaining() <= 1 || ticket.time_ratio <= 0.34F;
    } else {
        ticket.dish_name = "无订单";
        ticket.time_label = "等待结算";
        ticket.time_ratio = 0.0F;
        ticket.time_warning = false;
    }
    return ticket;
}

std::string restaurant_feedback_text(const RestaurantSession& session) {
    switch (session.last_feedback()) {
        case RestaurantFeedback::correct:
            return "正确上菜！顾客点了点头。";
        case RestaurantFeedback::wrong:
            return "错单了：这会影响声望和心情。";
        case RestaurantFeedback::timeout:
            return "超时：顾客等太久离开了。";
        case RestaurantFeedback::none:
            break;
    }
    return "等待下一位顾客。";
}

std::string signed_stat(int value) {
    if (value > 0) {
        return "+" + std::to_string(value);
    }
    return std::to_string(value);
}

RestaurantCompletionSummary restaurant_completion_summary(const RestaurantSession& session,
                                                          int preview_result_id) {
    const auto& stats = session.stats();
    const auto result = session.build_result(preview_result_id);
    RestaurantCompletionSummary summary;
    summary.stats_line = "正确 " + std::to_string(stats.correct) + "  错单 " +
                         std::to_string(stats.wrong) + "  超时 " +
                         std::to_string(stats.timeout);
    summary.accuracy_line = "正确率 " + std::to_string(accuracy_percent(stats)) + "%";
    summary.delta_line = "金钱 " + signed_stat(result.delta.money) + "  体力 " +
                         signed_stat(result.delta.stamina) + "  声望 " +
                         signed_stat(result.delta.reputation) + "  心情 " +
                         signed_stat(result.delta.mood);
    return summary;
}

}  // namespace pixel_town

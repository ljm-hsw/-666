#include <doctest/doctest.h>

#include "app/restaurant_ui_model.hpp"

TEST_CASE("restaurant tutorial covers goal controls rewards risks and ending") {
    const auto tutorial = pixel_town::restaurant_tutorial_lines();

    REQUIRE(tutorial.size() >= 5);
    CHECK(tutorial[0].find("目标") != std::string::npos);
    CHECK(tutorial[1].find("1-5") != std::string::npos);
    CHECK(tutorial[1].find("鼠标") != std::string::npos);
    CHECK(tutorial[2].find("收益") != std::string::npos);
    CHECK(tutorial[3].find("等待归零") != std::string::npos);
    CHECK(tutorial[4].find("结算") != std::string::npos);
}

TEST_CASE("restaurant order ticket exposes progress dish and time pressure") {
    pixel_town::RestaurantConfig config;
    config.total_orders = 3;
    config.base_wait_turns = 4;
    config.bonus_wait_turns = 0;
    pixel_town::RestaurantSession session(42, config);
    REQUIRE(session.skip_instructions());

    const auto ticket = pixel_town::restaurant_order_ticket(session);

    CHECK(ticket.order_progress == "第 1 / 3 单");
    CHECK(ticket.dish_name == pixel_town::dish_label(session.current_order()->required_dish));
    CHECK(ticket.time_label == "剩余 4 秒");
    CHECK(ticket.time_ratio == doctest::Approx(1.0F));
    CHECK_FALSE(ticket.time_warning);

    REQUIRE(session.advance_time());
    REQUIRE(session.advance_time());
    REQUIRE(session.advance_time());

    const auto urgent = pixel_town::restaurant_order_ticket(session);
    CHECK(urgent.time_label == "剩余 1 秒");
    CHECK(urgent.time_ratio == doctest::Approx(0.25F));
    CHECK(urgent.time_warning);
}

TEST_CASE("restaurant timeout enters a visible feedback state") {
    pixel_town::RestaurantConfig config;
    config.total_orders = 2;
    config.base_wait_turns = 1;
    config.bonus_wait_turns = 0;
    pixel_town::RestaurantSession session(42, config);
    REQUIRE(session.skip_instructions());

    REQUIRE(session.advance_time());

    CHECK(session.phase() == pixel_town::RestaurantPhase::order_feedback);
    CHECK(session.last_feedback() == pixel_town::RestaurantFeedback::timeout);
    CHECK(pixel_town::restaurant_feedback_text(session).find("超时") != std::string::npos);
}

TEST_CASE("restaurant completion summary includes deltas for final settlement") {
    pixel_town::RestaurantConfig config;
    pixel_town::RestaurantSession session(42, config);
    REQUIRE(session.skip_instructions());

    while (session.phase() != pixel_town::RestaurantPhase::finished) {
        if (session.phase() == pixel_town::RestaurantPhase::waiting_for_order) {
            const auto* order = session.current_order();
            REQUIRE(order != nullptr);
            REQUIRE(session.serve_dish(order->required_dish));
        } else if (session.phase() == pixel_town::RestaurantPhase::order_feedback) {
            REQUIRE(session.advance_time());
        }
    }

    const auto summary = pixel_town::restaurant_completion_summary(session, 99);

    CHECK(summary.stats_line.find("正确 6") != std::string::npos);
    CHECK(summary.accuracy_line.find("100%") != std::string::npos);
    CHECK(summary.delta_line.find("金钱 +30") != std::string::npos);
    CHECK(summary.delta_line.find("体力 -18") != std::string::npos);
    CHECK(summary.delta_line.find("心情 +5") != std::string::npos);
}

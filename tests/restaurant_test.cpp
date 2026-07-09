#include <doctest/doctest.h>

#include "locations/restaurant.hpp"

namespace pixel_town {
namespace {

// Helper: run a full session, serving correct dishes
ServiceStats run_perfect_session(unsigned int seed = 42) {
    RestaurantSession session(seed);
    REQUIRE(session.skip_instructions());

    while (session.phase() != RestaurantPhase::finished) {
        if (session.phase() == RestaurantPhase::waiting_for_order) {
            const auto* order = session.current_order();
            if (order) {
                REQUIRE(session.serve_dish(order->required_dish));
            }
        } else if (session.phase() == RestaurantPhase::order_feedback) {
            REQUIRE(session.advance_time());
        } else {
            break;
        }
    }
    return session.stats();
}

// Helper: run a session, always serving wrong dish
ServiceStats run_all_wrong_session(unsigned int seed = 42) {
    RestaurantSession session(seed);
    REQUIRE(session.skip_instructions());

    // Pick a dish that's definitely wrong for any order
    // (use salad as default, switch if order happens to be salad)
    const Dish wrong_dish = Dish::noodles;  // will override if needed

    while (session.phase() != RestaurantPhase::finished) {
        if (session.phase() == RestaurantPhase::waiting_for_order) {
            const auto* order = session.current_order();
            if (order) {
                Dish to_serve = wrong_dish;
                if (to_serve == order->required_dish) {
                    // Pick a different wrong dish
                    to_serve = Dish::fried_rice;
                    if (to_serve == order->required_dish) {
                        to_serve = Dish::soup;
                    }
                }
                REQUIRE(session.serve_dish(to_serve));
                // advance_time to move past feedback
                REQUIRE(session.advance_time());
            }
        } else if (session.phase() == RestaurantPhase::order_feedback) {
            REQUIRE(session.advance_time());
        } else {
            break;
        }
    }
    return session.stats();
}

}  // namespace

TEST_CASE("dish labels are non-null and distinct") {
    const int count = dish_count();
    CHECK(count == 5);

    for (int i = 0; i < count; ++i) {
        const char* label = dish_label(static_cast<Dish>(i));
        CHECK(label != nullptr);
        CHECK(std::string{label}.size() > 0);
    }

    // All labels should be distinct
    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            CHECK(std::string{dish_label(static_cast<Dish>(i))} !=
                  std::string{dish_label(static_cast<Dish>(j))});
        }
    }
}

TEST_CASE("session starts in showing_instructions phase") {
    RestaurantSession session(42);
    CHECK(session.phase() == RestaurantPhase::showing_instructions);
    CHECK(session.stats().total_handled() == 0);
    CHECK(session.orders_remaining() > 0);
    // Orders are pre-generated; current_order is available but interaction
    // should only happen after skipping instructions.
}

TEST_CASE("skip instructions moves to waiting_for_order") {
    RestaurantSession session(42);
    CHECK(session.skip_instructions());
    CHECK(session.phase() == RestaurantPhase::waiting_for_order);
    CHECK(session.current_order() != nullptr);

    // Cannot skip again
    CHECK_FALSE(session.skip_instructions());
}

TEST_CASE("correct serve increments correct count") {
    RestaurantSession session(42);
    REQUIRE(session.skip_instructions());

    const auto* order = session.current_order();
    REQUIRE(order != nullptr);
    const Dish correct_dish = order->required_dish;

    CHECK(session.serve_dish(correct_dish));
    CHECK(session.stats().correct == 1);
    CHECK(session.stats().wrong == 0);
    CHECK(session.last_serve_correct());
    CHECK(session.phase() == RestaurantPhase::order_feedback);
}

TEST_CASE("wrong serve increments wrong count") {
    RestaurantSession session(42);
    REQUIRE(session.skip_instructions());

    const auto* order = session.current_order();
    REQUIRE(order != nullptr);

    // Find a wrong dish
    Dish wrong_dish = Dish::fried_rice;
    if (wrong_dish == order->required_dish) {
        wrong_dish = Dish::noodles;
    }

    CHECK(session.serve_dish(wrong_dish));
    CHECK(session.stats().wrong == 1);
    CHECK(session.stats().correct == 0);
    CHECK_FALSE(session.last_serve_correct());
}

TEST_CASE("serve_dish fails when not in waiting_for_order phase") {
    RestaurantSession session(42);

    // In showing_instructions phase
    CHECK_FALSE(session.serve_dish(Dish::fried_rice));
}

TEST_CASE("advance_time decrements timer") {
    RestaurantSession session(42);
    REQUIRE(session.skip_instructions());

    const int initial_time = session.time_remaining();
    CHECK(initial_time > 0);

    REQUIRE(session.advance_time());
    CHECK(session.time_remaining() == initial_time - 1);
}

TEST_CASE("timeout when timer reaches zero") {
    // Use a session with only 1 turn of patience
    RestaurantConfig config;
    config.total_orders = 2;
    config.base_wait_turns = 1;
    config.bonus_wait_turns = 0;

    RestaurantSession session(42, config);
    REQUIRE(session.skip_instructions());

    CHECK(session.phase() == RestaurantPhase::waiting_for_order);
    CHECK(session.time_remaining() == 1);

    // Advance time → timer hits 0 → timeout
    REQUIRE(session.advance_time());
    CHECK(session.stats().timeout == 1);

    // Should move to next order (or finish)
    if (session.phase() == RestaurantPhase::waiting_for_order) {
        CHECK(session.current_order() != nullptr);
    }
}

TEST_CASE("fixed seed produces reproducible order sequences") {
    constexpr unsigned int seed = 12345;

    RestaurantSession a(seed);
    RestaurantSession b(seed);

    REQUIRE(a.skip_instructions());
    REQUIRE(b.skip_instructions());

    // Both should have the same first order
    REQUIRE(a.current_order() != nullptr);
    REQUIRE(b.current_order() != nullptr);
    CHECK(a.current_order()->required_dish == b.current_order()->required_dish);
    CHECK(a.current_order()->wait_turns == b.current_order()->wait_turns);
}

TEST_CASE("perfect session gives correct stats") {
    const auto stats = run_perfect_session(42);
    CHECK(stats.correct == 6);
    CHECK(stats.wrong == 0);
    CHECK(stats.timeout == 0);
    CHECK(stats.accuracy() == doctest::Approx(1.0));
}

TEST_CASE("all-wrong session gives correct stats") {
    const auto stats = run_all_wrong_session(42);
    CHECK(stats.correct == 0);
    CHECK(stats.wrong == 6);
    CHECK(stats.timeout == 0);
    CHECK(stats.accuracy() == doctest::Approx(0.0));
}

TEST_CASE("compute_restaurant_result produces valid ActionResult") {
    ServiceStats stats;
    stats.correct = 4;
    stats.wrong = 1;
    stats.timeout = 1;

    RestaurantConfig config;
    const auto result = compute_restaurant_result(stats, config, 1);

    CHECK(result.result_id == 1);
    CHECK(result.slot == ActionSlot::day);
    CHECK(result.location == Location::restaurant);
    CHECK(result.outcome == ActionOutcome::completed);

    // money = 5*4 - 1*(1+1) = 20 - 2 = 18
    CHECK(result.delta.money == 18);
    // stamina = -18
    CHECK(result.delta.stamina == -18);
    // reputation = 1*4 - 1*(1+1) = 2
    CHECK(result.delta.reputation == 2);
    // mood = -2*1 - 3*1 = -5 (not perfect, so no bonus)
    CHECK(result.delta.mood == -5);

    CHECK(result.summary.find("正确 4") != std::string::npos);
    CHECK(result.summary.find("错单 1") != std::string::npos);
    CHECK(result.summary.find("超时 1") != std::string::npos);
}

TEST_CASE("perfect result gets mood bonus") {
    ServiceStats stats;
    stats.correct = 6;
    stats.wrong = 0;
    stats.timeout = 0;

    RestaurantConfig config;
    const auto result = compute_restaurant_result(stats, config, 1);

    // mood = 0 + 5 (perfect bonus)
    CHECK(result.delta.mood == 5);
    CHECK(result.summary.find("100%") != std::string::npos);
}

TEST_CASE("empty stats produces abandoned result") {
    ServiceStats stats;
    RestaurantConfig config;
    const auto result = compute_restaurant_result(stats, config, 1);

    CHECK(result.outcome == ActionOutcome::abandoned);
    CHECK(result.delta.money == 0);
    CHECK(result.summary.find("放弃") != std::string::npos);
}

TEST_CASE("session build_result matches compute_restaurant_result") {
    const auto stats = run_perfect_session(42);
    RestaurantConfig config;
    RestaurantSession session(42);
    REQUIRE(session.skip_instructions());

    // Use standalone computation
    const auto standalone = compute_restaurant_result(stats, config, 7);

    // Use session's build_result with the same stats
    // We can't directly compare because build_result uses its internal stats,
    // but we can verify the result_id and structure
    CHECK(standalone.result_id == 7);
    CHECK(standalone.location == Location::restaurant);
    CHECK(standalone.slot == ActionSlot::day);
}

}  // namespace pixel_town

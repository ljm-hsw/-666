#include <doctest/doctest.h>

#include "app/location_runtime.hpp"
#include "app/ui_primitives.hpp"
#include "ui/ui_metrics.hpp"

TEST_CASE("restaurant layout stays inside the 960 by 540 canvas") {
    const Rectangle stats = pixel_town::scaled_rect(pixel_town::restaurant_stats_panel());
    const Rectangle start =
        pixel_town::scaled_rect(pixel_town::restaurant_instructions_start_button());

    CHECK(stats.x >= 0.0F);
    CHECK(stats.y >= 0.0F);
    CHECK(stats.x + stats.width <= pixel_town::ui::canvas_width);
    CHECK(stats.y + stats.height <= pixel_town::ui::canvas_height);
    CHECK(start.x + start.width <= pixel_town::ui::canvas_width);
    CHECK(start.y + start.height <= pixel_town::ui::canvas_height);
}

TEST_CASE("tavern hotspots and controls stay inside the logical canvas") {
    const auto layout = pixel_town::tavern_layout();
    const Rectangle bounds[] = {
        layout.npc_hotspot,
        layout.gomoku_hotspot,
        layout.dice_hotspot,
        layout.select_button,
        layout.back_button,
        layout.overlay_panel,
        layout.gomoku_card,
        layout.dice_card,
        layout.gomoku_confirm_button,
        layout.gomoku_abandon_button,
        layout.dice_panel,
        layout.round_result_panel,
    };

    for (const Rectangle design_bounds : bounds) {
        const Rectangle canvas_bounds = pixel_town::scaled_rect(design_bounds);
        CHECK(canvas_bounds.x >= 0.0F);
        CHECK(canvas_bounds.y >= 0.0F);
        CHECK(canvas_bounds.x + canvas_bounds.width <= pixel_town::ui::canvas_width);
        CHECK(canvas_bounds.y + canvas_bounds.height <= pixel_town::ui::canvas_height);
    }
    CHECK_FALSE(CheckCollisionRecs(pixel_town::scaled_rect(layout.gomoku_hotspot),
                                   pixel_town::scaled_rect(layout.dice_hotspot)));
}

TEST_CASE("store start rejects an over-budget plan without rewriting it") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::convenience_store));

    pixel_town::LocationRuntimeState runtime;
    pixel_town::prepare_store_runtime(runtime);
    runtime.store_purchase_plan.quantities["umbrella"] = 10;
    const auto original_plan = runtime.store_purchase_plan.quantities;
    std::string notice;

    CHECK_FALSE(pixel_town::start_pending_location(session, runtime, notice));
    CHECK_FALSE(session.location_started());
    CHECK(runtime.store_purchase_plan.quantities == original_plan);
    CHECK(notice.find("现金不足") != std::string::npos);
}

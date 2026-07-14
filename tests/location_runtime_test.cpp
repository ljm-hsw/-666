#include <doctest/doctest.h>

#include "app/tavern_layout.hpp"
#include "app/location_runtime.hpp"
#include "app/ui_primitives.hpp"
#include "ui/scene_viewport.hpp"
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
    const pixel_town::TavernRect bounds[] = {
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

    const auto as_rectangle = [](pixel_town::TavernRect bounds) {
        return Rectangle{bounds.x, bounds.y, bounds.width, bounds.height};
    };
    for (const pixel_town::TavernRect design_bounds : bounds) {
        const Rectangle canvas_bounds =
            pixel_town::scaled_rect(as_rectangle(design_bounds));
        CHECK(canvas_bounds.x >= 0.0F);
        CHECK(canvas_bounds.y >= 0.0F);
        CHECK(canvas_bounds.x + canvas_bounds.width <= pixel_town::ui::canvas_width);
        CHECK(canvas_bounds.y + canvas_bounds.height <= pixel_town::ui::canvas_height);
    }
    CHECK_FALSE(CheckCollisionRecs(
        pixel_town::scaled_rect(as_rectangle(layout.gomoku_hotspot)),
        pixel_town::scaled_rect(as_rectangle(layout.dice_hotspot))));
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
    CHECK(runtime.store_feedback == notice);
}

TEST_CASE("store plan actions provide immediate visible feedback") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::convenience_store));

    pixel_town::LocationRuntimeState runtime;
    pixel_town::prepare_store_runtime(runtime);

    const auto selected = pixel_town::apply_store_plan_action(
        runtime, session,
        {pixel_town::StorePlanActionType::select_product, 1});
    REQUIRE(selected.accepted);
    CHECK(selected.changed);
    CHECK(runtime.store_selected_product_index == 1);
    CHECK(selected.message.find("汽水") != std::string::npos);

    const auto increased = pixel_town::apply_store_plan_action(
        runtime, session,
        {pixel_town::StorePlanActionType::increase_purchase, 1});
    REQUIRE(increased.accepted);
    CHECK(increased.changed);
    CHECK(runtime.store_purchase_plan.quantities.at("soda") == 1);
    CHECK(increased.message.find("进货数量") != std::string::npos);

    const auto priced = pixel_town::apply_store_plan_action(
        runtime, session,
        {pixel_town::StorePlanActionType::set_high_price, 1});
    REQUIRE(priced.accepted);
    CHECK(priced.changed);
    CHECK(runtime.store_price_plan.tiers.at("soda") ==
          pixel_town::store::PriceTier::high);
    CHECK(priced.message.find("高价") != std::string::npos);
}

TEST_CASE("store plan controls report limits instead of silently doing nothing") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::convenience_store));

    pixel_town::LocationRuntimeState runtime;
    pixel_town::prepare_store_runtime(runtime);

    const auto result = pixel_town::apply_store_plan_action(
        runtime, session,
        {pixel_town::StorePlanActionType::decrease_purchase, 0});

    REQUIRE(result.accepted);
    CHECK_FALSE(result.changed);
    CHECK(result.message.find("已经是 0") != std::string::npos);
}

TEST_CASE("store mouse hotspots map to the intended product actions") {
    const Rectangle plus =
        pixel_town::scaled_rect(pixel_town::store_purchase_increase_button(2));
    const auto plus_action = pixel_town::store_plan_action_at(
        Vector2{plus.x + plus.width / 2.0F, plus.y + plus.height / 2.0F}, 4);

    REQUIRE(plus_action.has_value());
    CHECK(plus_action->type == pixel_town::StorePlanActionType::increase_purchase);
    CHECK(plus_action->product_index == 2);

    const Rectangle row = pixel_town::scaled_rect(pixel_town::store_product_row(3));
    const auto row_action = pixel_town::store_plan_action_at(
        Vector2{row.x + 5.0F, row.y + row.height / 2.0F}, 4);

    REQUIRE(row_action.has_value());
    CHECK(row_action->type == pixel_town::StorePlanActionType::select_product);
    CHECK(row_action->product_index == 3);
}

TEST_CASE("map notices wrap UTF-8 text and ellipsize at the line limit") {
    const auto lines = pixel_town::wrap_text_lines("一二三四五六七八九", 4, 2);

    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "一二三四");
    CHECK(lines[1] == "五六七…");
}

TEST_CASE("store runtime glyph manifest includes generated feedback copy") {
    const std::string glyphs = pixel_town::store_runtime_glyphs();

    CHECK(glyphs.find("已经是 0") != std::string::npos);
    CHECK(glyphs.find("方案已锁定") != std::string::npos);
    CHECK(glyphs.find("现金不足") != std::string::npos);
}

TEST_CASE("library starts at mode selection and can launch the organizing mode") {
    auto session = pixel_town::GameSession::new_game();
    REQUIRE(session.enter_location(pixel_town::Location::library));
    pixel_town::LocationRuntimeState runtime;
    std::string notice;

    REQUIRE(pixel_town::start_pending_location(session, runtime, notice));
    CHECK(runtime.library_mode == pixel_town::LibraryWorkMode::selection);
    CHECK(runtime.library_engine == nullptr);
    CHECK(runtime.library_organizing == nullptr);

    REQUIRE(pixel_town::select_library_mode(
        session, runtime, pixel_town::LibraryWorkMode::book_organizing, notice));
    CHECK(runtime.library_mode == pixel_town::LibraryWorkMode::book_organizing);
    REQUIRE(runtime.library_organizing != nullptr);
    CHECK(runtime.library_organizing->state().is_active);
    CHECK(runtime.library_engine == nullptr);
    CHECK(notice.find("整理") != std::string::npos);
}

TEST_CASE("library mode controls stay below the scene header and do not overlap") {
    const Rectangle reader = pixel_town::library::ui::reader_mode_button();
    const Rectangle organizing = pixel_town::library::ui::organizing_mode_button();

    CHECK(reader.y >= pixel_town::ui::scene_header_height);
    CHECK(organizing.y >= pixel_town::ui::scene_header_height);
    CHECK(reader.x >= 0.0F);
    CHECK(reader.x + reader.width <= pixel_town::ui::canvas_width);
    CHECK(organizing.x + organizing.width <= pixel_town::ui::canvas_width);
    CHECK_FALSE(CheckCollisionRecs(reader, organizing));
}

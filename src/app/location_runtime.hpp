#pragma once

#include <memory>
#include <string>

#include <raylib.h>

#include "core/game_session.hpp"
#include "core/tavern_rules.hpp"
#include "app/tavern_runtime.hpp"
#include "locations/convenience_store.hpp"
#include "locations/library_data.hpp"
#include "locations/library_rules.hpp"
#include "locations/library_scene.hpp"
#include "locations/library_ui.hpp"
#include "locations/restaurant.hpp"

namespace pixel_town {

struct LocationRuntimeState {
    std::unique_ptr<RestaurantSession> restaurant;
    float restaurant_timer{0.0F};
    TavernRuntimeState tavern;
    store::PurchasePlan store_purchase_plan;
    store::PricePlan store_price_plan;
    int store_selected_product_index{0};
    bool in_library{false};
    library::LibraryData library_data;
    std::unique_ptr<library::LibraryRuleEngine> library_engine;
    library::LibraryScene library_scene;
    library::ui::LibraryUIState library_ui_state;
    int library_visits{0};
};

[[nodiscard]] Rectangle location_back_button(bool is_tavern);
[[nodiscard]] Rectangle location_start_button(bool is_tavern);
[[nodiscard]] Rectangle location_abandon_button(bool is_tavern);
[[nodiscard]] Rectangle restaurant_abandon_button();
[[nodiscard]] Rectangle restaurant_dish_button(int dish_index);
[[nodiscard]] Rectangle restaurant_instructions_start_button();
[[nodiscard]] Rectangle restaurant_stats_panel();
[[nodiscard]] Rectangle store_back_button();
[[nodiscard]] Rectangle store_start_button();
[[nodiscard]] Rectangle store_abandon_button();

void prepare_restaurant_runtime(LocationRuntimeState& runtime, unsigned int seed);
void prepare_store_runtime(LocationRuntimeState& runtime);
void update_store_selection(LocationRuntimeState& runtime, const GameSession& session);
[[nodiscard]] bool start_pending_location(GameSession& session,
                                           LocationRuntimeState& runtime,
                                           std::string& notice);
[[nodiscard]] bool update_active_library(GameSession& session,
                                          LocationRuntimeState& runtime,
                                          std::string& notice,
                                          Vector2 logical_mouse);
[[nodiscard]] bool update_started_location(GameSession& session,
                                            LocationRuntimeState& runtime,
                                            std::string& notice,
                                            Vector2 logical_mouse);
void draw_active_library(const Font& font, const LocationRuntimeState& runtime,
                         Vector2 logical_mouse);

}  // namespace pixel_town

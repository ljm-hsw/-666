#pragma once

#include <memory>
#include <string>

#include <raylib.h>

#include "core/game_session.hpp"
#include "core/tavern_rules.hpp"
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
    ChallengeType tavern_challenge{ChallengeType::gomoku};
    BetTier tavern_bet{BetTier::medium};
    store::PriceTier store_price_tier{store::PriceTier::standard};
    int store_purchase_units{1};
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

void prepare_restaurant_runtime(LocationRuntimeState& runtime, unsigned int seed);
void update_tavern_selection(LocationRuntimeState& runtime);
void update_store_selection(LocationRuntimeState& runtime);
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

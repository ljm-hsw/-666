#pragma once

#include <array>
#include <memory>
#include <optional>
#include <string>

#include <raylib.h>

#include "app/library_runtime.hpp"
#include "app/library_room_runtime.hpp"
#include "app/npc_lobby_runtime.hpp"
#include "app/story_lifecycle_runtime.hpp"
#include "core/game_session.hpp"
#include "app/tavern_runtime.hpp"
#include "app/tavern_view.hpp"
#include "locations/convenience_store.hpp"
#include "locations/library_organizing_ui.hpp"
#include "locations/library_ui.hpp"
#include "locations/restaurant.hpp"

namespace pixel_town {

enum class StorePlanActionType {
    select_product,
    decrease_purchase,
    increase_purchase,
    set_low_price,
    set_standard_price,
    set_high_price,
};

struct StorePlanAction {
    StorePlanActionType type{StorePlanActionType::select_product};
    int product_index{0};
};

struct StorePlanFeedback {
    bool accepted{false};
    bool changed{false};
    std::string message;
};

struct LocationRuntimeState {
    std::unique_ptr<RestaurantSession> restaurant;
    float restaurant_timer{0.0F};
    TavernRuntime tavern;
    TavernVisualAssets tavern_assets;
    store::PurchasePlan store_purchase_plan;
    store::PricePlan store_price_plan;
    int store_selected_product_index{0};
    std::string store_feedback;
    NpcLobbyRuntime npc_lobby;
    StoryLifecycleRuntime story_lifecycle;
    LibraryRoomRuntime library_room;
    LibraryRuntime library;
    library::ui::LibraryUIState library_ui_state;
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
[[nodiscard]] Rectangle store_product_row(int product_index);
[[nodiscard]] Rectangle store_purchase_decrease_button(int product_index);
[[nodiscard]] Rectangle store_purchase_increase_button(int product_index);
[[nodiscard]] Rectangle store_price_button(int product_index, store::PriceTier tier);
[[nodiscard]] std::optional<StorePlanAction> store_plan_action_at(
    Vector2 logical_mouse, int product_count);

void prepare_restaurant_runtime(LocationRuntimeState& runtime, unsigned int seed);
void prepare_store_runtime(LocationRuntimeState& runtime);
[[nodiscard]] StorePlanFeedback apply_store_plan_action(
    LocationRuntimeState& runtime, const GameSession& session, StorePlanAction action);
[[nodiscard]] std::string store_runtime_glyphs();
void update_store_selection(LocationRuntimeState& runtime, const GameSession& session,
                            Vector2 logical_mouse, std::string& notice);
[[nodiscard]] bool select_library_mode(GameSession& session,
                                       LocationRuntimeState& runtime,
                                       LibraryRuntimeMode mode,
                                       std::string& notice);
[[nodiscard]] bool start_pending_location(GameSession& session,
                                           LocationRuntimeState& runtime,
                                           std::string& notice);
[[nodiscard]] LibraryRoomStepResult step_library_room(
    GameSession& session, LocationRuntimeState& runtime,
    const LibraryRoomInput& input, std::string& notice);
[[nodiscard]] NpcLobbyStepResult step_restaurant_lobby(
    GameSession& session, LocationRuntimeState& runtime,
    const NpcLobbyInput& input, std::string& notice);
[[nodiscard]] NpcLobbyStepResult step_store_lobby(
    GameSession& session, LocationRuntimeState& runtime,
    const NpcLobbyInput& input, std::string& notice);
[[nodiscard]] bool update_active_library(GameSession& session,
                                          LocationRuntimeState& runtime,
                                          std::string& notice,
                                          Vector2 logical_mouse);
[[nodiscard]] bool update_started_location(GameSession& session,
                                            LocationRuntimeState& runtime,
                                            std::string& notice,
                                            Vector2 logical_mouse);
void draw_active_library(const Font& font, const LocationRuntimeState& runtime,
                         Vector2 logical_mouse, const Texture2D& background,
                         const std::array<Texture2D, 6>& organizing_book_textures);

}  // namespace pixel_town

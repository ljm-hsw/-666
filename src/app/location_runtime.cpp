#include "app/location_runtime.hpp"

#include <algorithm>

#include "app/location_result_adapter.hpp"
#include "app/ui_primitives.hpp"
#include "locations/convenience_store.hpp"

namespace pixel_town {
namespace {

constexpr const char* library_data_path = "assets/data/library_data.txt";

store::StoreInventory to_store_inventory(const std::vector<StoreInventoryItem>& items) {
    store::StoreInventory inventory;
    for (const auto& item : items) {
        inventory[item.item_id] = item.quantity;
    }
    return inventory;
}

int inventory_quantity(const GameSession& session, const std::string& item_id) {
    for (const auto& item : session.store_inventory()) {
        if (item.item_id == item_id) {
            return item.quantity;
        }
    }
    return 0;
}

void ensure_store_runtime_plan(LocationRuntimeState& runtime, const store::StoreConfig& config) {
    for (const auto& product : config.products) {
        if (runtime.store_purchase_plan.quantities.find(product.id) ==
            runtime.store_purchase_plan.quantities.end()) {
            runtime.store_purchase_plan.quantities[product.id] = 0;
        }
        if (runtime.store_price_plan.tiers.find(product.id) ==
            runtime.store_price_plan.tiers.end()) {
            runtime.store_price_plan.tiers[product.id] = store::PriceTier::standard;
        }
    }
    if (runtime.store_selected_product_index < 0 ||
        runtime.store_selected_product_index >= static_cast<int>(config.products.size())) {
        runtime.store_selected_product_index = 0;
    }
}

const store::ProductConfig& selected_store_product(LocationRuntimeState& runtime,
                                                   const store::StoreConfig& config) {
    ensure_store_runtime_plan(runtime, config);
    return config.products[static_cast<std::size_t>(runtime.store_selected_product_index)];
}

void adjust_selected_store_purchase(LocationRuntimeState& runtime,
                                    const store::StoreConfig& config,
                                    const GameSession& session,
                                    int delta) {
    const auto& product = selected_store_product(runtime, config);
    int& quantity = runtime.store_purchase_plan.quantities[product.id];
    const int max_add =
        std::max(0, config.max_stock_per_product - inventory_quantity(session, product.id));
    quantity = std::max(0, std::min(max_add, quantity + delta));
}

store::DailyStoreContext make_store_context(const GameSession& session) {
    const auto day_context = session.current_day_context();
    return store::DailyStoreContext{session.day(),
                                    session.location_seed(Location::convenience_store),
                                    day_context.weather,
                                    day_context.event};
}

}  // namespace

Rectangle location_back_button(bool is_tavern) {
    return Rectangle{126, is_tavern ? 252.0F : 228.0F, 112, 34};
}

Rectangle location_start_button(bool is_tavern) {
    return Rectangle{264, is_tavern ? 252.0F : 228.0F, 112, 34};
}

Rectangle location_abandon_button(bool is_tavern) {
    return Rectangle{402, is_tavern ? 252.0F : 228.0F, 112, 34};
}

Rectangle restaurant_abandon_button() {
    return Rectangle{438.0F, 214.0F, 118.0F, 30.0F};
}

Rectangle restaurant_dish_button(int dish_index) {
    return Rectangle{58.0F + static_cast<float>(dish_index) * 116.0F, 274.0F, 102.0F, 50.0F};
}

Rectangle restaurant_instructions_start_button() {
    return Rectangle{232.0F, 314.0F, 176.0F, 28.0F};
}

Rectangle restaurant_stats_panel() {
    return Rectangle{58.0F, 326.0F, 540.0F, 20.0F};
}

Rectangle store_back_button() {
    return Rectangle{126.0F, 314.0F, 112.0F, 34.0F};
}

Rectangle store_start_button() {
    return Rectangle{264.0F, 314.0F, 112.0F, 34.0F};
}

Rectangle store_abandon_button() {
    return Rectangle{402.0F, 314.0F, 112.0F, 34.0F};
}

void prepare_restaurant_runtime(LocationRuntimeState& runtime, unsigned int seed) {
    runtime.restaurant = std::make_unique<RestaurantSession>(seed);
    runtime.restaurant_timer = 0.0F;
}

void prepare_store_runtime(LocationRuntimeState& runtime) {
    const auto config = store::default_store_config();
    runtime.store_purchase_plan = store::PurchasePlan{};
    runtime.store_price_plan = store::default_price_plan(config);
    runtime.store_selected_product_index = 0;
    ensure_store_runtime_plan(runtime, config);
}

void update_tavern_selection(LocationRuntimeState& runtime) {
    if (IsKeyPressed(KEY_ONE)) {
        runtime.tavern_challenge = ChallengeType::gomoku;
    }
    if (IsKeyPressed(KEY_TWO)) {
        runtime.tavern_challenge = ChallengeType::liars_dice;
    }
    if (IsKeyPressed(KEY_THREE)) {
        runtime.tavern_bet = BetTier::low;
    }
    if (IsKeyPressed(KEY_FOUR)) {
        runtime.tavern_bet = BetTier::medium;
    }
    if (IsKeyPressed(KEY_FIVE)) {
        runtime.tavern_bet = BetTier::high;
    }
}

void update_store_selection(LocationRuntimeState& runtime, const GameSession& session) {
    const auto config = store::default_store_config();
    ensure_store_runtime_plan(runtime, config);

    if (IsKeyPressed(KEY_ONE)) {
        runtime.store_selected_product_index = 0;
    }
    if (IsKeyPressed(KEY_TWO) && config.products.size() > 1) {
        runtime.store_selected_product_index = 1;
    }
    if (IsKeyPressed(KEY_THREE) && config.products.size() > 2) {
        runtime.store_selected_product_index = 2;
    }
    if (IsKeyPressed(KEY_FOUR) && config.products.size() > 3) {
        runtime.store_selected_product_index = 3;
    }
    if (IsKeyPressed(KEY_Q)) {
        const auto& product = selected_store_product(runtime, config);
        runtime.store_price_plan.tiers[product.id] = store::PriceTier::low;
    }
    if (IsKeyPressed(KEY_W)) {
        const auto& product = selected_store_product(runtime, config);
        runtime.store_price_plan.tiers[product.id] = store::PriceTier::standard;
    }
    if (IsKeyPressed(KEY_E)) {
        const auto& product = selected_store_product(runtime, config);
        runtime.store_price_plan.tiers[product.id] = store::PriceTier::high;
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        adjust_selected_store_purchase(runtime, config, session, -1);
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        adjust_selected_store_purchase(runtime, config, session, 1);
    }
}

bool start_pending_location(GameSession& session, LocationRuntimeState& runtime,
                            std::string& notice) {
    const bool is_tavern = session.phase() == GamePhase::night_location &&
                           session.pending_location() == Location::tavern;
    if (is_tavern) {
        const TavernChallengeConfig config;
        if (!can_afford_tavern_bet(session.player(), runtime.tavern_bet, config)) {
            notice = "金钱不足，无法选择该赌注档位。";
            return false;
        }
    }

    if (session.pending_location() == Location::library) {
        const auto load_result = library::load_library_data(library_data_path);
        if (!load_result.success) {
            notice = std::string{"图书馆数据加载失败："} + load_result.error_message;
            return false;
        }
        if (session.start_location() == 0) {
            notice = "图书馆地点会话启动失败。";
            return false;
        }
        runtime.library_data = std::move(load_result.data);
        runtime.library_engine = std::make_unique<library::LibraryRuleEngine>(
            runtime.library_data, library::default_library_config());
        ++runtime.library_visits;
        runtime.library_engine->start_session(
            make_library_daily_context(session, runtime.library_visits));
        runtime.library_engine->update_npc_relationship(session.player().knowledge,
                                                        runtime.library_visits);
        runtime.library_ui_state = library::ui::LibraryUIState{};
        runtime.in_library = true;
        notice = "已进入图书馆，请完成读者问答。";
        return true;
    }

    if (session.pending_location() == Location::convenience_store) {
        const auto config = store::default_store_config();
        ensure_store_runtime_plan(runtime, config);
        const auto validation = store::validate_purchase_plan(
            config, to_store_inventory(session.store_inventory()), runtime.store_purchase_plan,
            session.player().money);
        if (!validation.allowed) {
            notice = validation.message;
            return false;
        }
    }

    if (session.start_location() == 0) {
        return false;
    }

    if (is_tavern) {
        notice = std::string{"已选择"} + challenge_type_label(runtime.tavern_challenge) +
                 "、" + bet_tier_label(runtime.tavern_bet) + "赌注：按空格完成模拟。";
    } else {
        notice = "地点已开始：完成或放弃都会消耗本阶段。";
    }
    return true;
}

bool update_active_library(GameSession& session, LocationRuntimeState& runtime,
                           std::string& notice, Vector2 logical_mouse) {
    if (!runtime.in_library || !runtime.library_engine) {
        return false;
    }

    library::ui::update_library_ui(*runtime.library_engine, runtime.library_ui_state,
                                   runtime.library_scene);
    const bool exit_library = library::ui::handle_library_input(
        *runtime.library_engine, runtime.library_ui_state, runtime.library_scene, logical_mouse);
    if (!exit_library) {
        return true;
    }

    const auto library_result = runtime.library_engine->finish_session();
    const auto applied = session.apply_action_result(library_action_result(
        library_result, session.active_result_id(), action_slot_for_phase(session.phase())));
    runtime.in_library = false;
    runtime.library_ui_state = library::ui::LibraryUIState{};
    notice = applied.accepted ? library_result.summary : applied.message;
    return true;
}

bool update_started_location(GameSession& session, LocationRuntimeState& runtime,
                             std::string& notice, Vector2 logical_mouse) {
    const bool is_tavern = session.phase() == GamePhase::night_location &&
                           session.pending_location() == Location::tavern;

    if (session.pending_location() == Location::restaurant && runtime.restaurant) {
        auto& restaurant = *runtime.restaurant;
        const Rectangle close_button{568, 94, 28, 28};
        if (clicked(close_button, logical_mouse)) {
            const auto applied = session.apply_action_result(session.abandon_current_location());
            runtime.restaurant.reset();
            notice = applied.accepted ? "已放弃餐馆工作" : applied.message;
            return true;
        }

        if (restaurant.phase() == RestaurantPhase::showing_instructions) {
            const Rectangle start_button = restaurant_instructions_start_button();
            if (activated(start_button, logical_mouse, KEY_SPACE)) {
                (void)restaurant.skip_instructions();
            }
            return true;
        }

        if (restaurant.phase() == RestaurantPhase::waiting_for_order) {
            if (IsKeyPressed(KEY_I)) {
                (void)restaurant.view_instructions();
                return true;
            }
            for (int i = 0; i < dish_count(); ++i) {
                if (IsKeyPressed(static_cast<KeyboardKey>(KEY_ONE + i)) ||
                    clicked(restaurant_dish_button(i), logical_mouse)) {
                    (void)restaurant.serve_dish(static_cast<Dish>(i));
                    break;
                }
            }
            runtime.restaurant_timer += GetFrameTime();
            while (runtime.restaurant_timer >= 1.0F) {
                runtime.restaurant_timer -= 1.0F;
                (void)restaurant.advance_time();
            }
        }

        if (restaurant.phase() == RestaurantPhase::order_feedback && IsKeyPressed(KEY_SPACE)) {
            (void)restaurant.advance_time();
        }

        if (restaurant.phase() == RestaurantPhase::finished) {
            const Rectangle done_button{232, 300, 176, 28};
            if (activated(done_button, logical_mouse, KEY_SPACE)) {
                const auto result = restaurant.build_result(session.active_result_id());
                const auto applied = session.apply_action_result(result);
                runtime.restaurant.reset();
                notice = applied.message;
                return true;
            }
        }

        if (clicked(restaurant_abandon_button(), logical_mouse)) {
            const auto applied = session.apply_action_result(session.abandon_current_location());
            runtime.restaurant.reset();
            notice = applied.accepted ? "已放弃餐馆工作" : applied.message;
            return true;
        }
        return true;
    }

    const Rectangle finish_button = session.pending_location() == Location::convenience_store
                                        ? store_start_button()
                                        : location_start_button(is_tavern);
    if (activated(finish_button, logical_mouse, KEY_SPACE)) {
        if (is_tavern) {
            const TavernChallengeConfig config;
            const auto result = tavern_action_result(session, runtime.tavern_challenge,
                                                     runtime.tavern_bet, ChallengeOutcome::win,
                                                     config);
            if (result.result_id != 0) {
                const auto applied = session.apply_action_result(result);
                notice = applied.message;
            }
        } else if (session.pending_location() == Location::convenience_store) {
            const auto config = store::default_store_config();
            ensure_store_runtime_plan(runtime, config);
            const auto settlement = store::simulate_sales(
                config, to_store_inventory(session.store_inventory()),
                runtime.store_purchase_plan, runtime.store_price_plan,
                make_store_context(session), session.player().money);
            if (!settlement.accepted) {
                notice = settlement.message;
                return true;
            }
            const auto applied = session.apply_action_result(
                store::build_store_action_result(config, settlement, session.active_result_id()));
            notice = applied.accepted ? settlement.summary : applied.message;
        } else {
            notice = "地点运行状态缺失，无法结算；请返回最近存档重试。";
        }
        return true;
    }

    const Rectangle abandon_button = session.pending_location() == Location::convenience_store
                                         ? store_abandon_button()
                                         : location_abandon_button(is_tavern);
    if (clicked(abandon_button, logical_mouse)) {
        const auto applied = session.apply_action_result(session.abandon_current_location());
        notice = applied.message;
        return true;
    }
    return true;
}

void draw_active_library(const Font& font, const LocationRuntimeState& runtime,
                         Vector2 logical_mouse) {
    if (!runtime.in_library || !runtime.library_engine) {
        return;
    }
    library::ui::LibraryRenderConfig render_config;
    render_config.logical_width = 640;
    render_config.logical_height = 360;
    library::ui::draw_library_scene(*runtime.library_engine, runtime.library_ui_state,
                                    runtime.library_scene, render_config, font, logical_mouse);
}

}  // namespace pixel_town

// 餐馆、便利店和图书馆入口的共享调度；不拥有全局状态，只调用 GameSession。
#include "app/location_runtime.hpp"

#include <algorithm>

#include "app/ui_primitives.hpp"
#include "core/location_story.hpp"
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

const char* store_price_name(store::PriceTier tier) {
    switch (tier) {
        case store::PriceTier::low:
            return "低价";
        case store::PriceTier::standard:
            return "标准价";
        case store::PriceTier::high:
            return "高价";
    }
    return "标准价";
}

store::DailyStoreContext make_store_context(const GameSession& session) {
    const auto day_context = session.current_day_context();
    return store::DailyStoreContext{session.day(),
                                    session.location_seed(Location::convenience_store),
                                    day_context.weather,
                                    day_context.event};
}

}  // namespace

// ---- 页面几何：只返回逻辑画布坐标，不包含点击后的业务行为 ----
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
    return Rectangle{126.0F, 316.0F, 112.0F, 30.0F};
}

Rectangle store_start_button() {
    return Rectangle{264.0F, 316.0F, 112.0F, 30.0F};
}

Rectangle store_abandon_button() {
    return Rectangle{402.0F, 316.0F, 112.0F, 30.0F};
}

Rectangle store_product_row(int product_index) {
    return Rectangle{88.0F, 161.0F + static_cast<float>(product_index) * 22.0F,
                     456.0F, 20.0F};
}

Rectangle store_purchase_decrease_button(int product_index) {
    return Rectangle{248.0F, 160.0F + static_cast<float>(product_index) * 22.0F,
                     20.0F, 20.0F};
}

Rectangle store_purchase_increase_button(int product_index) {
    return Rectangle{306.0F, 160.0F + static_cast<float>(product_index) * 22.0F,
                     20.0F, 20.0F};
}

Rectangle store_price_button(int product_index, store::PriceTier tier) {
    float x = 344.0F;
    if (tier == store::PriceTier::standard) {
        x = 378.0F;
    } else if (tier == store::PriceTier::high) {
        x = 412.0F;
    }
    return Rectangle{x, 160.0F + static_cast<float>(product_index) * 22.0F,
                     30.0F, 20.0F};
}

std::optional<StorePlanAction> store_plan_action_at(Vector2 logical_mouse,
                                                    int product_count) {
    for (int product_index = 0; product_index < product_count; ++product_index) {
        if (CheckCollisionPointRec(
                logical_mouse,
                scaled_rect(store_purchase_decrease_button(product_index)))) {
            return StorePlanAction{StorePlanActionType::decrease_purchase, product_index};
        }
        if (CheckCollisionPointRec(
                logical_mouse,
                scaled_rect(store_purchase_increase_button(product_index)))) {
            return StorePlanAction{StorePlanActionType::increase_purchase, product_index};
        }
        if (CheckCollisionPointRec(
                logical_mouse,
                scaled_rect(store_price_button(product_index, store::PriceTier::low)))) {
            return StorePlanAction{StorePlanActionType::set_low_price, product_index};
        }
        if (CheckCollisionPointRec(
                logical_mouse,
                scaled_rect(store_price_button(product_index, store::PriceTier::standard)))) {
            return StorePlanAction{StorePlanActionType::set_standard_price, product_index};
        }
        if (CheckCollisionPointRec(
                logical_mouse,
                scaled_rect(store_price_button(product_index, store::PriceTier::high)))) {
            return StorePlanAction{StorePlanActionType::set_high_price, product_index};
        }
        if (CheckCollisionPointRec(logical_mouse,
                                   scaled_rect(store_product_row(product_index)))) {
            return StorePlanAction{StorePlanActionType::select_product, product_index};
        }
    }
    return std::nullopt;
}

// ---- 地点 Runtime 初始化：建立临时对象，尚未消耗 GameSession 行动 ----
void prepare_restaurant_runtime(LocationRuntimeState& runtime, unsigned int seed) {
    runtime.restaurant = std::make_unique<RestaurantSession>(seed);
    runtime.restaurant_timer = 0.0F;
}

void prepare_store_runtime(LocationRuntimeState& runtime) {
    const auto config = store::default_store_config();
    runtime.store_purchase_plan = store::PurchasePlan{};
    runtime.store_price_plan = store::default_price_plan(config);
    runtime.store_selected_product_index = 0;
    runtime.store_feedback = "点击商品行，再用减号/加号调整进货并选择价格档。";
    ensure_store_runtime_plan(runtime, config);
}

bool open_daytime_story_lobby(const GameSession& session,
                              LocationRuntimeState& runtime, Location location,
                              std::string& notice) {
    const LocationStorySelection selection = LocationStoryCatalog{}.select(
        location_story_context(session, location));
    if (selection.script.lines.empty()) {
        notice = "地点剧情暂时不可用。";
        return false;
    }

    switch (location) {
        case Location::restaurant:
        case Location::convenience_store:
            if (!runtime.npc_lobby.open(selection.script)) {
                notice = "地点 NPC 对话暂时不可用。";
                return false;
            }
            return true;
        case Location::library:
            if (!runtime.library_room.open(selection.script)) {
                notice = "图书馆场景加载失败。";
                return false;
            }
            return true;
        case Location::home:
        case Location::tavern:
            notice = "该地点不使用白天剧情大厅。";
            return false;
    }
    notice = "地点剧情暂时不可用。";
    return false;
}

StorePlanFeedback apply_store_plan_action(LocationRuntimeState& runtime,
                                          const GameSession& session,
                                          StorePlanAction action) {
    const auto config = store::default_store_config();
    ensure_store_runtime_plan(runtime, config);
    StorePlanFeedback feedback;
    if (action.product_index < 0 ||
        action.product_index >= static_cast<int>(config.products.size())) {
        feedback.message = "便利店商品选择无效。";
        return feedback;
    }

    feedback.accepted = true;
    const auto& product = config.products[static_cast<std::size_t>(action.product_index)];
    const int previous_selection = runtime.store_selected_product_index;
    runtime.store_selected_product_index = action.product_index;
    int& quantity = runtime.store_purchase_plan.quantities[product.id];
    auto& tier = runtime.store_price_plan.tiers[product.id];

    switch (action.type) {
        case StorePlanActionType::select_product:
            feedback.changed = previous_selection != action.product_index;
            feedback.message = std::string{"已选择"} + product.name + "：进货 " +
                               std::to_string(quantity) + "，价格 " +
                               store_price_name(tier) + "。";
            break;
        case StorePlanActionType::decrease_purchase:
            if (quantity == 0) {
                feedback.message = std::string{product.name} + "进货数量已经是 0。";
                break;
            }
            --quantity;
            feedback.changed = true;
            feedback.message = std::string{product.name} + "进货数量：" +
                               std::to_string(quantity) + "（成本 " +
                               std::to_string(quantity * product.unit_cost) + " 金币）。";
            break;
        case StorePlanActionType::increase_purchase: {
            const int max_add = std::max(
                0, config.max_stock_per_product - inventory_quantity(session, product.id));
            if (quantity >= max_add) {
                feedback.message = std::string{product.name} + "已达库存上限 " +
                                   std::to_string(config.max_stock_per_product) + "。";
                break;
            }
            ++quantity;
            feedback.changed = true;
            feedback.message = std::string{product.name} + "进货数量：" +
                               std::to_string(quantity) + "（成本 " +
                               std::to_string(quantity * product.unit_cost) + " 金币）。";
            break;
        }
        case StorePlanActionType::set_low_price:
        case StorePlanActionType::set_standard_price:
        case StorePlanActionType::set_high_price: {
            const store::PriceTier next_tier =
                action.type == StorePlanActionType::set_low_price
                    ? store::PriceTier::low
                    : (action.type == StorePlanActionType::set_standard_price
                           ? store::PriceTier::standard
                           : store::PriceTier::high);
            feedback.changed = tier != next_tier;
            tier = next_tier;
            feedback.message = std::string{product.name} + "价格档已设为" +
                               store_price_name(tier) + "，售价 " +
                               std::to_string(store::price_for_tier(product, tier)) +
                               " 金币。";
            break;
        }
    }
    runtime.store_feedback = feedback.message;
    return feedback;
}

std::string store_runtime_glyphs() {
    return std::string{store::convenience_store_glyphs()} +
           "低价标准价高价点击商品行，再用减号/加号调整进货并选择价格档。"
           "便利店商品选择无效。已选择：进货 0，价格。进货数量已经是 0。"
           "进货数量：（成本 金币）。已达库存上限。价格档已设为，售价 金币。"
           "方案已锁定：点击“结算销售”查看收入、利润和剩余库存。"
           "地点已开始：完成或放弃都会消耗本阶段。";
}

void update_store_selection(LocationRuntimeState& runtime, const GameSession& session,
                            Vector2 logical_mouse, std::string& notice) {
    const auto config = store::default_store_config();
    ensure_store_runtime_plan(runtime, config);
    if (runtime.store_feedback.empty()) {
        runtime.store_feedback = "点击商品行，再用减号/加号调整进货并选择价格档。";
    }

    const auto apply = [&](StorePlanActionType type, int index) {
        const auto feedback = apply_store_plan_action(runtime, session, {type, index});
        notice = feedback.message;
    };

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const auto action = store_plan_action_at(
            logical_mouse, static_cast<int>(config.products.size()));
        if (action.has_value()) {
            apply(action->type, action->product_index);
            return;
        }
    }

    if (IsKeyPressed(KEY_ONE)) {
        apply(StorePlanActionType::select_product, 0);
    }
    if (IsKeyPressed(KEY_TWO) && config.products.size() > 1) {
        apply(StorePlanActionType::select_product, 1);
    }
    if (IsKeyPressed(KEY_THREE) && config.products.size() > 2) {
        apply(StorePlanActionType::select_product, 2);
    }
    if (IsKeyPressed(KEY_FOUR) && config.products.size() > 3) {
        apply(StorePlanActionType::select_product, 3);
    }
    if (IsKeyPressed(KEY_Q)) {
        apply(StorePlanActionType::set_low_price, runtime.store_selected_product_index);
    }
    if (IsKeyPressed(KEY_W)) {
        apply(StorePlanActionType::set_standard_price, runtime.store_selected_product_index);
    }
    if (IsKeyPressed(KEY_E)) {
        apply(StorePlanActionType::set_high_price, runtime.store_selected_product_index);
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        apply(StorePlanActionType::decrease_purchase, runtime.store_selected_product_index);
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        apply(StorePlanActionType::increase_purchase, runtime.store_selected_product_index);
    }
}

// ---- 统一启动/推进/结束边界 ----
// start_pending_location 分配 active_result_id；update_started_location 最终把
// 规则结果提交给 Session。返回地图只关闭临时 Runtime，不应改变白天/夜晚标志。
bool start_pending_location(GameSession& session, LocationRuntimeState& runtime,
                            std::string& notice) {
    if (session.pending_location() == Location::library) {
        const auto load_result = library::load_library_data(library_data_path);
        if (!load_result.success) {
            notice = std::string{"图书馆数据加载失败："} + load_result.error_message;
            return false;
        }
        const auto opened = runtime.library.open(session, load_result.data);
        if (opened.status != LibraryOpenStatus::opened) {
            notice = opened.message;
            return false;
        }
        runtime.library_ui_state = library::ui::LibraryUIState{};
        notice = opened.message;
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
            runtime.store_feedback = validation.message;
            return false;
        }
    }

    if (session.start_location() == 0) {
        return false;
    }

    if (session.pending_location() == Location::convenience_store) {
        runtime.store_feedback =
            "方案已锁定：点击“结算销售”查看收入、利润和剩余库存。";
        notice = runtime.store_feedback;
    } else {
        notice = "地点已开始：完成或放弃都会消耗本阶段。";
    }
    return true;
}

LibraryRoomStepResult step_library_room(
    GameSession& session, LocationRuntimeState& runtime,
    const LibraryRoomInput& input, std::string& notice) {
    LibraryRoomStepResult result = runtime.library_room.step(input);
    if (!result.notice.empty()) {
        notice = result.notice;
    }
    if (result.status != LibraryRoomStepStatus::work_requested) {
        return result;
    }

    if (!session.enter_location(Location::library)) {
        notice = "当前阶段不能开始图书馆工作。";
        return {LibraryRoomStepStatus::rejected, notice};
    }
    if (!start_pending_location(session, runtime, notice)) {
        (void)session.return_to_map();
        return {LibraryRoomStepStatus::rejected, notice};
    }
    return result;
}

NpcLobbyStepResult step_restaurant_lobby(
    GameSession& session, LocationRuntimeState& runtime,
    const NpcLobbyInput& input, std::string& notice) {
    NpcLobbyStepResult result = runtime.npc_lobby.step(input);
    if (!result.notice.empty()) {
        notice = result.notice;
    }
    if (result.status != NpcLobbyStepStatus::activity_requested) {
        return result;
    }

    if (!session.enter_location(Location::restaurant)) {
        notice = "当前阶段不能开始餐馆工作。";
        return {NpcLobbyStepStatus::rejected, notice};
    }
    prepare_restaurant_runtime(
        runtime, session.location_seed(Location::restaurant));
    notice = "老板交代完毕，已进入餐馆工作准备。";
    return result;
}

NpcLobbyStepResult step_store_lobby(
    GameSession& session, LocationRuntimeState& runtime,
    const NpcLobbyInput& input, std::string& notice) {
    NpcLobbyStepResult result = runtime.npc_lobby.step(input);
    if (!result.notice.empty()) {
        notice = result.notice;
    }
    if (result.status != NpcLobbyStepStatus::activity_requested) {
        return result;
    }

    if (!session.enter_location(Location::convenience_store)) {
        notice = "当前阶段不能开始便利店经营。";
        return {NpcLobbyStepStatus::rejected, notice};
    }
    prepare_store_runtime(runtime);
    notice = "店主交代完毕，已进入便利店经营准备。";
    return result;
}

bool select_library_mode(GameSession& session, LocationRuntimeState& runtime,
                         LibraryRuntimeMode mode, std::string& notice) {
    LibraryIntent intent;
    if (mode == LibraryRuntimeMode::reader_consultation) {
        intent.type = LibraryIntentType::select_reader;
    } else if (mode == LibraryRuntimeMode::book_organizing) {
        intent.type = LibraryIntentType::select_organizing;
    } else {
        notice = "请选择一种图书馆工作模式。";
        return false;
    }
    const auto stepped = runtime.library.step(session, intent);
    notice = stepped.notice;
    if (stepped.status != LibraryStepStatus::changed) {
        return false;
    }
    if (mode == LibraryRuntimeMode::reader_consultation) {
        runtime.library_ui_state = library::ui::LibraryUIState{};
    }
    return true;
}

bool update_active_library(GameSession& session, LocationRuntimeState& runtime,
                           std::string& notice, Vector2 logical_mouse) {
    if (!runtime.library.active()) {
        return false;
    }

    auto presentation = runtime.library.presentation();
    if (presentation.mode == LibraryRuntimeMode::selection) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            notice = runtime.library
                         .step(session, {LibraryIntentType::abandon, {}})
                         .notice;
            return true;
        }
        const bool choose_reader =
            IsKeyPressed(KEY_ONE) ||
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
             CheckCollisionPointRec(logical_mouse, library::ui::reader_mode_button()));
        const bool choose_organizing =
            IsKeyPressed(KEY_TWO) ||
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
             CheckCollisionPointRec(logical_mouse, library::ui::organizing_mode_button()));
        if (choose_reader) {
            (void)select_library_mode(session, runtime,
                                      LibraryRuntimeMode::reader_consultation, notice);
        } else if (choose_organizing) {
            (void)select_library_mode(session, runtime,
                                      LibraryRuntimeMode::book_organizing, notice);
        }
        return true;
    }

    if (presentation.mode == LibraryRuntimeMode::book_organizing &&
        presentation.organizing) {
        const LibraryIntent intent = library::ui::handle_library_organizing_input(
            *presentation.organizing, logical_mouse);
        if (intent.type != LibraryIntentType::none) {
            const auto stepped = runtime.library.step(session, intent);
            notice = stepped.notice;
        }
        return true;
    }

    if (!presentation.reader) {
        notice = "图书馆工作模式未能启动。";
        return true;
    }

    library::ui::update_library_ui(*presentation.reader, runtime.library_ui_state);
    const LibraryIntent intent = library::ui::handle_library_input(
        *presentation.reader, runtime.library_ui_state, logical_mouse);
    if (intent.type != LibraryIntentType::none) {
        const auto stepped = runtime.library.step(session, intent);
        notice = stepped.notice;
        if (intent.type == LibraryIntentType::answer_category &&
            runtime.library.active()) {
            const auto updated = runtime.library.presentation();
            if (updated.reader) {
                runtime.library_ui_state.last_answer_correct =
                    updated.reader->last_answer_correct;
                runtime.library_ui_state.feedback_data.is_correct =
                    updated.reader->last_answer_correct;
            }
        }
    }
    return true;
}

bool update_started_location(GameSession& session, LocationRuntimeState& runtime,
                             std::string& notice, Vector2 logical_mouse) {
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
                                        : location_start_button(false);
    if (activated(finish_button, logical_mouse, KEY_SPACE)) {
        if (session.pending_location() == Location::convenience_store) {
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
                                         : location_abandon_button(false);
    if (clicked(abandon_button, logical_mouse)) {
        const auto applied = session.apply_action_result(session.abandon_current_location());
        notice = applied.message;
        return true;
    }
    return true;
}

void draw_active_library(const Font& font, const LocationRuntimeState& runtime,
                         Vector2 logical_mouse, const Texture2D& background,
                         const Texture2D& old_map,
                         const std::array<Texture2D, 6>& organizing_book_textures) {
    if (!runtime.library.active()) {
        return;
    }
    const auto presentation = runtime.library.presentation();
    if (presentation.mode == LibraryRuntimeMode::selection) {
        library::ui::draw_library_mode_selection(font, background, logical_mouse);
        return;
    }
    if (presentation.mode == LibraryRuntimeMode::book_organizing &&
        presentation.organizing) {
        library::ui::draw_library_organizing(
            font, background, organizing_book_textures, *presentation.organizing,
            logical_mouse);
        return;
    }
    if (!presentation.reader) {
        return;
    }
    library::ui::LibraryRenderConfig render_config;
    render_config.logical_width = 640;
    render_config.logical_height = 360;
    render_config.background = background;
    render_config.old_map = old_map;
    library::ui::draw_library_scene(*presentation.reader, runtime.library_ui_state,
                                    render_config, font, logical_mouse);
}

}  // namespace pixel_town

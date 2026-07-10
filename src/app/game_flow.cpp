#include "app/game_flow.hpp"

#include "app/tavern_view.hpp"

#include <array>
#include <string>

#include "app/restaurant_ui_model.hpp"
#include "app/ui_primitives.hpp"
#include "core/story_text.hpp"

namespace pixel_town {
namespace {

constexpr std::array<Location, 5> map_locations{
    Location::restaurant, Location::convenience_store, Location::library, Location::tavern,
    Location::home};

TavernFrameInput tavern_frame_input(Vector2 logical_mouse) {
    TavernFrameInput input;
    input.elapsed_seconds = GetFrameTime();
    input.pointer = TavernCanvasPoint{logical_mouse.x, logical_mouse.y,
                                      logical_mouse.x >= 0.0F &&
                                          logical_mouse.y >= 0.0F};
    input.primary_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.escape_pressed = IsKeyPressed(KEY_ESCAPE);
    input.enter_pressed = IsKeyPressed(KEY_ENTER);
    input.space_pressed = IsKeyPressed(KEY_SPACE);
    for (int digit = 1; digit <= 5; ++digit) {
        if (IsKeyPressed(static_cast<KeyboardKey>(KEY_ONE + digit - 1))) {
            input.digit_pressed = digit;
            break;
        }
    }
    return input;
}

constexpr std::array ui_texts{
    "像素小镇",
    "十日经营计划",
    "新游戏",
    "继续游戏",
    "第 1 天",
    "第 2 天",
    "白天选择",
    "白天地点",
    "夜晚选择",
    "夜晚地点",
    "每日总结",
    "最终结局",
    "金钱",
    "体力",
    "声望",
    "知识",
    "心情",
    "餐馆",
    "便利店",
    "图书馆",
    "酒馆",
    "家",
    "开放",
    "未开放",
    "返回地图",
    "开始模拟",
    "完成模拟",
    "主动放弃",
    "回家休息",
    "继续到下一天",
    "十日计划完成",
    "主结局",
    "最终状态",
    "静音",
    "平凡小镇新人",
    "结局之后不能继续提交地点行动或推进天数",
    "晴天",
    "多云",
    "小雨",
    "微风",
    "餐馆客流增加",
    "便利店零食更受欢迎",
    "图书馆读者变多",
    "小镇节奏平稳",
    "已恢复最近的阶段边界",
    "检测到已有存档，可继续游戏或开始新游戏",
    "检测到已有存档，再次点击新游戏将覆盖；Esc 取消",
    "已取消新游戏，原存档保持不变",
    "存档版本不兼容，原文件已保留",
    "存档损坏或缺字段，原文件已保留",
    "炒饭", "面条", "汤", "饺子", "沙拉",
    "正确", "错单", "超时", "餐馆工作完成",
    "准备根据顾客订单按至菜品选择上想要等待秒剩余结算",
    "五子棋",
    "骗子骰子",
    "低赌注",
    "中赌注",
    "高赌注",
    "挑战",
    "赌注",
    "价格档",
    "进货数量",
    "商品",
    "库存",
    "进货",
    "售价",
    "选中",
    "低价",
    "标准价",
    "高价",
    "今日提示",
    "当前库存",
    "每种商品",
    "按1-4选商品  A/D调进货  Q/W/E调价格",
    "咖啡",
    "金币",
    "选择挑战",
    "选择赌注",
    "空格开始/完成",
    "Esc返回",
    "已进入酒馆，选择挑战和赌注。",
    "金钱不足，无法选择该赌注档位。",
    "目标：在午餐高峰中按订单给顾客上对菜品。",
    "操作：按 1-5 或用鼠标点击菜品按钮上菜。",
    "收益：正确上菜会获得金钱和声望，完美服务会提升心情。",
    "风险：等待归零会超时，错单和超时会降低表现。",
    "结束：服务完全部顾客后点击结算；主动放弃会消耗白天且无收益。",
};

const char* store_price_tier_label(store::PriceTier tier) {
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

int store_plan_quantity(const store::PurchasePlan& plan, const std::string& item_id) {
    const auto found = plan.quantities.find(item_id);
    return found == plan.quantities.end() ? 0 : found->second;
}

store::PriceTier store_plan_tier(const store::PricePlan& plan, const std::string& item_id) {
    const auto found = plan.tiers.find(item_id);
    return found == plan.tiers.end() ? store::PriceTier::standard : found->second;
}

int store_inventory_quantity(const GameSession& session, const std::string& item_id) {
    for (const auto& item : session.store_inventory()) {
        if (item.item_id == item_id) {
            return item.quantity;
        }
    }
    return 0;
}

std::array<Rectangle, 5> location_bounds() {
    return {Rectangle{70, 74, 160, 86}, Rectangle{388, 74, 160, 86},
            Rectangle{70, 186, 160, 88}, Rectangle{420, 186, 152, 88},
            Rectangle{250, 186, 96, 88}};
}

Rectangle restaurant_prepare_back_button() {
    return Rectangle{118, 310, 132, 32};
}

Rectangle restaurant_prepare_start_button() {
    return Rectangle{390, 310, 132, 32};
}

Rectangle generated_sprite_destination(Location location) {
    switch (location) {
        case Location::restaurant:
            return Rectangle{92, 78, 92, 70};
        case Location::convenience_store:
            return Rectangle{416, 78, 96, 72};
        case Location::library:
            return Rectangle{92, 194, 92, 70};
        case Location::tavern:
            return Rectangle{448, 196, 88, 66};
        case Location::home:
            return Rectangle{268, 198, 58, 54};
    }
    return Rectangle{0, 0, 0, 0};
}

Rectangle generated_label_destination(Location location) {
    switch (location) {
        case Location::restaurant:
            return Rectangle{88, 132, 112, 31};
        case Location::convenience_store:
            return Rectangle{408, 132, 118, 31};
        case Location::library:
            return Rectangle{86, 248, 116, 31};
        case Location::tavern:
            return Rectangle{432, 248, 116, 31};
        case Location::home:
            return Rectangle{260, 248, 72, 31};
    }
    return Rectangle{0, 0, 0, 0};
}

Rectangle source_tile(int tile_index) {
    constexpr int columns = 12;
    constexpr int tile_size = 16;
    return Rectangle{static_cast<float>((tile_index % columns) * tile_size),
                     static_cast<float>((tile_index / columns) * tile_size),
                     tile_size, tile_size};
}

void draw_tile(const Texture2D& tiles, int tile_index, Rectangle destination) {
    if (tiles.id == 0) {
        return;
    }
    DrawTexturePro(tiles, source_tile(tile_index), scaled_rect(destination), Vector2{0.0F, 0.0F},
                   0.0F, WHITE);
}

Rectangle generated_building_source(std::size_t index) {
    constexpr float cell_width = 128.0F;
    constexpr float cell_height = 96.0F;
    return Rectangle{static_cast<float>(index) * cell_width, 0.0F, cell_width, cell_height};
}

void draw_status(const Font& font, const GameSession& session, bool audio_enabled) {
    const auto& player = session.player();
    DrawRectangle(0, 0, 960, 84, slate);
    text(font, "像素小镇", 16, 9, 22, RAYWHITE);
    text(font, std::string{"第 "} + std::to_string(session.day()) + " 天 · " +
                   phase_label(session.phase()),
         132, 16, 18, Color{255, 224, 154, 255});
    text(font, std::string{"金钱 "} + std::to_string(player.money), 316, 6, 18, RAYWHITE);
    text(font, std::string{"体力 "} + std::to_string(player.stamina), 410, 6, 18, RAYWHITE);
    text(font, std::string{"声望 "} + std::to_string(player.reputation), 504, 6, 18, RAYWHITE);
    text(font, std::string{"知识 "} + std::to_string(player.knowledge), 316, 30, 18, RAYWHITE);
    text(font, std::string{"心情 "} + std::to_string(player.mood), 410, 30, 18, RAYWHITE);
    if (!audio_enabled) {
        text(font, "静音", 548, 30, 18, Color{255, 208, 166, 255});
    }
}

Rectangle title_continue_button() {
    return Rectangle{202, 188, 236, 40};
}

Rectangle title_new_game_button(bool resume_available) {
    return resume_available ? Rectangle{232, 240, 176, 38} : Rectangle{232, 206, 176, 42};
}

void draw_title(const Font& font, const Texture2D& title_background, const GameAppState& state,
                Vector2 mouse) {
    if (title_background.id != 0) {
        DrawTexturePro(
            title_background,
            Rectangle{0.0F, 0.0F, static_cast<float>(title_background.width),
                      static_cast<float>(title_background.height)},
            Rectangle{0.0F, 0.0F, 960.0F, 540.0F}, Vector2{0.0F, 0.0F}, 0.0F, WHITE);
    } else {
        ClearBackground(Color{37, 50, 57, 255});
    }

    DrawRectangleRec(scaled_rect(Rectangle{150, 54, 340, 112}), Color{37, 50, 57, 205});
    DrawRectangleLinesEx(scaled_rect(Rectangle{150, 54, 340, 112}), 3.0F,
                         Color{255, 224, 154, 230});
    text(font, "像素小镇", 210, 76, 36, RAYWHITE);
    text(font, "十日经营计划", 220, 124, 22, Color{255, 224, 154, 255});
    if (state.resume_available && !state.confirm_new_game_overwrite) {
        const Rectangle continue_button = title_continue_button();
        panel(continue_button, hovered(continue_button, mouse) ? cream : green);
        centered_text(font, "继续游戏", continue_button, 18,
                      hovered(continue_button, mouse) ? ink : RAYWHITE);
    }
    const Rectangle start_button = title_new_game_button(state.resume_available);
    panel(start_button, hovered(start_button, mouse) ? cream : Color{65, 91, 89, 245});
    centered_text(font, "新游戏", start_button, 18,
                  hovered(start_button, mouse) ? ink : RAYWHITE);

    DrawRectangleRec(scaled_rect(Rectangle{74, 282, 492, 22}), Color{37, 50, 57, 185});
    text(font, state.notice, 92, 286, 14, Color{255, 224, 154, 255});
    DrawRectangleRec(scaled_rect(Rectangle{74, 300, 492, 56}), Color{37, 50, 57, 200});
    text_block(font, opening_story(), 92, 306, 13, 14, Color{235, 241, 226, 255});
}

void draw_tiled_grass(const Texture2D& tiles, Rectangle bounds) {
    if (tiles.id == 0) {
        DrawRectangleRec(bounds, grass);
        return;
    }
    for (float y = bounds.y; y < bounds.y + bounds.height; y += 16.0F) {
        for (float x = bounds.x; x < bounds.x + bounds.width; x += 16.0F) {
            draw_tile(tiles, 0, Rectangle{x, y, 16.0F, 16.0F});
        }
    }
}

void draw_road_tiles(const Texture2D& tiles, Rectangle bounds) {
    DrawRectangleRec(scaled_rect(bounds), road);
    if (tiles.id == 0) {
        return;
    }
    for (float y = bounds.y; y < bounds.y + bounds.height; y += 16.0F) {
        for (float x = bounds.x; x < bounds.x + bounds.width; x += 16.0F) {
            draw_tile(tiles, 48, Rectangle{x, y, 16.0F, 16.0F});
        }
    }
}

void draw_map_decoration(const Texture2D& marker, const Texture2D& tiles) {
    draw_road_tiles(tiles, Rectangle{78, 178, 498, 18});
    draw_road_tiles(tiles, Rectangle{132, 126, 18, 138});
    draw_road_tiles(tiles, Rectangle{244, 126, 18, 66});
    draw_road_tiles(tiles, Rectangle{386, 142, 18, 52});
    draw_road_tiles(tiles, Rectangle{528, 194, 18, 60});
    for (int x = 44; x < 594; x += 42) {
        draw_tile(tiles, 4, Rectangle{static_cast<float>(x), 78.0F, 16.0F, 16.0F});
        draw_tile(tiles, 5, Rectangle{static_cast<float>(x + 16), 272.0F, 16.0F, 16.0F});
    }
    DrawTextureEx(marker, scaled_point(Vector2{306, 184}), 0.0F, 3.0F, WHITE);
}

void draw_location_building(const Font& font, const Texture2D& tiles,
                            const Texture2D& generated_buildings, Rectangle bounds,
                            Location location, Color color, bool allowed, bool hovered,
                            int icon_tile, std::size_t index) {
    if (generated_buildings.id != 0) {
        const Rectangle sprite_destination = generated_sprite_destination(location);
        DrawTexturePro(generated_buildings, generated_building_source(index),
                       scaled_rect(sprite_destination), Vector2{0.0F, 0.0F}, 0.0F,
                       Fade(WHITE, allowed ? 1.0F : 0.45F));
        const Rectangle label = generated_label_destination(location);
        DrawRectangleRec(scaled_rect(Rectangle{label.x + 2, label.y + 2, label.width,
                                               label.height}),
                         shadow);
        DrawRectangleRec(scaled_rect(label),
                         allowed ? Color{250, 238, 203, 240} : Color{218, 213, 194, 248});
        DrawRectangleLinesEx(scaled_rect(label), hovered ? 4.0F : 3.0F, hovered ? cream : ink);
        centered_text(font, location_label(location), Rectangle{label.x, label.y + 1, label.width,
                                                                16},
                      16, allowed ? ink : Color{43, 50, 48, 255});
        centered_text(font, allowed ? "开放" : "未开放",
                      Rectangle{label.x, label.y + 16, label.width, 14}, 14,
                      allowed ? Color{35, 83, 51, 255} : Color{55, 62, 60, 255});
        return;
    }

    const Color fill = allowed ? (hovered ? cream : color) : disabled;
    DrawTriangle(scaled_point(Vector2{bounds.x - 6, bounds.y + 8}),
                 scaled_point(Vector2{bounds.x + bounds.width + 6, bounds.y + 8}),
                 scaled_point(Vector2{bounds.x + bounds.width / 2.0F, bounds.y - 22}), red);
    panel(bounds, fill);
    draw_tile(tiles, icon_tile, Rectangle{bounds.x + 10, bounds.y + 11, 32.0F, 32.0F});
    text(font, location_label(location), bounds.x + 48, bounds.y + 12, 20,
         allowed ? ink : Color{78, 78, 72, 255});
    text(font, allowed ? "开放" : "未开放", bounds.x + 48, bounds.y + 40, 18,
         allowed ? Color{35, 83, 51, 255} : Color{78, 78, 72, 255});
}

void draw_location_label(const Font& font, Location location, bool allowed, bool hovered) {
    const Rectangle label = generated_label_destination(location);
    DrawRectangleRec(scaled_rect(Rectangle{label.x + 2, label.y + 2, label.width, label.height}),
                     shadow);
    DrawRectangleRec(scaled_rect(label),
                     allowed ? Color{250, 238, 203, 238} : Color{218, 213, 194, 248});
    DrawRectangleLinesEx(scaled_rect(label), hovered ? 4.0F : 3.0F, hovered ? cream : ink);
    centered_text(font, location_label(location),
                  Rectangle{label.x, label.y + 1, label.width, 16}, 16,
                  allowed ? ink : Color{43, 50, 48, 255});
    centered_text(font, allowed ? "开放" : "未开放",
                  Rectangle{label.x, label.y + 16, label.width, 14}, 14,
                  allowed ? Color{35, 83, 51, 255} : Color{55, 62, 60, 255});
}

void draw_home_plot_decoration() {
    DrawRectangleRec(scaled_rect(Rectangle{366, 242, 58, 8}), Color{117, 83, 55, 255});
    DrawRectangleRec(scaled_rect(Rectangle{368, 240, 54, 4}), Color{166, 117, 72, 255});
    DrawRectangleRec(scaled_rect(Rectangle{376, 218, 8, 24}), Color{102, 78, 52, 255});
    DrawCircleV(scaled_point(Vector2{380, 216}), scaled(12.0F), Color{74, 139, 78, 255});
    DrawCircleV(scaled_point(Vector2{378, 214}), scaled(7.0F), Color{107, 168, 89, 255});
    DrawRectangleRec(scaled_rect(Rectangle{404, 220, 6, 22}), Color{102, 78, 52, 255});
    DrawCircleV(scaled_point(Vector2{407, 218}), scaled(9.0F), Color{74, 139, 78, 255});
    DrawCircleV(scaled_point(Vector2{405, 216}), scaled(5.0F), Color{107, 168, 89, 255});
    DrawRectangleRec(scaled_rect(Rectangle{390, 250, 22, 12}), Color{151, 104, 63, 255});
    DrawRectangleLinesEx(scaled_rect(Rectangle{390, 250, 22, 12}), 3.0F,
                         Color{91, 65, 45, 255});
    DrawCircleV(scaled_point(Vector2{374, 260}), scaled(3.0F), Color{255, 224, 154, 255});
    DrawCircleV(scaled_point(Vector2{418, 258}), scaled(3.0F), Color{255, 205, 214, 255});
}

void draw_restaurant_background() {
    ClearBackground(Color{76, 64, 55, 255});
    DrawRectangleRec(scaled_rect(Rectangle{0, 84, 960, 178}), Color{151, 93, 59, 255});
    DrawRectangleRec(scaled_rect(Rectangle{0, 262, 960, 278}), Color{92, 78, 63, 255});
    DrawRectangleRec(scaled_rect(Rectangle{48, 112, 230, 82}), Color{84, 61, 47, 255});
    DrawRectangleRec(scaled_rect(Rectangle{64, 126, 198, 16}), Color{196, 146, 84, 255});
    DrawRectangleRec(scaled_rect(Rectangle{64, 150, 198, 16}), Color{196, 146, 84, 255});
    DrawRectangleRec(scaled_rect(Rectangle{64, 174, 198, 16}), Color{196, 146, 84, 255});
    DrawRectangleRec(scaled_rect(Rectangle{690, 110, 170, 94}), Color{62, 75, 78, 255});
    DrawRectangleRec(scaled_rect(Rectangle{706, 126, 138, 62}), Color{118, 150, 154, 255});
    DrawRectangleRec(scaled_rect(Rectangle{0, 430, 960, 70}), Color{129, 86, 55, 255});
    DrawRectangleRec(scaled_rect(Rectangle{0, 498, 960, 42}), Color{86, 58, 42, 255});
}

void draw_food_icon(Dish dish, Rectangle bounds) {
    const float cx = bounds.x + 18.0F;
    const float cy = bounds.y + 18.0F;
    DrawCircleV(scaled_point(Vector2{cx, cy}), scaled(13.0F), Color{245, 238, 203, 255});
    DrawCircleV(scaled_point(Vector2{cx, cy}), scaled(10.0F), Color{230, 194, 117, 255});
    switch (dish) {
        case Dish::fried_rice:
            DrawCircleV(scaled_point(Vector2{cx - 4.0F, cy - 2.0F}), scaled(2.0F), green);
            DrawCircleV(scaled_point(Vector2{cx + 4.0F, cy + 2.0F}), scaled(2.0F), red);
            break;
        case Dish::noodles:
            DrawLineEx(scaled_point(Vector2{cx - 7.0F, cy - 2.0F}),
                       scaled_point(Vector2{cx + 7.0F, cy - 2.0F}), scaled(2.0F), cream);
            DrawLineEx(scaled_point(Vector2{cx - 5.0F, cy + 3.0F}),
                       scaled_point(Vector2{cx + 6.0F, cy + 3.0F}), scaled(2.0F), cream);
            break;
        case Dish::soup:
            DrawCircleV(scaled_point(Vector2{cx, cy}), scaled(8.0F), Color{207, 91, 67, 255});
            DrawCircleV(scaled_point(Vector2{cx + 5.0F, cy - 3.0F}), scaled(2.0F), grass);
            break;
        case Dish::dumplings:
            for (int i = 0; i < 3; ++i) {
                DrawCircleV(scaled_point(Vector2{cx - 6.0F + i * 6.0F, cy + 1.0F}),
                            scaled(4.0F), Color{250, 248, 230, 255});
            }
            break;
        case Dish::salad:
            DrawCircleV(scaled_point(Vector2{cx - 3.0F, cy}), scaled(5.0F), green);
            DrawCircleV(scaled_point(Vector2{cx + 4.0F, cy - 2.0F}), scaled(5.0F), grass);
            DrawCircleV(scaled_point(Vector2{cx + 1.0F, cy + 5.0F}), scaled(3.0F), red);
            break;
        case Dish::dish_count:
            break;
    }
}

void draw_restaurant_timer(Rectangle bounds, float ratio, bool warning) {
    DrawRectangleRec(scaled_rect(bounds), Color{80, 72, 63, 255});
    const Rectangle fill{bounds.x + 3.0F, bounds.y + 3.0F,
                         (bounds.width - 6.0F) * ratio, bounds.height - 6.0F};
    DrawRectangleRec(scaled_rect(fill), warning ? red : green);
    DrawRectangleLinesEx(scaled_rect(bounds), 3.0F, ink);
}

void draw_map(const Font& font, const Texture2D& marker, const Texture2D& tiles,
              const Texture2D& generated_full_map_scene,
              const Texture2D& generated_map_background, const Texture2D& generated_buildings,
              const GameAppState& state, bool audio_enabled, Vector2 mouse) {
    ClearBackground(Color{221, 211, 174, 255});
    const bool has_full_scene = generated_full_map_scene.id != 0;
    if (has_full_scene) {
        DrawTexturePro(
            generated_full_map_scene,
            Rectangle{0.0F, 0.0F, static_cast<float>(generated_full_map_scene.width),
                      static_cast<float>(generated_full_map_scene.height)},
            Rectangle{0.0F, 0.0F, 960.0F, 540.0F}, Vector2{0.0F, 0.0F}, 0.0F, WHITE);
    } else if (generated_map_background.id != 0) {
        DrawTexturePro(
            generated_map_background,
            Rectangle{0.0F, 0.0F, static_cast<float>(generated_map_background.width),
                      static_cast<float>(generated_map_background.height)},
            Rectangle{0.0F, 0.0F, 960.0F, 540.0F}, Vector2{0.0F, 0.0F}, 0.0F, WHITE);
    } else {
        panel(Rectangle{18, 66, 604, 224}, paper);
        draw_tiled_grass(tiles, Rectangle{28, 76, 584, 204});
        draw_map_decoration(marker, tiles);
    }
    draw_status(font, state.session, audio_enabled);
    if (!has_full_scene && generated_map_background.id != 0) {
        draw_home_plot_decoration();
    }

    const auto bounds = location_bounds();
    const std::array<Color, 5> colors{Color{231, 151, 103, 255}, gold,
                                      Color{161, 169, 196, 255}, Color{181, 122, 104, 255},
                                      Color{122, 176, 122, 255}};
    const std::array<int, 5> icons{84, 85, 96, 86, 72};
    for (std::size_t index = 0; index < map_locations.size(); ++index) {
        const auto permission = state.session.can_enter(map_locations[index]);
        const Rectangle button = bounds[index];
        const bool hovered = CheckCollisionPointRec(mouse, scaled_rect(button));
        if (has_full_scene) {
            draw_location_label(font, map_locations[index], permission.allowed, hovered);
        } else {
            draw_location_building(font, tiles, generated_buildings, button, map_locations[index],
                                   colors[index], permission.allowed, hovered, icons[index], index);
        }
    }

    panel(Rectangle{28, 300, 584, 42}, Color{65, 91, 89, 245});
    const auto context = state.session.current_day_context();
    text(font, std::string{"今日提示："} + daily_prompt(context.day), 42, 304, 18,
         Color{255, 224, 154, 255});
    text(font, state.notice, 42, 324, 18, RAYWHITE);
}

void draw_restaurant_ui(const Font& font, const GameAppState& state, bool audio_enabled,
                        Vector2 mouse) {
    draw_restaurant_background();
    draw_status(font, state.session, audio_enabled);
    const auto& rest = *state.locations.restaurant;

    panel(Rectangle{34, 96, 592, 250}, Color{255, 248, 226, 244});
    text(font, "餐馆", 56, 98, 24, red);
    text(font, "午餐高峰", 132, 104, 16, Color{78, 78, 72, 255});

    // X button
    const Rectangle close_btn{568, 94, 28, 28};
    DrawRectangleRec(scaled_rect(close_btn),
                     CheckCollisionPointRec(mouse, scaled_rect(close_btn)) ? red : Color{183, 83, 72, 255});
    centered_text(font, "X", close_btn, 18, RAYWHITE);

    if (!state.session.location_started()) {
        text(font, "餐馆工作准备", 56, 132, 20, ink);
        const auto lines = restaurant_tutorial_lines();
        float y = 154.0F;
        for (const auto& line : lines) {
            text(font, line, 62, y, 14, ink);
            y += 21.0F;
        }
        text(font, "现在返回地图不会消耗白天行动。", 62, 286, 14, Color{78, 78, 72, 255});
        const Rectangle back_btn = restaurant_prepare_back_button();
        const Rectangle start_btn = restaurant_prepare_start_button();
        panel(back_btn, hovered(back_btn, mouse) ? paper : Color{211, 202, 174, 255});
        centered_text(font, "返回地图", back_btn, 16, ink);
        panel(start_btn, hovered(start_btn, mouse) ? paper : green);
        centered_text(font, "开始工作", start_btn, 16, RAYWHITE);
        return;
    }

    if (rest.phase() == RestaurantPhase::showing_instructions) {
        text(font, "开始接待顾客前，再确认一次规则", 56, 132, 20, ink);
        const auto lines = restaurant_tutorial_lines();
        float y = 154.0F;
        for (const auto& line : lines) {
            text(font, line, 62, y, 14, ink);
            y += 21.0F;
        }
        const Rectangle start_btn = restaurant_instructions_start_button();
        panel(start_btn, CheckCollisionPointRec(mouse, scaled_rect(start_btn)) ? paper : green);
        centered_text(font, "开始接待", start_btn, 18, RAYWHITE);

    } else if (rest.phase() == RestaurantPhase::waiting_for_order ||
               rest.phase() == RestaurantPhase::order_feedback) {
        const auto ticket = restaurant_order_ticket(rest);
        const auto* order = rest.current_order();
        if (order) {
            panel(Rectangle{58, 134, 240, 96}, Color{250, 238, 203, 255});
            text(font, "订单票据", 76, 146, 16, Color{78, 78, 72, 255});
            text(font, ticket.order_progress, 196, 146, 16, ink);
            text(font, "顾客想要", 76, 174, 18, ink);
            text(font, ticket.dish_name, 190, 170, 24, red);
            text(font, ticket.time_label, 76, 204, 16, ticket.time_warning ? red : ink);
            draw_restaurant_timer(Rectangle{178, 205, 100, 12}, ticket.time_ratio,
                                  ticket.time_warning);
            if (rest.phase() == RestaurantPhase::order_feedback) {
                panel(Rectangle{320, 146, 246, 64},
                      rest.last_feedback() == RestaurantFeedback::correct
                          ? Color{230, 245, 230, 255}
                          : Color{245, 230, 220, 255});
                text(font, restaurant_feedback_text(rest), 338, 168, 16,
                     rest.last_feedback() == RestaurantFeedback::correct ? green : red);
                text(font, "按 Space 继续下一位顾客", 338, 190, 14, Color{78, 78, 72, 255});
            }
        }
        if (rest.phase() == RestaurantPhase::waiting_for_order) {
            text(font, "菜品选择：按 1-5 或点击；按 I 重看说明", 58, 248, 16, ink);
            for (int i = 0; i < dish_count(); ++i) {
                const Rectangle btn = restaurant_dish_button(i);
                panel(btn, CheckCollisionPointRec(mouse, scaled_rect(btn)) ? cream
                                                                           : Color{211, 202, 174, 255});
                draw_food_icon(static_cast<Dish>(i), btn);
                const std::string label =
                    std::to_string(i + 1) + "." + dish_label(static_cast<Dish>(i));
                text(font, label, btn.x + 38, btn.y + 14, 15, ink);
            }
        }
        const auto& stats = rest.stats();
        const Rectangle stats_panel = restaurant_stats_panel();
        panel(stats_panel, Color{65, 91, 89, 245});
        text(font, std::string("正确 ") + std::to_string(stats.correct) +
                 "  错单 " + std::to_string(stats.wrong) +
                 "  超时 " + std::to_string(stats.timeout) +
                 "  剩余 " + std::to_string(rest.orders_remaining()),
             76, 328, 14, RAYWHITE);
    } else if (rest.phase() == RestaurantPhase::finished) {
        const auto summary = restaurant_completion_summary(rest, state.session.active_result_id());
        text(font, "餐馆工作完成", 56, 136, 22, ink);
        text(font, summary.stats_line, 56, 168, 18, ink);
        text(font, summary.accuracy_line, 56, 198, 18, Color{35, 83, 51, 255});
        text(font, summary.delta_line, 56, 228, 16, ink);
        text(font, "点击完成结算后进入夜晚选择。", 56, 258, 15, Color{78, 78, 72, 255});
        const Rectangle done_btn{232, 300, 176, 28};
        panel(done_btn, CheckCollisionPointRec(mouse, scaled_rect(done_btn)) ? paper : green);
        centered_text(font, "完成结算", done_btn, 16, RAYWHITE);
    }

    if (rest.phase() != RestaurantPhase::finished) {
        const Rectangle abandon_btn = restaurant_abandon_button();
        panel(abandon_btn, hovered(abandon_btn, mouse) ? paper : red);
        centered_text(font, "主动放弃", abandon_btn, 15, RAYWHITE);
    }
}


void draw_location(const Font& font, const GameAppState& state, bool audio_enabled,
                   Vector2 mouse) {
    if (state.session.pending_location() == Location::restaurant && state.locations.restaurant) {
        draw_restaurant_ui(font, state, audio_enabled, mouse);
        return;
    }
    const Location location = state.session.pending_location();
    const bool is_tavern = state.session.phase() == GamePhase::night_location &&
                           location == Location::tavern;

    if (is_tavern) {
        draw_tavern_view(font, state.locations.tavern.presentation(),
                         state.locations.tavern_assets, mouse);
        draw_status(font, state.session, audio_enabled);
        return;
    }

    ClearBackground(Color{215, 221, 194, 255});
    draw_status(font, state.session, audio_enabled);
    if (location == Location::convenience_store) {
        panel(Rectangle{72, 70, 496, 232}, cream);
        text(font, location_label(location), 96, 82, 28, red);
        const auto context = state.session.current_day_context();
        text(font, std::string{"今日提示："} + context.weather + " / " + context.event,
             96, 116, 15, ink);

        text(font, "商品", 96, 142, 14, Color{78, 78, 72, 255});
        text(font, "库存", 202, 142, 14, Color{78, 78, 72, 255});
        text(font, "进货", 260, 142, 14, Color{78, 78, 72, 255});
        text(font, "价格档", 324, 142, 14, Color{78, 78, 72, 255});
        text(font, "售价", 410, 142, 14, Color{78, 78, 72, 255});

        const auto config = store::default_store_config();
        int total_cost = 0;
        for (std::size_t index = 0; index < config.products.size(); ++index) {
            const auto& product = config.products[index];
            const float y = 164.0F + static_cast<float>(index) * 22.0F;
            const bool selected =
                static_cast<int>(index) == state.locations.store_selected_product_index;
            if (selected) {
                DrawRectangleRec(scaled_rect(Rectangle{88, y - 3.0F, 410, 20}),
                                 Color{255, 224, 154, 110});
            }
            const int purchase = store_plan_quantity(state.locations.store_purchase_plan,
                                                     product.id);
            const auto tier = store_plan_tier(state.locations.store_price_plan, product.id);
            total_cost += product.unit_cost * purchase;
            text(font, selected ? ">" : "", 88, y, 14, red);
            text(font, product.name, 104, y, 14, ink);
            text(font, std::to_string(store_inventory_quantity(state.session, product.id)),
                 210, y, 14, ink);
            text(font, std::to_string(purchase), 272, y, 14, ink);
            text(font, store_price_tier_label(tier), 326, y, 14, ink);
            text(font, std::to_string(store::price_for_tier(product, tier)), 418, y, 14, ink);
        }

        const int balance_after = state.session.player().money - total_cost;
        text(font, std::string{"进货成本 "} + std::to_string(total_cost) +
                       " / 剩余金钱 " + std::to_string(balance_after),
             96, 258, 14, balance_after >= 0 ? Color{35, 83, 51, 255} : red);
        text(font, "按1-4选商品  A/D调进货  Q/W/E调价格  空格开始/完成",
             96, 278, 13, Color{78, 78, 72, 255});
    } else {
        panel(Rectangle{96, 78, 448, 210}, cream);
        text(font, location_label(location), 126, 106, 30, red);
        text(font,
             state.session.location_started() ? "地点已开始：完成模拟或主动放弃都会消耗本阶段。"
                                              : "尚未开始：现在返回地图不会消耗本阶段。",
             126, 154, 16, ink);
    }

    const bool is_store = location == Location::convenience_store;
    const Rectangle back_button = is_store ? store_back_button()
                                           : Rectangle{126, 228, 112, 34};
    const Rectangle start_button = is_store ? store_start_button()
                                            : Rectangle{264, 228, 112, 34};
    const Rectangle abandon_button = is_store ? store_abandon_button()
                                              : Rectangle{402, 228, 112, 34};
    if (!state.session.location_started()) {
        panel(back_button, hovered(back_button, mouse) ? paper : Color{211, 202, 174, 255});
        centered_text(font, "返回地图", back_button, 16, ink);
        panel(start_button, hovered(start_button, mouse) ? paper : green);
        centered_text(font, "开始模拟", start_button, 16, RAYWHITE);
    } else {
        panel(start_button, hovered(start_button, mouse) ? paper : green);
        centered_text(font, "完成模拟", start_button, 16, RAYWHITE);
        panel(abandon_button, hovered(abandon_button, mouse) ? paper : red);
        centered_text(font, "主动放弃", abandon_button, 16, RAYWHITE);
    }
}

void draw_summary(const Font& font, const GameAppState& state, bool audio_enabled,
                  Vector2 mouse) {
    ClearBackground(Color{37, 50, 57, 255});
    draw_status(font, state.session, audio_enabled);
    panel(Rectangle{90, 88, 460, 188}, cream);
    text(font, "每日总结", 124, 116, 28, red);
    text_block(font, state.session.last_summary(), 124, 154, 14, 16, ink);
    text(font, day_closing_summary(state.session.day()), 124, 194, 14, Color{78, 78, 72, 255});
    text(font, state.session.day() == 10 ? "确认后进入占位主结局。"
                                         : "确认后进入下一游戏日。",
         124, 214, 16, ink);
    const Rectangle next_button{242, 224, 156, 34};
    panel(next_button, hovered(next_button, mouse) ? paper : green);
    centered_text(font, "继续到下一天", next_button, 16, RAYWHITE);
}

void draw_ending(const Font& font, const GameAppState& state, bool audio_enabled) {
    ClearBackground(Color{37, 50, 57, 255});
    draw_status(font, state.session, audio_enabled);
    panel(Rectangle{72, 82, 496, 276}, cream);
    const auto& player = state.session.player();
    text(font, "十日计划完成", 118, 112, 28, red);
    text(font, std::string{"主结局："} + state.session.main_ending(), 118, 154, 20, ink);
    text_block(font, state.session.final_summary(), 118, 188, 13, 15, ink);
    text(font, "最终状态", 118, 282, 18, Color{35, 83, 51, 255});
    text(font,
         std::string{"金钱 "} + std::to_string(player.money) + "  体力 " +
             std::to_string(player.stamina) + "  声望 " + std::to_string(player.reputation),
         118, 306, 16, ink);
    text(font,
         std::string{"知识 "} + std::to_string(player.knowledge) + "  心情 " +
             std::to_string(player.mood) + "  成长路线：均衡体验",
         118, 330, 16, ink);
}

void draw_pause_overlay(const Font& font, bool audio_enabled) {
    DrawRectangle(0, 0, 960, 540, Color{20, 24, 28, 150});
    panel(Rectangle{194, 110, 252, 126}, Color{250, 238, 203, 245});
    text(font, "已暂停", 284, 132, 28, red);
    text(font, "按 P 继续", 258, 176, 18, ink);
    text(font, audio_enabled ? "按 M 切换静音" : "按 M 恢复声音", 232, 202, 18,
         Color{78, 78, 72, 255});
}

}  // namespace

const char* game_flow_glyphs() {
    static const std::string glyphs = [] {
        std::string result =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789. "
            "·/：，。；“”‘’+-！？《》【】、———（）「」『』℃°";
        for (const char* value : ui_texts) {
            result += value;
        }
        result +=
            "点击开始第一天不可进入尚未地点已开始完成模拟或主动放弃都会消耗本阶段现在返回地图不会"
            "消耗本阶段今天的白天行动已经完成现在是白天回家休息只能在夜晚选择酒馆夜晚开放当前"
            "阶段不能进入今晚的行动已经完成白天工作"
            "已经结束不能再进入该地点当前正在处理另一个阶段不能选择新地点餐馆模拟工作完成服务了"
            "午餐客流获得金钱与声望便利店模拟经营完成一次进货与销售结算图书馆帮助读者找书并提升"
            "便利店经营完成进货成本销售收入利润雨伞汽水便当数量都记进了账本店主在账本边角画了个"
            "小勾明天还能再调价格今日提示价格档低价标准价高价当前库存每种商品按选择调整无"
            "知识行动完成回家休息恢复体力并结束今天主动放弃阶段已消耗本次无收益确认后进入下一游戏日"
            "占位主结局最终状态成长路线摘要均衡体验小镇生活十日经营计划已经结束不能继续选择地点"
            "点击地点查看原因成长路线均衡体验已暂停按P继续按M切换静音恢复声音"
            "演示参数错误预设加载失败已加载正式存档不会被读取或覆盖"
            "炒饭面条汤饺子沙拉正确错单超时餐馆工作完成准备根据顾客订单按至菜品选择上想要等待秒"
            "剩余结算"
            "五子棋骗子骰子低赌注中赌注高赌注挑战金币选择空格开始完成Esc返回"
            "已进入酒馆选择和赌注金钱不足无法选择该档位"
            "赌注不足当前金钱不足以支付获胜赢得失败损失平局退还酒馆挑战"
            "已选择按空格完成模拟！（，）"
            "历史科学文学艺术技术地理生物人类登月光速红楼梦蒙娜丽莎计算机海洋DNA四大发明牛顿莎士"
            "比亚梵高互联网赤道光合作用第二次世界大战回答正确回答错误正确答案继续答题结算声望"
            "欢迎来到小镇图书馆读者会提出各种问题你需要从书架上找到正确的书籍类别来回答答对可获"
            "得知识和声望奖励连续答对还有额外奖励开始工作选择书籍类别按ESC放弃工作正确答案是继续下"
            "一题图书馆工作完成体力金钱知识声望心情按任意键继续公里每秒作者是谁作品基本组成部分最大"
            "全称四大发明运动定律包含多少部戏剧最著名的前身叫什么名字周长大约是主要发生在部位结束于"
            "借书卡盒子敲掉窗台晒摊位标记盖章指空白翻递街边缘旧集市地图管理员今年事写上去孩子考试"
            "人类第一次登月是在哪一年这是1969年的历史性事件光速大约是多少公里每秒大约每秒30万公里"
            "离开了图书馆什么也没做在图书馆工作了一天答对答错最高连续答对题获得金币表现出色表现不错"
            "还需要继续努力帮孩子找到了考试要用的书又多盖了一个章旧地图递给你说可以先看看不急着还"
            "书页翻动的声音很轻窗外的街也慢下来盒子里多了几张新记录收回去前指了指边上的空白处"
            "说以后可以把今年的事也写上去递给你一杯热茶梧桐树影落在书页上今天的图书馆格外安静"
            "管理员说点击对话像素小镇科学类历史类文学类艺术类技术类地理类生物类人类类"
            "红楼梦的作者是谁清代作家曹雪芹蒙娜丽莎是谁的作品文艺复兴时期的达芬奇计算机的基本组成部分有哪些"
            "CPU内存硬盘等世界上最大的海洋是什么太平洋是最大的海洋DNA的全称是什么脱氧核糖核酸中国的四大发明是什么"
            "造纸术印刷术火药指南针牛顿提出了哪些运动定律三大运动定律莎士比亚全集包含多少部戏剧大约37部"
            "梵高最著名的作品是什么向日葵星空等互联网的前身叫什么名字ARPANET地球的赤道周长大约是多少公里"
            "约4万公里光合作用主要发生在植物的哪个部位叶绿体第二次世界大战结束于哪一年1945年中国古代的科举制度始于哪个朝代"
            "隋朝水的化学式是什么H2O西游记中的唐僧师徒共有几人师徒四人中国的国画四君子是指什么梅兰竹菊"
            "第一台电子计算机叫什么名字ENIAC世界上最长的河流是什么尼罗河人体最大的器官是什么皮肤法国大革命发生在哪一年"
            "1789年地球围绕太阳公转一周需要多长时间一年三国演义的作者是谁罗贯中"
            "借书卡盒子有点卡管理员敲了两下掉出一张旧集市地图先看看吧书架按类别分的考试的孩子常来找历史和科学类"
            "你帮他们找找书就行窗台晒着借书卡今天适合把书页翻慢一点你想看那本旧的吗以前这里很热闹的地图边缘有摊位标记"
            "你可以先看看借书卡盒子里多了几张新记录管理员把旧地图收回去前指了指边上的空白处以后可以把今年的事也写上去"
            "你帮孩子找到了考试要用的书借书卡又多盖了一个章书页翻动的声音很轻窗外的街也慢下来"
            "开始工作继续收下地图查看提示隐藏提示返回地图"
            "秦始皇统一六国公元前221年原子中心原子核水浒传108位好汉清明上河图张择端"
            "诞生撒哈拉沙漠206块骨头长城修建春秋战国自转聊斋志异蒲松龄达·芬奇"
            "擅长科学发明智能手机2007年珠穆朗玛峰肝脏孔子春秋时期100度"
            "儒林外史吴敬梓书法楷行草隶篆人工智能英文缩写北京阳光水二氧化碳"
            "原子核秦始皇撒哈拉蒲松龄达·芬奇穆朗玛肝脏吴敬梓楷行草隶篆二氧化碳"
            "读者问达·芬奇除了绘画还擅长什么回答正确达·芬奇还是科学家和发明家"
            "回答错误达·芬奇还擅长科学发明";
        result += story_text_glyphs();
        result += tavern_ui_glyphs();
        return result;
    }();
    return glyphs.c_str();
}

void update_game_flow(GameAppState& state, Vector2 logical_mouse) {
    if (!state.has_session) {
        const Rectangle continue_button = title_continue_button();
        const Rectangle start_button = title_new_game_button(state.resume_available);
        if (state.confirm_new_game_overwrite && IsKeyPressed(KEY_ESCAPE)) {
            state.confirm_new_game_overwrite = false;
            state.notice = "已取消新游戏，原存档保持不变。";
            return;
        }
        const bool continue_requested =
            state.resume_available && !state.confirm_new_game_overwrite &&
            (clicked(continue_button, logical_mouse) || IsKeyPressed(KEY_ENTER));
        if (continue_requested) {
            state.has_session = true;
            state.session = state.resume_session;
            state.confirm_new_game_overwrite = false;
            state.notice = "已恢复最近的阶段边界。";
            return;
        }
        const bool new_game_requested =
            clicked(start_button, logical_mouse) ||
            (!state.resume_available && IsKeyPressed(KEY_ENTER)) ||
            (state.confirm_new_game_overwrite && IsKeyPressed(KEY_ENTER));
        if (new_game_requested) {
            if (state.save_present && !state.confirm_new_game_overwrite) {
                state.confirm_new_game_overwrite = true;
                state.notice = "检测到已有存档，再次点击新游戏将覆盖；Esc 取消。";
                return;
            }
            state.has_session = true;
            state.session = GameSession::new_game();
            state.save_present = true;
            state.resume_available = false;
            state.confirm_new_game_overwrite = false;
            state.notice = "第 1 天开始：请选择一个白天工作地点。";
        }
        return;
    }

    if (update_active_library(state.session, state.locations, state.notice, logical_mouse)) {
        return;
    }

    if (state.session.phase() == GamePhase::day_choice ||
        state.session.phase() == GamePhase::night_choice) {
        const auto bounds = location_bounds();
        for (std::size_t index = 0; index < map_locations.size(); ++index) {
            if (!clicked(bounds[index], logical_mouse)) {
                continue;
            }
            const Location location = map_locations[index];
            const auto permission = state.session.can_enter(location);
            if (!permission.allowed) {
                state.notice = permission.reason;
                return;
            }
            if (location == Location::home) {
                const auto result = state.session.home_rest_result();
                const auto applied = state.session.apply_action_result(result);
                state.notice = applied.message;
                return;
            }
            if (location == Location::tavern) {
                ensure_tavern_assets_loaded(state.locations.tavern_assets);
                const auto opened = state.locations.tavern.open(state.session);
                state.notice = opened.message;
                return;
            }
            if (state.session.enter_location(location)) {
                if (location == Location::restaurant) {
                    prepare_restaurant_runtime(state.locations,
                                               state.session.location_seed(Location::restaurant));
                    state.notice = "已进入餐馆";
                } else if (location == Location::convenience_store) {
                    prepare_store_runtime(state.locations);
                    state.notice = "已进入便利店，选择每种商品的进货数量和价格档。";
                } else {
                    state.notice =
                        std::string{"已进入"} + location_label(location) + "，开始前可返回地图。";
                }
            }
            return;
        }
        return;
    }

    if (state.session.phase() == GamePhase::day_location ||
        state.session.phase() == GamePhase::night_location) {
        const bool is_tavern = state.session.phase() == GamePhase::night_location &&
                               state.session.pending_location() == Location::tavern;

        if (is_tavern) {
            const auto result = state.locations.tavern.step(
                state.session, tavern_frame_input(logical_mouse));
            if (result.notice.has_value()) {
                state.notice = *result.notice;
            }
            return;
        }
        if (state.session.pending_location() == Location::convenience_store &&
            !state.session.location_started()) {
            update_store_selection(state.locations, state.session);
        }

        const bool is_restaurant = state.session.pending_location() == Location::restaurant;
        const bool is_store = state.session.pending_location() == Location::convenience_store;
        const Rectangle back_button =
            is_restaurant ? restaurant_prepare_back_button()
                          : (is_store ? store_back_button() : location_back_button(is_tavern));
        const Rectangle start_button_location =
            is_restaurant ? restaurant_prepare_start_button()
                          : (is_store ? store_start_button() : location_start_button(is_tavern));
        if (!state.session.location_started()) {
            if (activated(back_button, logical_mouse, KEY_ESCAPE)) {
                if (state.session.return_to_map()) {
                    state.notice = "已返回地图：阶段未消耗。";
                }
                return;
            }
            if (activated(start_button_location, logical_mouse, KEY_SPACE)) {
                (void)start_pending_location(state.session, state.locations, state.notice);
                return;
            }
        } else {
            (void)update_started_location(state.session, state.locations, state.notice,
                                          logical_mouse);
        }
        return;
    }

    if (state.session.phase() == GamePhase::day_summary) {
        const Rectangle next_button{242, 224, 156, 34};
        if (activated(next_button, logical_mouse, KEY_ENTER)) {
            if (state.session.finish_day_summary()) {
                state.notice = std::string{"第 "} + std::to_string(state.session.day()) +
                               " 天开始：请选择一个白天工作地点。";
            }
        }
    }
}

void draw_game_flow(const Font& font, const Texture2D& title_background,
                    const Texture2D& town_marker,
                    const Texture2D& kenney_tiles, const Texture2D& generated_full_map_scene,
                    const Texture2D& generated_map_background,
                    const Texture2D& generated_buildings, const GameAppState& state,
                    bool audio_enabled, bool paused, Vector2 logical_mouse) {
    if (!state.has_session) {
        draw_title(font, title_background, state, logical_mouse);
        if (paused) {
            draw_pause_overlay(font, audio_enabled);
        }
        return;
    }

    if (state.locations.in_library && state.locations.library_engine) {
        draw_active_library(font, state.locations, logical_mouse);
        if (!audio_enabled) {
            text(font, "静音", 586, 10, 18, Color{255, 208, 166, 255});
        }
        if (paused) {
            draw_pause_overlay(font, audio_enabled);
        }
        return;
    }

    if (state.session.phase() == GamePhase::day_choice ||
        state.session.phase() == GamePhase::night_choice) {
        draw_map(font, town_marker, kenney_tiles, generated_full_map_scene,
                 generated_map_background, generated_buildings, state, audio_enabled,
                 logical_mouse);
    } else if (state.session.phase() == GamePhase::day_location ||
               state.session.phase() == GamePhase::night_location) {
        draw_location(font, state, audio_enabled, logical_mouse);
    } else if (state.session.phase() == GamePhase::day_summary) {
        draw_summary(font, state, audio_enabled, logical_mouse);
    } else {
        draw_ending(font, state, audio_enabled);
    }
    if (paused) {
        draw_pause_overlay(font, audio_enabled);
    }
}

}  // namespace pixel_town

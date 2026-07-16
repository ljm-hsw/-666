// 全局页面路由和每帧输入分发；地点规则应留在 Runtime/locations 层。
#include "app/game_flow.hpp"

#include "app/tavern_view.hpp"
#include "app/tavern_layout.hpp"

#include <array>
#include <string>

#include "app/restaurant_ui_model.hpp"
#include "app/npc_sprite_view.hpp"
#include "app/ui_primitives.hpp"
#include "core/ending_rules.hpp"
#include "core/location_lobby.hpp"
#include "core/location_story.hpp"
#include "core/scene_collision.hpp"
#include "core/story_text.hpp"
#include "ui/scene_viewport.hpp"

namespace pixel_town {
namespace {

constexpr std::array<Location, 5> map_locations{
    Location::restaurant, Location::convenience_store, Location::library, Location::tavern,
    Location::home};

void draw_scene_navigation(const Font& font,
                           const Texture2D& protagonist_texture,
                           const SceneNavigationPresentation& navigation,
                           bool collision_debug_visible);

Rectangle scene_viewport_rectangle() {
    const auto viewport = ui::indoor_scene_viewport();
    return Rectangle{viewport.x, viewport.y, viewport.width, viewport.height};
}

Rectangle scene_canvas_rectangle(Rectangle bounds) {
    const auto transformed = ui::scene_canvas_to_viewport(
        {bounds.x, bounds.y, bounds.width, bounds.height});
    return Rectangle{transformed.x, transformed.y, transformed.width,
                     transformed.height};
}

Rectangle scene_design_rectangle(Rectangle bounds) {
    const auto transformed = ui::scene_design_to_screen_design(
        {bounds.x, bounds.y, bounds.width, bounds.height});
    return Rectangle{transformed.x, transformed.y, transformed.width,
                     transformed.height};
}

bool canvas_point_in_tavern_rect(Vector2 point, TavernRect bounds) {
    return point.x >= bounds.x * ui::design_to_canvas_scale &&
           point.x <= (bounds.x + bounds.width) * ui::design_to_canvas_scale &&
           point.y >= bounds.y * ui::design_to_canvas_scale &&
           point.y <= (bounds.y + bounds.height) * ui::design_to_canvas_scale;
}

TavernFrameInput tavern_frame_input(Vector2 logical_mouse, TavernScreen screen) {
    TavernFrameInput input;
    input.elapsed_seconds = GetFrameTime();
    Vector2 runtime_pointer = logical_mouse;
    if (screen == TavernScreen::lobby) {
        const TavernLayout layout = tavern_layout();
        const bool over_fixed_button =
            canvas_point_in_tavern_rect(logical_mouse, layout.select_button) ||
            canvas_point_in_tavern_rect(logical_mouse, layout.back_button);
        if (!over_fixed_button) {
            const auto scene_point = ui::viewport_to_scene_canvas(
                {logical_mouse.x, logical_mouse.y});
            runtime_pointer = Vector2{scene_point.x, scene_point.y};
        }
    }
    input.pointer = TavernCanvasPoint{runtime_pointer.x, runtime_pointer.y,
                                      logical_mouse.x >= 0.0F && logical_mouse.y >= 0.0F};
    input.primary_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.escape_pressed = IsKeyPressed(KEY_ESCAPE);
    input.enter_pressed = IsKeyPressed(KEY_ENTER);
    input.space_pressed =
        screen != TavernScreen::lobby && IsKeyPressed(KEY_SPACE);
    for (int digit = 1; digit <= 5; ++digit) {
        if (IsKeyPressed(static_cast<KeyboardKey>(KEY_ONE + digit - 1))) {
            input.digit_pressed = digit;
            break;
        }
    }
    return input;
}

SceneNavigationInput scene_navigation_frame_input() {
    SceneNavigationInput input;
    input.elapsed_seconds = GetFrameTime();
    input.move_left = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
    input.move_right = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
    input.move_up = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
    input.move_down = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
    input.interact_pressed =
        IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE);
    return input;
}

const char* scene_interaction_prompt(Location location,
                                     SceneInteractionKind interaction) {
    switch (interaction) {
        case SceneInteractionKind::npc:
            switch (location) {
                case Location::restaurant:
                    return "E / Space：与餐馆老板交谈";
                case Location::convenience_store:
                    return "E / Space：与便利店店主交谈";
                case Location::library:
                    return "E / Space：与图书馆管理员交谈";
                case Location::tavern:
                    return "E / Space：与酒保交谈";
                case Location::home:
                    return "E / Space：互动";
            }
            break;
        case SceneInteractionKind::primary_activity:
            if (location == Location::tavern) {
                return "E / Space：进入五子棋桌";
            }
            if (location == Location::home) {
                return "E / Space：准备休息";
            }
            return "E / Space：开始地点活动";
        case SceneInteractionKind::secondary_activity:
            return "E / Space：进入骗子骰子桌";
        case SceneInteractionKind::exit:
            return "E / Space：返回小镇地图";
    }
    return "WASD / 方向键移动 · E / Space 互动";
}

std::string scene_navigation_prompt(const SceneNavigationPresentation& view) {
    if (!view.nearby_interaction.has_value()) {
        return "WASD / 方向键移动 · E / Space 互动";
    }
    return scene_interaction_prompt(view.location, *view.nearby_interaction);
}

void apply_tavern_navigation_interaction(SceneInteractionKind interaction,
                                         TavernFrameInput& input) {
    const TavernLayout layout = tavern_layout();
    TavernRect target{};
    switch (interaction) {
        case SceneInteractionKind::npc:
            target = layout.npc_hotspot;
            break;
        case SceneInteractionKind::primary_activity:
            target = layout.gomoku_hotspot;
            break;
        case SceneInteractionKind::secondary_activity:
            target = layout.dice_hotspot;
            break;
        case SceneInteractionKind::exit:
            input.escape_pressed = true;
            return;
    }
    input.pointer = TavernCanvasPoint{
        (target.x + target.width * 0.5F) * ui::design_to_canvas_scale,
        (target.y + target.height * 0.5F) * ui::design_to_canvas_scale, true};
    input.primary_pressed = true;
}

constexpr std::array ui_texts{
    "像素小镇",
    "十日经营计划",
    "五日展示版本",
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
    "五日展示完成",
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
    "今晚要休息吗？",
    "确认休息",
    "F3 显示或隐藏碰撞箱",
    "WASD / 方向键移动 · E / Space 互动",
    "E / Space：与餐馆老板交谈",
    "E / Space：与便利店店主交谈",
    "E / Space：与图书馆管理员交谈",
    "E / Space：与酒保交谈",
    "E / Space：进入五子棋桌",
    "E / Space：进入骗子骰子桌",
    "E / Space：准备休息",
    "E / Space：返回小镇地图",
    "餐馆大厅",
    "便利店大厅",
    "图书馆大厅",
    "餐馆老板",
    "便利店店主",
    "图书馆管理员（预留）",
    "夜间访客",
    "今晚的访客来敲门了。",
    "镇长正在介绍五日展示计划。",
    "进入餐馆工作",
    "开始经营",
    "开始图书馆工作",
    "点击管理员或按 Space 交谈 · Esc 返回地图",
    "图书馆管理员",
    "已进入图书馆：点击柜台管理员开始交谈。",
    "图书馆场景加载失败。",
    "管理员对话结束，请选择今天的图书馆工作。",
    "已开始与管理员交谈。",
    "图书馆室内场景尚未打开。",
    "当前阶段不能开始图书馆工作。",
    "图书馆场景收到非法帧时间。",
    "准备休息",
    "对话接口已预留，后续接入正式 NPC 内容。",
    "诊断：场景大厅与 NPC 预留热点。",
    "已进入大厅；可先查看场景或尝试 NPC 预留互动。",
    "地点大厅配置缺失，已返回地图。",
    "无法进入该地点，请返回地图重试。",
    "已进入餐馆工作准备。",
    "已进入便利店经营：请设置进货数量和价格档。",
    "已进入便利店大厅：点击店主或开始经营按钮开始交谈。",
    "便利店店主对话暂时不可用。",
    "当前阶段不能开始便利店经营。",
    "店主交代完毕，已进入便利店经营准备。",
    "诊断：餐馆老板固定热点。",
    "诊断：餐馆老板主线对话。",
    "诊断：便利店店主固定热点。",
    "诊断：便利店店主主线对话。",
    "镇长正在介绍十日计划。",
    "开场对话暂时不可用。",
    "休息前，主角想和自己说几句话。",
    "回家独白暂时不可用。",
    "诊断：镇长与主角开场对话。",
    "诊断：主角回家独白。",
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

Rectangle home_preview_back_button() {
    return Rectangle{118, 310, 132, 32};
}

Rectangle home_preview_rest_button() {
    return Rectangle{390, 310, 132, 32};
}

Rectangle lobby_rectangle(LobbyRect value) {
    return Rectangle{value.x, value.y, value.width, value.height};
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
    DrawRectangle(0, 0, 960, static_cast<int>(ui::scene_header_height), slate);
    text(font, "像素小镇", 16, 7, 22, RAYWHITE);
    text(font, std::string{"第 "} + std::to_string(session.day()) + " 天 · " +
                   phase_label(session.phase()),
         132, 11, 18, Color{255, 224, 154, 255});
    text(font, std::string{"金钱 "} + std::to_string(player.money), 316, 1, 18, RAYWHITE);
    text(font, std::string{"体力 "} + std::to_string(player.stamina), 410, 1, 18, RAYWHITE);
    text(font, std::string{"声望 "} + std::to_string(player.reputation), 504, 1, 18, RAYWHITE);
    text(font, std::string{"知识 "} + std::to_string(player.knowledge), 316, 20, 18, RAYWHITE);
    text(font, std::string{"心情 "} + std::to_string(player.mood), 410, 20, 18, RAYWHITE);
    if (!audio_enabled) {
        text(font, "静音", 548, 20, 18, Color{255, 208, 166, 255});
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
    text(font, game_plan_subtitle(), 220, 124, 22,
         Color{255, 224, 154, 255});
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

void draw_scene_texture(const Texture2D& texture) {
    DrawRectangle(0, static_cast<int>(ui::scene_header_height), ui::canvas_width,
                  ui::canvas_height - static_cast<int>(ui::scene_header_height),
                  Color{34, 39, 40, 255});
    DrawTexturePro(texture,
                   Rectangle{0.0F, 0.0F, static_cast<float>(texture.width),
                             static_cast<float>(texture.height)},
                   scene_viewport_rectangle(), Vector2{0.0F, 0.0F},
                   0.0F, WHITE);
}

void draw_restaurant_background(const Texture2D& background) {
    if (background.id != 0) {
        draw_scene_texture(background);
        return;
    }
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

    panel(Rectangle{28, 282, 584, 64}, Color{65, 91, 89, 245});
    const auto context = state.session.current_day_context();
    const auto prompt_lines = wrap_text_lines(
        std::string{"今日提示："} + daily_prompt(context.day), 34, 1);
    if (!prompt_lines.empty()) {
        text(font, prompt_lines.front(), 42, 286, 18, Color{255, 224, 154, 255});
    }
    const auto notice_lines = wrap_text_lines(state.notice, 34, 2);
    for (std::size_t index = 0; index < notice_lines.size(); ++index) {
        text(font, notice_lines[index], 42, 306.0F + static_cast<float>(index) * 18.0F,
             17, RAYWHITE);
    }
}

void draw_restaurant_ui(const Font& font, const GameAppState& state, bool audio_enabled,
                        Vector2 mouse, const Texture2D& background) {
    draw_restaurant_background(background);
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
                   Vector2 mouse, const SceneVisualAssets& scene_assets) {
    if (state.session.pending_location() == Location::restaurant && state.locations.restaurant) {
        draw_restaurant_ui(font, state, audio_enabled, mouse,
                           scene_assets.restaurant_interior);
        return;
    }
    const Location location = state.session.pending_location();
    const bool is_tavern = state.session.phase() == GamePhase::night_location &&
                           location == Location::tavern;

    if (is_tavern) {
        const TavernPresentation tavern = state.locations.tavern.presentation();
        draw_tavern_view(font, tavern,
                         state.locations.tavern_assets, mouse);
        if (tavern.screen == TavernScreen::lobby) {
            draw_scene_navigation(
                font, scene_assets.protagonist,
                state.locations.scene_navigation.presentation(),
                state.collision_debug_visible);
        }
        draw_status(font, state.session, audio_enabled);
        return;
    }

    if (location == Location::convenience_store &&
        scene_assets.convenience_store_interior.id != 0) {
        draw_scene_texture(scene_assets.convenience_store_interior);
    } else {
        ClearBackground(Color{215, 221, 194, 255});
    }
    draw_status(font, state.session, audio_enabled);
    if (location == Location::convenience_store) {
        panel(Rectangle{72, 70, 496, 240}, Color{255, 248, 226, 244});
        text(font, location_label(location), 96, 82, 28, red);
        const auto context = state.session.current_day_context();
        text(font, std::string{"今日提示："} + context.weather + " / " + context.event,
             96, 116, 15, ink);

        text(font, "商品", 96, 142, 14, Color{78, 78, 72, 255});
        text(font, "库存", 200, 142, 14, Color{78, 78, 72, 255});
        text(font, "进货 - / +", 248, 142, 14, Color{78, 78, 72, 255});
        text(font, "价格 低/标/高", 344, 142, 14, Color{78, 78, 72, 255});
        text(font, "售价", 478, 142, 14, Color{78, 78, 72, 255});

        const auto config = store::default_store_config();
        int total_cost = 0;
        const bool controls_enabled = !state.session.location_started();
        for (std::size_t index = 0; index < config.products.size(); ++index) {
            const auto& product = config.products[index];
            const float y = 164.0F + static_cast<float>(index) * 22.0F;
            const int product_index = static_cast<int>(index);
            const bool selected =
                product_index == state.locations.store_selected_product_index;
            const Rectangle row = store_product_row(product_index);
            const Color row_fill =
                selected ? Color{255, 224, 154, 150}
                         : (controls_enabled && hovered(row, mouse)
                                ? Color{250, 238, 203, 180}
                                : Color{255, 248, 226, 0});
            DrawRectangleRec(scaled_rect(row), row_fill);
            const int purchase = store_plan_quantity(state.locations.store_purchase_plan,
                                                     product.id);
            const auto tier = store_plan_tier(state.locations.store_price_plan, product.id);
            total_cost += product.unit_cost * purchase;
            text(font, selected ? ">" : "", 88, y, 14, red);
            text(font, product.name, 104, y, 14, ink);
            text(font, std::to_string(store_inventory_quantity(state.session, product.id)),
                 210, y, 14, ink);
            const auto draw_control = [&](Rectangle bounds, const char* label, bool active) {
                Color fill = controls_enabled ? paper : Color{211, 202, 174, 255};
                if (controls_enabled && hovered(bounds, mouse)) {
                    fill = gold;
                } else if (active) {
                    fill = green;
                }
                const Rectangle scaled_bounds = scaled_rect(bounds);
                DrawRectangleRec(scaled_bounds, fill);
                DrawRectangleLinesEx(scaled_bounds, 2.0F, ink);
                centered_text(font, label, bounds, 12,
                              active ? RAYWHITE : (controls_enabled ? ink : disabled));
            };
            draw_control(store_purchase_decrease_button(product_index), "-", false);
            text(font, std::to_string(purchase), 280, y, 14, ink);
            draw_control(store_purchase_increase_button(product_index), "+", false);
            draw_control(store_price_button(product_index, store::PriceTier::low), "低",
                         tier == store::PriceTier::low);
            draw_control(store_price_button(product_index, store::PriceTier::standard), "标",
                         tier == store::PriceTier::standard);
            draw_control(store_price_button(product_index, store::PriceTier::high), "高",
                         tier == store::PriceTier::high);
            text(font, std::to_string(store::price_for_tier(product, tier)), 484, y, 14, ink);
        }

        const int balance_after = state.session.player().money - total_cost;
        text(font, std::string{"进货成本 "} + std::to_string(total_cost) +
                       " / 剩余金钱 " + std::to_string(balance_after),
             96, 254, 14, balance_after >= 0 ? Color{35, 83, 51, 255} : red);
        text(font, state.locations.store_feedback, 96, 274, 13,
             balance_after >= 0 ? Color{78, 78, 72, 255} : red);
        text(font, "键盘：1-4选商品，A/D调进货，Q/W/E调价格",
             96, 290, 13, Color{78, 78, 72, 255});
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
        centered_text(font, is_store ? "锁定方案" : "开始模拟", start_button, 16, RAYWHITE);
    } else {
        panel(start_button, hovered(start_button, mouse) ? paper : green);
        centered_text(font, is_store ? "结算销售" : "完成模拟", start_button, 16, RAYWHITE);
        panel(abandon_button, hovered(abandon_button, mouse) ? paper : red);
        centered_text(font, "主动放弃", abandon_button, 16, RAYWHITE);
    }
}

void draw_collision_debug(Location location) {
    const IndoorSceneLayout* layout = find_indoor_scene_layout(location);
    if (layout == nullptr) {
        return;
    }

    const auto to_rectangle = [](SceneRect value) {
        return scene_canvas_rectangle(
            Rectangle{value.x, value.y, value.width, value.height});
    };
    DrawRectangleLinesEx(to_rectangle(layout->walkable_bounds), 3.0F,
                         Color{75, 255, 120, 255});
    for (const auto& collider : layout->static_colliders) {
        const Rectangle bounds = to_rectangle(collider.bounds);
        DrawRectangleRec(bounds, Color{255, 65, 65, 70});
        DrawRectangleLinesEx(bounds, 2.0F, Color{255, 80, 80, 255});
    }
    DrawRectangleRec(to_rectangle(layout->exit_trigger), Color{70, 150, 255, 85});
    DrawRectangleLinesEx(to_rectangle(layout->exit_trigger), 3.0F,
                         Color{80, 170, 255, 255});
}

void draw_scene_navigation(const Font& font,
                           const Texture2D& protagonist_texture,
                           const SceneNavigationPresentation& navigation,
                           bool collision_debug_visible) {
    if (!navigation.active) {
        return;
    }
    const auto viewport_position = ui::scene_canvas_to_viewport(
        ui::SceneViewportPoint{navigation.player_position.x,
                               navigation.player_position.y});
    const float animation_seconds =
        navigation.moving ? navigation.animation_seconds : 0.0F;
    DrawEllipse(static_cast<int>(viewport_position.x),
                static_cast<int>(viewport_position.y - 2.0F), 14.0F, 5.0F,
                Color{20, 24, 25, 105});
    if (protagonist_texture.id != 0) {
        draw_npc_sprite(
            protagonist_texture, NpcSpriteKind::protagonist,
            animation_seconds,
            Rectangle{viewport_position.x - 30.0F,
                      viewport_position.y - 90.0F, 60.0F, 90.0F});
    } else {
        DrawCircleV(Vector2{viewport_position.x, viewport_position.y - 38.0F},
                    8.0F, Color{222, 188, 146, 255});
        DrawRectangleRec(
            Rectangle{viewport_position.x - 9.0F,
                      viewport_position.y - 30.0F, 18.0F, 28.0F},
            green);
    }

    if (collision_debug_visible) {
        const SceneRect actor = scene_actor_bounds(
            navigation.player_position, SceneSize{24.0F, 24.0F});
        const Rectangle actor_bounds = scene_canvas_rectangle(
            Rectangle{actor.x, actor.y, actor.width, actor.height});
        DrawRectangleLinesEx(actor_bounds, 2.0F, Color{80, 210, 255, 255});
    }

    // Keep the navigation hint in the bottom HUD lane. The previous top
    // overlay covered scene windows and wall art in every indoor background.
    const Rectangle prompt{348.0F, 288.0F, 274.0F, 24.0F};
    panel(prompt, Color{46, 58, 57, 235}, Color{255, 224, 154, 220});
    centered_text(font, scene_navigation_prompt(navigation).c_str(), prompt,
                  12.0F, RAYWHITE);
}

void draw_npc_dialogue(const Font& font,
                       const DialoguePresentation& dialogue,
                       Vector2 mouse, const Texture2D* npc_texture = nullptr,
                       NpcSpriteKind npc_kind = NpcSpriteKind::salesclerk,
                       const Texture2D* protagonist_texture = nullptr,
                       const Texture2D* mayor_texture = nullptr);

void draw_location_lobby(const Font& font, const SceneVisualAssets& scene_assets,
                         const GameAppState& state, bool audio_enabled,
                         Vector2 mouse) {
    if (!state.location_lobby.has_value()) {
        return;
    }
    const Location location = *state.location_lobby;
    const LocationLobbySpec* spec = find_location_lobby_spec(location);
    if (spec == nullptr) {
        return;
    }

    const Texture2D& background = scene_interior_texture(scene_assets, location);
    if (background.id != 0) {
        draw_scene_texture(background);
    } else {
        ClearBackground(Color{117, 91, 70, 255});
    }
    draw_status(font, state.session, audio_enabled);

    panel(Rectangle{18, 62, 176, 34}, Color{46, 58, 57, 232});
    centered_text(font, spec->title.c_str(), Rectangle{18, 62, 176, 34}, 18,
                  Color{255, 224, 154, 255});

    const Rectangle npc_hotspot =
        scene_design_rectangle(lobby_rectangle(spec->npc_hotspot));
    const bool has_fixed_npc = location != Location::home;
    const bool uses_npc_lobby =
        location == Location::restaurant ||
        location == Location::convenience_store;
    const NpcLobbyPresentation npc_lobby =
        uses_npc_lobby ? state.locations.npc_lobby.presentation()
                       : NpcLobbyPresentation{};
    const bool dialogue_active = npc_lobby.dialogue.has_value();
    const bool npc_hovered =
        has_fixed_npc && !dialogue_active && hovered(npc_hotspot, mouse);
    const Texture2D& npc_texture = scene_npc_texture(scene_assets, location);
    const NpcSpriteKind npc_kind =
        location == Location::restaurant ? NpcSpriteKind::restaurant_chef
                                         : NpcSpriteKind::salesclerk;
    if (has_fixed_npc) {
        if (npc_hovered) {
            DrawRectangleRec(scaled_rect(npc_hotspot),
                             Color{255, 224, 154, 55});
            DrawRectangleLinesEx(scaled_rect(npc_hotspot), 3.0F, gold);
        }
        const float npc_center_x =
            npc_hotspot.x + npc_hotspot.width * 0.5F;
        const int idle_frame =
            static_cast<int>(npc_lobby.npc_animation_seconds / 0.18F) % 4;
        const float bob =
            idle_frame == 1 ? -1.0F : (idle_frame == 3 ? 1.0F : 0.0F);
        const float npc_base_y =
            npc_hotspot.y + npc_hotspot.height - 28.0F + bob;
        const Rectangle npc_label{
            npc_hotspot.x, npc_hotspot.y + npc_hotspot.height - 22.0F,
            npc_hotspot.width, 24.0F};
        if (npc_texture.id != 0) {
            draw_npc_sprite(
                npc_texture, npc_kind, npc_lobby.npc_animation_seconds,
                scaled_rect(Rectangle{npc_center_x - 22.0F,
                                      npc_label.y - 64.0F + bob, 44.0F,
                                      66.0F}));
        } else {
            DrawCircleV(
                scaled_point(Vector2{npc_center_x, npc_base_y - 18.0F}),
                scaled(7.0F), Color{242, 207, 159, 220});
            DrawRectangleRec(
                scaled_rect(Rectangle{npc_center_x - 7.0F,
                                      npc_base_y - 11.0F, 14.0F, 18.0F}),
                Color{67, 78, 74, 220});
        }
        panel(npc_label, npc_hovered ? Color{250, 238, 203, 248}
                                     : Color{46, 58, 57, 230});
        centered_text(font, spec->npc_label.c_str(), npc_label, 13,
                      npc_hovered ? ink : RAYWHITE);
    }

    draw_scene_navigation(font, scene_assets.protagonist,
                          state.locations.scene_navigation.presentation(),
                          state.collision_debug_visible);

    panel(Rectangle{18, 288, 330, 58}, Color{46, 58, 57, 232});
    const auto notice_lines = wrap_text_lines(state.notice, 22, 2);
    for (std::size_t index = 0; index < notice_lines.size(); ++index) {
        text(font, notice_lines[index], 32, 299.0F + static_cast<float>(index) * 19.0F,
             14, RAYWHITE);
    }

    const Rectangle back = lobby_rectangle(spec->back_button);
    const Rectangle action = lobby_rectangle(spec->action_button);
    panel(back, hovered(back, mouse) ? paper : Color{211, 202, 174, 245});
    centered_text(font, "返回地图", back, 15, ink);
    panel(action, hovered(action, mouse) ? paper : green);
    centered_text(font, spec->action_label.c_str(), action, 14, RAYWHITE);

    if (npc_lobby.dialogue.has_value()) {
        draw_npc_dialogue(font, *npc_lobby.dialogue, mouse, &npc_texture,
                          npc_kind, &scene_assets.protagonist,
                          &scene_assets.mayor);
    }
}

void draw_npc_dialogue(const Font& font,
                       const DialoguePresentation& dialogue,
                       Vector2 mouse, const Texture2D* npc_texture,
                       NpcSpriteKind npc_kind,
                       const Texture2D* protagonist_texture,
                       const Texture2D* mayor_texture) {
    DrawRectangle(0, 0, ui::canvas_width, ui::canvas_height,
                  Color{20, 27, 29, 155});
    const Rectangle bounds{48, 226, 544, 116};
    panel(bounds, cream, gold);
    const Rectangle portrait_slot{60, 238, 62, 84};
    panel(portrait_slot, Color{239, 220, 182, 255},
          Color{157, 111, 72, 255});
    const bool protagonist_speaking = dialogue.speaker == "主角";
    const bool mayor_speaking = dialogue.speaker == "镇长";
    const Texture2D* speaker_texture = protagonist_speaking
                                           ? protagonist_texture
                                           : (mayor_speaking ? mayor_texture
                                                             : npc_texture);
    const NpcSpriteKind speaker_kind = protagonist_speaking
                                           ? NpcSpriteKind::protagonist
                                           : (mayor_speaking
                                                  ? NpcSpriteKind::mayor
                                                  : npc_kind);
    if (speaker_texture != nullptr && speaker_texture->id != 0) {
        const NpcSpriteSpec& sprite = npc_sprite_spec(speaker_kind);
        constexpr float portrait_height = 80.0F;
        const float portrait_width =
            portrait_height * static_cast<float>(sprite.frame_width) /
            static_cast<float>(sprite.frame_height);
        const Rectangle portrait_destination{
            portrait_slot.x + (portrait_slot.width - portrait_width) * 0.5F,
            portrait_slot.y + (portrait_slot.height - portrait_height) * 0.5F,
            portrait_width, portrait_height};
        // Dialogue portraits are identity anchors, not ambient scene animation.
        // Freeze frame zero so per-frame transparent padding cannot make the
        // character appear to drift inside the portrait slot.
        draw_npc_sprite(*speaker_texture, speaker_kind, 0.0F,
                        scaled_rect(portrait_destination));
    } else {
        const Color coat = dialogue.speaker == "主角" ? green : slate;
        DrawCircleV(scaled_point(Vector2{91, 259}), scaled(10),
                    Color{222, 188, 146, 255});
        DrawRectangleRec(scaled_rect(Rectangle{78, 270, 26, 36}), coat);
    }

    text(font, dialogue.speaker, 136, 238, 18, red);
    const std::string progress = std::to_string(dialogue.current_line) + " / " +
                                 std::to_string(dialogue.total_lines);
    text(font, progress, 500, 240, 12, Color{102, 105, 101, 255});
    const auto lines = wrap_text_lines(dialogue.text, 30, 3);
    for (std::size_t index = 0; index < lines.size(); ++index) {
        text(font, lines[index], 136,
             266.0F + static_cast<float>(index) * 20.0F, 14, ink);
    }
    const Rectangle next{470, 306, 104, 28};
    panel(next, hovered(next, mouse) ? paper : Color{215, 204, 180, 255});
    centered_text(font,
                  dialogue.current_line == dialogue.total_lines ? "关闭"
                                                                 : "下一句",
                  next, 13, ink);
    text(font, "Enter / Space / 点击继续 · Esc 跳过", 136, 320, 11,
         Color{102, 105, 101, 255});
}

void draw_library_room(const Font& font, const Texture2D& background,
                       const Texture2D& administrator_texture,
                       const Texture2D& protagonist_texture,
                       const Texture2D& mayor_texture,
                       const GameAppState& state, bool audio_enabled,
                       Vector2 mouse) {
    if (background.id != 0) {
        draw_scene_texture(background);
    } else {
        ClearBackground(Color{117, 91, 70, 255});
    }
    draw_status(font, state.session, audio_enabled);

    const LibraryRoomPresentation room =
        state.locations.library_room.presentation();
    const LocationLobbySpec* spec =
        find_location_lobby_spec(Location::library);
    const Rectangle administrator_hotspot =
        spec == nullptr
            ? Rectangle{48.0F, 62.0F, 160.0F, 100.0F}
            : scene_design_rectangle(lobby_rectangle(spec->npc_hotspot));
    const bool administrator_hovered = hovered(administrator_hotspot, mouse);
    if (administrator_hovered) {
        DrawRectangleRec(scaled_rect(administrator_hotspot),
                         Color{255, 224, 154, 45});
        DrawRectangleLinesEx(scaled_rect(administrator_hotspot), 3.0F, gold);
    }

    const int idle_frame =
        static_cast<int>(room.administrator_animation_seconds / 0.18F) % 4;
    const float bob = idle_frame == 1 ? -1.0F : (idle_frame == 3 ? 1.0F : 0.0F);
    const float center_x =
        administrator_hotspot.x + administrator_hotspot.width * 0.70F;
    const float base_y =
        administrator_hotspot.y + administrator_hotspot.height * 0.78F + bob;
    const Rectangle administrator_label{
        administrator_hotspot.x + 8.0F,
        administrator_hotspot.y + administrator_hotspot.height - 24.0F,
        administrator_hotspot.width - 16.0F, 24.0F};
    if (administrator_texture.id != 0) {
        draw_npc_sprite(
            administrator_texture, NpcSpriteKind::librarian,
            room.administrator_animation_seconds,
            scaled_rect(Rectangle{center_x - 22.0F,
                                  administrator_label.y - 64.0F + bob, 44.0F,
                                  66.0F}));
    } else {
        DrawEllipse(static_cast<int>(scaled(center_x)),
                    static_cast<int>(scaled(base_y + 12.0F)), scaled(11.0F),
                    scaled(4.0F), Color{25, 30, 31, 95});
        DrawCircleV(scaled_point(Vector2{center_x, base_y - 13.0F}),
                    scaled(8.0F), Color{222, 188, 146, 255});
        DrawRectangleRec(
            scaled_rect(Rectangle{center_x - 9.0F, base_y - 5.0F, 18.0F,
                                  25.0F}),
            slate);
    }
    panel(administrator_label,
          administrator_hovered ? Color{250, 238, 203, 248}
                                : Color{46, 58, 57, 230});
    centered_text(font, "图书馆管理员", administrator_label, 13,
                  administrator_hovered ? ink : RAYWHITE);

    draw_scene_navigation(font, protagonist_texture,
                          state.locations.scene_navigation.presentation(),
                          state.collision_debug_visible);

    panel(Rectangle{12, 318, 616, 30}, Color{46, 58, 57, 232});
    text(font, "点击管理员或按 Space 交谈 · Esc 返回地图", 24, 326, 12,
         RAYWHITE);

    if (room.dialogue.has_value()) {
        draw_npc_dialogue(font, *room.dialogue, mouse,
                          &administrator_texture, NpcSpriteKind::librarian,
                          &protagonist_texture, &mayor_texture);
    }
}

void draw_home_preview(const Font& font, const Texture2D& background,
                       const GameAppState& state, bool audio_enabled,
                       Vector2 mouse) {
    if (background.id != 0) {
        draw_scene_texture(background);
    } else {
        ClearBackground(Color{117, 91, 70, 255});
    }
    draw_status(font, state.session, audio_enabled);
    panel(Rectangle{72, 250, 496, 96}, Color{46, 58, 57, 235});
    text(font, "今晚要休息吗？", 96, 266, 22, Color{255, 224, 154, 255});
    text(font, "确认后将恢复体力并进入每日总结。", 96, 294, 15, RAYWHITE);

    const Rectangle back = home_preview_back_button();
    const Rectangle rest = home_preview_rest_button();
    panel(back, hovered(back, mouse) ? paper : Color{211, 202, 174, 255});
    centered_text(font, "返回地图", back, 16, ink);
    panel(rest, hovered(rest, mouse) ? paper : green);
    centered_text(font, "确认休息", rest, 16, RAYWHITE);
}

void draw_summary(const Font& font, const GameAppState& state, bool audio_enabled,
                  Vector2 mouse) {
    ClearBackground(Color{37, 50, 57, 255});
    draw_status(font, state.session, audio_enabled);
    panel(Rectangle{90, 88, 460, 188}, cream);
    text(font, "每日总结", 124, 116, 28, red);
    text_block(font, state.session.last_summary(), 124, 154, 14, 16, ink);
    text(font, day_closing_summary(state.session.day()), 124, 194, 14, Color{78, 78, 72, 255});
    text(font, state.session.day() == state.session.day_limit()
                   ? "确认后进入最终主结局。"
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
    text(font, game_plan_completion_title(), 118, 112, 28, red);
    text(font, std::string{"主结局："} + state.session.main_ending(), 118, 154, 20, ink);
    text_block(font, state.session.final_summary(), 118, 188, 13, 15, ink);
    text(font, "最终状态", 118, 282, 18, Color{35, 83, 51, 255});
    text(font,
         std::string{"金钱 "} + std::to_string(player.money) + "  体力 " +
             std::to_string(player.stamina) + "  声望 " + std::to_string(player.reputation),
         118, 306, 16, ink);
    text(font,
         std::string{"知识 "} + std::to_string(player.knowledge) + "  心情 " +
             std::to_string(player.mood) + "  酒馆胜 " +
             std::to_string(state.session.tavern_wins()) + " / 负 " +
             std::to_string(state.session.tavern_losses()),
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
            "·/：，。；“”‘’+-！？《》【】、———（）「」『』℃°…";
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
            "点击商品行减号加号锁定方案结算销售已选择进货数量价格档已设为售价已经是达到库存上限"
            "方案已锁定查看销售结果便利店商品选择无效"
            "知识行动完成回家休息恢复体力并结束今天主动放弃阶段已消耗本次无收益确认后进入下一游戏日"
            "最终主结局最终状态成长路线判定依据库存清算十日经营计划已经结束不能继续选择地点"
            "点击地点查看原因已暂停按P继续按M切换静音恢复声音酒馆胜负"
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
            "回答错误达·芬奇还擅长科学发明"
            "选择本次工作数字也可以选择今天想怎样帮助图书馆读者咨询根据需求选择书籍类别"
            "书籍整理拾取散落或错架书并归位放弃本次图书馆行动已归位错误点击场景中的书本拾取"
            "再点击对应正确分类书架手中空已拾取完成图书馆整理归位错误尝试次已拿起分类正确书籍已归位"
            "全部书籍已归位正在结算分类不匹配请换一个书架这里没有待整理的书请点击带光圈的书本素材"
            "请点击书架高亮区域完成归位散落错架"
            "物理奥秘唐诗宋词世界地图集历史书架科学书架文学书架艺术书架技术书架地理书架"
            "请选择读者咨询或书籍整理当前不能切换图书馆工作模式整理数据不完整无法开始"
            "已选择根据需求匹配拿起待整理书再点击正确书架工作模式未能启动整理工作没有结算";
        result += story_text_glyphs();
        result += ending_rules_glyphs();
        result += store_runtime_glyphs();
        result += tavern_ui_glyphs();
        result += StoryDialogueCatalog{}.glyphs();
        result += LocationStoryCatalog{}.glyphs();
        result += scene_navigation_glyphs();
        return result;
    }();
    return glyphs.c_str();
}

void update_game_flow(GameAppState& state, Vector2 logical_mouse) {
    // 这里是应用层路由，不是规则层：先消费互斥的剧情/室内 Runtime，再处理地点和阶段。
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
            if (should_replay_new_game_opening(state.session)) {
                (void)state.locations.story_lifecycle.open(
                    StoryLifecycleContext::new_game_opening);
            }
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
            if (state.locations.story_lifecycle.open(
                    StoryLifecycleContext::new_game_opening)) {
                state.notice = game_plan_intro_notice();
            } else {
                state.notice = "开场对话暂时不可用。";
            }
        }
        return;
    }

    if (IsKeyPressed(KEY_F3)) {
        state.collision_debug_visible = !state.collision_debug_visible;
    }

    if (state.locations.story_lifecycle.active()) {
        // 对话 active 时整帧提前返回，避免同一输入同时移动、互动或提交行动。
        DialogueFrameInput input;
        input.advance_pressed =
            IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        input.skip_pressed = IsKeyPressed(KEY_ESCAPE);
        const auto stepped = step_story_lifecycle(
            state.session, state.locations.story_lifecycle, input);
        if (!stepped.notice.empty()) {
            state.notice = stepped.notice;
        }
        if (stepped.context == StoryLifecycleContext::home_rest &&
            (stepped.action_applied ||
             stepped.status == StoryLifecycleStepStatus::rejected)) {
            state.home_preview_open = false;
        }
        return;
    }

    if (state.locations.library_room.active()) {
        const LibraryRoomPresentation room =
            state.locations.library_room.presentation();
        LibraryRoomInput input;
        input.elapsed_seconds = GetFrameTime();
        if (room.dialogue.has_value()) {
            input.dialogue.advance_pressed =
                IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            input.dialogue.skip_pressed = IsKeyPressed(KEY_ESCAPE);
        } else {
            const LocationLobbySpec* spec =
                find_location_lobby_spec(Location::library);
            const Rectangle administrator_hotspot =
                spec == nullptr
                    ? Rectangle{48.0F, 62.0F, 160.0F, 100.0F}
                    : scene_design_rectangle(lobby_rectangle(spec->npc_hotspot));
            const SceneNavigationStepResult navigation =
                state.locations.scene_navigation.step(
                    scene_navigation_frame_input());
            if (!navigation.notice.empty()) {
                state.notice = navigation.notice;
            }
            const bool navigation_administrator =
                navigation.interaction == SceneInteractionKind::npc;
            const bool navigation_exit =
                navigation.interaction == SceneInteractionKind::exit;
            input.administrator_activated =
                clicked(administrator_hotspot, logical_mouse) ||
                navigation_administrator;
            input.back_pressed = IsKeyPressed(KEY_ESCAPE) || navigation_exit;
        }
        const LibraryRoomStepResult stepped = step_library_room(
            state.session, state.locations, input, state.notice);
        if (stepped.status == LibraryRoomStepStatus::work_requested ||
            stepped.status == LibraryRoomStepStatus::closed) {
            state.locations.scene_navigation.close();
        }
        return;
    }

    if (state.location_lobby.has_value()) {
        const Location location = *state.location_lobby;
        const LocationLobbySpec* spec = find_location_lobby_spec(location);
        if (spec == nullptr) {
            state.location_lobby.reset();
            state.notice = "地点大厅配置缺失，已返回地图。";
            return;
        }
        const Rectangle back = lobby_rectangle(spec->back_button);
        const Rectangle action = lobby_rectangle(spec->action_button);
        const Rectangle npc =
            scene_design_rectangle(lobby_rectangle(spec->npc_hotspot));
        if (location == Location::restaurant ||
            location == Location::convenience_store) {
            const NpcLobbyPresentation lobby =
                state.locations.npc_lobby.presentation();
            NpcLobbyInput input;
            input.elapsed_seconds = GetFrameTime();
            if (lobby.dialogue.has_value()) {
                input.dialogue.advance_pressed =
                    IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                input.dialogue.skip_pressed = IsKeyPressed(KEY_ESCAPE);
            } else {
                const SceneNavigationStepResult navigation =
                    state.locations.scene_navigation.step(
                        scene_navigation_frame_input());
                if (!navigation.notice.empty()) {
                    state.notice = navigation.notice;
                }
                input.interaction_activated =
                    clicked(npc, logical_mouse) ||
                    clicked(action, logical_mouse) ||
                    navigation.interaction == SceneInteractionKind::npc;
                input.back_pressed =
                    clicked(back, logical_mouse) || IsKeyPressed(KEY_ESCAPE) ||
                    navigation.interaction == SceneInteractionKind::exit;
            }
            const auto stepped =
                location == Location::restaurant
                    ? step_restaurant_lobby(state.session, state.locations,
                                            input, state.notice)
                    : step_store_lobby(state.session, state.locations, input,
                                       state.notice);
            if (stepped.status == NpcLobbyStepStatus::activity_requested ||
                stepped.status == NpcLobbyStepStatus::closed) {
                state.location_lobby.reset();
                state.locations.scene_navigation.close();
            }
            return;
        }
        const SceneNavigationStepResult navigation =
            state.locations.scene_navigation.step(scene_navigation_frame_input());
        if (!navigation.notice.empty()) {
            state.notice = navigation.notice;
        }
        if (clicked(back, logical_mouse) || IsKeyPressed(KEY_ESCAPE) ||
            navigation.interaction == SceneInteractionKind::exit) {
            state.location_lobby.reset();
            state.locations.scene_navigation.close();
            state.notice = "已返回地图：阶段未消耗。";
            return;
        }
        if (location != Location::home && clicked(npc, logical_mouse)) {
            state.notice = spec->npc_label + "：对话接口已预留，后续接入正式 NPC 内容。";
            return;
        }
        if (clicked(action, logical_mouse) ||
            navigation.interaction == SceneInteractionKind::primary_activity) {
            if (location == Location::home) {
                state.location_lobby.reset();
                state.locations.scene_navigation.close();
                state.home_preview_open = true;
                state.notice = "已回到家中；确认休息后才会消耗夜晚阶段。";
                return;
            }
            if (!state.session.enter_location(location)) {
                state.notice = "无法进入该地点，请返回地图重试。";
                return;
            }
            if (location == Location::library) {
                if (start_pending_location(state.session, state.locations, state.notice)) {
                    state.location_lobby.reset();
                } else {
                    (void)state.session.return_to_map();
                }
                return;
            }
        }
        return;
    }

    if (state.home_preview_open) {
        if (activated(home_preview_back_button(), logical_mouse, KEY_ESCAPE)) {
            state.home_preview_open = false;
            state.notice = "已返回地图：夜晚阶段未消耗。";
            return;
        }
        if (activated(home_preview_rest_button(), logical_mouse, KEY_ENTER) ||
            IsKeyPressed(KEY_SPACE)) {
            if (state.locations.story_lifecycle.open_home_rest(state.session)) {
                state.notice = "今晚的访客来敲门了。";
            } else {
                state.notice = "回家独白暂时不可用。";
            }
            return;
        }
        return;
    }

    if (update_active_library(state.session, state.locations, state.notice, logical_mouse)) {
        return;
    }

    if (state.session.phase() == GamePhase::day_choice ||
        state.session.phase() == GamePhase::night_choice) {
        // 选择阶段只负责进入地点；真正消耗行动要等地点 Runtime 提交结果。
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
            if (location == Location::tavern) {
                ensure_tavern_assets_loaded(state.locations.tavern_assets);
                const auto opened = state.locations.tavern.open(state.session);
                state.notice = opened.message;
                if (opened.status == TavernOpenStatus::opened &&
                    !state.locations.scene_navigation.open(location)) {
                    state.notice = "酒馆已打开，但主角导航初始化失败。";
                }
                return;
            }
            if (location == Location::library) {
                if (open_daytime_story_lobby(state.session, state.locations, location,
                                             state.notice)) {
                    if (!state.locations.scene_navigation.open(location)) {
                        state.notice = "图书馆已打开，但主角导航初始化失败。";
                        return;
                    }
                    state.notice =
                        "已进入图书馆：用 WASD 靠近柜台管理员后按 E 交谈。";
                } else {
                    state.notice = "图书馆场景加载失败。";
                }
                return;
            }
            if (location == Location::restaurant ||
                location == Location::convenience_store) {
                if (!open_daytime_story_lobby(state.session, state.locations, location,
                                             state.notice)) {
                    state.notice =
                        location == Location::restaurant
                            ? "餐馆老板对话暂时不可用。"
                            : "便利店店主对话暂时不可用。";
                    return;
                }
                if (!state.locations.scene_navigation.open(location)) {
                    state.notice = "地点已打开，但主角导航初始化失败。";
                    return;
                }
            }
            state.location_lobby = location;
            if (location == Location::home &&
                !state.locations.scene_navigation.open(location)) {
                state.location_lobby.reset();
                state.notice = "家中场景的主角导航初始化失败。";
                return;
            }
            if (location == Location::restaurant) {
                state.notice =
                    "已进入餐馆：用 WASD 靠近老板后按 E 交谈。";
            } else if (location == Location::convenience_store) {
                state.notice =
                    "已进入便利店：用 WASD 靠近店主后按 E 交谈。";
            } else {
                state.notice = std::string{"已进入"} + location_label(location) +
                               "；使用 WASD 移动，靠近目标后按 E 互动。";
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
            const TavernScreen screen =
                state.locations.tavern.presentation().screen;
            TavernFrameInput input = tavern_frame_input(logical_mouse, screen);
            if (screen == TavernScreen::lobby) {
                const SceneNavigationStepResult navigation =
                    state.locations.scene_navigation.step(
                        scene_navigation_frame_input());
                if (!navigation.notice.empty()) {
                    state.notice = navigation.notice;
                }
                if (navigation.interaction.has_value()) {
                    apply_tavern_navigation_interaction(
                        *navigation.interaction, input);
                }
            }
            const auto result =
                state.locations.tavern.step(state.session, input);
            if (result.notice.has_value()) {
                state.notice = *result.notice;
            }
            if (result.status == TavernStepStatus::returned_to_map ||
                result.status == TavernStepStatus::settled) {
                state.locations.scene_navigation.close();
            }
            return;
        }
        if (state.session.pending_location() == Location::convenience_store &&
            !state.session.location_started()) {
            update_store_selection(state.locations, state.session, logical_mouse, state.notice);
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
                    const Texture2D& generated_buildings,
                    const SceneVisualAssets& scene_assets, const GameAppState& state,
                    bool audio_enabled, bool paused, Vector2 logical_mouse) {
    if (!state.has_session) {
        draw_title(font, title_background, state, logical_mouse);
        if (paused) {
            draw_pause_overlay(font, audio_enabled);
        }
        return;
    }

    if (state.locations.story_lifecycle.active()) {
        const StoryLifecyclePresentation lifecycle =
            state.locations.story_lifecycle.presentation();
        if (lifecycle.context == StoryLifecycleContext::home_rest) {
            draw_home_preview(font, scene_assets.home_interior, state,
                              audio_enabled, logical_mouse);
        } else {
            draw_map(font, town_marker, kenney_tiles, generated_full_map_scene,
                     generated_map_background, generated_buildings, state,
                     audio_enabled, logical_mouse);
        }
        if (lifecycle.dialogue.has_value()) {
            const Texture2D* visitor_texture = nullptr;
            NpcSpriteKind visitor_kind = NpcSpriteKind::salesclerk;
            if (lifecycle.dialogue->speaker == "餐馆老板") {
                visitor_texture = &scene_assets.restaurant_npc;
                visitor_kind = NpcSpriteKind::restaurant_chef;
            } else if (lifecycle.dialogue->speaker == "便利店店主") {
                visitor_texture = &scene_assets.convenience_store_npc;
                visitor_kind = NpcSpriteKind::salesclerk;
            } else if (lifecycle.dialogue->speaker == "管理员") {
                visitor_texture = &scene_assets.library_npc;
                visitor_kind = NpcSpriteKind::librarian;
            }
            draw_npc_dialogue(font, *lifecycle.dialogue, logical_mouse,
                              visitor_texture, visitor_kind,
                              &scene_assets.protagonist, &scene_assets.mayor);
        }
        if (paused) {
            draw_pause_overlay(font, audio_enabled);
        }
        return;
    }

    if (state.locations.library_room.active()) {
        draw_library_room(font, scene_assets.library_interior,
                          scene_assets.library_npc, scene_assets.protagonist,
                          scene_assets.mayor, state, audio_enabled,
                          logical_mouse);
        if (paused) {
            draw_pause_overlay(font, audio_enabled);
        }
        return;
    }

    if (state.location_lobby.has_value()) {
        draw_location_lobby(font, scene_assets, state, audio_enabled, logical_mouse);
        if (state.collision_debug_visible) {
            draw_collision_debug(*state.location_lobby);
        }
        if (paused) {
            draw_pause_overlay(font, audio_enabled);
        }
        return;
    }

    if (state.home_preview_open) {
        draw_home_preview(font, scene_assets.home_interior, state, audio_enabled,
                          logical_mouse);
        if (state.collision_debug_visible) {
            draw_collision_debug(Location::home);
        }
        if (paused) {
            draw_pause_overlay(font, audio_enabled);
        }
        return;
    }

    if (state.locations.library.active()) {
        draw_active_library(font, state.locations, logical_mouse,
                            scene_assets.library_interior,
                            scene_assets.library_organizing_books);
        if (state.collision_debug_visible) {
            draw_collision_debug(Location::library);
        }
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
        draw_location(font, state, audio_enabled, logical_mouse, scene_assets);
        if (state.collision_debug_visible) {
            draw_collision_debug(state.session.pending_location());
        }
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

#include "app/game_flow.hpp"

#include <array>
#include <cmath>
#include <sstream>
#include <string>

#include "core/story_text.hpp"

namespace pixel_town {
namespace {

constexpr Color ink{45, 52, 54, 255};
constexpr Color paper{250, 238, 203, 255};
constexpr Color cream{255, 248, 226, 255};
constexpr Color green{82, 137, 92, 255};
constexpr Color grass{144, 190, 119, 255};
constexpr Color road{194, 170, 121, 255};
constexpr Color disabled{145, 143, 132, 255};
constexpr Color red{183, 83, 72, 255};
constexpr Color gold{224, 169, 74, 255};
constexpr Color shadow{39, 48, 53, 120};
constexpr Color slate{60, 79, 82, 255};
constexpr float native_ui_scale = 1.5F;

constexpr std::array<Location, 5> map_locations{
    Location::restaurant, Location::convenience_store, Location::library, Location::tavern,
    Location::home};

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
};

float scaled(float value) {
    return std::round(value * native_ui_scale);
}

float scaled_font_size(float design_size) {
    if (design_size <= 20.0F) {
        return 24.0F;
    }
    if (design_size <= 30.0F) {
        return 36.0F;
    }
    return 48.0F;
}

Vector2 scaled_point(Vector2 value) {
    return Vector2{scaled(value.x), scaled(value.y)};
}

Rectangle scaled_rect(Rectangle value) {
    return Rectangle{scaled(value.x), scaled(value.y), scaled(value.width), scaled(value.height)};
}

void text(const Font& font, const char* value, float x, float y, float size, Color color = ink) {
    DrawTextEx(font, value, Vector2{scaled(x), scaled(y)}, scaled_font_size(size), 1.0F, color);
}

void text(const Font& font, const std::string& value, float x, float y, float size,
          Color color = ink) {
    text(font, value.c_str(), x, y, size, color);
}

void text_block(const Font& font, const char* value, float x, float y, float size,
                float line_gap, Color color = ink) {
    std::stringstream stream(value);
    std::string line;
    float offset = 0.0F;
    while (std::getline(stream, line)) {
        text(font, line, x, y + offset, size, color);
        offset += line_gap;
    }
}

void text_block(const Font& font, const std::string& value, float x, float y, float size,
                float line_gap, Color color = ink) {
    text_block(font, value.c_str(), x, y, size, line_gap, color);
}

void centered_text(const Font& font, const char* value, Rectangle bounds, float size,
                   Color color = ink) {
    const Rectangle scaled_bounds = scaled_rect(bounds);
    const float font_size = scaled_font_size(size);
    const Vector2 measured = MeasureTextEx(font, value, font_size, 1.0F);
    DrawTextEx(font, value,
               Vector2{scaled_bounds.x + (scaled_bounds.width - measured.x) / 2.0F,
                       scaled_bounds.y + (scaled_bounds.height - measured.y) / 2.0F},
               font_size, 1.0F, color);
}

void panel(Rectangle bounds, Color fill, Color border = ink) {
    const Rectangle scaled_bounds = scaled_rect(bounds);
    DrawRectangleRec(Rectangle{scaled_bounds.x + 4, scaled_bounds.y + 4, scaled_bounds.width,
                               scaled_bounds.height},
                     shadow);
    DrawRectangleRec(scaled_bounds, fill);
    DrawRectangleLinesEx(scaled_bounds, 3.0F, border);
}

bool clicked(Rectangle bounds, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, scaled_rect(bounds)) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool activated(Rectangle bounds, Vector2 mouse, KeyboardKey key) {
    return clicked(bounds, mouse) || IsKeyPressed(key);
}

bool hovered(Rectangle bounds, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, scaled_rect(bounds));
}

std::array<Rectangle, 5> location_bounds() {
    return {Rectangle{70, 74, 160, 86}, Rectangle{388, 74, 160, 86},
            Rectangle{70, 186, 160, 88}, Rectangle{420, 186, 152, 88},
            Rectangle{250, 186, 96, 88}};
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

void draw_location(const Font& font, const GameAppState& state, Vector2 mouse) {
    ClearBackground(Color{215, 221, 194, 255});
    draw_status(font, state.session, true);
    panel(Rectangle{96, 78, 448, 210}, cream);
    const Location location = state.session.pending_location();
    text(font, location_label(location), 126, 106, 30, red);
    text(font,
         state.session.location_started() ? "地点已开始：完成模拟或主动放弃都会消耗本阶段。"
                                          : "尚未开始：现在返回地图不会消耗本阶段。",
         126, 154, 16, ink);

    const Rectangle back_button{126, 228, 112, 34};
    const Rectangle start_button{264, 228, 112, 34};
    const Rectangle abandon_button{402, 228, 112, 34};
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

void draw_summary(const Font& font, const GameAppState& state, Vector2 mouse) {
    ClearBackground(Color{37, 50, 57, 255});
    draw_status(font, state.session, true);
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

void draw_ending(const Font& font, const GameAppState& state) {
    ClearBackground(Color{37, 50, 57, 255});
    draw_status(font, state.session, true);
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
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 "
            "·/：，。；“”+-";
        for (const char* value : ui_texts) {
            result += value;
        }
        result +=
            "点击开始第一天不可进入尚未地点已开始完成模拟或主动放弃都会消耗本阶段现在返回地图不会"
            "消耗本阶段今天的白天行动已经完成现在是白天回家休息只能在夜晚选择酒馆夜晚开放当前"
            "阶段不能进入今晚的行动已经完成酒馆玩法将在后续issue接入本切片先开放回家休息白天工作"
            "已经结束不能再进入该地点当前正在处理另一个阶段不能选择新地点餐馆模拟工作完成服务了"
            "午餐客流获得金钱与声望便利店模拟经营完成一次进货与销售结算图书馆帮助读者找书并提升"
            "知识行动完成回家休息恢复体力并结束今天主动放弃阶段已消耗本次无收益确认后进入下一游戏日"
            "占位主结局最终状态成长路线摘要均衡体验小镇生活十日经营计划已经结束不能继续选择地点"
            "点击地点查看原因成长路线均衡体验已暂停按P继续按M切换静音恢复声音"
            "演示参数错误预设加载失败已加载正式存档不会被读取或覆盖";
        result += story_text_glyphs();
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
            if (state.session.enter_location(location)) {
                state.notice =
                    std::string{"已进入"} + location_label(location) + "，开始前可返回地图。";
            }
            return;
        }
        return;
    }

    if (state.session.phase() == GamePhase::day_location ||
        state.session.phase() == GamePhase::night_location) {
        const Rectangle back_button{126, 228, 112, 34};
        const Rectangle start_button_location{264, 228, 112, 34};
        const Rectangle abandon_button{402, 228, 112, 34};
        if (!state.session.location_started()) {
            if (activated(back_button, logical_mouse, KEY_ESCAPE)) {
                if (state.session.return_to_map()) {
                    state.notice = "已返回地图：阶段未消耗。";
                }
                return;
            }
            if (activated(start_button_location, logical_mouse, KEY_SPACE)) {
                if (state.session.start_location() != 0) {
                    state.notice = "地点已开始：完成或放弃都会消耗本阶段。";
                }
                return;
            }
        } else {
            if (activated(start_button_location, logical_mouse, KEY_SPACE)) {
                const auto applied =
                    state.session.apply_action_result(state.session.simulated_success_result());
                state.notice = applied.message;
                return;
            }
            if (clicked(abandon_button, logical_mouse)) {
                const auto applied =
                    state.session.apply_action_result(state.session.abandon_current_location());
                state.notice = applied.message;
                return;
            }
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

    if (state.session.phase() == GamePhase::day_choice ||
        state.session.phase() == GamePhase::night_choice) {
        draw_map(font, town_marker, kenney_tiles, generated_full_map_scene,
                 generated_map_background, generated_buildings, state, audio_enabled,
                 logical_mouse);
    } else if (state.session.phase() == GamePhase::day_location ||
               state.session.phase() == GamePhase::night_location) {
        draw_location(font, state, logical_mouse);
    } else if (state.session.phase() == GamePhase::day_summary) {
        draw_summary(font, state, logical_mouse);
    } else {
        draw_ending(font, state);
    }

    if (!audio_enabled && state.has_session) {
        text(font, "静音", 586, 330, 18, red);
    }
    if (paused) {
        draw_pause_overlay(font, audio_enabled);
    }
}

}  // namespace pixel_town

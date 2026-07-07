#include "app/visual_prototype.hpp"

#include <array>
#include <string>

namespace pixel_town {
namespace {

constexpr Color ink{45, 52, 54, 255};
constexpr Color paper{250, 238, 203, 255};
constexpr Color cream{255, 248, 226, 255};
constexpr Color green{82, 137, 92, 255};
constexpr Color grass{144, 190, 119, 255};
constexpr Color red{183, 83, 72, 255};
constexpr Color gold{224, 169, 74, 255};
constexpr Color shadow{39, 48, 53, 120};

constexpr std::array<const char*, 4> location_names{"餐馆", "便利店", "图书馆", "酒馆"};
constexpr std::array<const char*, 3> variant_names{"A 地图优先", "B 日程侧栏", "C 地点卡片"};
constexpr int tiny_farm_columns = 12;
constexpr int tiny_farm_tile_size = 16;

void text(const Font& font, const char* value, float x, float y, float size, Color color = ink) {
    DrawTextEx(font, value, Vector2{x, y}, size, 1.0F, color);
}

void panel(Rectangle bounds, Color fill, Color border = ink) {
    DrawRectangleRec(Rectangle{bounds.x + 3, bounds.y + 3, bounds.width, bounds.height}, shadow);
    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, 2.0F, border);
}

bool clicked(Rectangle bounds, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, bounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void location_button(const Font& font, Rectangle bounds, const char* label, Color color,
                     Vector2 mouse) {
    const bool hovered = CheckCollisionPointRec(mouse, bounds);
    panel(bounds, hovered ? cream : color);
    text(font, label, bounds.x + 10, bounds.y + 8, 12, ink);
}

Rectangle tiny_farm_source(int tile_index) {
    return Rectangle{static_cast<float>((tile_index % tiny_farm_columns) * tiny_farm_tile_size),
                     static_cast<float>((tile_index / tiny_farm_columns) * tiny_farm_tile_size),
                     tiny_farm_tile_size, tiny_farm_tile_size};
}

void draw_tiny_farm_tile(const Texture2D& tiny_farm_tiles, int tile_index, Rectangle destination) {
    if (tiny_farm_tiles.id == 0) {
        return;
    }
    DrawTexturePro(tiny_farm_tiles, tiny_farm_source(tile_index), destination,
                   Vector2{0.0F, 0.0F}, 0.0F, WHITE);
}

void draw_status_bar(const Font& font, bool audio_enabled) {
    DrawRectangle(0, 0, 640, 43, Color{60, 79, 82, 255});
    text(font, "像素小镇 · 十日经营计划", 14, 12, 12, RAYWHITE);
    text(font, "第 3 天  白天", 250, 12, 12, Color{255, 224, 154, 255});
    text(font, "金钱 128  体力 72  声望 18", 362, 5, 12, RAYWHITE);
    text(font, "知识 9  心情 64", 362, 23, 12, RAYWHITE);
    if (!audio_enabled) {
        text(font, "静音", 594, 23, 12, Color{255, 183, 157, 255});
    }
}

void draw_map(const Font& font, const Texture2D& town_marker, const Texture2D& tiny_farm_tiles,
              Rectangle bounds, Vector2 mouse) {
    DrawRectangleRec(bounds, grass);
    DrawRectangle(static_cast<int>(bounds.x), static_cast<int>(bounds.y + bounds.height * 0.67F),
                  static_cast<int>(bounds.width), static_cast<int>(bounds.height * 0.14F),
                  Color{194, 170, 121, 255});
    DrawRectangle(static_cast<int>(bounds.x + bounds.width * 0.44F), static_cast<int>(bounds.y),
                  static_cast<int>(bounds.width * 0.052F),
                  static_cast<int>(bounds.height), Color{117, 180, 202, 255});
    if (tiny_farm_tiles.id != 0) {
        for (float x = bounds.x + 18; x < bounds.x + bounds.width - 18; x += 48) {
            draw_tiny_farm_tile(tiny_farm_tiles, 3, Rectangle{x, bounds.y + 17, 16, 16});
            draw_tiny_farm_tile(tiny_farm_tiles, 3,
                                Rectangle{x + 18, bounds.y + bounds.height - 38, 16, 16});
        }
        for (float y = bounds.y + bounds.height * 0.67F; y < bounds.y + bounds.height * 0.81F;
             y += 16) {
            for (float x = bounds.x + 6; x < bounds.x + bounds.width - 10; x += 16) {
                draw_tiny_farm_tile(tiny_farm_tiles, 48, Rectangle{x, y, 16, 16});
            }
        }
    }

    const float building_width = bounds.width * 0.18F;
    const float building_height = bounds.height * 0.24F;
    const std::array<Rectangle, 4> buildings{
        Rectangle{bounds.x + bounds.width * 0.04F, bounds.y + bounds.height * 0.13F,
                  building_width, building_height},
        Rectangle{bounds.x + bounds.width * 0.26F, bounds.y + bounds.height * 0.45F,
                  building_width, building_height},
        Rectangle{bounds.x + bounds.width * 0.55F, bounds.y + bounds.height * 0.12F,
                  building_width, building_height},
        Rectangle{bounds.x + bounds.width * 0.76F, bounds.y + bounds.height * 0.47F,
                  building_width, building_height},
    };
    const std::array<Color, 4> colors{Color{231, 151, 103, 255}, gold,
                                      Color{161, 169, 196, 255}, Color{181, 122, 104, 255}};
    const std::array<int, 4> tiny_icons{110, 98, 111, 115};
    for (std::size_t index = 0; index < buildings.size(); ++index) {
        const Rectangle building = buildings[index];
        DrawTriangle(Vector2{building.x - 6, building.y + 4},
                     Vector2{building.x + building.width + 6, building.y + 4},
                     Vector2{building.x + building.width / 2, building.y - 28}, red);
        location_button(font, building, location_names[index], colors[index], mouse);
        draw_tiny_farm_tile(tiny_farm_tiles, tiny_icons[index],
                            Rectangle{building.x + building.width - 28, building.y + 7, 16, 16});
    }
    DrawTextureEx(town_marker,
                  Vector2{bounds.x + bounds.width * 0.46F, bounds.y + bounds.height * 0.43F},
                  0.0F, 2.0F, WHITE);
    text(font, "中央河", bounds.x + bounds.width * 0.44F,
         bounds.y + bounds.height * 0.84F, 12, Color{36, 93, 122, 255});
}

void draw_variant_a(const Font& font, const Texture2D& town_marker, const Texture2D& tiny_farm_tiles,
                    bool audio_enabled, Vector2 mouse) {
    ClearBackground(Color{221, 211, 174, 255});
    draw_status_bar(font, audio_enabled);
    panel(Rectangle{14, 52, 612, 256}, paper);
    draw_map(font, town_marker, tiny_farm_tiles, Rectangle{22, 60, 596, 240}, mouse);
    panel(Rectangle{22, 268, 596, 30}, Color{65, 91, 89, 245});
    text(font, "今日提示：晴天，餐馆客流增加。请选择一个白天地点。", 34, 275, 12, RAYWHITE);
}

void draw_variant_b(const Font& font, const Texture2D& town_marker, const Texture2D& tiny_farm_tiles,
                    bool audio_enabled, Vector2 mouse) {
    ClearBackground(Color{215, 221, 194, 255});
    DrawRectangle(0, 0, 190, 360, Color{51, 67, 72, 255});
    text(font, "像素小镇", 18, 15, 24, RAYWHITE);
    text(font, "第 3 天 · 白天", 18, 47, 12, Color{255, 220, 142, 255});
    text(font, "今日安排", 18, 78, 12, RAYWHITE);
    const std::array<Color, 4> colors{Color{231, 151, 103, 255}, gold,
                                      Color{161, 169, 196, 255}, Color{181, 122, 104, 255}};
    for (int index = 0; index < 4; ++index) {
        location_button(font, Rectangle{16, 102.0F + index * 48.0F, 158, 37},
                        location_names[static_cast<std::size_t>(index)], colors[index], mouse);
    }
    text(font, "金钱 128", 18, 298, 12, RAYWHITE);
    text(font, "体力 72  声望 18", 18, 316, 12, RAYWHITE);
    if (!audio_enabled) {
        text(font, "当前静音", 104, 334, 12, Color{255, 183, 157, 255});
    }

    text(font, "小镇地图", 210, 12, 24, ink);
    text(font, "晴天 · 餐馆客流增加", 438, 20, 12, Color{78, 95, 84, 255});
    panel(Rectangle{204, 50, 420, 250}, paper);
    draw_map(font, town_marker, tiny_farm_tiles, Rectangle{212, 58, 404, 234}, mouse);
}

void draw_variant_c(const Font& font, const Texture2D& tiny_farm_tiles, bool audio_enabled,
                    Vector2 mouse) {
    ClearBackground(Color{37, 50, 57, 255});
    text(font, "今天想过怎样的小镇生活？", 20, 12, 24, RAYWHITE);
    text(font, "第 3 天 · 白天行动", 462, 20, 12, Color{255, 221, 145, 255});
    const std::array<const char*, 4> descriptions{
        "忙碌服务 · 收益稳定", "进货定价 · 策略经营", "整理图书 · 增长知识", "夜晚开放 · 风险娱乐"};
    const std::array<Color, 4> colors{Color{231, 151, 103, 255}, gold,
                                      Color{161, 169, 196, 255}, Color{181, 122, 104, 255}};
    const std::array<int, 4> tiny_icons{110, 98, 111, 115};
    for (int index = 0; index < 4; ++index) {
        const float x = 20.0F + (index % 2) * 306.0F;
        const float y = 58.0F + (index / 2) * 116.0F;
        const Rectangle card{x, y, 294, 98};
        panel(card, colors[static_cast<std::size_t>(index)]);
        DrawRectangle(static_cast<int>(x + 12), static_cast<int>(y + 12), 52, 52,
                      Color{245, 235, 199, 255});
        DrawRectangleLinesEx(Rectangle{x + 12, y + 12, 52, 52}, 2.0F, ink);
        draw_tiny_farm_tile(tiny_farm_tiles, tiny_icons[static_cast<std::size_t>(index)],
                            Rectangle{x + 30, y + 30, 16, 16});
        text(font, location_names[static_cast<std::size_t>(index)], x + 77, y + 12, 24, ink);
        text(font, descriptions[static_cast<std::size_t>(index)], x + 77, y + 44, 12, ink);
        text(font, index == 3 ? "夜晚可用" : "点击查看", x + 205, y + 72, 12, ink);
        if (CheckCollisionPointRec(mouse, card)) {
            DrawRectangleLinesEx(Rectangle{x + 3, y + 3, 288, 92}, 3.0F, cream);
        }
    }
    panel(Rectangle{20, 292, 600, 30}, Color{238, 226, 185, 255});
    text(font, "状态：金钱 128 · 体力 72 · 声望 18 · 知识 9 · 心情 64", 32, 301, 12, ink);
    if (!audio_enabled) {
        text(font, "静音", 570, 301, 12, red);
    }
}

void draw_modal(const Font& font, const VisualPrototypeState& state) {
    DrawRectangle(0, 0, 640, 360, Color{22, 27, 30, 135});
    panel(Rectangle{150, 88, 340, 175}, cream);
    text(font, "地点预览", 174, 110, 12, red);
    text(font, location_names[static_cast<std::size_t>(state.selected_location)], 174, 137, 24, ink);
    text(font, "这里展示玩法说明、体力消耗和预期收益。", 174, 177, 12, ink);
    panel(Rectangle{174, 215, 112, 30}, green);
    text(font, "进入体验", 194, 223, 12, RAYWHITE);
    panel(Rectangle{304, 215, 112, 30}, Color{211, 202, 174, 255});
    text(font, "稍后再说", 324, 223, 12, ink);
}

void draw_switcher(const Font& font, const VisualPrototypeState& state) {
    panel(Rectangle{167, 329, 306, 25}, Color{30, 38, 43, 245}, Color{240, 225, 179, 255});
    text(font, "<", 181, 334, 12, RAYWHITE);
    text(font, variant_names[static_cast<std::size_t>(state.variant)], 240, 334, 12,
         Color{255, 225, 151, 255});
    text(font, ">", 449, 334, 12, RAYWHITE);
    text(font, "P0 视觉原型", 18, 334, 12, Color{220, 224, 216, 255});
}

}  // namespace

const char* visual_prototype_glyphs() noexcept {
    return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 <>+-/·：，。"
           "像素小镇十日经营计划第天白天金钱体力声望知识心情静音餐馆便利店图书馆酒馆中央河"
           "今日提示晴天客流增加请选择一个地点安排当前地图忙碌服务收益稳定进货定价策略经营整理"
           "增长夜晚开放风险娱乐想过怎样的生活行动点击查看可用状态预览这里展示玩法说明消耗和预期"
           "进入体验稍后再说视觉原型";
}

void update_visual_prototype(VisualPrototypeState& state, Vector2 logical_mouse) {
    if (IsKeyPressed(KEY_LEFT)) {
        state.variant = (state.variant + 2) % 3;
        state.modal_open = false;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        state.variant = (state.variant + 1) % 3;
        state.modal_open = false;
    }
    if (clicked(Rectangle{167, 329, 45, 25}, logical_mouse)) {
        state.variant = (state.variant + 2) % 3;
        state.modal_open = false;
    }
    if (clicked(Rectangle{428, 329, 45, 25}, logical_mouse)) {
        state.variant = (state.variant + 1) % 3;
        state.modal_open = false;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        state.modal_open = false;
    }

    if (state.modal_open) {
        if (clicked(Rectangle{174, 215, 242, 30}, logical_mouse)) {
            state.modal_open = false;
        }
        return;
    }

    std::array<Rectangle, 4> targets{};
    if (state.variant == 0) {
        targets = {Rectangle{48, 91, 105, 58}, Rectangle{177, 167, 105, 58},
                   Rectangle{346, 88, 112, 61}, Rectangle{462, 172, 105, 58}};
    } else if (state.variant == 1) {
        targets = {Rectangle{16, 102, 158, 37}, Rectangle{16, 150, 158, 37},
                   Rectangle{16, 198, 158, 37}, Rectangle{16, 246, 158, 37}};
    } else {
        targets = {Rectangle{20, 58, 294, 98}, Rectangle{326, 58, 294, 98},
                   Rectangle{20, 174, 294, 98}, Rectangle{326, 174, 294, 98}};
    }
    for (int index = 0; index < 4; ++index) {
        if (clicked(targets[static_cast<std::size_t>(index)], logical_mouse)) {
            state.selected_location = index;
            state.modal_open = true;
        }
    }
}

void draw_visual_prototype(const Font& font, const Texture2D& town_marker,
                           const Texture2D& tiny_farm_tiles,
                           const VisualPrototypeState& state, bool audio_enabled,
                           Vector2 logical_mouse) {
    if (state.variant == 0) {
        draw_variant_a(font, town_marker, tiny_farm_tiles, audio_enabled, logical_mouse);
    } else if (state.variant == 1) {
        draw_variant_b(font, town_marker, tiny_farm_tiles, audio_enabled, logical_mouse);
    } else {
        draw_variant_c(font, tiny_farm_tiles, audio_enabled, logical_mouse);
    }
    if (state.modal_open) {
        draw_modal(font, state);
    }
    draw_switcher(font, state);
}

}  // namespace pixel_town

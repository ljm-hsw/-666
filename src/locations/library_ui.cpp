#include "locations/library_ui.hpp"

#include "locations/library_scene.hpp"

#include "ui/scene_viewport.hpp"
#include "ui/ui_metrics.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include <raylib.h>

namespace pixel_town::library::ui {

namespace {

constexpr Color ink{45, 52, 54, 255};
constexpr Color paper{250, 238, 203, 255};
constexpr Color cream{255, 248, 226, 255};
constexpr Color green{82, 137, 92, 255};
constexpr Color gold{224, 169, 74, 255};
constexpr Color slate{60, 79, 82, 255};
constexpr Color shadow{39, 48, 53, 120};
constexpr Color wood{139, 90, 43, 255};
constexpr Color shelf_wood{205, 133, 63, 255};
constexpr Color book_red{178, 34, 34, 255};
constexpr Color book_blue{70, 130, 180, 255};
constexpr Color book_green{34, 139, 34, 255};
constexpr Color book_yellow{255, 215, 0, 255};
constexpr Color book_purple{128, 0, 128, 255};
constexpr Color book_brown{139, 69, 19, 255};
constexpr Color book_orange{255, 165, 0, 255};
constexpr Color correct_color{34, 139, 34, 255};
constexpr Color wrong_color{178, 34, 34, 255};
constexpr Color faded_ink{100, 100, 100, 200};
constexpr Color plot_highlight{180, 150, 80, 255};
constexpr Color wall_color{240, 230, 210, 255};
constexpr Color window_frame{139, 90, 43, 255};
constexpr Color window_glass{173, 216, 230, 150};
constexpr Color plant_green{34, 139, 34, 255};
constexpr Color door_brown{139, 90, 43, 255};

float scaled(float value) {
    return std::round(value * ::pixel_town::ui::design_to_canvas_scale);
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

Rectangle indoor_scene_rectangle() {
    const auto viewport = ::pixel_town::ui::indoor_scene_viewport();
    return Rectangle{viewport.x, viewport.y, viewport.width, viewport.height};
}

[[maybe_unused]] Camera2D indoor_scene_camera() {
    const auto viewport = ::pixel_town::ui::indoor_scene_viewport();
    Camera2D camera{};
    camera.offset = Vector2{viewport.x, viewport.y};
    camera.target = Vector2{0.0F, 0.0F};
    camera.rotation = 0.0F;
    camera.zoom = ::pixel_town::ui::scene_viewport_scale;
    return camera;
}

[[maybe_unused]] Vector2 scene_design_mouse(Vector2 logical_mouse) {
    const auto transformed = ::pixel_town::ui::viewport_to_scene_design(
        {logical_mouse.x, logical_mouse.y});
    return Vector2{transformed.x, transformed.y};
}

void panel(Rectangle bounds, Color fill, Color border = ink) {
    const Rectangle scaled_bounds = scaled_rect(bounds);
    DrawRectangle(scaled_bounds.x + 4, scaled_bounds.y + 4, scaled_bounds.width, scaled_bounds.height, shadow);
    DrawRectangleRec(scaled_bounds, fill);
    DrawRectangleLinesEx(scaled_bounds, 3.0F, border);
}

void text(const Font& font, const std::string& value, float x, float y, float size, Color color = ink) {
    DrawTextEx(font, value.c_str(), Vector2{scaled(x), scaled(y)}, scaled_font_size(size), 1.0F, color);
}

void draw_text_wrapped(const Font& font, const std::string& text, float x, float y, float max_width, float line_height, float size, Color color = ink) {
    const float font_size = scaled_font_size(size);
    float current_x = scaled(x);
    float current_y = scaled(y);
    
    std::string line;
    for (char c : text) {
        if (c == '\n') {
            DrawTextEx(font, line.c_str(), Vector2{current_x, current_y}, font_size, 1.0F, color);
            line.clear();
            current_y += scaled(line_height);
            current_x = scaled(x);
            continue;
        }
        
        std::string test_line = line + c;
        Vector2 test_size = MeasureTextEx(font, test_line.c_str(), font_size, 1.0F);
        
        if (test_size.x > max_width && !line.empty()) {
            DrawTextEx(font, line.c_str(), Vector2{current_x, current_y}, font_size, 1.0F, color);
            line = c;
            current_y += scaled(line_height);
            current_x = scaled(x);
        } else {
            line += c;
        }
    }
    
    if (!line.empty()) {
        DrawTextEx(font, line.c_str(), Vector2{current_x, current_y}, font_size, 1.0F, color);
    }
}

void centered_text(const Font& font, const std::string& value, Rectangle bounds, float size, Color color = ink) {
    const Rectangle scaled_bounds = scaled_rect(bounds);
    const float font_size = scaled_font_size(size);
    const Vector2 measured = MeasureTextEx(font, value.c_str(), font_size, 1.0F);
    DrawTextEx(font, value.c_str(),
               Vector2{scaled_bounds.x + (scaled_bounds.width - measured.x) / 2.0F,
                       scaled_bounds.y + (scaled_bounds.height - measured.y) / 2.0F},
               font_size, 1.0F, color);
}

void draw_pixel_bookshelf(int x, int y, int width, int height) {
    constexpr int tile_size = 16;
    const int shelves = height / (tile_size * 2);
    const int books_per_shelf = width / tile_size;

    DrawRectangle(scaled(x), scaled(y), scaled(width), scaled(height), shelf_wood);

    for (int shelf_idx = 0; shelf_idx < shelves; ++shelf_idx) {
        int shelf_y = y + shelf_idx * tile_size * 2;
        DrawRectangle(scaled(x), scaled(shelf_y), scaled(width), scaled(4), wood);

        const Color book_colors[] = {book_red, book_blue, book_green, book_yellow, book_purple, book_brown, book_orange};
        for (int book_idx = 0; book_idx < books_per_shelf; ++book_idx) {
            int book_x = x + book_idx * tile_size;
            Color book_color = book_colors[(book_idx + shelf_idx * 2) % 7];
            DrawRectangle(scaled(book_x + 2), scaled(shelf_y + 6), scaled(tile_size - 4), scaled(tile_size - 8), book_color);
            DrawRectangle(scaled(book_x + tile_size - 2), scaled(shelf_y + 6), scaled(2), scaled(tile_size - 8), Color{0, 0, 0, 60});
        }
    }
}

void draw_category_button(int x, int y, int width, int height, const std::string& name, bool is_hovered, bool is_selected, const Font& font) {
    Color fill = is_selected ? green : (is_hovered ? cream : paper);
    Color text_color = is_selected ? RAYWHITE : ink;
    panel(Rectangle{static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)}, fill, ink);
    centered_text(font, name, Rectangle{static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)}, 18, text_color);
}

[[maybe_unused]] void draw_library_floor(const LibraryRenderConfig& config) {
    const float tile_size = scaled(16);
    const int cols = static_cast<int>(scaled(config.logical_width) / tile_size);
    const int rows = static_cast<int>(scaled(config.logical_height) / tile_size);
    
    const Color floor_light = Color{245, 238, 220, 255};
    const Color floor_dark = Color{235, 228, 210, 255};
    const Color floor_line = Color{220, 210, 190, 255};

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float x = col * tile_size;
            float y = row * tile_size;
            
            Color tile_color = ((row / 2) + (col / 2)) % 2 == 0 ? floor_light : floor_dark;
            DrawRectangle(x, y, tile_size, tile_size, tile_color);
        }
    }
    
    for (int col = 0; col <= cols; ++col) {
        float x = col * tile_size;
        DrawRectangle(x, 0, scaled(1), scaled(config.logical_height), floor_line);
    }
    for (int row = 0; row <= rows; ++row) {
        float y = row * tile_size;
        DrawRectangle(0, y, scaled(config.logical_width), scaled(1), floor_line);
    }
}

[[maybe_unused]] void draw_library_walls(const LibraryRenderConfig& config) {
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(80), wall_color);
}

void draw_library_backdrop(const LibraryRenderConfig& config) {
    ClearBackground(Color{221, 211, 174, 255});
    if (config.background.id == 0) {
        return;
    }
    DrawTexturePro(
        config.background,
        Rectangle{0.0F, 0.0F, static_cast<float>(config.background.width),
                  static_cast<float>(config.background.height)},
        indoor_scene_rectangle(),
        Vector2{0.0F, 0.0F}, 0.0F, WHITE);
}

[[maybe_unused]] void draw_scene_element(const SceneElement& element) {
    const Vector2 pos = scaled_point(element.position);
    const float w = scaled(element.width);
    const float h = scaled(element.height);

    switch (element.type) {
        case SceneElementType::bookshelf: {
            const Color wood_dark = Color{80, 50, 20, 255};
            const Color wood_medium = Color{139, 90, 43, 255};
            const Color shelf_bottom = Color{60, 35, 15, 255};
            
            const float side_width = scaled(6);
            const float shelf_height = scaled(4);
            const int num_shelves = 5;
            const float shelf_spacing = (h - side_width * 2) / num_shelves;
            
            DrawRectangle(pos.x, pos.y, w, h, wood_medium);
            
            DrawRectangle(pos.x, pos.y, side_width, h, wood_dark);
            DrawRectangle(pos.x + w - side_width, pos.y, side_width, h, wood_dark);
            
            DrawRectangle(pos.x, pos.y, w, side_width, wood_dark);
            DrawRectangle(pos.x, pos.y + h - side_width, w, side_width, wood_dark);
            
            for (int i = 0; i <= num_shelves; ++i) {
                float shelf_y = pos.y + side_width + i * shelf_spacing;
                DrawRectangle(pos.x + side_width, shelf_y, w - side_width * 2, shelf_height, wood_dark);
                DrawRectangle(pos.x + side_width, shelf_y + shelf_height, w - side_width * 2, scaled(2), shelf_bottom);
            }
            
            const Color book_colors[] = {
                Color{178, 34, 34, 255}, Color{70, 130, 180, 255}, Color{34, 139, 34, 255},
                Color{255, 215, 0, 255}, Color{128, 0, 128, 255}, Color{255, 140, 0, 255},
                Color{192, 192, 192, 255}, Color{60, 60, 60, 255}, Color{255, 99, 71, 255},
                Color{100, 149, 237, 255}, Color{205, 92, 92, 255}, Color{65, 105, 225, 255},
            };
            
            for (int shelf_idx = 0; shelf_idx < num_shelves; ++shelf_idx) {
                float shelf_y = pos.y + side_width + shelf_idx * shelf_spacing;
                float book_start_y = shelf_y - scaled(2);
                
                float current_x = pos.x + side_width + scaled(4);
                float max_x = pos.x + w - side_width - scaled(4);
                
                while (current_x < max_x) {
                    int color_idx = (shelf_idx * 5 + static_cast<int>(current_x / scaled(8))) % 12;
                    float book_width = scaled(6) + (scaled(color_idx % 3) * 2);
                    float book_height = shelf_spacing - shelf_height - scaled(6) - scaled(color_idx % 4);
                    
                    if (current_x + book_width > max_x) {
                        book_width = max_x - current_x;
                    }
                    
                    DrawRectangle(current_x, book_start_y - book_height, book_width, book_height, book_colors[color_idx]);
                    float spine_line_x = current_x + book_width - scaled(1);
                    DrawRectangle(spine_line_x, book_start_y - book_height, scaled(1), book_height, Color{0, 0, 0, 50});
                    DrawRectangle(current_x, book_start_y - book_height, book_width, scaled(1), Color{255, 255, 255, 30});
                    
                    current_x += book_width + scaled(1);
                }
            }
            break;
        }
        case SceneElementType::plant: {
            DrawRectangle(pos.x + scaled(5), pos.y + scaled(20), scaled(20), scaled(20), plant_green);
            DrawRectangle(pos.x + scaled(10), pos.y + scaled(10), scaled(10), scaled(15), plant_green);
            DrawRectangle(pos.x + scaled(12), pos.y, scaled(6), scaled(10), Color{139, 90, 43, 255});
            break;
        }
        case SceneElementType::window: {
            DrawRectangle(pos.x, pos.y, w, h, window_frame);
            DrawRectangle(pos.x + scaled(5), pos.y + scaled(5), w - scaled(10), h - scaled(10), window_glass);
            DrawRectangle(pos.x + w / 2, pos.y, scaled(5), h, window_frame);
            DrawRectangle(pos.x, pos.y + h / 2, w, scaled(5), window_frame);
            break;
        }
        case SceneElementType::door: {
            DrawRectangle(pos.x, pos.y, w, h, door_brown);
            DrawCircle(pos.x + w - scaled(10), pos.y + scaled(20), scaled(3), Color{255, 215, 0, 255});
            break;
        }
        default:
            break;
    }
}

[[maybe_unused]] void draw_npc_sprite(const NpcState& state, const NpcData& data,
                                      bool is_hovered, const Font& font) {
    const Vector2 pos = scaled_point(state.position);
    
    DrawCircle(pos.x, pos.y + scaled(15), scaled(18), is_hovered ? Color{255, 255, 200, 100} : Color{0, 0, 0, 30});
    
    const Color skin = Color{255, 220, 180, 255};
    const Color hair = data.id == "librarian" ? Color{80, 50, 30, 255} : Color{139, 90, 43, 255};
    const Color clothes = data.id == "librarian" ? Color{60, 100, 160, 255} : Color{70, 130, 180, 255};
    const Color pants = Color{40, 60, 100, 255};
    const Color shoe = Color{30, 30, 30, 255};
    
    const float head_size = scaled(10);
    const float body_width = scaled(14);
    const float body_height = scaled(16);
    const float leg_width = scaled(5);
    const float leg_height = scaled(12);
    
    DrawRectangle(pos.x - leg_width, pos.y + body_height, leg_width, leg_height, pants);
    DrawRectangle(pos.x, pos.y + body_height, leg_width, leg_height, pants);
    
    DrawRectangle(pos.x - leg_width - scaled(2), pos.y + body_height + leg_height, leg_width + scaled(2), scaled(3), shoe);
    DrawRectangle(pos.x + scaled(2), pos.y + body_height + leg_height, leg_width + scaled(2), scaled(3), shoe);
    
    DrawRectangle(pos.x - body_width / 2, pos.y + scaled(4), body_width, body_height, clothes);
    
    DrawRectangle(pos.x - body_width / 2, pos.y + scaled(4), scaled(3), body_height, Color{0, 0, 0, 30});
    
    DrawRectangle(pos.x - body_width / 2 - scaled(3), pos.y + scaled(8), scaled(4), scaled(10), clothes);
    DrawRectangle(pos.x + body_width / 2 - scaled(1), pos.y + scaled(8), scaled(4), scaled(10), clothes);
    
    DrawCircle(pos.x, pos.y - scaled(4), head_size, skin);
    
    DrawCircle(pos.x, pos.y - scaled(8), head_size * 0.7F, hair);
    
    DrawCircle(pos.x - scaled(3), pos.y - scaled(5), scaled(2), Color{45, 52, 54, 255});
    DrawCircle(pos.x + scaled(3), pos.y - scaled(5), scaled(2), Color{45, 52, 54, 255});
    
    DrawCircle(pos.x - scaled(2.5F), pos.y - scaled(4.5F), scaled(0.8F), WHITE);
    DrawCircle(pos.x + scaled(2.5F), pos.y - scaled(4.5F), scaled(0.8F), WHITE);
    
    DrawEllipse(pos.x, pos.y + scaled(1), scaled(2), scaled(1.5F), Color{200, 100, 100, 255});
    
    if (data.id == "librarian") {
        DrawRectangle(pos.x - head_size - scaled(2), pos.y - scaled(6), scaled(3), scaled(6), Color{200, 180, 150, 255});
        DrawRectangle(pos.x + head_size - scaled(1), pos.y - scaled(6), scaled(3), scaled(6), Color{200, 180, 150, 255});
        DrawRectangle(pos.x - head_size - scaled(2), pos.y - scaled(6), scaled(7), scaled(2), Color{200, 180, 150, 255});
    }
    
    // 在NPC头上方显示交互提示框
    const float box_width = scaled(80);
    const float box_height = scaled(24);
    const float box_x = pos.x - box_width / 2;
    const float box_y = pos.y - scaled(35) - box_height;
    
    // 提示框背景
    Color box_bg = Color{255, 248, 220, 230};
    Color box_border = Color{180, 150, 100, 255};
    
    if (is_hovered) {
        box_bg = Color{255, 240, 200, 250};
        box_border = Color{200, 170, 80, 255};
    }
    
    DrawRectangle(box_x, box_y, box_width, box_height, Color{0, 0, 0, 40});
    DrawRectangle(box_x + scaled(2), box_y + scaled(2), box_width - scaled(4), box_height - scaled(4), box_bg);
    DrawRectangleLinesEx(Rectangle{box_x + scaled(2), box_y + scaled(2), box_width - scaled(4), box_height - scaled(4)}, scaled(2), box_border);
    
    // 提示文字
    const std::string hint_text = "点击对话";
    const float text_x = box_x + box_width / 2 - scaled(24);
    const float text_y = box_y + scaled(6);
    DrawTextEx(font, hint_text.c_str(), Vector2{text_x, text_y}, scaled_font_size(12), 1.0F, Color{80, 60, 40, 255});
}

[[maybe_unused]] void draw_room_ui(const LibraryRenderConfig& config,
                                   const Font& font) {
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(40), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);
    
    text(font, "点击NPC进行互动", 10, 330, 14, faded_ink);
}

[[maybe_unused]] void draw_transition_effect(float progress,
                                             const LibraryRenderConfig& config) {
    const unsigned char fade_alpha = static_cast<unsigned char>(progress * 255.0F);
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height), 
                  Color{0, 0, 0, fade_alpha});
}

void draw_intro_screen(const LibraryReaderPresentation& presentation,
                       const LibraryRenderConfig& config, const Font& font,
                       Vector2 logical_mouse) {
    draw_library_backdrop(config);

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height),
                  config.background.id == 0 ? slate : Color{37, 50, 57, 78});
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(40), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    if (config.background.id == 0) {
        draw_pixel_bookshelf(30, 80, 160, 120);
        draw_pixel_bookshelf(220, 80, 200, 140);
        draw_pixel_bookshelf(450, 80, 160, 120);

        DrawRectangle(scaled(300), scaled(100), scaled(40), scaled(80), Color{139, 90, 43, 255});
        DrawRectangle(scaled(295), scaled(175), scaled(50), scaled(10), Color{100, 60, 30, 255});
        DrawCircle(scaled(320), scaled(115), scaled(12), Color{255, 220, 180, 255});
    }

    panel(Rectangle{50, 202, 540, 104}, paper);
    
    text(font, presentation.welcome_message, 70, 216, 20, ink);
    draw_text_wrapped(font, presentation.work_intro, 70, 244, scaled(500), 20,
                      14, ink);

    const int btn_y = 314;
    const int btn_w = 200;
    const int btn_h = 34;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "开始工作", hovered, false, font);
}

void draw_npc_talk_screen(const LibraryReaderPresentation& presentation,
                          const LibraryRenderConfig& config, const Font& font,
                          Vector2 logical_mouse) {
    draw_library_backdrop(config);

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height),
                  config.background.id == 0 ? slate : Color{37, 50, 57, 78});
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(40), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    if (config.background.id == 0) {
        draw_pixel_bookshelf(30, 80, 160, 120);
        draw_pixel_bookshelf(220, 80, 200, 140);
        draw_pixel_bookshelf(450, 80, 160, 120);

        DrawRectangle(scaled(300), scaled(100), scaled(40), scaled(80), Color{139, 90, 43, 255});
        DrawRectangle(scaled(295), scaled(175), scaled(50), scaled(10), Color{100, 60, 30, 255});
        DrawCircle(scaled(320), scaled(115), scaled(12), Color{255, 220, 180, 255});
    }

    const std::string& dialogue = presentation.interaction.current_dialogue.empty()
                                      ? presentation.introduction_dialogue
                                      : presentation.interaction.current_dialogue;

    panel(Rectangle{50, 184, 540, 108}, paper);
    text(font, "管理员说：", 70, 199, 18, ink);
    draw_text_wrapped(font, dialogue, 70, 224, scaled(500), 20, 14, ink);

    const int btn_y = 312;
    const int btn_w = 200;
    const int btn_h = 34;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "继续", hovered, false, font);
}

void draw_plot_event_screen(const LibraryReaderPresentation& presentation,
                            const LibraryRenderConfig& config, const Font& font,
                            Vector2 logical_mouse) {
    draw_library_backdrop(config);

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height), slate);
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(40), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    if (config.background.id == 0) {
        draw_pixel_bookshelf(30, 80, 160, 120);
        draw_pixel_bookshelf(450, 80, 160, 120);
    }

    const auto& interaction = presentation.interaction;
    
    panel(Rectangle{50, 80, 540, 200}, plot_highlight);
    text(font, "【剧情事件】", 70, 95, 24, gold);
    draw_text_wrapped(font, interaction.current_plot_title, 70, 125, scaled(500), 36, 28, ink);
    
    panel(Rectangle{70, 170, 500, 80}, paper);
    draw_text_wrapped(font, interaction.current_plot_description, 90, 185, scaled(460), 24, 16, ink);

    const int btn_y = 312;
    const int btn_w = 200;
    const int btn_h = 34;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "了解详情", hovered, false, font);
}

void draw_map_reveal_screen(const LibraryReaderPresentation& presentation,
                            const LibraryRenderConfig& config, const Font& font,
                            Vector2 logical_mouse) {
    draw_library_backdrop(config);

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height), slate);
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(40), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    if (config.background.id == 0) {
        draw_pixel_bookshelf(30, 80, 160, 120);
        draw_pixel_bookshelf(450, 80, 160, 120);
    }

    panel(Rectangle{50, 80, 540, 180}, Color{240, 230, 210, 255});
    DrawRectangle(scaled(70), scaled(100), scaled(500), scaled(140), Color{255, 250, 240, 255});
    DrawRectangleLinesEx(scaled_rect(Rectangle{70, 100, 500, 140}), 3.0F, Color{100, 80, 60, 255});

    DrawLine(scaled(120), scaled(130), scaled(180), scaled(130), Color{150, 50, 50, 200});
    DrawLine(scaled(120), scaled(150), scaled(200), scaled(150), Color{50, 100, 150, 200});
    DrawLine(scaled(120), scaled(170), scaled(220), scaled(170), Color{150, 100, 50, 200});
    DrawLine(scaled(120), scaled(190), scaled(160), scaled(190), Color{50, 150, 100, 200});
    DrawLine(scaled(280), scaled(130), scaled(380), scaled(130), Color{150, 50, 50, 200});
    DrawLine(scaled(280), scaled(150), scaled(420), scaled(150), Color{50, 100, 150, 200});
    DrawLine(scaled(280), scaled(170), scaled(360), scaled(170), Color{150, 100, 50, 200});
    DrawLine(scaled(280), scaled(190), scaled(320), scaled(190), Color{50, 150, 100, 200});

    DrawCircle(scaled(150), scaled(125), scaled(4), Color{255, 0, 0, 200});
    DrawCircle(scaled(330), scaled(125), scaled(4), Color{255, 0, 0, 200});
    DrawCircle(scaled(150), scaled(195), scaled(4), Color{255, 0, 0, 200});
    DrawCircle(scaled(330), scaled(195), scaled(4), Color{255, 0, 0, 200});

    text(font, "旧地图", 270, 85, 24, Color{100, 60, 30, 255});

    panel(Rectangle{50, 254, 540, 50}, paper);
    draw_text_wrapped(font, presentation.old_map_reveal_dialogue, 70, 266,
                      scaled(500), 18, 14, ink);

    const int btn_y = 314;
    const int btn_w = 200;
    const int btn_h = 34;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "收下地图", hovered, false, font);
}

void draw_answering_screen(const LibraryReaderPresentation& presentation,
                           const LibraryUIState& ui_state,
                           const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    draw_library_backdrop(config);

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(40), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    const SessionState& session = presentation.state;
    char progress[32];
    snprintf(progress, sizeof(progress), "问题 %d/%d", session.current_question_index + 1,
             static_cast<int>(session.answers.size()) + (session.is_active ? 1 : 0));
    text(font, progress, 10, 70, 18, ink);

    if (session.current_combo > 1) {
        char combo[32];
        snprintf(combo, sizeof(combo), "连续正确: %d!", session.current_combo);
        text(font, combo, config.logical_width - 120, 70, 16, gold);
    }

    if (!presentation.current_question.has_value()) {
        return;
    }
    const ReaderQuestion& question = *presentation.current_question;

    panel(Rectangle{50, 100, 540, 70}, paper);
    text(font, "读者问：", 70, 110, 18, ink);
    draw_text_wrapped(font, question.question, 70, 135, scaled(500), 20, 14, ink);

    if (ui_state.show_hint && !question.hint.empty()) {
        panel(Rectangle{50, 175, 540, 34}, Color{245, 240, 220, 255});
        text(font, "提示：" + question.hint, 70, 184, 14, faded_ink);
    }

    const int btn_w = 100;
    const int btn_h = ui_state.show_hint ? 28 : 30;
    const int btn_x = ui_state.show_hint ? 470 : config.logical_width / 2 - btn_w / 2;
    const int btn_y = ui_state.show_hint ? 178 : 180;

    bool hint_hovered = CheckCollisionPointRec(logical_mouse,
                                               Rectangle{static_cast<float>(btn_x),
                                                         static_cast<float>(btn_y),
                                                         static_cast<float>(btn_w),
                                                         static_cast<float>(btn_h)});

    Color hint_fill = hint_hovered ? cream : Color{245, 240, 220, 255};
    panel(Rectangle{static_cast<float>(btn_x), static_cast<float>(btn_y), static_cast<float>(btn_w), static_cast<float>(btn_h)}, hint_fill, ink);
    centered_text(font, ui_state.show_hint ? "隐藏提示" : "查看提示", Rectangle{static_cast<float>(btn_x), static_cast<float>(btn_y), static_cast<float>(btn_w), static_cast<float>(btn_h)}, 14, ink);

    panel(Rectangle{50, 218, 540, 112}, paper);
    text(font, "选择书籍类别：", 70, 230, 18, ink);

    const std::vector<BookCategory>& categories = presentation.categories;
    const int cols = 4;
    const int cat_btn_w = 120;
    const int cat_btn_h = 30;
    const int gap = 10;
    const int start_x = (config.logical_width - (cols * cat_btn_w + (cols - 1) * gap)) / 2;
    const int start_y = 252;

    for (size_t i = 0; i < categories.size(); ++i) {
        const int row = static_cast<int>(i / cols);
        const int col = static_cast<int>(i % cols);
        const int x = start_x + col * (cat_btn_w + gap);
        const int y = start_y + row * (cat_btn_h + gap);

        bool hovered = CheckCollisionPointRec(
            logical_mouse,
            Rectangle{static_cast<float>(x), static_cast<float>(y), static_cast<float>(cat_btn_w),
                      static_cast<float>(cat_btn_h)});

        draw_category_button(x, y, cat_btn_w, cat_btn_h, categories[i].name, hovered,
                             ui_state.selected_category_id == categories[i].id, font);
    }

    const Rectangle instruction_panel{
        50.0F, static_cast<float>(config.logical_height - 28), 250.0F, 22.0F};
    panel(instruction_panel, Color{46, 58, 57, 232});
    text(font, "按 I 重看说明 / ESC 放弃工作", 60,
         config.logical_height - 24, 12, RAYWHITE);
}

void draw_feedback_screen(const LibraryUIState& ui_state,
                          const LibraryRenderConfig& config, const Font& font,
                          Vector2 logical_mouse) {
    (void)logical_mouse;
    draw_library_backdrop(config);

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(40), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    const Color feedback_color = ui_state.last_answer_correct ? correct_color : wrong_color;
    const char* feedback_text = ui_state.last_answer_correct ? "回答正确！" : "回答错误";
    
    panel(Rectangle{config.logical_width / 2.0F - 140.0F, 100.0F, 280.0F, 160.0F}, ui_state.last_answer_correct ? Color{230, 245, 230, 255} : Color{245, 230, 230, 255});
    text(font, feedback_text, config.logical_width / 2 - 70, 130, 32, feedback_color);

    const auto& feedback = ui_state.feedback_data;
    
    if (feedback.is_correct && !feedback.feedback_correct.empty()) {
        draw_text_wrapped(font, feedback.feedback_correct, config.logical_width / 2 - 120, 170, scaled(240), 24, 16, ink);
    } else if (!feedback.is_correct && !feedback.feedback_wrong.empty()) {
        draw_text_wrapped(font, feedback.feedback_wrong, config.logical_width / 2 - 120, 170, scaled(240), 24, 16, ink);
    }

    if (!feedback.is_correct && !feedback.correct_category_name.empty()) {
        std::string hint = "正确答案是：" + feedback.correct_category_name;
        text(font, hint.c_str(), config.logical_width / 2 - 90, 215, 20, ink);
    }

    if (feedback.is_correct && feedback.knowledge_reward > 0) {
        char knowledge_text[32];
        snprintf(knowledge_text, sizeof(knowledge_text), "获得知识 +%d", feedback.knowledge_reward);
        text(font, knowledge_text, config.logical_width / 2 - 70, 235, 18, gold);
    }

    text(font, "继续下一题...", config.logical_width / 2 - 70, 275, 18, Color{100, 100, 100, 255});
}

void draw_summary_screen(const LibraryWorkResult& result,
                         const LibraryRenderConfig& config, const Font& font,
                         Vector2 logical_mouse) {
    draw_library_backdrop(config);

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(40), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    panel(Rectangle{50, 80, 540, 240}, paper);
    text(font, "图书馆工作完成", config.logical_width / 2 - 100, 95, 24, ink);

    draw_text_wrapped(font, result.summary, 70, 130, scaled(500), 24, 16, ink);

    std::vector<std::pair<std::string, Color>> delta_lines;
    auto add_delta = [&delta_lines](const char* label, int value, Color gain_color) {
        if (value == 0) {
            return;
        }
        char line[64];
        snprintf(line, sizeof(line), "%s: %+d", label, value);
        delta_lines.emplace_back(line, value < 0 ? wrong_color : gain_color);
    };
    add_delta("体力", result.stamina_change, correct_color);
    add_delta("金钱", result.money_change, gold);
    add_delta("知识", result.knowledge_change, correct_color);
    add_delta("声望", result.reputation_change, gold);
    add_delta("心情", result.mood_change, correct_color);

    int y = 188;
    for (std::size_t index = 0; index < delta_lines.size(); ++index) {
        const int x = index % 2 == 0 ? 70 : 320;
        const int line_y = y + static_cast<int>(index / 2) * 26;
        text(font, delta_lines[index].first, x, line_y, 18, delta_lines[index].second);
    }
    if (!delta_lines.empty()) {
        y += static_cast<int>((delta_lines.size() + 1) / 2) * 26 + 4;
    }

    if (result.plot_triggered) {
        panel(Rectangle{50.0F, static_cast<float>(y) + 5.0F, 540.0F, 70.0F}, plot_highlight);
        text(font, "【剧情触发】", 70, y + 15, 16, gold);
        text(font, result.plot_title.c_str(), 170, y + 15, 16, ink);
        draw_text_wrapped(font, result.plot_description, 70, y + 35, scaled(500), 22, 14, faded_ink);
        y += 80;
    }

    if (!result.narrative_echo.empty()) {
        panel(Rectangle{50.0F, static_cast<float>(y) + 5.0F, 540.0F, 42.0F}, Color{245, 240, 220, 255});
        draw_text_wrapped(font, result.narrative_echo, 70, y + 15, scaled(500), 18, 14, faded_ink);
        y += 60;
    }

    const int btn_y = config.logical_height - 60;
    const int btn_w = 200;
    const int btn_h = 40;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "返回地图", hovered, false, font);
}

}  // namespace

void draw_library_scene(const LibraryReaderPresentation& presentation,
                        const LibraryUIState& ui_state,
                        const LibraryRenderConfig& render_config, const Font& font,
                        Vector2 logical_mouse) {
    const Vector2 library_mouse = {
        logical_mouse.x / ::pixel_town::ui::design_to_canvas_scale,
        logical_mouse.y / ::pixel_town::ui::design_to_canvas_scale};

    switch (ui_state.scene_state) {
        case LibrarySceneState::intro:
            draw_intro_screen(presentation, render_config, font, library_mouse);
            break;
        case LibrarySceneState::npc_talk:
            draw_npc_talk_screen(presentation, render_config, font, library_mouse);
            break;
        case LibrarySceneState::plot_event:
            draw_plot_event_screen(presentation, render_config, font, library_mouse);
            break;
        case LibrarySceneState::answering:
            draw_answering_screen(presentation, ui_state, render_config, font,
                                  library_mouse);
            break;
        case LibrarySceneState::feedback:
            draw_feedback_screen(ui_state, render_config, font, library_mouse);
            break;
        case LibrarySceneState::map_reveal:
            draw_map_reveal_screen(presentation, render_config, font, library_mouse);
            break;
        case LibrarySceneState::summary: {
            draw_summary_screen(presentation.result, render_config, font,
                                library_mouse);
            break;
        }
        case LibrarySceneState::exit:
            break;
    }

}

void update_library_ui(const LibraryReaderPresentation& presentation,
                       LibraryUIState& ui_state) {
    if (ui_state.scene_state == LibrarySceneState::feedback) {
        ui_state.feedback_timer++;
        if (ui_state.feedback_timer >= 60) {
            ui_state.feedback_timer = 0;
            if (presentation.state.is_active) {
                ui_state.scene_state = LibrarySceneState::answering;
            } else {
                ui_state.scene_state = LibrarySceneState::summary;
            }
        }
    }
}

LibraryIntent handle_library_input(const LibraryReaderPresentation& presentation,
                                   LibraryUIState& ui_state,
                                   Vector2 logical_mouse) {
    const Vector2 library_mouse = {
        logical_mouse.x / ::pixel_town::ui::design_to_canvas_scale,
        logical_mouse.y / ::pixel_town::ui::design_to_canvas_scale};

    if (ui_state.scene_state == LibrarySceneState::intro) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            advance_from_intro(ui_state);
            return {};
        }
    } else if (ui_state.scene_state == LibrarySceneState::npc_talk) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (presentation.pending_plot_event) {
                ui_state.scene_state = LibrarySceneState::plot_event;
            } else if (presentation.should_reveal_map) {
                ui_state.scene_state = LibrarySceneState::map_reveal;
            } else {
                ui_state.scene_state = LibrarySceneState::answering;
            }
            return {};
        }
    } else if (ui_state.scene_state == LibrarySceneState::plot_event) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::answering;
            return {};
        }
    } else if (ui_state.scene_state == LibrarySceneState::answering) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            return {LibraryIntentType::abandon, {}};
        }

        if (IsKeyPressed(KEY_I)) {
            request_instruction_review(ui_state);
            return {};
        }

        const int hint_btn_w = 100;
        const int hint_btn_h = ui_state.show_hint ? 28 : 30;
        const int hint_btn_x = ui_state.show_hint ? 470 : 640 / 2 - hint_btn_w / 2;
        const int hint_btn_y = ui_state.show_hint ? 178 : 180;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(
                    library_mouse,
                    Rectangle{static_cast<float>(hint_btn_x), static_cast<float>(hint_btn_y),
                              static_cast<float>(hint_btn_w), static_cast<float>(hint_btn_h)})) {
                ui_state.show_hint = !ui_state.show_hint;
                return {};
            }

            const std::vector<BookCategory>& categories = presentation.categories;
            const int cols = 4;
            const int btn_w = 120;
            const int btn_h = 30;
            const int gap = 10;
            const int start_x =
                (640 - (cols * btn_w + (cols - 1) * gap)) / 2;
            const int start_y = 252;

            for (size_t i = 0; i < categories.size(); ++i) {
                const int row = static_cast<int>(i / cols);
                const int col = static_cast<int>(i % cols);
                const int x = start_x + col * (btn_w + gap);
                const int y = start_y + row * (btn_h + gap);

                if (CheckCollisionPointRec(
                        library_mouse,
                        Rectangle{static_cast<float>(x), static_cast<float>(y),
                                  static_cast<float>(btn_w), static_cast<float>(btn_h)})) {
                    if (!presentation.current_question.has_value()) {
                        return {};
                    }
                    const auto& current_question = *presentation.current_question;
                    std::string correct_name;
                    for (const auto& cat : presentation.categories) {
                        if (cat.id == current_question.correct_category_id) {
                            correct_name = cat.name;
                            break;
                        }
                    }
                    
                    ui_state.feedback_data.question = current_question.question;
                    ui_state.feedback_data.feedback_correct = current_question.feedback_correct;
                    ui_state.feedback_data.feedback_wrong = current_question.feedback_wrong;
                    ui_state.feedback_data.correct_category_name = correct_name;
                    ui_state.feedback_data.knowledge_reward =
                        presentation.correct_knowledge_reward;

                    ui_state.selected_category_id = categories[i].id;
                    ui_state.last_answer_correct =
                        categories[i].id == current_question.correct_category_id;
                    ui_state.feedback_data.is_correct = ui_state.last_answer_correct;
                    ui_state.scene_state = LibrarySceneState::feedback;
                    return {LibraryIntentType::answer_category, categories[i].id};
                }
            }
        }
    } else if (ui_state.scene_state == LibrarySceneState::map_reveal) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::answering;
            return {};
        }
    } else if (ui_state.scene_state == LibrarySceneState::summary) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return {LibraryIntentType::finish_reader, {}};
        }
    }

    return {};
}

}  // namespace pixel_town::library::ui

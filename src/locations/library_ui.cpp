#include "locations/library_ui.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include <raylib.h>

namespace pixel_town::library::ui {

namespace {

constexpr float native_ui_scale = 1.5F;

constexpr int shelf_design_w = 70;
constexpr int shelf_design_h = 90;
constexpr int shelf_design_gap = 12;
constexpr int shelves_per_row = 4;
constexpr int shelf_row1_design_y = 80;
constexpr int shelf_row_gap = 15;

constexpr int book_design_w = 50;
constexpr int book_design_h = 12;
constexpr int book_grid_unit = 20;
constexpr int book_grid_offset_x = 40;
constexpr int book_grid_offset_y = 120;

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
constexpr Color floor_color{221, 211, 174, 255};
constexpr Color wall_color{240, 230, 210, 255};
constexpr Color window_frame{139, 90, 43, 255};
constexpr Color window_glass{173, 216, 230, 150};
constexpr Color plant_green{34, 139, 34, 255};
constexpr Color lamp_gold{255, 215, 0, 255};
constexpr Color rug_color{139, 69, 19, 180};
constexpr Color door_brown{139, 90, 43, 255};
constexpr Color npc_skin{255, 220, 180, 255};
constexpr Color npc_clothes{70, 130, 180, 255};
constexpr Color npc_hair{139, 90, 43, 255};

float scaled(float value) {
    return std::round(value * native_ui_scale);
}

void draw_library_floor(const LibraryRenderConfig& config);
void draw_library_walls(const LibraryRenderConfig& config);

Texture2D get_library_background() {
    static Texture2D texture = {0};
    static bool loaded = false;
    if (!loaded) {
        if (FileExists("assets/textures/imagegen_backgrounds/library_interior.png")) {
            texture = LoadTexture("assets/textures/imagegen_backgrounds/library_interior.png");
            SetTextureFilter(texture, TEXTURE_FILTER_POINT);
        }
        loaded = true;
    }
    return texture;
}

void draw_library_background(const LibraryRenderConfig& config) {
    Texture2D bg = get_library_background();
    if (bg.id != 0) {
        const float dest_w = static_cast<float>(config.logical_width);
        const float dest_h = static_cast<float>(config.logical_height);
        DrawTexturePro(bg,
                       Rectangle{0.0F, 0.0F, static_cast<float>(bg.width), static_cast<float>(bg.height)},
                       Rectangle{0.0F, 0.0F, dest_w, dest_h},
                       Vector2{0.0F, 0.0F}, 0.0F, WHITE);
    } else {
        draw_library_floor(config);
        draw_library_walls(config);
    }
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

struct ShelfLayout {
    int x;
    int y;
    int width;
    int height;
};

std::vector<ShelfLayout> compute_shelf_layouts(int logical_width, size_t shelf_count) {
    std::vector<ShelfLayout> layouts;
    if (shelf_count == 0) {
        return layouts;
    }

    const int predefined_coords[6][2] = {
        {507, 79},
        {628, 79},
        {748, 79},
        {507, 140},
        {628, 140},
        {748, 140}
    };

    layouts.reserve(shelf_count);
    for (size_t i = 0; i < shelf_count && i < 6; ++i) {
        ShelfLayout layout;
        layout.x = predefined_coords[i][0];
        layout.y = predefined_coords[i][1];
        layout.width = shelf_design_w;
        layout.height = shelf_design_h;
        layouts.push_back(layout);
    }
    return layouts;
}

Rectangle book_bounds(const Book& book) {
    return Rectangle{static_cast<float>(book.x),
                     static_cast<float>(book.y),
                     static_cast<float>(book_design_w),
                     static_cast<float>(book_design_h)};
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
    const float scaled_max_width = scaled(max_width);
    const float scaled_line_height = scaled(line_height);
    const float start_x = scaled(x);
    float current_x = start_x;
    float current_y = scaled(y);
    
    std::string line;
    for (char c : text) {
        if (c == '\n') {
            DrawTextEx(font, line.c_str(), Vector2{current_x, current_y}, font_size, 1.0F, color);
            line.clear();
            current_y += scaled_line_height;
            current_x = start_x;
            continue;
        }
        
        std::string test_line = line + c;
        Vector2 test_size = MeasureTextEx(font, test_line.c_str(), font_size, 1.0F);
        
        if (test_size.x > scaled_max_width && !line.empty()) {
            DrawTextEx(font, line.c_str(), Vector2{current_x, current_y}, font_size, 1.0F, color);
            line = c;
            current_y += scaled_line_height;
            current_x = start_x;
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

void draw_library_floor(const LibraryRenderConfig& config) {
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

void draw_library_walls(const LibraryRenderConfig& config) {
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(80), wall_color);
}

void draw_scene_element(const SceneElement& element) {
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

void draw_npc_sprite(const NpcState& state, const NpcData& data, bool is_hovered, const Font& font) {
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

void draw_room_ui(const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);
    
    text(font, "点击NPC进行互动", 10, 330, 14, faded_ink);
}

void draw_transition_effect(float progress, const LibraryRenderConfig& config) {
    const unsigned char fade_alpha = static_cast<unsigned char>(progress * 255.0F);
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height), 
                  Color{0, 0, 0, fade_alpha});
}

void draw_intro_screen(const LibraryRuleEngine& engine, const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height), slate);
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    draw_pixel_bookshelf(30, 80, 160, 120);
    draw_pixel_bookshelf(220, 80, 200, 140);
    draw_pixel_bookshelf(450, 80, 160, 120);

    DrawRectangle(scaled(300), scaled(100), scaled(40), scaled(80), Color{139, 90, 43, 255});
    DrawRectangle(scaled(295), scaled(175), scaled(50), scaled(10), Color{100, 60, 30, 255});
    DrawCircle(scaled(320), scaled(115), scaled(12), Color{255, 220, 180, 255});

    panel(Rectangle{50, 180, 540, 180}, paper);
    
    const std::string& welcome = engine.get_data().welcome_message.empty() ? engine.get_dialogue().greeting : engine.get_data().welcome_message;
    text(font, welcome, 70, 195, 20, ink);

    text(font, "📚 游戏目标", 70, 220, 18, gold);
    draw_text_wrapped(font, "整理图书馆的书籍，把散落在地上的书放到正确的书架上，同时找出并修正放错位置的书籍。", 70, 242, 500, 22, 14, ink);

    text(font, "🎮 操作说明", 70, 275, 18, gold);
    text(font, "1. 点击地上闪烁的书捡起它", 70, 297, 14, ink);
    text(font, "2. 点击书架将书放上去（正确的书架会显示绿色✓）", 70, 315, 14, ink);
    text(font, "3. 点击书架上的感叹号查看放错的书", 70, 333, 14, ink);
    text(font, "4. 放完所有书后完成工作", 70, 351, 14, ink);

    const int btn_y = 380;
    const int btn_w = 200;
    const int btn_h = 40;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "开始工作", hovered, false, font);
}

void draw_npc_talk_screen(const LibraryRuleEngine& engine, const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height), slate);
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    draw_pixel_bookshelf(30, 80, 160, 120);
    draw_pixel_bookshelf(220, 80, 200, 140);
    draw_pixel_bookshelf(450, 80, 160, 120);

    DrawRectangle(scaled(300), scaled(100), scaled(40), scaled(80), Color{139, 90, 43, 255});
    DrawRectangle(scaled(295), scaled(175), scaled(50), scaled(10), Color{100, 60, 30, 255});
    DrawCircle(scaled(320), scaled(115), scaled(12), Color{255, 220, 180, 255});

    const auto& interaction = engine.get_npc_interaction();
    const std::string& dialogue = interaction.current_dialogue.empty() ? engine.get_dialogue().introduction : interaction.current_dialogue;

    panel(Rectangle{50, 200, 540, 120}, paper);
    text(font, "管理员说：", 70, 215, 18, ink);
    draw_text_wrapped(font, dialogue, 70, 240, 500, 22, 14, ink);

    const int btn_y = 330;
    const int btn_w = 200;
    const int btn_h = 40;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "继续", hovered, false, font);
}

void draw_plot_event_screen(const LibraryRuleEngine& engine, const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height), slate);
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    draw_pixel_bookshelf(30, 80, 160, 120);
    draw_pixel_bookshelf(450, 80, 160, 120);

    const auto& interaction = engine.get_npc_interaction();
    
    panel(Rectangle{50, 80, 540, 200}, plot_highlight);
    text(font, "【剧情事件】", 70, 95, 24, gold);
    draw_text_wrapped(font, interaction.current_plot_title, 70, 125, 500, 36, 28, ink);
    
    panel(Rectangle{70, 170, 500, 80}, paper);
    draw_text_wrapped(font, interaction.current_plot_description, 90, 185, 460, 24, 16, ink);

    const int btn_y = 340;
    const int btn_w = 200;
    const int btn_h = 40;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "了解详情", hovered, false, font);
}

void draw_map_reveal_screen(const LibraryRuleEngine& engine, const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(config.logical_height), slate);
    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    draw_pixel_bookshelf(30, 80, 160, 120);
    draw_pixel_bookshelf(450, 80, 160, 120);

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

    const auto& dialogue = engine.get_dialogue().old_map_reveal;
    panel(Rectangle{50, 275, 540, 65}, paper);
    draw_text_wrapped(font, dialogue, 70, 290, 500, 22, 14, ink);

    const int btn_y = 340;
    const int btn_w = 200;
    const int btn_h = 40;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hovered = CheckCollisionPointRec(logical_mouse,
                                          Rectangle{static_cast<float>(btn_x),
                                                    static_cast<float>(btn_y),
                                                    static_cast<float>(btn_w),
                                                    static_cast<float>(btn_h)});

    draw_category_button(btn_x, btn_y, btn_w, btn_h, "收下地图", hovered, false, font);
}

void draw_room_shelf(int x, int y, const Shelf& shelf, bool is_hovered, bool has_correct_book, 
                     const Font& font) {
    const int shelf_w = scaled(70);
    const int shelf_h = scaled(90);
    
    Color shelf_color = is_hovered ? Color{220, 180, 130, 255} : shelf_wood;
    Color shelf_dark = Color{80, 50, 20, 255};
    Color shelf_light = Color{220, 180, 130, 255};
    
    DrawRectangle(x, y, shelf_w, shelf_h, shelf_color);
    
    DrawRectangle(x, y, scaled(5), shelf_h, shelf_dark);
    DrawRectangle(x + shelf_w - scaled(5), y, scaled(5), shelf_h, shelf_dark);
    DrawRectangle(x, y, shelf_w, scaled(5), shelf_dark);
    DrawRectangle(x, y + shelf_h - scaled(5), shelf_w, scaled(5), shelf_dark);
    
    const int num_shelves = 4;
    const float shelf_spacing = (shelf_h - scaled(10)) / num_shelves;
    
    for (int i = 0; i < num_shelves; ++i) {
        float shelf_y = y + scaled(5) + i * shelf_spacing;
        DrawRectangle(x + scaled(5), shelf_y, shelf_w - scaled(10), scaled(3), shelf_dark);
        
        float book_start_y = shelf_y - scaled(2);
        float current_x = x + scaled(7);
        float max_x = x + shelf_w - scaled(7);
        
        const Color book_colors[] = {book_red, book_blue, book_green, book_yellow, book_purple, book_brown, book_orange};
        while (current_x < max_x) {
            int color_idx = (i * 3 + static_cast<int>(current_x / scaled(6))) % 7;
            float book_width = scaled(4) + (scaled(color_idx % 3));
            float book_height = shelf_spacing - scaled(8);
            
            if (current_x + book_width > max_x) {
                book_width = max_x - current_x;
            }
            
            DrawRectangle(current_x, book_start_y - book_height, book_width, book_height, book_colors[color_idx]);
            float spine_x = current_x + book_width - scaled(1);
            DrawRectangle(spine_x, book_start_y - book_height, scaled(1), book_height, Color{0, 0, 0, 40});
            
            current_x += book_width + scaled(1);
        }
    }
    
    if (has_correct_book) {
        DrawCircle(x + shelf_w - scaled(12), y + scaled(10), scaled(3), green);
    }

    const float label_w = scaled(70);
    const float label_h = scaled(22);
    const float label_x = x;
    const float label_y = y - label_h - scaled(3);
    
    DrawRectangle(label_x, label_y, label_w, label_h, Color{250, 238, 203, 230});
    DrawRectangleLinesEx(Rectangle{label_x, label_y, label_w, label_h}, scaled(1), ink);
    
    const float font_size = scaled_font_size(10);
    const Vector2 measured = MeasureTextEx(font, shelf.name.c_str(), font_size, 1.0F);
    DrawTextEx(font, shelf.name.c_str(),
               Vector2{label_x + (label_w - measured.x) / 2.0F,
                       label_y + (label_h - measured.y) / 2.0F},
               font_size, 1.0F, ink);
}

void draw_misplaced_indicator(int x, int y, const Font& font) {
    DrawCircle(x, y, scaled(6), Color{255, 100, 100, 200});
    DrawCircle(x, y, scaled(4), Color{255, 50, 50, 255});
    text(font, "!", x / native_ui_scale - 2, y / native_ui_scale - 5, 10, RAYWHITE);
}

void draw_held_book(const std::string& title, const std::string& category, 
                    const std::vector<BookCategory>& categories, const Font& font) {
    const float box_w = 300.0F;
    const float box_h = 40.0F;
    const float x = 220.0F;
    const float y = 5.0F;

    DrawRectangle(x, y, box_w, box_h, Color{245, 235, 210, 245});
    DrawRectangleLinesEx(Rectangle{x, y, box_w, box_h}, 3.0F, Color{100, 70, 40, 255});
    DrawRectangle(x + 3, y + 3, box_w - 6, box_h - 6, Color{255, 248, 235, 255});
    
    float title_width = MeasureTextEx(font, title.c_str(), 14.0F, 1.0F).x;
    float title_x = x + (box_w - title_width) / 2.0F;
    DrawTextEx(font, title.c_str(), Vector2{title_x, y + 15}, 14.0F, 1.0F, Color{60, 40, 20, 255});
}

void draw_organizing_screen(const LibraryRuleEngine& engine, const LibraryUIState& ui_state,
                            const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    draw_library_background(config);

    DrawRectangle(0, 0, config.logical_width, 60, Color{60, 79, 82, 200});
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    const SessionState& session = engine.get_session_state();
    char progress[64];
    snprintf(progress, sizeof(progress), "整理: %d/%d 纠错: %d/%d", 
             session.placed_count, session.total_scattered,
             session.corrected_count, session.total_misplaced);
    text(font, progress, 10, 70, 16, ink);

    if (session.current_combo > 1) {
        char combo[32];
        snprintf(combo, sizeof(combo), "连续正确: %d!", session.current_combo);
        text(font, combo, config.logical_width - 120, 70, 16, gold);
    }

    const std::vector<Shelf>& shelves = engine.get_shelves();
    const std::vector<ShelfLayout> layouts = compute_shelf_layouts(config.logical_width, shelves.size());

    for (size_t i = 0; i < shelves.size(); ++i) {
        const int x = layouts[i].x;
        const int y = layouts[i].y;
        const int shelf_w = layouts[i].width;
        
        const float label_w = static_cast<float>(shelf_w);
        const float label_h = 18.0F;
        const float label_x = static_cast<float>(x);
        const float label_y = static_cast<float>(y) - label_h + 2.0F;

        DrawRectangle(label_x, label_y, label_w, label_h, Color{250, 238, 203, 230});
        DrawRectangleLinesEx(Rectangle{label_x, label_y, label_w, label_h}, 1.0F, ink);
        
        const char* text_ptr = shelves[i].name.c_str();
        float text_width = MeasureTextEx(font, text_ptr, 12.0F, 1.0F).x;
        float text_x = label_x + (label_w - text_width) / 2.0F;
        DrawTextEx(font, text_ptr, Vector2{text_x, label_y + 3.0F}, 12.0F, 1.0F, ink);
    }

    for (size_t i = 0; i < shelves.size(); ++i) {
        const int x = layouts[i].x;
        const int y = layouts[i].y;
        const int shelf_w = layouts[i].width;
        
        for (const auto& book : session.misplaced_books) {
            if (book.current_shelf_id == shelves[i].id) {
                float exclamation_x = x + shelf_w / 2.0F;
                float exclamation_y = y + layouts[i].height / 2.0F;
                
                int pulse_frame = static_cast<int>(GetTime() * 6.0F) % 4;
                float pulse_scale = pulse_frame < 2 ? 1.0F : 0.85F;
                float radius = 12.0F * pulse_scale;
                
                DrawCircle(exclamation_x, exclamation_y, radius, Color{183, 83, 72, 255});
                DrawTextEx(font, "!", Vector2{exclamation_x - 6.0F, exclamation_y - 12.0F}, 24.0F, 1.0F, WHITE);

                if (ui_state.highlighted_misplaced == book.id) {
                    float tooltip_w = 120.0F;
                    float tooltip_h = 40.0F;
                    float tooltip_x = exclamation_x - tooltip_w / 2.0F;
                    float tooltip_y = y - tooltip_h - 10.0F;
                    DrawRectangle(tooltip_x, tooltip_y, tooltip_w, tooltip_h, Color{255, 255, 255, 240});
                    DrawRectangleLinesEx(Rectangle{tooltip_x, tooltip_y, tooltip_w, tooltip_h}, 2.0F, Color{100, 70, 40, 255});
                    float title_width = MeasureTextEx(font, book.title.c_str(), 14.0F, 1.0F).x;
                    DrawTextEx(font, book.title.c_str(), Vector2{tooltip_x + (tooltip_w - title_width) / 2.0F, tooltip_y + 10.0F}, 14.0F, 1.0F, ink);
                }
            }
        }
    }

    if (!session.scattered_books.empty()) {
        const Book& current_book = session.scattered_books[0];
        const Rectangle bounds = book_bounds(current_book);
        bool hovered = CheckCollisionPointRec(logical_mouse, bounds);
        
        int pulse_frame = static_cast<int>(GetTime() * 8.0F) % 4;
        if (pulse_frame < 2) {
            unsigned char alpha = static_cast<unsigned char>(150 + pulse_frame * 50);
            DrawRectangleLinesEx(Rectangle{bounds.x - 25, bounds.y - 6, bounds.width + 50, bounds.height + 12}, 3.0F, Color{224, 169, 74, alpha});
        }
        
        float title_width = MeasureTextEx(font, current_book.title.c_str(), 16.0F, 1.0F).x;
        float title_x = bounds.x + (bounds.width - title_width) / 2.0F;
        DrawTextEx(font, current_book.title.c_str(), Vector2{title_x, bounds.y - 26.0F}, 16.0F, 1.0F, gold);
        
        text(font, "→ 点击捡起", bounds.x + (bounds.width - 60) / 2, bounds.y + bounds.height + 5, 11, ink);
        
        if (hovered) {
            DrawRectangleLinesEx(Rectangle{bounds.x - 3, bounds.y - 3, bounds.width + 6, bounds.height + 6}, 2.0F, gold);
        }
    }

    if (engine.is_holding_book()) {
        draw_held_book(engine.get_held_book_title(), engine.get_held_book_category(), 
                       engine.get_categories(), font);
        
        for (size_t i = 0; i < shelves.size(); ++i) {
            const int x = layouts[i].x;
            const int y = layouts[i].y;
            const int shelf_w = layouts[i].width;
            const int shelf_h = layouts[i].height;
            
            const Rectangle logical_rect = Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(shelf_w),
                static_cast<float>(shelf_h)};
            bool hovered = CheckCollisionPointRec(logical_mouse, logical_rect);

            if (hovered) {
                bool is_correct = (engine.get_held_book_category() == shelves[i].category_id);
                Color glow_color = is_correct ? Color{50, 200, 50, 100} : Color{200, 50, 50, 100};
                DrawRectangle(x - 5, y - 5, shelf_w + 10, shelf_h + 10, glow_color);
                
                if (is_correct) {
                    text(font, "✓ 放这里", x + 5, y + shelf_h + 5, 12, green);
                } else {
                    text(font, "✗ 不对", x + 10, y + shelf_h + 5, 12, wrong_color);
                }
            }
        }
    }

    if (!session.scattered_books.empty()) {
        text(font, "📖 当前目标：将「" + session.scattered_books[0].title + "」放到正确的书架", 10, config.logical_height - 35, 11, ink);
    } else if (!session.misplaced_books.empty()) {
        text(font, "🔍 当前目标：点击感叹号查看并修正放错的书籍", 10, config.logical_height - 35, 11, ink);
    } else {
        text(font, "🎉 所有书籍已整理完毕！", 10, config.logical_height - 35, 11, gold);
    }
    
    if (engine.is_holding_book()) {
        text(font, "🖱️ 点击书架放置书籍（绿色=正确，红色=错误）", 10, config.logical_height - 20, 10, faded_ink);
    } else {
        text(font, "🖱️ 点击地上的书捡起，点击感叹号查看错放书籍", 10, config.logical_height - 20, 10, faded_ink);
    }
    text(font, "按 ESC 放弃工作", config.logical_width - 100, config.logical_height - 20, 10, Color{128, 128, 128, 255});
}

void draw_feedback_screen(const LibraryRuleEngine& engine, const LibraryUIState& ui_state,
                          const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    const Color feedback_color = ui_state.last_answer_correct ? correct_color : wrong_color;
    const char* feedback_text = ui_state.last_answer_correct ? "放置正确！" : "放错了";
    
    panel(Rectangle{config.logical_width / 2.0F - 140.0F, 100.0F, 280.0F, 120.0F}, ui_state.last_answer_correct ? Color{230, 245, 230, 255} : Color{245, 230, 230, 255});
    text(font, feedback_text, config.logical_width / 2 - 70, 130, 32, feedback_color);

    const auto& feedback = ui_state.feedback_data;
    
    if (!feedback.book_title.empty()) {
        float title_width = MeasureTextEx(font, feedback.book_title.c_str(), 18.0F, 1.0F).x;
        text(font, "\"" + feedback.book_title + "\"", config.logical_width / 2 - title_width / 2 - 10, 170, 18, ink);
    }

    if (feedback.is_correct && feedback.knowledge_reward > 0) {
        char knowledge_text[32];
        snprintf(knowledge_text, sizeof(knowledge_text), "获得知识 +%d", feedback.knowledge_reward);
        text(font, knowledge_text, config.logical_width / 2 - 70, 200, 18, gold);
    }

    text(font, "继续整理...", config.logical_width / 2 - 70, 230, 18, Color{100, 100, 100, 255});
}

void draw_summary_screen(const ActionResult& result, const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    panel(Rectangle{50, 80, 540, 240}, paper);
    text(font, "图书馆工作完成", config.logical_width / 2 - 100, 95, 24, ink);

    draw_text_wrapped(font, result.summary, 70, 130, 500, 24, 16, ink);

    int y = 170;

    if (result.stamina_change != 0) {
        char stamina[64];
        snprintf(stamina, sizeof(stamina), "体力: %+d", result.stamina_change);
        text(font, stamina, 70, y, 18, result.stamina_change < 0 ? wrong_color : correct_color);
        y += 28;
    }

    if (result.money_change != 0) {
        char money[64];
        snprintf(money, sizeof(money), "金钱: %+d", result.money_change);
        text(font, money, 70, y, 18, result.money_change < 0 ? wrong_color : gold);
        y += 28;
    }

    if (result.knowledge_change != 0) {
        char knowledge[64];
        snprintf(knowledge, sizeof(knowledge), "知识: %+d", result.knowledge_change);
        text(font, knowledge, 70, y, 18, result.knowledge_change < 0 ? wrong_color : correct_color);
        y += 28;
    }

    if (result.reputation_change != 0) {
        char reputation[64];
        snprintf(reputation, sizeof(reputation), "声望: %+d", result.reputation_change);
        text(font, reputation, 70, y, 18, result.reputation_change < 0 ? wrong_color : gold);
        y += 28;
    }

    if (result.mood_change != 0) {
        char mood[64];
        snprintf(mood, sizeof(mood), "心情: %+d", result.mood_change);
        text(font, mood, 70, y, 18, result.mood_change < 0 ? wrong_color : correct_color);
        y += 28;
    }

    if (result.plot_triggered) {
        panel(Rectangle{50.0F, static_cast<float>(y) + 5.0F, 540.0F, 70.0F}, plot_highlight);
        text(font, "【剧情触发】", 70, y + 15, 16, gold);
        text(font, result.plot_title.c_str(), 170, y + 15, 16, ink);
        draw_text_wrapped(font, result.plot_description, 70, y + 35, 500, 22, 14, faded_ink);
    }

    if (!result.narrative_echo.empty()) {
        panel(Rectangle{50.0F, static_cast<float>(y) + 5.0F, 540.0F, 50.0F}, Color{245, 240, 220, 255});
        draw_text_wrapped(font, result.narrative_echo, 70, y + 15, 500, 22, 14, faded_ink);
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

void draw_library_room_scene(const LibraryScene& scene, const LibraryUIState& ui_state,
                             const LibraryRenderConfig& render_config, const Font& font, Vector2 logical_mouse) {
    const Vector2 library_mouse = {logical_mouse.x / native_ui_scale, logical_mouse.y / native_ui_scale};

    draw_library_background(render_config);

    for (const auto& element : scene.get_elements()) {
        draw_scene_element(element);
    }

    const auto& npc_states = scene.get_npc_manager().get_npc_states();
    for (const auto& state : npc_states) {
        const auto* data = scene.get_npc_manager().get_npc_data(state.npc_id);
        if (!data) continue;

        const float dist = std::sqrt(std::pow(library_mouse.x - state.position.x, 2) + 
                                     std::pow(library_mouse.y - state.position.y, 2));
        bool is_hovered = dist <= data->interaction_radius;

        draw_npc_sprite(state, *data, is_hovered, font);
    }

    draw_room_ui(render_config, font, library_mouse);

    if (ui_state.is_transitioning) {
        draw_transition_effect(ui_state.transition_progress, render_config);
    }
}

void draw_library_scene(const LibraryRuleEngine& engine, const LibraryUIState& ui_state,
                        const LibraryScene& scene, const LibraryRenderConfig& render_config, const Font& font, Vector2 logical_mouse) {
    const Vector2 library_mouse = {logical_mouse.x / native_ui_scale, logical_mouse.y / native_ui_scale};

    switch (ui_state.scene_state) {
        case LibrarySceneState::room_view: {
            draw_library_room_scene(scene, ui_state, render_config, font, logical_mouse);
            break;
        }
        case LibrarySceneState::intro:
            draw_intro_screen(engine, render_config, font, library_mouse);
            break;
        case LibrarySceneState::npc_talk:
            draw_npc_talk_screen(engine, render_config, font, library_mouse);
            break;
        case LibrarySceneState::plot_event:
            draw_plot_event_screen(engine, render_config, font, library_mouse);
            break;
        case LibrarySceneState::organizing:
            draw_organizing_screen(engine, ui_state, render_config, font, library_mouse);
            break;
        case LibrarySceneState::feedback:
            draw_feedback_screen(engine, ui_state, render_config, font, library_mouse);
            break;
        case LibrarySceneState::map_reveal:
            draw_map_reveal_screen(engine, render_config, font, library_mouse);
            break;
        case LibrarySceneState::summary: {
            const ActionResult result = engine.finish_session();
            draw_summary_screen(result, render_config, font, library_mouse);
            break;
        }
        case LibrarySceneState::exit:
            break;
    }

    if (ui_state.is_transitioning) {
        draw_transition_effect(ui_state.transition_progress, render_config);
    }
}

void update_library_ui(LibraryRuleEngine& engine, LibraryUIState& ui_state, LibraryScene& scene) {
    scene.update(1.0F / 60.0F);

    if (ui_state.is_transitioning) {
        ui_state.transition_progress += 0.05F;
        if (ui_state.transition_progress >= 1.0F) {
            ui_state.is_transitioning = false;
            ui_state.transition_progress = 0.0F;
            
            if (ui_state.scene_state == LibrarySceneState::room_view) {
                ui_state.scene_state = LibrarySceneState::intro;
            }
        }
        return;
    }

    if (ui_state.scene_state == LibrarySceneState::feedback) {
        ui_state.feedback_timer++;
        if (ui_state.feedback_timer >= 60) {
            ui_state.feedback_timer = 0;
            if (engine.is_session_active()) {
                ui_state.scene_state = LibrarySceneState::organizing;
            } else {
                ui_state.scene_state = LibrarySceneState::summary;
            }
        }
    }
}

bool handle_library_input(LibraryRuleEngine& engine, LibraryUIState& ui_state, 
                          LibraryScene& scene, const LibraryRenderConfig& render_config, Vector2 logical_mouse) {
    const Vector2 library_mouse = logical_mouse;

    if (ui_state.is_transitioning) {
        return false;
    }

    if (ui_state.scene_state == LibrarySceneState::room_view) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::string npc_id;
            if (scene.handle_click(library_mouse, npc_id)) {
                ui_state.clicked_npc_id = npc_id;
                ui_state.is_transitioning = true;
                ui_state.transition_progress = 0.0F;
                return false;
            }
        }
        return false;
    } else if (ui_state.scene_state == LibrarySceneState::intro) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::npc_talk;
            return false;
        }
    } else if (ui_state.scene_state == LibrarySceneState::npc_talk) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (engine.has_pending_plot_event()) {
                ui_state.scene_state = LibrarySceneState::plot_event;
            } else if (engine.should_reveal_map(engine.get_data().books.size(), engine.get_npc_interaction().relationship != NpcRelationship::stranger ? 2 : 0)) {
                ui_state.scene_state = LibrarySceneState::map_reveal;
            } else {
                ui_state.scene_state = LibrarySceneState::organizing;
            }
            return false;
        }
    } else if (ui_state.scene_state == LibrarySceneState::plot_event) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::organizing;
            return false;
        }
    } else if (ui_state.scene_state == LibrarySceneState::feedback) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::organizing;
            return false;
        }
    } else if (ui_state.scene_state == LibrarySceneState::organizing) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            engine.give_up();
            return true;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            const SessionState& session = engine.get_session_state();
            
            if (!session.scattered_books.empty() && session.current_scattered_index < static_cast<int>(session.scattered_books.size())) {
                const Book& current_book = session.scattered_books[session.current_scattered_index];
                const Rectangle bounds = book_bounds(current_book);
                if (CheckCollisionPointRec(library_mouse, bounds)) {
                    engine.pick_up_book(current_book.id);
                    ui_state.is_holding_book = true;
                    ui_state.held_book_title = current_book.title;
                    ui_state.held_book_category = current_book.category_id;
                    return false;
                }
            }

            const std::vector<Shelf>& shelves = engine.get_shelves();
            const std::vector<ShelfLayout> layouts = compute_shelf_layouts(render_config.logical_width, shelves.size());

            for (size_t i = 0; i < shelves.size(); ++i) {
                const int x = layouts[i].x;
                const int y = layouts[i].y;
                const int shelf_w = layouts[i].width;
                const int shelf_h = layouts[i].height;
                
                float exclamation_x = x + shelf_w / 2.0F;
                float exclamation_y = y + shelf_h / 2.0F;
                
                Rectangle exclamation_rect = Rectangle{
                    exclamation_x - 10.0F,
                    exclamation_y - 10.0F,
                    20.0F,
                    20.0F};

                bool clicked_exclamation = CheckCollisionPointRec(library_mouse, exclamation_rect);
                bool clicked_shelf = false;
                
                const Rectangle shelf_rect = Rectangle{
                    static_cast<float>(x),
                    static_cast<float>(y) - 20.0F,
                    static_cast<float>(shelf_w),
                    static_cast<float>(shelf_h) + 20.0F};
                
                if (CheckCollisionPointRec(library_mouse, shelf_rect)) {
                    if (!clicked_exclamation) {
                        clicked_shelf = true;
                    }
                }

                if (clicked_exclamation) {
                    for (const auto& book : session.misplaced_books) {
                        if (book.current_shelf_id == shelves[i].id) {
                            if (ui_state.highlighted_misplaced == book.id) {
                                engine.pick_misplaced_book(book.id);
                                ui_state.is_holding_book = true;
                                ui_state.held_book_title = book.title;
                                ui_state.held_book_category = book.category_id;
                                ui_state.highlighted_misplaced.clear();
                            } else {
                                ui_state.highlighted_misplaced = book.id;
                            }
                            return false;
                        }
                    }
                }

                if (clicked_shelf && engine.is_holding_book()) {
                    bool correct = engine.place_book_on_shelf(shelves[i].id);
                    
                    ui_state.last_answer_correct = correct;
                    ui_state.feedback_data.book_title = engine.get_held_book_title();
                    ui_state.feedback_data.is_correct = correct;
                    
                    std::string category_name = "";
                    for (const auto& cat : engine.get_categories()) {
                        if (cat.id == shelves[i].category_id) {
                            category_name = cat.name;
                            break;
                        }
                    }
                    ui_state.feedback_data.correct_category_name = category_name;
                    ui_state.feedback_data.knowledge_reward = correct ? 5 : 0;
                    
                    ui_state.is_holding_book = false;
                    ui_state.held_book_title.clear();
                    ui_state.held_book_category.clear();
                    ui_state.highlighted_misplaced.clear();
                    
                    if (engine.is_session_completed()) {
                        ui_state.scene_state = LibrarySceneState::summary;
                    } else {
                        ui_state.scene_state = LibrarySceneState::feedback;
                    }
                    return false;
                }
            }
        }
    } else if (ui_state.scene_state == LibrarySceneState::map_reveal) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::organizing;
            return false;
        }
    } else if (ui_state.scene_state == LibrarySceneState::summary) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
}

}  // namespace pixel_town::library::ui
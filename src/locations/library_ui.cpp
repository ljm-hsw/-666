#include "locations/library_ui.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include <raylib.h>

namespace pixel_town::library::ui {

namespace {

constexpr float native_ui_scale = 1.5F;

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

    panel(Rectangle{50, 230, 540, 110}, paper);
    
    const std::string& welcome = engine.get_data().welcome_message.empty() ? engine.get_dialogue().greeting : engine.get_data().welcome_message;
    text(font, welcome, 70, 245, 20, ink);
    
    const std::string& intro = engine.get_data().work_intro.empty() ? "读者会提出各种问题，你需要从书架上找到正确的书籍类别来回答" : engine.get_data().work_intro;
    draw_text_wrapped(font, intro, 70, 272, scaled(500), 22, 14, ink);

    const int btn_y = 320;
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
    draw_text_wrapped(font, dialogue.c_str(), 70, 240, scaled(500), 22, 14, ink);

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
    draw_text_wrapped(font, interaction.current_plot_title.c_str(), 70, 125, scaled(500), 36, 28, ink);
    
    panel(Rectangle{70, 170, 500, 80}, paper);
    draw_text_wrapped(font, interaction.current_plot_description.c_str(), 90, 185, scaled(460), 24, 16, ink);

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
    draw_text_wrapped(font, dialogue.c_str(), 70, 290, scaled(500), 22, 14, ink);

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

void draw_answering_screen(const LibraryRuleEngine& engine, const LibraryUIState& ui_state,
                           const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    const SessionState& session = engine.get_session_state();
    char progress[32];
    snprintf(progress, sizeof(progress), "问题 %d/%d", session.current_question_index + 1,
             static_cast<int>(session.answers.size()) + (session.is_active ? 1 : 0));
    text(font, progress, 10, 70, 18, ink);

    if (session.current_combo > 1) {
        char combo[32];
        snprintf(combo, sizeof(combo), "连续正确: %d!", session.current_combo);
        text(font, combo, config.logical_width - 120, 70, 16, gold);
    }

    const ReaderQuestion& question = engine.get_current_question();

    panel(Rectangle{50, 100, 540, 60}, paper);
    text(font, "读者问：", 70, 110, 18, ink);
    draw_text_wrapped(font, question.question.c_str(), 70, 135, scaled(500), 22, 14, ink);

    if (ui_state.show_hint && !question.hint.empty()) {
        panel(Rectangle{50, 155, 540, 35}, Color{245, 240, 220, 255});
        text(font, "提示：" + question.hint, 70, 165, 14, faded_ink);
    }

    const int btn_y = 185;
    const int btn_w = 100;
    const int btn_h = 30;
    const int btn_x = config.logical_width / 2 - btn_w / 2;

    bool hint_hovered = CheckCollisionPointRec(logical_mouse,
                                               Rectangle{static_cast<float>(btn_x),
                                                         static_cast<float>(btn_y),
                                                         static_cast<float>(btn_w),
                                                         static_cast<float>(btn_h)});

    Color hint_fill = hint_hovered ? cream : Color{245, 240, 220, 255};
    panel(Rectangle{static_cast<float>(btn_x), static_cast<float>(btn_y), static_cast<float>(btn_w), static_cast<float>(btn_h)}, hint_fill, ink);
    centered_text(font, ui_state.show_hint ? "隐藏提示" : "查看提示", Rectangle{static_cast<float>(btn_x), static_cast<float>(btn_y), static_cast<float>(btn_w), static_cast<float>(btn_h)}, 14, ink);

    panel(Rectangle{50, 215, 540, 80}, paper);
    text(font, "选择书籍类别：", 70, 230, 18, ink);

    const std::vector<BookCategory>& categories = engine.get_categories();
    const int cols = 4;
    const int cat_btn_w = 120;
    const int cat_btn_h = 36;
    const int gap = 15;
    const int start_x = (config.logical_width - (cols * cat_btn_w + (cols - 1) * gap)) / 2;
    const int start_y = 250;

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

    text(font, "按 ESC 放弃工作", 50, config.logical_height - 25, 14, Color{128, 128, 128, 255});
}

void draw_feedback_screen(const LibraryRuleEngine& engine, const LibraryUIState& ui_state,
                          const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    const Color feedback_color = ui_state.last_answer_correct ? correct_color : wrong_color;
    const char* feedback_text = ui_state.last_answer_correct ? "回答正确！" : "回答错误";
    
    panel(Rectangle{config.logical_width / 2 - 140, 100, 280, 160}, ui_state.last_answer_correct ? Color{230, 245, 230, 255} : Color{245, 230, 230, 255});
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

void draw_summary_screen(const ActionResult& result, const LibraryRenderConfig& config, const Font& font, Vector2 logical_mouse) {
    ClearBackground(Color{221, 211, 174, 255});

    DrawRectangle(0, 0, scaled(config.logical_width), scaled(60), slate);
    text(font, "像素小镇", 10, 8, 22, RAYWHITE);
    text(font, "图书馆", config.logical_width / 2 - 50, 8, 22, gold);

    panel(Rectangle{50, 80, 540, 240}, paper);
    text(font, "图书馆工作完成", config.logical_width / 2 - 100, 95, 24, ink);

    draw_text_wrapped(font, result.summary.c_str(), 70, 130, scaled(500), 24, 16, ink);

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
        panel(Rectangle{50, y + 5, 540, 70}, plot_highlight);
        text(font, "【剧情触发】", 70, y + 15, 16, gold);
        text(font, result.plot_title.c_str(), 170, y + 15, 16, ink);
        draw_text_wrapped(font, result.plot_description.c_str(), 70, y + 35, scaled(500), 22, 14, faded_ink);
        y += 80;
    }

    if (!result.narrative_echo.empty()) {
        panel(Rectangle{50, y + 5, 540, 50}, Color{245, 240, 220, 255});
        draw_text_wrapped(font, result.narrative_echo.c_str(), 70, y + 15, scaled(500), 22, 14, faded_ink);
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

void draw_library_scene(const LibraryRuleEngine& engine, const LibraryUIState& ui_state,
                        const LibraryRenderConfig& render_config, const Font& font, Vector2 logical_mouse) {
    const Vector2 library_mouse = {logical_mouse.x / native_ui_scale, logical_mouse.y / native_ui_scale};

    switch (ui_state.scene_state) {
        case LibrarySceneState::intro:
            draw_intro_screen(engine, render_config, font, library_mouse);
            break;
        case LibrarySceneState::npc_talk:
            draw_npc_talk_screen(engine, render_config, font, library_mouse);
            break;
        case LibrarySceneState::plot_event:
            draw_plot_event_screen(engine, render_config, font, library_mouse);
            break;
        case LibrarySceneState::answering:
            draw_answering_screen(engine, ui_state, render_config, font, library_mouse);
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
}

void update_library_ui(LibraryRuleEngine& engine, LibraryUIState& ui_state) {
    if (ui_state.scene_state == LibrarySceneState::feedback) {
        ui_state.feedback_timer++;
        if (ui_state.feedback_timer >= 60) {
            ui_state.feedback_timer = 0;
            if (engine.is_session_active()) {
                ui_state.scene_state = LibrarySceneState::answering;
            } else {
                ui_state.scene_state = LibrarySceneState::summary;
            }
        }
    }
}

bool handle_library_input(LibraryRuleEngine& engine, LibraryUIState& ui_state, Vector2 logical_mouse) {
    const Vector2 library_mouse = {logical_mouse.x / native_ui_scale, logical_mouse.y / native_ui_scale};

    if (ui_state.scene_state == LibrarySceneState::intro) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::npc_talk;
            return false;
        }
    } else if (ui_state.scene_state == LibrarySceneState::npc_talk) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (engine.has_pending_plot_event()) {
                ui_state.scene_state = LibrarySceneState::plot_event;
            } else if (engine.should_reveal_map(engine.get_data().questions.size(), engine.get_npc_interaction().relationship != NpcRelationship::stranger ? 2 : 0)) {
                ui_state.scene_state = LibrarySceneState::map_reveal;
            } else {
                ui_state.scene_state = LibrarySceneState::answering;
            }
            return false;
        }
    } else if (ui_state.scene_state == LibrarySceneState::plot_event) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::answering;
            return false;
        }
    } else if (ui_state.scene_state == LibrarySceneState::answering) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            engine.give_up();
            return true;
        }

        const int hint_btn_y = 185;
        const int hint_btn_w = 100;
        const int hint_btn_h = 30;
        const int hint_btn_x = 640 / 2 - hint_btn_w / 2;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(
                    library_mouse,
                    Rectangle{static_cast<float>(hint_btn_x), static_cast<float>(hint_btn_y),
                              static_cast<float>(hint_btn_w), static_cast<float>(hint_btn_h)})) {
                ui_state.show_hint = !ui_state.show_hint;
                return false;
            }

            const std::vector<BookCategory>& categories = engine.get_categories();
            const int cols = 4;
            const int btn_w = 120;
            const int btn_h = 36;
            const int gap = 15;
            const int start_x =
                (640 - (cols * btn_w + (cols - 1) * gap)) / 2;
            const int start_y = 250;

            for (size_t i = 0; i < categories.size(); ++i) {
                const int row = static_cast<int>(i / cols);
                const int col = static_cast<int>(i % cols);
                const int x = start_x + col * (btn_w + gap);
                const int y = start_y + row * (btn_h + gap);

                if (CheckCollisionPointRec(
                        library_mouse,
                        Rectangle{static_cast<float>(x), static_cast<float>(y),
                                  static_cast<float>(btn_w), static_cast<float>(btn_h)})) {
                    const auto& current_question = engine.get_current_question();
                    std::string correct_name;
                    for (const auto& cat : engine.get_categories()) {
                        if (cat.id == current_question.correct_category_id) {
                            correct_name = cat.name;
                            break;
                        }
                    }
                    
                    ui_state.feedback_data.question = current_question.question;
                    ui_state.feedback_data.feedback_correct = current_question.feedback_correct;
                    ui_state.feedback_data.feedback_wrong = current_question.feedback_wrong;
                    ui_state.feedback_data.correct_category_name = correct_name;
                    ui_state.feedback_data.knowledge_reward = 10;
                    
                    engine.select_category(categories[i].id);
                    ui_state.selected_category_id = categories[i].id;
                    ui_state.last_answer_correct = engine.was_last_answer_correct();
                    ui_state.feedback_data.is_correct = ui_state.last_answer_correct;
                    ui_state.scene_state = LibrarySceneState::feedback;
                    return false;
                }
            }
        }
    } else if (ui_state.scene_state == LibrarySceneState::map_reveal) {
        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ui_state.scene_state = LibrarySceneState::answering;
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
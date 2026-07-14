#include "locations/library_organizing_ui.hpp"

#include <algorithm>

#include "ui/scene_viewport.hpp"

namespace pixel_town::library::ui {
namespace {

constexpr Color slate{60, 79, 82, 255};
constexpr Color ink{45, 52, 54, 255};
constexpr Color paper{250, 238, 203, 245};
constexpr Color cream{255, 248, 226, 245};
constexpr Color green{82, 137, 92, 255};
constexpr Color gold{224, 169, 74, 255};

void draw_text(const Font& font, const std::string& value, float x, float y,
               float size, Color color = ink) {
    DrawTextEx(font, value.c_str(), Vector2{x, y}, size, 1.0F, color);
}

void draw_centered(const Font& font, const std::string& value, Rectangle bounds,
                   float size, Color color = ink) {
    const Vector2 measured = MeasureTextEx(font, value.c_str(), size, 1.0F);
    DrawTextEx(font, value.c_str(),
               Vector2{bounds.x + (bounds.width - measured.x) / 2.0F,
                       bounds.y + (bounds.height - measured.y) / 2.0F},
               size, 1.0F, color);
}

void draw_panel(Rectangle bounds, Color fill, Color border = ink) {
    DrawRectangleRec(Rectangle{bounds.x + 4.0F, bounds.y + 4.0F, bounds.width,
                               bounds.height},
                     Color{20, 28, 30, 110});
    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, 3.0F, border);
}

void draw_background(const Texture2D& background) {
    DrawRectangle(0, 0, ::pixel_town::ui::canvas_width,
                  ::pixel_town::ui::canvas_height, Color{37, 50, 57, 255});
    const auto viewport = ::pixel_town::ui::indoor_scene_viewport();
    if (background.id != 0) {
        DrawTexturePro(background,
                       Rectangle{0.0F, 0.0F, static_cast<float>(background.width),
                                 static_cast<float>(background.height)},
                       Rectangle{viewport.x, viewport.y, viewport.width, viewport.height},
                       Vector2{0.0F, 0.0F}, 0.0F, WHITE);
    } else {
        DrawRectangleRec(Rectangle{viewport.x, viewport.y, viewport.width,
                                   viewport.height},
                         Color{196, 176, 139, 255});
    }
    DrawRectangle(0, 0, ::pixel_town::ui::canvas_width,
                  static_cast<int>(::pixel_town::ui::scene_header_height), slate);
}

Rectangle canvas_rect(const OrganizingShelf& shelf) {
    const auto bounds = ::pixel_town::ui::scene_canvas_to_viewport(
        {static_cast<float>(shelf.x), static_cast<float>(shelf.y),
         static_cast<float>(shelf.width), static_cast<float>(shelf.height)});
    return Rectangle{bounds.x, bounds.y, bounds.width, bounds.height};
}

Rectangle task_rect(const OrganizingBookTask& task) {
    const auto center = ::pixel_town::ui::scene_canvas_to_viewport(
        ::pixel_town::ui::SceneViewportPoint{static_cast<float>(task.x),
                                             static_cast<float>(task.y)});
    return Rectangle{center.x - 24.0F, center.y - 14.0F, 48.0F, 28.0F};
}

const OrganizingBookTask* held_task(const LibraryOrganizingSession& session) {
    const auto& held_id = session.state().held_book_id;
    const auto found = std::find_if(session.tasks().begin(), session.tasks().end(),
                                    [&held_id](const auto& task) {
                                        return task.id == held_id;
                                    });
    return found == session.tasks().end() ? nullptr : &*found;
}

}  // namespace

Rectangle reader_mode_button() { return Rectangle{180.0F, 225.0F, 270.0F, 150.0F}; }

Rectangle organizing_mode_button() { return Rectangle{510.0F, 225.0F, 270.0F, 150.0F}; }

void draw_library_mode_selection(const Font& font, const Texture2D& background,
                                 Vector2 logical_mouse) {
    draw_background(background);
    draw_text(font, "图书馆 · 选择本次工作", 75.0F, 16.0F, 30.0F, RAYWHITE);
    draw_text(font, "1 / 2 选择", 690.0F, 20.0F, 24.0F,
              Color{255, 218, 156, 255});

    draw_panel(Rectangle{120.0F, 115.0F, 720.0F, 330.0F},
               Color{38, 54, 56, 218}, Color{224, 169, 74, 255});
    draw_centered(font, "今天想怎样帮助图书馆？",
                  Rectangle{220.0F, 145.0F, 520.0F, 42.0F}, 30.0F, cream);

    const Rectangle reader = reader_mode_button();
    const Rectangle organizing = organizing_mode_button();
    draw_panel(reader, CheckCollisionPointRec(logical_mouse, reader) ? cream : paper,
               gold);
    draw_panel(organizing,
               CheckCollisionPointRec(logical_mouse, organizing) ? cream : paper, gold);
    draw_centered(font, "1 读者咨询", Rectangle{reader.x, reader.y + 18, reader.width, 42},
                  30.0F, green);
    draw_centered(font, "根据需求选择书籍类别",
                  Rectangle{reader.x, reader.y + 74, reader.width, 34}, 24.0F);
    draw_centered(font, "2 书籍整理",
                  Rectangle{organizing.x, organizing.y + 18, organizing.width, 42},
                  30.0F, green);
    draw_centered(font, "拾取散落或错架书并归位",
                  Rectangle{organizing.x, organizing.y + 74, organizing.width, 34},
                  24.0F);
    draw_centered(font, "ESC 放弃本次图书馆行动",
                  Rectangle{250.0F, 400.0F, 460.0F, 30.0F}, 24.0F,
                  Color{226, 218, 195, 255});
}

void draw_library_organizing(const Font& font, const Texture2D& background,
                             const LibraryOrganizingSession& session,
                             const OrganizingUIState& ui_state,
                             Vector2 logical_mouse) {
    draw_background(background);
    draw_text(font, "图书馆 · 书籍整理", 75.0F, 16.0F, 30.0F, RAYWHITE);
    const std::string progress = "已归位 " +
                                 std::to_string(session.state().completed_count) + "/" +
                                 std::to_string(session.tasks().size()) + "  错误 " +
                                 std::to_string(session.state().wrong_count);
    draw_text(font, progress, 650.0F, 20.0F, 24.0F,
              Color{255, 218, 156, 255});

    for (const auto& shelf : session.shelves()) {
        const Rectangle bounds = canvas_rect(shelf);
        const bool hovered = CheckCollisionPointRec(logical_mouse, bounds);
        DrawRectangleRec(bounds, hovered ? Color{255, 232, 164, 70}
                                         : Color{42, 92, 74, 28});
        DrawRectangleLinesEx(bounds, hovered ? 4.0F : 2.0F,
                             hovered ? gold : Color{245, 229, 184, 190});
        draw_centered(font, shelf.name,
                      Rectangle{bounds.x, bounds.y + bounds.height - 28.0F, bounds.width,
                                25.0F},
                      20.0F, cream);
    }

    for (std::size_t index = 0; index < session.tasks().size(); ++index) {
        if (session.state().completed_tasks[index] ||
            session.state().held_book_id == session.tasks()[index].id) {
            continue;
        }
        const auto& task = session.tasks()[index];
        const Rectangle bounds = task_rect(task);
        const bool hovered = CheckCollisionPointRec(logical_mouse, bounds);
        DrawRectangleRec(bounds, task.source == OrganizingBookSource::misplaced
                                     ? Color{183, 83, 72, 240}
                                     : Color{82, 137, 92, 240});
        DrawRectangleLinesEx(bounds, hovered ? 4.0F : 2.0F,
                             hovered ? gold : cream);
        draw_centered(font, std::to_string(index + 1), bounds, 20.0F, RAYWHITE);
    }

    draw_panel(Rectangle{95.0F, 454.0F, 770.0F, 70.0F},
               Color{38, 54, 56, 232}, gold);
    if (const auto* held = held_task(session)) {
        draw_text(font, ui_state.feedback, 120.0F, 463.0F, 22.0F, cream);
        draw_text(font, "手中：《" + held->title + "》  点击正确分类书架",
                  120.0F, 493.0F, 22.0F, Color{255, 218, 156, 255});
    } else {
        draw_text(font, ui_state.feedback, 120.0F, 476.0F, 24.0F, cream);
    }
    draw_text(font, "ESC 放弃", 750.0F, 496.0F, 20.0F,
              Color{240, 190, 174, 255});

    if (ui_state.show_summary) {
        DrawRectangle(0, 60, 960, 480, Color{20, 28, 30, 165});
        draw_panel(Rectangle{220.0F, 155.0F, 520.0F, 245.0F}, paper, gold);
        draw_centered(font, "书籍整理完成",
                      Rectangle{300.0F, 185.0F, 360.0F, 45.0F}, 34.0F, green);
        const std::string summary =
            "归位 " + std::to_string(session.state().completed_count) +
            " 本 · 错误 " + std::to_string(session.state().wrong_count) + " 次";
        draw_centered(font, summary,
                      Rectangle{255.0F, 250.0F, 450.0F, 45.0F}, 24.0F);
        draw_panel(Rectangle{350.0F, 325.0F, 260.0F, 48.0F}, green, ink);
        draw_centered(font, "返回地图", Rectangle{350.0F, 325.0F, 260.0F, 48.0F},
                      26.0F, RAYWHITE);
    }
}

OrganizingExitAction handle_library_organizing_input(
    LibraryOrganizingSession& session, OrganizingUIState& ui_state,
    Vector2 logical_mouse) {
    if (ui_state.show_summary) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
             CheckCollisionPointRec(logical_mouse,
                                    Rectangle{350.0F, 325.0F, 260.0F, 48.0F}))) {
            return OrganizingExitAction::finish;
        }
        return OrganizingExitAction::none;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        session.give_up();
        return OrganizingExitAction::abandon;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return OrganizingExitAction::none;
    }

    if (session.state().held_book_id.empty()) {
        for (std::size_t index = 0; index < session.tasks().size(); ++index) {
            if (session.state().completed_tasks[index]) {
                continue;
            }
            if (CheckCollisionPointRec(logical_mouse, task_rect(session.tasks()[index]))) {
                ui_state.feedback = session.pick_up(session.tasks()[index].id).message;
                return OrganizingExitAction::none;
            }
        }
        ui_state.feedback = "这里没有待整理的书，请点击带编号的书本标记。";
        return OrganizingExitAction::none;
    }

    for (const auto& shelf : session.shelves()) {
        if (!CheckCollisionPointRec(logical_mouse, canvas_rect(shelf))) {
            continue;
        }
        const auto feedback = session.place_on_shelf(shelf.id);
        ui_state.feedback = feedback.message;
        if (session.is_completed()) {
            ui_state.show_summary = true;
        }
        return OrganizingExitAction::none;
    }
    ui_state.feedback = "请点击书架高亮区域完成归位。";
    return OrganizingExitAction::none;
}

}  // namespace pixel_town::library::ui

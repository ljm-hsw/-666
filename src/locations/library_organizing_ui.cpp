#include "locations/library_organizing_ui.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>

#include "ui/scene_viewport.hpp"

namespace pixel_town::library::ui {
namespace {

constexpr Color slate{60, 79, 82, 255};
constexpr Color ink{45, 52, 54, 255};
constexpr Color paper{250, 238, 203, 245};
constexpr Color cream{255, 248, 226, 245};
constexpr Color green{82, 137, 92, 255};
constexpr Color gold{224, 169, 74, 255};
constexpr Color red{183, 83, 72, 255};

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

void draw_fitted(const Font& font, const std::string& value, Rectangle bounds,
                 float preferred_size, float minimum_size, Color color) {
    float size = preferred_size;
    while (size > minimum_size &&
           MeasureTextEx(font, value.c_str(), size, 1.0F).x > bounds.width) {
        size -= 1.0F;
    }
    draw_text(font, value, bounds.x, bounds.y + (bounds.height - size) / 2.0F,
              size, color);
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

const OrganizingBookTask* held_task(const LibraryOrganizingSession& session) {
    const auto& held_id = session.state().held_book_id;
    const auto found = std::find_if(session.tasks().begin(), session.tasks().end(),
                                    [&held_id](const auto& task) {
                                        return task.id == held_id;
                                    });
    return found == session.tasks().end() ? nullptr : &*found;
}

std::optional<std::size_t> book_texture_index(std::string_view category_id) {
    if (category_id == "history") {
        return 0;
    }
    if (category_id == "science") {
        return 1;
    }
    if (category_id == "literature") {
        return 2;
    }
    if (category_id == "art") {
        return 3;
    }
    if (category_id == "technology") {
        return 4;
    }
    if (category_id == "geography") {
        return 5;
    }
    return std::nullopt;
}

void draw_book_sprite(const std::array<Texture2D, 6>& textures,
                      std::string_view category_id, Rectangle bounds,
                      Color tint = WHITE) {
    const auto texture_index = book_texture_index(category_id);
    const Texture2D* texture = texture_index ? &textures[*texture_index] : nullptr;
    if (texture != nullptr && texture->id != 0) {
        const float scale = std::min(bounds.width / static_cast<float>(texture->width),
                                     bounds.height / static_cast<float>(texture->height));
        const float width = static_cast<float>(texture->width) * scale;
        const float height = static_cast<float>(texture->height) * scale;
        DrawTexturePro(*texture,
                       Rectangle{0.0F, 0.0F, static_cast<float>(texture->width),
                                 static_cast<float>(texture->height)},
                       Rectangle{bounds.x + (bounds.width - width) / 2.0F,
                                 bounds.y + (bounds.height - height) / 2.0F,
                                 width, height},
                       Vector2{0.0F, 0.0F}, 0.0F, tint);
        return;
    }

    // Missing optional art still uses a book-shaped fallback rather than a number.
    const Color cover = category_id == "science" ? Color{55, 91, 138, 255}
                                                   : Color{126, 68, 48, 255};
    DrawRectangleRec(Rectangle{bounds.x + 8.0F, bounds.y + 9.0F,
                               bounds.width - 16.0F, bounds.height - 16.0F},
                     cover);
    DrawRectangleLinesEx(Rectangle{bounds.x + 8.0F, bounds.y + 9.0F,
                                   bounds.width - 16.0F, bounds.height - 16.0F},
                         2.0F, ink);
    DrawLineEx(Vector2{bounds.x + 13.0F, bounds.y + 12.0F},
               Vector2{bounds.x + 13.0F, bounds.y + bounds.height - 10.0F},
               2.0F, gold);
}

}  // namespace

Rectangle reader_mode_button() { return Rectangle{180.0F, 225.0F, 270.0F, 150.0F}; }

Rectangle organizing_mode_button() { return Rectangle{510.0F, 225.0F, 270.0F, 150.0F}; }

Rectangle organizing_book_hitbox(const OrganizingBookTask& task) {
    const auto center = ::pixel_town::ui::scene_canvas_to_viewport(
        ::pixel_town::ui::SceneViewportPoint{static_cast<float>(task.x),
                                             static_cast<float>(task.y)});
    return Rectangle{center.x - 29.0F, center.y - 24.0F, 58.0F, 48.0F};
}

Rectangle organizing_held_book_slot() {
    return Rectangle{625.0F, 461.0F, 130.0F, 54.0F};
}

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
    draw_panel(reader, CheckCollisionPointRec(logical_mouse, scaled_rect(reader)) ? cream : paper,
               gold);
    draw_panel(organizing,
               CheckCollisionPointRec(logical_mouse, scaled_rect(organizing)) ? cream : paper, gold);
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
                             const std::array<Texture2D, 6>& book_textures,
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
        const Rectangle bounds = organizing_book_hitbox(task);
        const bool hovered = CheckCollisionPointRec(logical_mouse, bounds);
        const Color status_color =
            task.source == OrganizingBookSource::misplaced ? red : green;
        DrawCircleV(Vector2{bounds.x + bounds.width / 2.0F,
                            bounds.y + bounds.height / 2.0F},
                    hovered ? 28.0F : 24.0F,
                    Color{status_color.r, status_color.g, status_color.b,
                          static_cast<unsigned char>(hovered ? 125 : 72)});
        draw_book_sprite(book_textures, task.category_id,
                         Rectangle{bounds.x + 7.0F, bounds.y + 2.0F,
                                   bounds.width - 14.0F, bounds.height - 4.0F});
        DrawRectangleLinesEx(bounds, hovered ? 3.0F : 1.0F,
                             hovered ? gold : Color{cream.r, cream.g, cream.b, 150});
        if (hovered) {
            const std::string label =
                (task.source == OrganizingBookSource::misplaced ? "错架：" : "散落：") +
                task.title;
            const Rectangle tooltip{
                std::clamp(bounds.x - 50.0F, 70.0F, 730.0F), bounds.y - 32.0F,
                160.0F, 28.0F};
            DrawRectangleRec(tooltip, Color{38, 54, 56, 235});
            DrawRectangleLinesEx(tooltip, 2.0F, gold);
            draw_centered(font, label, tooltip, 18.0F, cream);
        }
    }

    draw_panel(Rectangle{95.0F, 454.0F, 770.0F, 70.0F},
               Color{38, 54, 56, 232}, gold);
    const Rectangle held_slot = organizing_held_book_slot();
    DrawRectangleRec(held_slot, Color{247, 232, 196, 235});
    DrawRectangleLinesEx(held_slot, 2.0F, gold);
    if (const auto* held = held_task(session)) {
        draw_fitted(font, ui_state.feedback,
                    Rectangle{115.0F, 459.0F, 480.0F, 27.0F}, 22.0F, 17.0F,
                    cream);
        draw_fitted(font, "已拾取《" + held->title + "》，点击正确分类书架",
                    Rectangle{115.0F, 490.0F, 480.0F, 26.0F}, 21.0F, 16.0F,
                    Color{255, 218, 156, 255});
        draw_book_sprite(book_textures, held->category_id,
                         Rectangle{held_slot.x + 4.0F, held_slot.y + 5.0F,
                                   44.0F, 44.0F});
        draw_fitted(font, held->title,
                    Rectangle{held_slot.x + 48.0F, held_slot.y + 5.0F,
                              held_slot.width - 54.0F, 44.0F},
                    18.0F, 14.0F, ink);
    } else {
        draw_fitted(font, ui_state.feedback,
                    Rectangle{115.0F, 467.0F, 480.0F, 38.0F}, 23.0F, 17.0F,
                    cream);
        draw_centered(font, "手中空", held_slot, 18.0F,
                      Color{93, 101, 99, 255});
    }
    draw_text(font, "ESC 放弃", 775.0F, 496.0F, 18.0F,
              Color{240, 190, 174, 255});
}

OrganizingExitAction place_held_book(LibraryOrganizingSession& session,
                                     OrganizingUIState& ui_state,
                                     const std::string& shelf_id) {
    const auto feedback = session.place_on_shelf(shelf_id);
    ui_state.feedback = feedback.message;
    if (feedback.status == OrganizingActionStatus::accepted && session.is_completed()) {
        ui_state.feedback = "分类正确，全部书籍已归位，正在结算。";
        return OrganizingExitAction::finish;
    }
    return OrganizingExitAction::none;
}

OrganizingExitAction handle_library_organizing_input(
    LibraryOrganizingSession& session, OrganizingUIState& ui_state,
    Vector2 logical_mouse) {
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
            if (CheckCollisionPointRec(logical_mouse,
                                       organizing_book_hitbox(session.tasks()[index]))) {
                ui_state.feedback = session.pick_up(session.tasks()[index].id).message;
                return OrganizingExitAction::none;
            }
        }
        ui_state.feedback = "这里没有待整理的书，请点击带光圈的书本素材。";
        return OrganizingExitAction::none;
    }

    for (const auto& shelf : session.shelves()) {
        if (!CheckCollisionPointRec(logical_mouse, canvas_rect(shelf))) {
            continue;
        }
        return place_held_book(session, ui_state, shelf.id);
    }
    ui_state.feedback = "请点击书架高亮区域完成归位。";
    return OrganizingExitAction::none;
}

}  // namespace pixel_town::library::ui

#pragma once

#include <array>
#include <string>

#include <raylib.h>

#include "locations/library_organizing.hpp"

namespace pixel_town::library::ui {

struct OrganizingUIState {
    std::string feedback{"点击场景中的书本拾取，再点击对应分类书架归位。"};
};

enum class OrganizingExitAction {
    none,
    finish,
    abandon,
};

[[nodiscard]] Rectangle reader_mode_button();
[[nodiscard]] Rectangle organizing_mode_button();
[[nodiscard]] Rectangle organizing_book_hitbox(const OrganizingBookTask& task);
[[nodiscard]] Rectangle organizing_held_book_slot();

void draw_library_mode_selection(const Font& font, const Texture2D& background,
                                 Vector2 logical_mouse);
void draw_library_organizing(const Font& font, const Texture2D& background,
                             const std::array<Texture2D, 6>& book_textures,
                             const LibraryOrganizingSession& session,
                             const OrganizingUIState& ui_state,
                             Vector2 logical_mouse);
[[nodiscard]] OrganizingExitAction place_held_book(
    LibraryOrganizingSession& session, OrganizingUIState& ui_state,
    const std::string& shelf_id);
[[nodiscard]] OrganizingExitAction handle_library_organizing_input(
    LibraryOrganizingSession& session, OrganizingUIState& ui_state,
    Vector2 logical_mouse);

}  // namespace pixel_town::library::ui

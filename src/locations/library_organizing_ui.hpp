#pragma once

#include <string>

#include <raylib.h>

#include "locations/library_organizing.hpp"

namespace pixel_town::library::ui {

struct OrganizingUIState {
    std::string feedback{"先点击散落或错架的书，再点击正确分类书架。"};
    bool show_summary{false};
};

enum class OrganizingExitAction {
    none,
    finish,
    abandon,
};

[[nodiscard]] Rectangle reader_mode_button();
[[nodiscard]] Rectangle organizing_mode_button();

void draw_library_mode_selection(const Font& font, const Texture2D& background,
                                 Vector2 logical_mouse);
void draw_library_organizing(const Font& font, const Texture2D& background,
                             const LibraryOrganizingSession& session,
                             const OrganizingUIState& ui_state,
                             Vector2 logical_mouse);
[[nodiscard]] OrganizingExitAction handle_library_organizing_input(
    LibraryOrganizingSession& session, OrganizingUIState& ui_state,
    Vector2 logical_mouse);

}  // namespace pixel_town::library::ui

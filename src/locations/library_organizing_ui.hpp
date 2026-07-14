#pragma once

#include <array>

#include <raylib.h>

#include "locations/library_runtime_contract.hpp"

namespace pixel_town::library::ui {

[[nodiscard]] Rectangle reader_mode_button();
[[nodiscard]] Rectangle organizing_mode_button();
[[nodiscard]] Rectangle organizing_book_hitbox(const OrganizingBookTask& task);
[[nodiscard]] Rectangle organizing_held_book_slot();

void draw_library_mode_selection(const Font& font, const Texture2D& background,
                                 Vector2 logical_mouse);
void draw_library_organizing(const Font& font, const Texture2D& background,
                             const std::array<Texture2D, 6>& book_textures,
                             const LibraryOrganizingPresentation& presentation,
                             Vector2 logical_mouse);
[[nodiscard]] LibraryIntent handle_library_organizing_input(
    const LibraryOrganizingPresentation& presentation,
    Vector2 logical_mouse);

}  // namespace pixel_town::library::ui

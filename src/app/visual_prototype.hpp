#pragma once

#include <raylib.h>

namespace pixel_town {

struct VisualPrototypeState {
    int variant{0};
    int selected_location{0};
    bool modal_open{true};
};

[[nodiscard]] const char* visual_prototype_glyphs();
void update_visual_prototype(VisualPrototypeState& state, Vector2 logical_mouse);
void draw_visual_prototype(const Font& font, const Texture2D& town_marker,
                           const Texture2D& kenney_tiles,
                           const VisualPrototypeState& state, bool audio_enabled,
                           Vector2 logical_mouse);

}  // namespace pixel_town

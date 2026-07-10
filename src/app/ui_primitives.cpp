#include "app/ui_primitives.hpp"

#include <cmath>
#include <sstream>

namespace pixel_town {

float scaled(float value) {
    return std::round(value * ui::design_to_canvas_scale);
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
    return Rectangle{scaled(value.x), scaled(value.y), scaled(value.width),
                     scaled(value.height)};
}

void text(const Font& font, const char* value, float x, float y, float size,
          Color color) {
    DrawTextEx(font, value, Vector2{scaled(x), scaled(y)}, scaled_font_size(size), 1.0F,
               color);
}

void text(const Font& font, const std::string& value, float x, float y, float size,
          Color color) {
    text(font, value.c_str(), x, y, size, color);
}

void text_block(const Font& font, const char* value, float x, float y, float size,
                float line_gap, Color color) {
    std::stringstream stream(value);
    std::string line;
    float offset = 0.0F;
    while (std::getline(stream, line)) {
        text(font, line, x, y + offset, size, color);
        offset += line_gap;
    }
}

void text_block(const Font& font, const std::string& value, float x, float y,
                float size, float line_gap, Color color) {
    text_block(font, value.c_str(), x, y, size, line_gap, color);
}

void centered_text(const Font& font, const char* value, Rectangle bounds,
                   float size, Color color) {
    const Rectangle scaled_bounds = scaled_rect(bounds);
    const float font_size = scaled_font_size(size);
    const Vector2 measured = MeasureTextEx(font, value, font_size, 1.0F);
    DrawTextEx(font, value,
               Vector2{scaled_bounds.x + (scaled_bounds.width - measured.x) / 2.0F,
                       scaled_bounds.y + (scaled_bounds.height - measured.y) / 2.0F},
               font_size, 1.0F, color);
}

void panel(Rectangle bounds, Color fill, Color border) {
    const Rectangle scaled_bounds = scaled_rect(bounds);
    DrawRectangleRec(Rectangle{scaled_bounds.x + 4, scaled_bounds.y + 4,
                               scaled_bounds.width, scaled_bounds.height},
                     shadow);
    DrawRectangleRec(scaled_bounds, fill);
    DrawRectangleLinesEx(scaled_bounds, 3.0F, border);
}

bool clicked(Rectangle bounds, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, scaled_rect(bounds)) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool activated(Rectangle bounds, Vector2 mouse, KeyboardKey key) {
    return clicked(bounds, mouse) || IsKeyPressed(key);
}

bool hovered(Rectangle bounds, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, scaled_rect(bounds));
}

}  // namespace pixel_town

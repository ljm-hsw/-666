#include "app/ui_primitives.hpp"

#include <cmath>
#include <sstream>

namespace pixel_town {
namespace {

std::size_t utf8_codepoint_length(unsigned char leading_byte) {
    if ((leading_byte & 0x80U) == 0U) {
        return 1;
    }
    if ((leading_byte & 0xE0U) == 0xC0U) {
        return 2;
    }
    if ((leading_byte & 0xF0U) == 0xE0U) {
        return 3;
    }
    if ((leading_byte & 0xF8U) == 0xF0U) {
        return 4;
    }
    return 1;
}

void replace_last_codepoint_with_ellipsis(std::string& line) {
    if (line.empty()) {
        line = "…";
        return;
    }
    std::size_t offset = line.size() - 1;
    while (offset > 0 &&
           (static_cast<unsigned char>(line[offset]) & 0xC0U) == 0x80U) {
        --offset;
    }
    line.erase(offset);
    line += "…";
}

}  // namespace

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

std::vector<std::string> wrap_text_lines(const std::string& value,
                                         std::size_t max_codepoints_per_line,
                                         std::size_t max_lines) {
    if (value.empty() || max_codepoints_per_line == 0 || max_lines == 0) {
        return {};
    }

    std::vector<std::string> lines;
    std::string current;
    std::size_t current_count = 0;
    bool truncated = false;
    for (std::size_t offset = 0; offset < value.size();) {
        if (value[offset] == '\n') {
            lines.push_back(current);
            current.clear();
            current_count = 0;
            ++offset;
            if (lines.size() == max_lines && offset < value.size()) {
                truncated = true;
                break;
            }
            continue;
        }

        if (current_count == max_codepoints_per_line) {
            lines.push_back(current);
            current.clear();
            current_count = 0;
            if (lines.size() == max_lines) {
                truncated = true;
                break;
            }
        }

        std::size_t length = utf8_codepoint_length(
            static_cast<unsigned char>(value[offset]));
        if (offset + length > value.size()) {
            length = 1;
        }
        current.append(value, offset, length);
        offset += length;
        ++current_count;
    }

    if (!truncated && !current.empty() && lines.size() < max_lines) {
        lines.push_back(current);
    }
    if (truncated && !lines.empty()) {
        replace_last_codepoint_with_ellipsis(lines.back());
    }
    return lines;
}

}  // namespace pixel_town

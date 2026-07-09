#pragma once

#include <string>

#include <raylib.h>

namespace pixel_town {

inline constexpr Color ink{45, 52, 54, 255};
inline constexpr Color paper{250, 238, 203, 255};
inline constexpr Color cream{255, 248, 226, 255};
inline constexpr Color green{82, 137, 92, 255};
inline constexpr Color grass{144, 190, 119, 255};
inline constexpr Color road{194, 170, 121, 255};
inline constexpr Color disabled{145, 143, 132, 255};
inline constexpr Color red{183, 83, 72, 255};
inline constexpr Color gold{224, 169, 74, 255};
inline constexpr Color shadow{39, 48, 53, 120};
inline constexpr Color slate{60, 79, 82, 255};
inline constexpr float native_ui_scale = 1.5F;

[[nodiscard]] float scaled(float value);
[[nodiscard]] float scaled_font_size(float design_size);
[[nodiscard]] Vector2 scaled_point(Vector2 value);
[[nodiscard]] Rectangle scaled_rect(Rectangle value);

void text(const Font& font, const char* value, float x, float y, float size,
          Color color = ink);
void text(const Font& font, const std::string& value, float x, float y, float size,
          Color color = ink);
void text_block(const Font& font, const char* value, float x, float y, float size,
                float line_gap, Color color = ink);
void text_block(const Font& font, const std::string& value, float x, float y,
                float size, float line_gap, Color color = ink);
void centered_text(const Font& font, const char* value, Rectangle bounds,
                   float size, Color color = ink);
void panel(Rectangle bounds, Color fill, Color border = ink);

[[nodiscard]] bool clicked(Rectangle bounds, Vector2 mouse);
[[nodiscard]] bool activated(Rectangle bounds, Vector2 mouse, KeyboardKey key);
[[nodiscard]] bool hovered(Rectangle bounds, Vector2 mouse);

}  // namespace pixel_town

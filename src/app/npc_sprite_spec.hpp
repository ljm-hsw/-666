#pragma once

namespace pixel_town {

enum class NpcSpriteKind {
    salesclerk,
    bartender,
    restaurant_chef,
    librarian,
};

struct NpcSpriteSpec {
    const char* runtime_path;
    int frame_width;
    int frame_height;
    int frame_count;
    float frame_duration_seconds;
};

[[nodiscard]] const NpcSpriteSpec& npc_sprite_spec(NpcSpriteKind kind);
[[nodiscard]] int npc_sprite_frame(const NpcSpriteSpec& spec,
                                   float animation_seconds) noexcept;

}  // namespace pixel_town

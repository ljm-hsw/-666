#include "app/npc_sprite_view.hpp"

namespace pixel_town {

void draw_npc_sprite(const Texture2D& texture, NpcSpriteKind kind,
                     float animation_seconds, Rectangle destination,
                     Color tint) {
    if (texture.id == 0) {
        return;
    }
    const NpcSpriteSpec& spec = npc_sprite_spec(kind);
    const int frame = npc_sprite_frame(spec, animation_seconds);
    const Rectangle source{static_cast<float>(frame * spec.frame_width), 0.0F,
                           static_cast<float>(spec.frame_width),
                           static_cast<float>(spec.frame_height)};
    DrawTexturePro(texture, source, destination, Vector2{0.0F, 0.0F}, 0.0F,
                   tint);
}

}  // namespace pixel_town

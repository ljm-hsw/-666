#pragma once

#include <raylib.h>

#include "app/npc_sprite_spec.hpp"

namespace pixel_town {

void draw_npc_sprite(const Texture2D& texture, NpcSpriteKind kind,
                     float animation_seconds, Rectangle destination,
                     Color tint = WHITE);

}  // namespace pixel_town

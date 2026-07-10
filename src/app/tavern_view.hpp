#pragma once

#include <raylib.h>

#include "app/tavern_runtime.hpp"

namespace pixel_town {

void draw_tavern_view(const Font& font, const GameSession& session,
                      const TavernRuntimeState& runtime, Vector2 logical_mouse);

}  // namespace pixel_town

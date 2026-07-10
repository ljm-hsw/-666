#pragma once

#include <raylib.h>

#include "app/tavern_runtime.hpp"

namespace pixel_town {

struct TavernVisualAssets {
    Texture2D lobby_background{};
    Texture2D bartender_sheet{};
    bool attempted{false};
};

void ensure_tavern_assets_loaded(TavernVisualAssets& assets);
void unload_tavern_assets(TavernVisualAssets& assets);

void draw_tavern_view(const Font& font, const TavernPresentation& presentation,
                      const TavernVisualAssets& assets, Vector2 logical_mouse);

}  // namespace pixel_town

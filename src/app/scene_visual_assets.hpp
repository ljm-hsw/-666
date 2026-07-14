#pragma once

#include <array>

#include <raylib.h>

#include "core/game_session.hpp"

namespace pixel_town {

struct SceneVisualAssets {
    Texture2D restaurant_interior{};
    Texture2D convenience_store_interior{};
    Texture2D home_interior{};
    Texture2D library_interior{};
    std::array<Texture2D, 6> library_organizing_books{};
};

void load_scene_visual_assets(SceneVisualAssets& assets);
void unload_scene_visual_assets(SceneVisualAssets& assets);
[[nodiscard]] const Texture2D& scene_interior_texture(
    const SceneVisualAssets& assets, Location location);

}  // namespace pixel_town

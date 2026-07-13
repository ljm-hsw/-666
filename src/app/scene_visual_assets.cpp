#include "app/scene_visual_assets.hpp"

#include <filesystem>

namespace pixel_town {
namespace {

Texture2D load_optional_texture(const char* path) {
    if (!std::filesystem::is_regular_file(path)) {
        return {};
    }
    Texture2D texture = LoadTexture(path);
    if (texture.id != 0) {
        SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    }
    return texture;
}

void unload_texture(Texture2D& texture) {
    if (texture.id != 0) {
        UnloadTexture(texture);
        texture = {};
    }
}

}  // namespace

void load_scene_visual_assets(SceneVisualAssets& assets) {
    if (assets.restaurant_interior.id == 0) {
        assets.restaurant_interior =
            load_optional_texture("assets/textures/ui/restaurant/restaurant_interior.png");
    }
    if (assets.home_interior.id == 0) {
        assets.home_interior =
            load_optional_texture("assets/textures/ui/home/home_interior.png");
    }
    if (assets.library_interior.id == 0) {
        assets.library_interior =
            load_optional_texture("assets/textures/ui/library/library_interior.png");
    }
}

void unload_scene_visual_assets(SceneVisualAssets& assets) {
    unload_texture(assets.restaurant_interior);
    unload_texture(assets.home_interior);
    unload_texture(assets.library_interior);
}

const Texture2D& scene_interior_texture(const SceneVisualAssets& assets,
                                        Location location) {
    switch (location) {
        case Location::restaurant:
            return assets.restaurant_interior;
        case Location::home:
            return assets.home_interior;
        case Location::library:
            return assets.library_interior;
        case Location::convenience_store:
        case Location::tavern:
            break;
    }
    static const Texture2D missing{};
    return missing;
}

}  // namespace pixel_town

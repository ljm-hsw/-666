#include "app/scene_visual_assets.hpp"

#include <filesystem>

#include "app/npc_sprite_spec.hpp"

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
    if (assets.convenience_store_interior.id == 0) {
        assets.convenience_store_interior = load_optional_texture(
            "assets/textures/ui/convenience_store/convenience_store_interior.png");
    }
    if (assets.home_interior.id == 0) {
        assets.home_interior =
            load_optional_texture("assets/textures/ui/home/home_interior.png");
    }
    if (assets.library_interior.id == 0) {
        assets.library_interior =
            load_optional_texture("assets/textures/ui/library/library_interior.png");
    }
    if (assets.restaurant_npc.id == 0) {
        assets.restaurant_npc = load_optional_texture(
            npc_sprite_spec(NpcSpriteKind::restaurant_chef).runtime_path);
    }
    if (assets.convenience_store_npc.id == 0) {
        assets.convenience_store_npc = load_optional_texture(
            npc_sprite_spec(NpcSpriteKind::salesclerk).runtime_path);
    }
    if (assets.library_npc.id == 0) {
        assets.library_npc = load_optional_texture(
            npc_sprite_spec(NpcSpriteKind::librarian).runtime_path);
    }
    if (assets.protagonist.id == 0) {
        assets.protagonist = load_optional_texture(
            npc_sprite_spec(NpcSpriteKind::protagonist).runtime_path);
    }
    constexpr std::array<const char*, 6> organizing_book_paths = {
        "assets/textures/ui/library/organizing_books/history.png",
        "assets/textures/ui/library/organizing_books/science.png",
        "assets/textures/ui/library/organizing_books/literature.png",
        "assets/textures/ui/library/organizing_books/art.png",
        "assets/textures/ui/library/organizing_books/technology.png",
        "assets/textures/ui/library/organizing_books/geography.png",
    };
    for (std::size_t index = 0; index < organizing_book_paths.size(); ++index) {
        if (assets.library_organizing_books[index].id == 0) {
            assets.library_organizing_books[index] =
                load_optional_texture(organizing_book_paths[index]);
        }
    }
}

void unload_scene_visual_assets(SceneVisualAssets& assets) {
    unload_texture(assets.restaurant_interior);
    unload_texture(assets.convenience_store_interior);
    unload_texture(assets.home_interior);
    unload_texture(assets.library_interior);
    unload_texture(assets.restaurant_npc);
    unload_texture(assets.convenience_store_npc);
    unload_texture(assets.library_npc);
    unload_texture(assets.protagonist);
    for (auto& texture : assets.library_organizing_books) {
        unload_texture(texture);
    }
}

const Texture2D& scene_npc_texture(const SceneVisualAssets& assets,
                                   Location location) {
    switch (location) {
        case Location::restaurant:
            return assets.restaurant_npc;
        case Location::convenience_store:
            return assets.convenience_store_npc;
        case Location::library:
            return assets.library_npc;
        case Location::home:
        case Location::tavern:
            break;
    }
    static const Texture2D missing{};
    return missing;
}

const Texture2D& scene_interior_texture(const SceneVisualAssets& assets,
                                        Location location) {
    switch (location) {
        case Location::restaurant:
            return assets.restaurant_interior;
        case Location::convenience_store:
            return assets.convenience_store_interior;
        case Location::home:
            return assets.home_interior;
        case Location::library:
            return assets.library_interior;
        case Location::tavern:
            break;
    }
    static const Texture2D missing{};
    return missing;
}

}  // namespace pixel_town

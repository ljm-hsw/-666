#include "app/npc_sprite_spec.hpp"

namespace pixel_town {
namespace {

constexpr NpcSpriteSpec salesclerk{
    "assets/textures/ui/convenience_store/npc/salesclerk_idle_sheet.png",
    64, 96, 4, 0.18F};
constexpr NpcSpriteSpec bartender{
    "assets/textures/ui/tavern/npc/bartender_idle_sheet.png",
    64, 96, 4, 0.18F};
constexpr NpcSpriteSpec restaurant_chef{
    "assets/textures/ui/restaurant/npc/chef_idle_sheet.png",
    64, 96, 4, 0.18F};
constexpr NpcSpriteSpec librarian{
    "assets/textures/ui/library/npc/librarian_idle_sheet.png",
    64, 96, 4, 0.18F};
constexpr NpcSpriteSpec protagonist{
    "assets/textures/ui/characters/protagonist_idle_sheet.png",
    64, 96, 4, 0.18F};

}  // namespace

const NpcSpriteSpec& npc_sprite_spec(NpcSpriteKind kind) {
    switch (kind) {
        case NpcSpriteKind::salesclerk:
            return salesclerk;
        case NpcSpriteKind::bartender:
            return bartender;
        case NpcSpriteKind::restaurant_chef:
            return restaurant_chef;
        case NpcSpriteKind::librarian:
            return librarian;
        case NpcSpriteKind::protagonist:
            return protagonist;
    }
    return salesclerk;
}

int npc_sprite_frame(const NpcSpriteSpec& spec,
                     float animation_seconds) noexcept {
    if (animation_seconds <= 0.0F || spec.frame_duration_seconds <= 0.0F ||
        spec.frame_count <= 0) {
        return 0;
    }
    return static_cast<int>(animation_seconds / spec.frame_duration_seconds) %
           spec.frame_count;
}

}  // namespace pixel_town

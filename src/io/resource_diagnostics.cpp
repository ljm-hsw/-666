#include "io/resource_diagnostics.hpp"

#include <cctype>
#include <fstream>
#include <string>

namespace pixel_town {
namespace {

std::string read_prefix(const std::filesystem::path& path, std::size_t size = 8) {
    std::ifstream input(path, std::ios::binary);
    std::string prefix(size, '\0');
    input.read(prefix.data(), static_cast<std::streamsize>(prefix.size()));
    prefix.resize(static_cast<std::size_t>(input.gcount()));
    return prefix;
}

bool contains_non_whitespace(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    char value = '\0';
    while (input.get(value)) {
        if (!std::isspace(static_cast<unsigned char>(value))) {
            return true;
        }
    }
    return false;
}

bool has_valid_signature(const std::filesystem::path& path, ResourceKind kind) {
    const std::string prefix = read_prefix(path);
    switch (kind) {
        case ResourceKind::font:
            return prefix.compare(0, 4, "\0\1\0\0", 4) == 0 || prefix.compare(0, 4, "OTTO") == 0;
        case ResourceKind::texture:
            return prefix.compare(0, 8, "\x89PNG\r\n\x1a\n", 8) == 0;
        case ResourceKind::data:
            return contains_non_whitespace(path);
        case ResourceKind::audio: {
            const bool mp3_frame =
                prefix.size() >= 2 && static_cast<unsigned char>(prefix[0]) == 0xffU &&
                (static_cast<unsigned char>(prefix[1]) & 0xe0U) == 0xe0U;
            return prefix.compare(0, 4, "OggS") == 0 ||
                   prefix.compare(0, 4, "RIFF") == 0 ||
                   prefix.compare(0, 3, "ID3") == 0 || mp3_frame;
        }
    }
    return false;
}

}  // namespace

ResourceReport validate_resources(const std::filesystem::path& root,
                                  const std::vector<ResourceSpec>& manifest) {
    ResourceReport report;

    for (const ResourceSpec& resource : manifest) {
        const std::filesystem::path path = root / resource.relative_path;
        std::error_code error;
        const bool is_file = std::filesystem::is_regular_file(path, error);
        const bool is_valid = is_file && !error && has_valid_signature(path, resource.kind);
        if (is_valid) {
            continue;
        }

        const std::string message = is_file && !error ? "invalid resource" : "missing resource";
        report.issues.push_back(ResourceIssue{resource.relative_path, message, resource.required});
        if (resource.required) {
            report.can_start = false;
        }
        if (resource.kind == ResourceKind::audio) {
            report.audio_enabled = false;
        }
    }

    return report;
}

std::vector<ResourceSpec> baseline_resource_manifest() {
    return {
        {"fonts/fusion-pixel-12px-proportional-zh_hans.ttf", ResourceKind::font, true},
        {"textures/town_marker.png", ResourceKind::texture, true},
        {"textures/kenney_tiny_town/Tilemap/tilemap_packed.png", ResourceKind::texture, false},
        {"textures/kenney_tiny_farm/Tilemap/tilemap_packed.png", ResourceKind::texture, false},
        {"textures/imagegen_buildings/p1_building_sprites.png", ResourceKind::texture, false},
        {"textures/ui/tavern/tavern_lobby.png", ResourceKind::texture, false},
        {"textures/ui/convenience_store/convenience_store_interior.png",
         ResourceKind::texture, false},
        {"textures/ui/tavern/npc/bartender_idle_sheet.png", ResourceKind::texture, false},
        {"textures/ui/restaurant/npc/chef_idle_sheet.png", ResourceKind::texture, false},
        {"textures/ui/convenience_store/npc/salesclerk_idle_sheet.png",
         ResourceKind::texture, false},
        {"textures/ui/library/npc/librarian_idle_sheet.png", ResourceKind::texture, false},
        {"textures/ui/characters/protagonist_idle_sheet.png", ResourceKind::texture,
         false},
        {"textures/ui/characters/mayor_idle_sheet.png", ResourceKind::texture,
         false},
        {"textures/ui/restaurant/restaurant_interior.png", ResourceKind::texture, false},
        {"textures/ui/home/home_interior.png", ResourceKind::texture, false},
        {"textures/ui/library/library_interior.png", ResourceKind::texture, false},
        {"textures/ui/library/organizing_books/history.png", ResourceKind::texture, false},
        {"textures/ui/library/organizing_books/science.png", ResourceKind::texture, false},
        {"textures/ui/library/organizing_books/literature.png", ResourceKind::texture,
         false},
        {"textures/ui/library/organizing_books/art.png", ResourceKind::texture, false},
        {"textures/ui/library/organizing_books/technology.png", ResourceKind::texture,
         false},
        {"textures/ui/library/organizing_books/geography.png", ResourceKind::texture,
         false},
        {"data/baseline.txt", ResourceKind::data, true},
        {"data/library_data.txt", ResourceKind::data, false},
        {"audio/bgm_main_map.mp3", ResourceKind::audio, false},
        {"audio/bgm_rainy_day.mp3", ResourceKind::audio, false},
        {"audio/bgm_restaurant.mp3", ResourceKind::audio, false},
        {"audio/bgm_store.mp3", ResourceKind::audio, false},
        {"audio/bgm_library.mp3", ResourceKind::audio, false},
        {"audio/bgm_tavern.mp3", ResourceKind::audio, false},
        {"audio/bgm_home.mp3", ResourceKind::audio, false},
        {"audio/sfx_location_switch.mp3", ResourceKind::audio, false},
        {"audio/sfx_return_home.mp3", ResourceKind::audio, false},
        {"audio/sfx_success.mp3", ResourceKind::audio, false},
        {"audio/sfx_failure.mp3", ResourceKind::audio, false},
    };
}

}  // namespace pixel_town

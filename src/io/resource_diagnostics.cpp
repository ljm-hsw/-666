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
        case ResourceKind::audio:
            return prefix.compare(0, 4, "OggS") == 0 || prefix.compare(0, 4, "RIFF") == 0;
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
        {"textures/ui/tavern/npc/bartender_idle_sheet.png", ResourceKind::texture, false},
        {"data/baseline.txt", ResourceKind::data, true},
        {"data/library_data.txt", ResourceKind::data, false},
        {"audio/theme.ogg", ResourceKind::audio, false},
    };
}

}  // namespace pixel_town

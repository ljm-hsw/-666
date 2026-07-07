#include "io/demo_preset.hpp"

#include <string_view>

#include "io/save_game.hpp"

namespace pixel_town {
namespace {

bool is_valid_preset_id(std::string_view id) {
    if (id.empty()) {
        return false;
    }
    for (char value : id) {
        const bool digit = value >= '0' && value <= '9';
        const bool lower = value >= 'a' && value <= 'z';
        if (!digit && !lower && value != '-') {
            return false;
        }
    }
    return true;
}

}  // namespace

DemoPresetArgs parse_demo_preset_args(int argc, const char* const argv[]) {
    DemoPresetArgs result;
    for (int index = 1; index < argc; ++index) {
        if (std::string_view{argv[index]} != "--demo-preset") {
            continue;
        }
        result.requested = true;
        if (index + 1 >= argc) {
            result.error = "missing demo preset id";
            return result;
        }
        result.id = argv[index + 1];
        if (!is_valid_preset_id(result.id)) {
            result.error = "demo preset id contains unsupported characters";
            return result;
        }
        if (index + 2 != argc) {
            result.error = "demo preset mode does not accept extra arguments";
            return result;
        }
        return result;
    }
    return result;
}

std::filesystem::path demo_preset_path(const std::filesystem::path& assets_root,
                                       std::string_view id) {
    return assets_root / "demo_presets" / (std::string{id} + ".sav");
}

DemoPresetLoadResult load_demo_preset(const std::filesystem::path& assets_root,
                                      std::string_view id) {
    if (!is_valid_preset_id(id)) {
        return {DemoPresetStatus::corrupt, GameSession::new_game(), std::string{id},
                "demo preset id is invalid"};
    }

    const auto loaded = load_session(demo_preset_path(assets_root, id));
    switch (loaded.status) {
        case SaveStatus::ok:
            return {DemoPresetStatus::ok, loaded.session, std::string{id},
                    "demo preset loaded"};
        case SaveStatus::not_found:
            return {DemoPresetStatus::not_found, GameSession::new_game(), std::string{id},
                    "demo preset was not found"};
        case SaveStatus::incompatible_version:
            return {DemoPresetStatus::incompatible_version, GameSession::new_game(),
                    std::string{id}, "demo preset version is not compatible"};
        case SaveStatus::corrupt:
        case SaveStatus::already_exists:
        case SaveStatus::write_failed:
            return {DemoPresetStatus::corrupt, GameSession::new_game(), std::string{id},
                    loaded.message.empty() ? "demo preset is corrupt" : loaded.message};
    }
    return {DemoPresetStatus::corrupt, GameSession::new_game(), std::string{id},
            "demo preset could not be loaded"};
}

}  // namespace pixel_town

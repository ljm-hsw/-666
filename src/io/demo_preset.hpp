#pragma once

#include <filesystem>
#include <string>

#include "core/game_session.hpp"

namespace pixel_town {

struct DemoPresetArgs {
    bool requested{false};
    std::string id;
    std::string error;
};

enum class DemoPresetStatus {
    ok,
    not_requested,
    not_found,
    corrupt,
    incompatible_version,
};

struct DemoPresetLoadResult {
    DemoPresetStatus status{DemoPresetStatus::not_requested};
    GameSession session{GameSession::new_game()};
    std::string id;
    std::string message;
};

[[nodiscard]] DemoPresetArgs parse_demo_preset_args(int argc, const char* const argv[]);
[[nodiscard]] std::filesystem::path demo_preset_path(const std::filesystem::path& assets_root,
                                                     std::string_view id);
[[nodiscard]] DemoPresetLoadResult load_demo_preset(const std::filesystem::path& assets_root,
                                                    std::string_view id);

}  // namespace pixel_town

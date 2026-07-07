#pragma once

#include <filesystem>
#include <optional>

namespace pixel_town {

struct AppSettings {
    bool muted{false};
};

[[nodiscard]] std::filesystem::path default_settings_path(
    const std::filesystem::path& application_directory);
[[nodiscard]] std::optional<AppSettings> load_app_settings(const std::filesystem::path& path);
[[nodiscard]] bool save_app_settings(const std::filesystem::path& path,
                                     const AppSettings& settings);

}  // namespace pixel_town

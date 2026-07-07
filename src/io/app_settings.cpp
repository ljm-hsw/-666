#include "io/app_settings.hpp"

#include <fstream>
#include <string>

namespace pixel_town {

std::filesystem::path default_settings_path(const std::filesystem::path& application_directory) {
    return application_directory / "saves" / "settings.ini";
}

std::optional<AppSettings> load_app_settings(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        return AppSettings{};
    }

    std::ifstream input(path);
    if (!input) {
        return std::nullopt;
    }

    AppSettings settings;
    std::string line;
    while (std::getline(input, line)) {
        if (line == "muted=1") {
            settings.muted = true;
        } else if (line == "muted=0") {
            settings.muted = false;
        } else if (!line.empty()) {
            return std::nullopt;
        }
    }
    return settings;
}

bool save_app_settings(const std::filesystem::path& path, const AppSettings& settings) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return false;
    }

    const std::filesystem::path temporary = path.string() + ".tmp";
    {
        std::ofstream output(temporary, std::ios::trunc);
        if (!output) {
            return false;
        }
        output << "muted=" << (settings.muted ? "1" : "0") << '\n';
        output.flush();
        if (!output) {
            return false;
        }
    }

    std::filesystem::remove(path, error);
    error.clear();
    std::filesystem::rename(temporary, path, error);
    if (error) {
        std::filesystem::remove(temporary, error);
        return false;
    }
    return true;
}

}  // namespace pixel_town

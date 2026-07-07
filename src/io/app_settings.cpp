#include "io/app_settings.hpp"

#include <cstdio>
#include <fstream>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace pixel_town {
namespace {

bool replace_file_atomically(const std::filesystem::path& temporary,
                             const std::filesystem::path& destination) {
#ifdef _WIN32
    return MoveFileExW(temporary.wstring().c_str(), destination.wstring().c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(temporary.c_str(), destination.c_str()) == 0;
#endif
}

}  // namespace

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
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
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

    if (!replace_file_atomically(temporary, path)) {
        std::filesystem::remove(temporary, error);
        return false;
    }
    return true;
}

}  // namespace pixel_town

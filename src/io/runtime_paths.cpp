#include "io/runtime_paths.hpp"

namespace pixel_town {

std::filesystem::path select_runtime_root(
    const std::filesystem::path& application_directory,
    const std::filesystem::path& launch_directory) {
    std::error_code error;
    if (!application_directory.empty() &&
        std::filesystem::is_directory(application_directory / "assets", error) && !error) {
        return application_directory;
    }
    return launch_directory;
}

}  // namespace pixel_town

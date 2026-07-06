#include "io/startup_log.hpp"

#include <fstream>

namespace pixel_town {

bool write_latest_log(const std::filesystem::path& path, std::string_view app_version,
                      std::string_view startup_stage, const ResourceReport& report) {
    std::error_code error;
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
    }

    std::ofstream output(path, std::ios::trunc);
    if (!output) {
        return false;
    }

    output << "app_version=" << app_version << '\n';
    output << "startup_stage=" << startup_stage << '\n';
    output << "resource_status=" << (report.can_start ? "ready" : "blocked") << '\n';
    output << "audio_status=" << (report.audio_enabled ? "enabled" : "muted") << '\n';
    for (const ResourceIssue& issue : report.issues) {
        output << "resource_issue=" << (issue.required ? "required" : "optional") << ','
               << issue.relative_path.generic_string() << ',' << issue.message << '\n';
    }

    output.flush();
    return output.good();
}

}  // namespace pixel_town

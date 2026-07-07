#pragma once

#include <filesystem>
#include <string_view>

#include "io/resource_diagnostics.hpp"

namespace pixel_town {

[[nodiscard]] bool write_latest_log(const std::filesystem::path& path,
                                    std::string_view app_version,
                                    std::string_view startup_stage,
                                    const ResourceReport& report,
                                    std::string_view launch_note = {});

}  // namespace pixel_town

// 启动诊断日志接口：记录版本、启动阶段、资源报告和演示参数摘要。
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

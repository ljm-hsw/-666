// 跨平台运行根目录选择：发布包优先使用可执行文件旁资源，开发构建回退到启动目录。
#pragma once

#include <filesystem>

namespace pixel_town {

[[nodiscard]] std::filesystem::path select_runtime_root(
    const std::filesystem::path& application_directory,
    const std::filesystem::path& launch_directory);

}  // namespace pixel_town

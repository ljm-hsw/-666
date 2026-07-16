// 存档边界。
//
// 文件格式面向兼容性而不是内存布局：保存 GameSessionSnapshot 的版本化字段，
// 解析失败或版本不兼容时保留原文件。写入采用临时文件替换，避免异常退出造成
// 半个存档；演示预设不应复用这里的正式存档路径。
#pragma once

#include <filesystem>
#include <string>

#include "core/game_session.hpp"

namespace pixel_town {

enum class SaveStatus {
    ok,
    not_found,
    already_exists,
    corrupt,
    incompatible_version,
    write_failed,
};

enum class SaveOverwrite {
    replace_existing,
    fail_if_exists,
};

struct SaveResult {
    SaveStatus status{SaveStatus::ok};
    std::string message;
};

struct LoadGameResult {
    SaveStatus status{SaveStatus::not_found};
    GameSession session{GameSession::new_game()};
    std::string message;
};

[[nodiscard]] std::filesystem::path default_save_path(
    const std::filesystem::path& application_directory);
[[nodiscard]] bool has_save(const std::filesystem::path& path);
[[nodiscard]] SaveResult save_session_atomic(
    const std::filesystem::path& path, const GameSession& session,
    SaveOverwrite overwrite = SaveOverwrite::replace_existing);
[[nodiscard]] LoadGameResult load_session(const std::filesystem::path& path);

}  // namespace pixel_town

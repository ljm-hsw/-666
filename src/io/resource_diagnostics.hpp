// 启动前资源契约检查：把缺失/损坏资源转换成可展示和可记录的报告。
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pixel_town {

enum class ResourceKind {
    font,
    texture,
    data,
    audio,
};

struct ResourceSpec {
    std::filesystem::path relative_path;
    ResourceKind kind;
    bool required;
};

struct ResourceIssue {
    std::filesystem::path relative_path;
    std::string message;
    bool required;
};

struct ResourceReport {
    bool can_start{true};
    bool audio_enabled{true};
    std::vector<ResourceIssue> issues;
};

[[nodiscard]] ResourceReport validate_resources(
    const std::filesystem::path& root, const std::vector<ResourceSpec>& manifest);

[[nodiscard]] std::vector<ResourceSpec> baseline_resource_manifest();

}  // namespace pixel_town

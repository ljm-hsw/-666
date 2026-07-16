#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>

#include "io/runtime_paths.hpp"

namespace {

class TemporaryRuntimeTree {
public:
    TemporaryRuntimeTree() {
        static std::atomic<unsigned long long> sequence{0};
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        root_ = std::filesystem::temp_directory_path() /
                ("pixel_town_runtime_paths_" + std::to_string(stamp) + "_" +
                 std::to_string(sequence.fetch_add(1)));
        std::filesystem::create_directories(root_);
    }

    ~TemporaryRuntimeTree() {
        std::error_code ignored;
        std::filesystem::remove_all(root_, ignored);
    }

    [[nodiscard]] const std::filesystem::path& root() const noexcept { return root_; }

private:
    std::filesystem::path root_;
};

}  // namespace

TEST_CASE("packaged runtime prefers assets beside the executable") {
    TemporaryRuntimeTree tree;
    const auto application_directory = tree.root() / "package";
    const auto launch_directory = tree.root() / "shortcut-working-directory";
    std::filesystem::create_directories(application_directory / "assets");
    std::filesystem::create_directories(launch_directory / "assets");

    CHECK(pixel_town::select_runtime_root(application_directory, launch_directory) ==
          application_directory);
}

TEST_CASE("development runtime falls back to the launch directory") {
    TemporaryRuntimeTree tree;
    const auto application_directory = tree.root() / "build" / "Release";
    const auto launch_directory = tree.root() / "repository";
    std::filesystem::create_directories(application_directory);
    std::filesystem::create_directories(launch_directory / "assets");

    CHECK(pixel_town::select_runtime_root(application_directory, launch_directory) ==
          launch_directory);
}

TEST_CASE("missing runtime assets still select a deterministic diagnostic root") {
    TemporaryRuntimeTree tree;
    const auto application_directory = tree.root() / "package";
    const auto launch_directory = tree.root() / "empty-working-directory";
    std::filesystem::create_directories(application_directory);
    std::filesystem::create_directories(launch_directory);

    CHECK(pixel_town::select_runtime_root(application_directory, launch_directory) ==
          launch_directory);
}

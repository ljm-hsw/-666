#include <doctest/doctest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "io/resource_diagnostics.hpp"
#include "io/startup_log.hpp"

namespace {

std::filesystem::path unique_temp_path(const std::string& name) {
    static std::atomic<unsigned long long> sequence{0};
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           (name + "-" + std::to_string(timestamp) + "-" +
            std::to_string(sequence.fetch_add(1)));
}

class TemporaryDirectory {
public:
    explicit TemporaryDirectory(const std::string& name) : path_(unique_temp_path(name)) {
        std::filesystem::remove_all(path_);
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

void write_file(const std::filesystem::path& root, const std::filesystem::path& relative_path,
                const std::string& contents) {
    const auto path = root / relative_path;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary);
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
}

std::vector<pixel_town::ResourceSpec> baseline_manifest() {
    using pixel_town::ResourceKind;
    return {
        {"fonts/ui.ttf", ResourceKind::font, true},
        {"textures/town.png", ResourceKind::texture, true},
        {"data/baseline.txt", ResourceKind::data, true},
        {"audio/theme.ogg", ResourceKind::audio, false},
    };
}

}  // namespace

TEST_CASE("production baseline manifest keeps the startup resource contract") {
    const auto manifest = pixel_town::baseline_resource_manifest();

    auto require_spec = [&](const std::filesystem::path& relative_path,
                            pixel_town::ResourceKind kind, bool required) {
        const auto match = std::find_if(
            manifest.begin(), manifest.end(), [&](const pixel_town::ResourceSpec& spec) {
                return spec.relative_path == relative_path;
            });
        REQUIRE(match != manifest.end());
        CHECK(match->kind == kind);
        CHECK(match->required == required);
    };

    using pixel_town::ResourceKind;
    require_spec("fonts/fusion-pixel-12px-proportional-zh_hans.ttf", ResourceKind::font, true);
    require_spec("textures/town_marker.png", ResourceKind::texture, true);
    require_spec("textures/kenney_tiny_town/Tilemap/tilemap_packed.png", ResourceKind::texture,
                 false);
    require_spec("textures/kenney_tiny_farm/Tilemap/tilemap_packed.png", ResourceKind::texture,
                 false);
    require_spec("textures/imagegen_buildings/p1_building_sprites.png", ResourceKind::texture,
                 false);
    require_spec("textures/ui/tavern/tavern_lobby.png", ResourceKind::texture, false);
    require_spec("textures/ui/convenience_store/convenience_store_interior.png",
                 ResourceKind::texture, false);
    require_spec("textures/ui/tavern/npc/bartender_idle_sheet.png", ResourceKind::texture,
                 false);
    require_spec("textures/ui/restaurant/restaurant_interior.png", ResourceKind::texture,
                 false);
    require_spec("textures/ui/home/home_interior.png", ResourceKind::texture, false);
    require_spec("textures/ui/library/library_interior.png", ResourceKind::texture, false);
    require_spec("data/baseline.txt", ResourceKind::data, true);
    require_spec("audio/theme.ogg", ResourceKind::audio, false);
}

TEST_CASE("complete required resources allow normal startup") {
    TemporaryDirectory resources("pixel-town-resource-test-complete");
    write_file(resources.path(), "fonts/ui.ttf", std::string{"\0\1\0\0font", 8});
    write_file(resources.path(), "textures/town.png", std::string{"\x89PNG\r\n\x1a\n", 8});
    write_file(resources.path(), "data/baseline.txt", "baseline=ready\n");
    write_file(resources.path(), "audio/theme.ogg", "OggSbaseline");

    const auto report = pixel_town::validate_resources(resources.path(), baseline_manifest());

    CHECK(report.can_start);
    CHECK(report.audio_enabled);
    CHECK(report.issues.empty());
}

TEST_CASE("missing required resource blocks normal startup") {
    TemporaryDirectory resources("pixel-town-resource-test-required-missing");
    write_file(resources.path(), "fonts/ui.ttf", std::string{"\0\1\0\0font", 8});
    write_file(resources.path(), "textures/town.png", std::string{"\x89PNG\r\n\x1a\n", 8});
    write_file(resources.path(), "audio/theme.ogg", "OggSbaseline");

    const auto report = pixel_town::validate_resources(resources.path(), baseline_manifest());

    CHECK_FALSE(report.can_start);
    REQUIRE(report.issues.size() == 1);
    CHECK(report.issues.front().relative_path == std::filesystem::path{"data/baseline.txt"});
    CHECK(report.issues.front().required);
}

TEST_CASE("missing optional audio starts silently with a diagnostic") {
    TemporaryDirectory resources("pixel-town-resource-test-audio-missing");
    write_file(resources.path(), "fonts/ui.ttf", std::string{"\0\1\0\0font", 8});
    write_file(resources.path(), "textures/town.png", std::string{"\x89PNG\r\n\x1a\n", 8});
    write_file(resources.path(), "data/baseline.txt", "baseline=ready\n");

    const auto report = pixel_town::validate_resources(resources.path(), baseline_manifest());

    CHECK(report.can_start);
    CHECK_FALSE(report.audio_enabled);
    REQUIRE(report.issues.size() == 1);
    CHECK(report.issues.front().relative_path == std::filesystem::path{"audio/theme.ogg"});
    CHECK_FALSE(report.issues.front().required);
}

TEST_CASE("invalid required resource blocks normal startup") {
    TemporaryDirectory resources("pixel-town-resource-test-required-invalid");
    write_file(resources.path(), "fonts/ui.ttf", std::string{"\0\1\0\0font", 8});
    write_file(resources.path(), "textures/town.png", "not-an-image\n");
    write_file(resources.path(), "data/baseline.txt", "baseline=ready\n");
    write_file(resources.path(), "audio/theme.ogg", "OggSbaseline");

    const auto report = pixel_town::validate_resources(resources.path(), baseline_manifest());

    CHECK_FALSE(report.can_start);
    REQUIRE(report.issues.size() == 1);
    CHECK(report.issues.front().message == "invalid resource");
}

TEST_CASE("startup log overwrites the previous launch with diagnostic state") {
    TemporaryDirectory output("pixel-town-startup-log-test");
    const auto log_path = output.path() / "logs/latest.log";
    write_file(output.path(), "logs/latest.log", "previous launch\n");
    pixel_town::ResourceReport report;
    report.audio_enabled = false;
    report.issues.push_back({"audio/theme.ogg", "missing resource", false});

    REQUIRE(pixel_town::write_latest_log(log_path, "0.1.0", "baseline_scene", report,
                                         "demo_preset_error=missing"));

    std::ifstream input(log_path);
    const std::string contents((std::istreambuf_iterator<char>(input)),
                               std::istreambuf_iterator<char>());
    CHECK(contents.find("previous launch") == std::string::npos);
    CHECK(contents.find("app_version=0.1.0") != std::string::npos);
    CHECK(contents.find("startup_stage=baseline_scene") != std::string::npos);
    CHECK(contents.find("launch_note=demo_preset_error=missing") != std::string::npos);
    CHECK(contents.find("audio_status=muted") != std::string::npos);
    CHECK(contents.find("optional,audio/theme.ogg,missing resource") != std::string::npos);
}

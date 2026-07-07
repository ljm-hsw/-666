#include <doctest/doctest.h>

#include <chrono>
#include <filesystem>
#include <string>

#include "io/app_settings.hpp"

namespace {

class TempSettingsDir {
public:
    TempSettingsDir() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                ("pixel_town_settings_test_" + std::to_string(stamp));
        std::filesystem::create_directories(path_);
    }

    ~TempSettingsDir() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    [[nodiscard]] std::filesystem::path path() const { return path_; }

private:
    std::filesystem::path path_;
};

}  // namespace

TEST_CASE("mute setting round trips beside the application directory") {
    TempSettingsDir temp;
    const auto settings_path = pixel_town::default_settings_path(temp.path());

    pixel_town::AppSettings settings;
    settings.muted = true;

    REQUIRE(pixel_town::save_app_settings(settings_path, settings));

    const auto loaded = pixel_town::load_app_settings(settings_path);

    REQUIRE(loaded.has_value());
    CHECK(loaded->muted);
    CHECK(settings_path == temp.path() / "saves" / "settings.ini");
}

TEST_CASE("saving settings replaces an existing value") {
    TempSettingsDir temp;
    const auto settings_path = pixel_town::default_settings_path(temp.path());

    REQUIRE(pixel_town::save_app_settings(settings_path, pixel_town::AppSettings{true}));
    REQUIRE(pixel_town::save_app_settings(settings_path, pixel_town::AppSettings{false}));

    const auto loaded = pixel_town::load_app_settings(settings_path);

    REQUIRE(loaded.has_value());
    CHECK_FALSE(loaded->muted);
}

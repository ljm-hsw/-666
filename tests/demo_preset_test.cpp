#include <doctest/doctest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "core/game_session.hpp"
#include "io/demo_preset.hpp"
#include "io/save_game.hpp"
#include "test_game_session_helpers.hpp"

namespace {

class TempDemoDir {
public:
    TempDemoDir() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ =
            std::filesystem::temp_directory_path() / ("pixel_town_demo_test_" + std::to_string(stamp));
        std::filesystem::create_directories(path_);
    }

    ~TempDemoDir() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    [[nodiscard]] std::filesystem::path path() const { return path_; }

private:
    std::filesystem::path path_;
};

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

pixel_town::GameSession make_midgame_session() {
    auto session = pixel_town::GameSession::new_game(20260707);
    for (int day = 1; day < 5; ++day) {
        REQUIRE(session.enter_location(pixel_town::Location::restaurant));
        REQUIRE(session.start_location() != 0);
        REQUIRE(session.apply_action_result(
                    pixel_town::test_support::completed_location_result(session))
                    .accepted);
        REQUIRE(session.apply_action_result(session.home_rest_result()).accepted);
        REQUIRE(session.finish_day_summary());
    }
    return session;
}

void write_preset(const std::filesystem::path& root, std::string_view id,
                  const pixel_town::GameSession& session) {
    const auto path = root / "demo_presets" / (std::string{id} + ".sav");
    REQUIRE(pixel_town::save_session_atomic(path, session).status == pixel_town::SaveStatus::ok);
}

}  // namespace

TEST_CASE("demo preset command line is explicit and hidden from normal launch") {
    const char* normal[] = {"pixel_town"};
    const char* demo[] = {"pixel_town", "--demo-preset", "midgame"};
    const char* malformed[] = {"pixel_town", "--demo-preset"};

    const auto normal_args = pixel_town::parse_demo_preset_args(1, normal);
    CHECK_FALSE(normal_args.requested);
    CHECK(normal_args.error.empty());

    const auto demo_args = pixel_town::parse_demo_preset_args(3, demo);
    CHECK(demo_args.requested);
    CHECK(demo_args.id == "midgame");
    CHECK(demo_args.error.empty());

    const auto malformed_args = pixel_town::parse_demo_preset_args(2, malformed);
    CHECK(malformed_args.requested);
    CHECK_FALSE(malformed_args.error.empty());
}

TEST_CASE("loading a demo preset does not read or overwrite the formal autosave") {
    TempDemoDir temp;
    const auto formal_save = temp.path() / "saves/slot1.sav";
    const auto formal_session = pixel_town::GameSession::new_game(99);
    REQUIRE(pixel_town::save_session_atomic(formal_save, formal_session).status ==
            pixel_town::SaveStatus::ok);
    const std::string before = read_text(formal_save);

    write_preset(temp.path(), "midgame", make_midgame_session());

    const auto loaded = pixel_town::load_demo_preset(temp.path(), "midgame");

    REQUIRE(loaded.status == pixel_town::DemoPresetStatus::ok);
    CHECK(loaded.session.day() == 5);
    CHECK(loaded.session.snapshot().seed == 20260707);
    CHECK(read_text(formal_save) == before);
}

TEST_CASE("missing or corrupt demo presets return explicit errors") {
    TempDemoDir temp;
    const auto corrupt_path = temp.path() / "demo_presets/corrupt.sav";
    std::filesystem::create_directories(corrupt_path.parent_path());
    std::ofstream(corrupt_path) << "format_version=1\nseed=42\nphase=day_choice\n";

    const auto missing = pixel_town::load_demo_preset(temp.path(), "missing");
    const auto corrupt = pixel_town::load_demo_preset(temp.path(), "corrupt");

    CHECK(missing.status == pixel_town::DemoPresetStatus::not_found);
    CHECK_FALSE(missing.message.empty());
    CHECK(corrupt.status == pixel_town::DemoPresetStatus::corrupt);
    CHECK_FALSE(corrupt.message.empty());
}

TEST_CASE("bundled demo presets are valid fixed-seed snapshots") {
    const auto midgame = pixel_town::load_demo_preset("assets/data", "midgame");
    const auto ending_eve = pixel_town::load_demo_preset("assets/data", "ending-eve");

    REQUIRE(midgame.status == pixel_town::DemoPresetStatus::ok);
    CHECK(midgame.session.day() == 5);
    CHECK(midgame.session.snapshot().seed == 20260707);

    REQUIRE(ending_eve.status == pixel_town::DemoPresetStatus::ok);
    CHECK(ending_eve.session.day() == 10);
    CHECK(ending_eve.session.phase() == pixel_town::GamePhase::night_choice);
    CHECK(ending_eve.session.snapshot().seed == 20260707);
}

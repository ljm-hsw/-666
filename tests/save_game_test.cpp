#include <doctest/doctest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "core/game_session.hpp"
#include "io/save_game.hpp"
#include "locations/convenience_store.hpp"
#include "test_game_session_helpers.hpp"

namespace {

class TempSaveDir {
public:
    TempSaveDir() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                ("pixel_town_save_test_" + std::to_string(stamp));
        std::filesystem::create_directories(path_);
    }

    ~TempSaveDir() {
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

void write_text(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::trunc);
    output << text;
}

pixel_town::GameSession session_after_day_result() {
    auto session = pixel_town::GameSession::new_game(42);
    REQUIRE(session.enter_location(pixel_town::Location::library));
    REQUIRE(session.start_location() != 0);
    REQUIRE(session.apply_action_result(
                pixel_town::test_support::completed_location_result(session))
                .accepted);
    return session;
}

}  // namespace

TEST_CASE("save file round trips a completed phase boundary") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "saves/slot1.sav";
    const auto saved = session_after_day_result();

    const auto save_result = pixel_town::save_session_atomic(save_path, saved);
    REQUIRE(save_result.status == pixel_town::SaveStatus::ok);

    const auto loaded = pixel_town::load_session(save_path);
    REQUIRE(loaded.status == pixel_town::SaveStatus::ok);

    CHECK(loaded.session.day() == saved.day());
    CHECK(loaded.session.phase() == saved.phase());
    CHECK(loaded.session.player().money == saved.player().money);
    CHECK(loaded.session.player().stamina == saved.player().stamina);
    CHECK(loaded.session.player().reputation == saved.player().reputation);
    CHECK(loaded.session.player().knowledge == saved.player().knowledge);
    CHECK(loaded.session.player().mood == saved.player().mood);
    CHECK(loaded.session.current_day_context().weather == saved.current_day_context().weather);
    CHECK(loaded.session.current_day_context().event == saved.current_day_context().event);

    auto resumed = loaded.session;
    REQUIRE(resumed.apply_action_result(resumed.home_rest_result()).accepted);
    CHECK(resumed.phase() == pixel_town::GamePhase::day_summary);
}

TEST_CASE("save file includes versioned future module fields") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "slot1.sav";

    REQUIRE(pixel_town::save_session_atomic(save_path, session_after_day_result()).status ==
            pixel_town::SaveStatus::ok);

    const std::string text = read_text(save_path);
    CHECK(text.find("format_version=1\n") != std::string::npos);
    CHECK(text.find("seed=42\n") != std::string::npos);
    CHECK(text.find("day=1\n") != std::string::npos);
    CHECK(text.find("phase=night_choice\n") != std::string::npos);
    CHECK(text.find("player_money=") != std::string::npos);
    CHECK(text.find("store_inventory=") != std::string::npos);
    CHECK(text.find("tavern_wins=") != std::string::npos);
    CHECK(text.find("tavern_losses=") != std::string::npos);
}

TEST_CASE("legacy v1 saves without tavern record default to zero") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "legacy-v1.sav";

    write_text(save_path,
               "format_version=1\n"
               "seed=42\n"
               "day=1\n"
               "phase=night_choice\n"
               "next_result_id=2\n"
               "active_result_id=0\n"
               "player_money=56\n"
               "player_stamina=72\n"
               "player_reputation=5\n"
               "player_knowledge=6\n"
               "player_mood=72\n"
               "has_pending_location=0\n"
               "pending_location=home\n"
               "location_started=0\n"
               "day_action_done=1\n"
               "night_action_done=0\n"
               "last_summary=legacy\n"
               "main_ending=\n"
               "final_summary=\n"
               "applied_result_ids=1\n"
               "store_inventory=none\n");

    const auto loaded = pixel_town::load_session(save_path);

    REQUIRE(loaded.status == pixel_town::SaveStatus::ok);
    CHECK(loaded.session.tavern_wins() == 0);
    CHECK(loaded.session.tavern_losses() == 0);
}

TEST_CASE("store inventory round trips through save file") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "slot1.sav";
    auto session = pixel_town::GameSession::new_game(42);
    REQUIRE(session.enter_location(pixel_town::Location::convenience_store));
    const int result_id = session.start_location();
    REQUIRE(result_id != 0);

    const auto config = pixel_town::store::default_store_config();
    pixel_town::store::StoreInventory inventory = {{"umbrella", 1}};
    pixel_town::store::PurchasePlan purchase;
    purchase.quantities = {{"umbrella", 2}, {"soda", 3}};
    const auto prices = pixel_town::store::default_price_plan(config);
    pixel_town::store::DailyStoreContext context;
    context.seed = 42;
    context.weather = "小雨";
    context.event = "便利店零食更受欢迎";

    const auto settlement =
        pixel_town::store::simulate_sales(config, inventory, purchase, prices, context, 50);
    REQUIRE(settlement.accepted);
    REQUIRE(session.apply_action_result(
                pixel_town::store::build_store_action_result(config, settlement, result_id))
                .accepted);

    REQUIRE(pixel_town::save_session_atomic(save_path, session).status ==
            pixel_town::SaveStatus::ok);
    const std::string text = read_text(save_path);
    CHECK(text.find("store_inventory=none\n") == std::string::npos);
    CHECK(text.find("umbrella:") != std::string::npos);

    const auto loaded = pixel_town::load_session(save_path);
    REQUIRE(loaded.status == pixel_town::SaveStatus::ok);
    CHECK(loaded.session.store_inventory() == session.store_inventory());
}

TEST_CASE("saving without overwrite confirmation preserves an existing save") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "slot1.sav";
    auto original = pixel_town::GameSession::new_game(7);
    auto replacement = pixel_town::GameSession::new_game(99);

    REQUIRE(pixel_town::save_session_atomic(save_path, original).status ==
            pixel_town::SaveStatus::ok);
    const std::string before = read_text(save_path);

    const auto result = pixel_town::save_session_atomic(
        save_path, replacement, pixel_town::SaveOverwrite::fail_if_exists);

    CHECK(result.status == pixel_town::SaveStatus::already_exists);
    CHECK(read_text(save_path) == before);
}

TEST_CASE("corrupt and incompatible saves are reported without deleting the file") {
    TempSaveDir temp;
    const auto corrupt_path = temp.path() / "corrupt.sav";
    const auto incompatible_path = temp.path() / "incompatible.sav";

    write_text(corrupt_path, "format_version=1\nseed=42\nphase=night_choice\n");
    write_text(incompatible_path, "format_version=999\nseed=42\nday=1\nphase=day_choice\n");

    const auto corrupt = pixel_town::load_session(corrupt_path);
    const auto incompatible = pixel_town::load_session(incompatible_path);

    CHECK(corrupt.status == pixel_town::SaveStatus::corrupt);
    CHECK(incompatible.status == pixel_town::SaveStatus::incompatible_version);
    CHECK(std::filesystem::is_regular_file(corrupt_path));
    CHECK(std::filesystem::is_regular_file(incompatible_path));
}

TEST_CASE("field-complete saves with unreachable phase state are rejected") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "unreachable.sav";

    write_text(save_path,
               "format_version=1\n"
               "seed=42\n"
               "day=1\n"
               "phase=day_choice\n"
               "next_result_id=2\n"
               "active_result_id=1\n"
               "player_money=50\n"
               "player_stamina=80\n"
               "player_reputation=0\n"
               "player_knowledge=0\n"
               "player_mood=70\n"
               "has_pending_location=1\n"
               "pending_location=restaurant\n"
               "location_started=1\n"
               "day_action_done=0\n"
               "night_action_done=0\n"
               "last_summary=\n"
               "main_ending=\n"
               "final_summary=\n"
               "applied_result_ids=\n"
               "store_inventory=none\n"
               "tavern_wins=0\n"
               "tavern_losses=0\n");

    const auto loaded = pixel_town::load_session(save_path);

    CHECK(loaded.status == pixel_town::SaveStatus::corrupt);
    CHECK(std::filesystem::is_regular_file(save_path));
}

TEST_CASE("saves with invalid player ranges are rejected") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "invalid-player.sav";

    write_text(save_path,
               "format_version=1\n"
               "seed=42\n"
               "day=1\n"
               "phase=day_choice\n"
               "next_result_id=1\n"
               "active_result_id=0\n"
               "player_money=-99\n"
               "player_stamina=80\n"
               "player_reputation=0\n"
               "player_knowledge=0\n"
               "player_mood=999\n"
               "has_pending_location=0\n"
               "pending_location=home\n"
               "location_started=0\n"
               "day_action_done=0\n"
               "night_action_done=0\n"
               "last_summary=\n"
               "main_ending=\n"
               "final_summary=\n"
               "applied_result_ids=\n"
               "store_inventory=none\n"
               "tavern_wins=0\n"
               "tavern_losses=0\n");

    const auto loaded = pixel_town::load_session(save_path);

    CHECK(loaded.status == pixel_town::SaveStatus::corrupt);
    CHECK(std::filesystem::is_regular_file(save_path));
}

TEST_CASE("saves with invalid store inventory are rejected") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "invalid-store-inventory.sav";

    write_text(save_path,
               "format_version=1\n"
               "seed=42\n"
               "day=1\n"
               "phase=day_choice\n"
               "next_result_id=1\n"
               "active_result_id=0\n"
               "player_money=50\n"
               "player_stamina=80\n"
               "player_reputation=0\n"
               "player_knowledge=0\n"
               "player_mood=70\n"
               "has_pending_location=0\n"
               "pending_location=home\n"
               "location_started=0\n"
               "day_action_done=0\n"
               "night_action_done=0\n"
               "last_summary=\n"
               "main_ending=\n"
               "final_summary=\n"
               "applied_result_ids=\n"
               "store_inventory=unknown:1,umbrella:11\n"
               "tavern_wins=0\n"
               "tavern_losses=0\n");

    const auto loaded = pixel_town::load_session(save_path);

    CHECK(loaded.status == pixel_town::SaveStatus::corrupt);
    CHECK(std::filesystem::is_regular_file(save_path));
}

TEST_CASE("ending saves require an ending label and final summary") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "empty-ending.sav";

    write_text(save_path,
               "format_version=1\n"
               "seed=42\n"
               "day=10\n"
               "phase=ending\n"
               "next_result_id=21\n"
               "active_result_id=0\n"
               "player_money=170\n"
               "player_stamina=50\n"
               "player_reputation=20\n"
               "player_knowledge=10\n"
               "player_mood=60\n"
               "has_pending_location=0\n"
               "pending_location=home\n"
               "location_started=0\n"
               "day_action_done=1\n"
               "night_action_done=1\n"
               "last_summary=\n"
               "main_ending=\n"
               "final_summary=\n"
               "applied_result_ids=1,2\n"
               "store_inventory=none\n"
               "tavern_wins=0\n"
               "tavern_losses=0\n");

    const auto loaded = pixel_town::load_session(save_path);

    CHECK(loaded.status == pixel_town::SaveStatus::corrupt);
    CHECK(std::filesystem::is_regular_file(save_path));
}

TEST_CASE("formal ending and liquidated inventory survive save reload") {
    TempSaveDir temp;
    const auto save_path = temp.path() / "formal-ending.sav";
    auto snapshot = pixel_town::GameSession::new_game(20260710U).snapshot();
    snapshot.day = 10;
    snapshot.phase = pixel_town::GamePhase::day_summary;
    snapshot.day_action_done = true;
    snapshot.night_action_done = true;
    snapshot.player.money = 50;
    snapshot.player.reputation = 20;
    snapshot.store_inventory = {{"umbrella", 2}};
    auto session = pixel_town::GameSession::from_snapshot(snapshot);
    REQUIRE(session.finish_day_summary());
    REQUIRE(pixel_town::save_session_atomic(save_path, session).status ==
            pixel_town::SaveStatus::ok);

    const auto loaded = pixel_town::load_session(save_path);

    REQUIRE(loaded.status == pixel_town::SaveStatus::ok);
    CHECK(loaded.session.phase() == pixel_town::GamePhase::ending);
    CHECK(loaded.session.main_ending() == session.main_ending());
    CHECK(loaded.session.final_summary() == session.final_summary());
    CHECK(loaded.session.player().money == 56);
    CHECK(loaded.session.store_inventory().empty());
    auto resumed = loaded.session;
    CHECK_FALSE(resumed.finish_day_summary());
}

TEST_CASE("default save path stays beside the application directory") {
    const auto base = std::filesystem::path{"portable"};
    CHECK(pixel_town::default_save_path(base) == base / "saves" / "slot1.sav");
}

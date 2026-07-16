// 版本化行式存档、解析/迁移和原子替换写入实现。
#include "io/save_game.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace pixel_town {
namespace {

constexpr int save_format_version = 2;
constexpr int legacy_save_format_version = 1;

std::string phase_name(GamePhase phase) {
    switch (phase) {
        case GamePhase::day_choice:
            return "day_choice";
        case GamePhase::day_location:
            return "day_location";
        case GamePhase::night_choice:
            return "night_choice";
        case GamePhase::night_location:
            return "night_location";
        case GamePhase::day_summary:
            return "day_summary";
        case GamePhase::ending:
            return "ending";
    }
    return "unknown";
}

bool parse_phase(const std::string& value, GamePhase& phase) {
    if (value == "day_choice") {
        phase = GamePhase::day_choice;
    } else if (value == "day_location") {
        phase = GamePhase::day_location;
    } else if (value == "night_choice") {
        phase = GamePhase::night_choice;
    } else if (value == "night_location") {
        phase = GamePhase::night_location;
    } else if (value == "day_summary") {
        phase = GamePhase::day_summary;
    } else if (value == "ending") {
        phase = GamePhase::ending;
    } else {
        return false;
    }
    return true;
}

std::string location_name(Location location) {
    switch (location) {
        case Location::home:
            return "home";
        case Location::restaurant:
            return "restaurant";
        case Location::convenience_store:
            return "convenience_store";
        case Location::library:
            return "library";
        case Location::tavern:
            return "tavern";
    }
    return "unknown";
}

bool parse_location(const std::string& value, Location& location) {
    if (value == "home") {
        location = Location::home;
    } else if (value == "restaurant") {
        location = Location::restaurant;
    } else if (value == "convenience_store") {
        location = Location::convenience_store;
    } else if (value == "library") {
        location = Location::library;
    } else if (value == "tavern") {
        location = Location::tavern;
    } else {
        return false;
    }
    return true;
}

std::string escape_value(const std::string& value) {
    std::string escaped;
    for (const char ch : value) {
        if (ch == '%') {
            escaped += "%25";
        } else if (ch == '\n') {
            escaped += "%0A";
        } else if (ch == '=') {
            escaped += "%3D";
        } else {
            escaped += ch;
        }
    }
    return escaped;
}

bool hex_digit(char ch, int& value) {
    if (ch >= '0' && ch <= '9') {
        value = ch - '0';
        return true;
    }
    if (ch >= 'A' && ch <= 'F') {
        value = ch - 'A' + 10;
        return true;
    }
    return false;
}

bool unescape_value(const std::string& value, std::string& unescaped) {
    unescaped.clear();
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] != '%') {
            unescaped += value[index];
            continue;
        }
        if (index + 2 >= value.size()) {
            return false;
        }
        int high = 0;
        int low = 0;
        if (!hex_digit(value[index + 1], high) || !hex_digit(value[index + 2], low)) {
            return false;
        }
        unescaped += static_cast<char>(high * 16 + low);
        index += 2;
    }
    return true;
}

std::string join_ints(const std::vector<int>& values) {
    std::string joined;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            joined += ",";
        }
        joined += std::to_string(values[index]);
    }
    return joined;
}

std::string serialize_store_inventory(const std::vector<StoreInventoryItem>& inventory) {
    if (inventory.empty()) {
        return "none";
    }
    std::string serialized;
    for (std::size_t index = 0; index < inventory.size(); ++index) {
        if (index != 0) {
            serialized += ",";
        }
        serialized += inventory[index].item_id;
        serialized += ":";
        serialized += std::to_string(inventory[index].quantity);
    }
    return serialized;
}

bool parse_int(const std::unordered_map<std::string, std::string>& values, const char* key,
               int& parsed) {
    const auto found = values.find(key);
    if (found == values.end()) {
        return false;
    }
    try {
        std::size_t consumed = 0;
        parsed = std::stoi(found->second, &consumed);
        return consumed == found->second.size();
    } catch (...) {
        return false;
    }
}

bool parse_uint(const std::unordered_map<std::string, std::string>& values, const char* key,
                unsigned int& parsed) {
    int signed_value = 0;
    if (!parse_int(values, key, signed_value) || signed_value < 0) {
        return false;
    }
    parsed = static_cast<unsigned int>(signed_value);
    return true;
}

bool parse_bool(const std::unordered_map<std::string, std::string>& values, const char* key,
                bool& parsed) {
    int integer = 0;
    if (!parse_int(values, key, integer) || (integer != 0 && integer != 1)) {
        return false;
    }
    parsed = integer == 1;
    return true;
}

bool parse_string(const std::unordered_map<std::string, std::string>& values, const char* key,
                  std::string& parsed) {
    const auto found = values.find(key);
    if (found == values.end()) {
        return false;
    }
    return unescape_value(found->second, parsed);
}

bool parse_applied_results(const std::string& raw, std::vector<int>& result_ids) {
    result_ids.clear();
    if (raw.empty()) {
        return true;
    }
    std::stringstream stream(raw);
    std::string token;
    while (std::getline(stream, token, ',')) {
        try {
            std::size_t consumed = 0;
            const int value = std::stoi(token, &consumed);
            if (consumed != token.size()) {
                return false;
            }
            result_ids.push_back(value);
        } catch (...) {
            return false;
        }
    }
    return true;
}

bool parse_store_inventory(const std::string& raw, std::vector<StoreInventoryItem>& inventory) {
    inventory.clear();
    if (raw == "none" || raw.empty()) {
        return true;
    }
    std::stringstream stream(raw);
    std::string token;
    while (std::getline(stream, token, ',')) {
        const std::size_t separator = token.find(':');
        if (separator == std::string::npos || separator == 0 ||
            separator + 1 >= token.size()) {
            return false;
        }
        StoreInventoryItem item;
        item.item_id = token.substr(0, separator);
        try {
            std::size_t consumed = 0;
            item.quantity = std::stoi(token.substr(separator + 1), &consumed);
            if (consumed != token.size() - separator - 1 || item.quantity < 0) {
                return false;
            }
        } catch (...) {
            return false;
        }
        inventory.push_back(item);
    }
    return true;
}

std::string serialize_session(const GameSession& session) {
    const GameSessionSnapshot snapshot = session.snapshot();
    std::ostringstream output;
    output << "format_version=" << save_format_version << "\n";
    output << "seed=" << snapshot.seed << "\n";
    output << "day=" << snapshot.day << "\n";
    output << "phase=" << phase_name(snapshot.phase) << "\n";
    output << "next_result_id=" << snapshot.next_result_id << "\n";
    output << "active_result_id=" << snapshot.active_result_id << "\n";
    output << "player_money=" << snapshot.player.money << "\n";
    output << "player_stamina=" << snapshot.player.stamina << "\n";
    output << "player_reputation=" << snapshot.player.reputation << "\n";
    output << "player_knowledge=" << snapshot.player.knowledge << "\n";
    output << "player_mood=" << snapshot.player.mood << "\n";
    output << "has_pending_location=" << (snapshot.has_pending_location ? 1 : 0) << "\n";
    output << "pending_location=" << location_name(snapshot.pending_location) << "\n";
    output << "location_started=" << (snapshot.location_started ? 1 : 0) << "\n";
    output << "day_action_done=" << (snapshot.day_action_done ? 1 : 0) << "\n";
    output << "night_action_done=" << (snapshot.night_action_done ? 1 : 0) << "\n";
    output << "last_summary=" << escape_value(snapshot.last_summary) << "\n";
    output << "main_ending=" << escape_value(snapshot.main_ending) << "\n";
    output << "final_summary=" << escape_value(snapshot.final_summary) << "\n";
    output << "applied_result_ids=" << join_ints(snapshot.applied_result_ids) << "\n";
    output << "store_inventory=" << serialize_store_inventory(snapshot.store_inventory) << "\n";
    output << "tavern_wins=" << snapshot.tavern_wins << "\n";
    output << "tavern_losses=" << snapshot.tavern_losses << "\n";
    output << "location_visits_home=" << snapshot.location_visits.home << "\n";
    output << "location_visits_restaurant=" << snapshot.location_visits.restaurant << "\n";
    output << "location_visits_convenience_store="
           << snapshot.location_visits.convenience_store << "\n";
    output << "location_visits_library=" << snapshot.location_visits.library << "\n";
    output << "location_visits_tavern=" << snapshot.location_visits.tavern << "\n";
    return output.str();
}

bool replace_file_atomically(const std::filesystem::path& temporary,
                             const std::filesystem::path& destination) {
#ifdef _WIN32
    return MoveFileExW(temporary.wstring().c_str(), destination.wstring().c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(temporary.c_str(), destination.c_str()) == 0;
#endif
}

bool is_day_work_location(Location location) {
    return location == Location::restaurant || location == Location::convenience_store ||
           location == Location::library;
}

bool is_valid_player_state(const PlayerState& player) {
    return player.money >= 0 && player.money <= 999 && player.stamina >= 0 &&
           player.stamina <= 100 && player.reputation >= 0 && player.reputation <= 100 &&
           player.knowledge >= 0 && player.knowledge <= 100 && player.mood >= 0 &&
           player.mood <= 100;
}

bool is_known_store_item_id(const std::string& item_id) {
    return item_id == "umbrella" || item_id == "soda" || item_id == "bento" ||
           item_id == "coffee";
}

bool is_valid_store_inventory(const std::vector<StoreInventoryItem>& inventory) {
    for (std::size_t index = 0; index < inventory.size(); ++index) {
        if (!is_known_store_item_id(inventory[index].item_id) || inventory[index].quantity < 0 ||
            inventory[index].quantity > 10) {
            return false;
        }
        for (std::size_t previous = 0; previous < index; ++previous) {
            if (inventory[previous].item_id == inventory[index].item_id) {
                return false;
            }
        }
    }
    return true;
}

bool is_valid_location_visits(const LocationVisitCounts& visits) {
    return visits.home >= 0 && visits.home <= 10 && visits.restaurant >= 0 &&
           visits.restaurant <= 10 && visits.convenience_store >= 0 &&
           visits.convenience_store <= 10 && visits.library >= 0 &&
           visits.library <= 10 && visits.tavern >= 0 && visits.tavern <= 10;
}

bool is_valid_snapshot_combination(const GameSessionSnapshot& snapshot) {
    if (!is_valid_player_state(snapshot.player)) {
        return false;
    }
    if (!is_valid_store_inventory(snapshot.store_inventory)) {
        return false;
    }
    if (!is_valid_location_visits(snapshot.location_visits)) {
        return false;
    }
    for (std::size_t index = 0; index < snapshot.applied_result_ids.size(); ++index) {
        const int result_id = snapshot.applied_result_ids[index];
        if (result_id <= 0 || result_id >= snapshot.next_result_id ||
            result_id == snapshot.active_result_id) {
            return false;
        }
        for (std::size_t previous = 0; previous < index; ++previous) {
            if (snapshot.applied_result_ids[previous] == result_id) {
                return false;
            }
        }
    }

    if (snapshot.location_started && !snapshot.has_pending_location) {
        return false;
    }
    if (snapshot.location_started != (snapshot.active_result_id > 0)) {
        return false;
    }
    if (!snapshot.location_started && snapshot.active_result_id != 0) {
        return false;
    }

    switch (snapshot.phase) {
        case GamePhase::day_choice:
            return !snapshot.has_pending_location && !snapshot.day_action_done &&
                   !snapshot.night_action_done;
        case GamePhase::day_location:
            return snapshot.has_pending_location &&
                   is_day_work_location(snapshot.pending_location) && !snapshot.day_action_done &&
                   !snapshot.night_action_done;
        case GamePhase::night_choice:
            return !snapshot.has_pending_location && snapshot.day_action_done &&
                   !snapshot.night_action_done;
        case GamePhase::night_location:
            return snapshot.has_pending_location &&
                   (snapshot.pending_location == Location::home ||
                    snapshot.pending_location == Location::tavern) &&
                   snapshot.day_action_done && !snapshot.night_action_done;
        case GamePhase::day_summary:
            return !snapshot.has_pending_location && snapshot.day_action_done &&
                   snapshot.night_action_done;
        case GamePhase::ending:
            return snapshot.day == configured_game_day_limit() &&
                   !snapshot.has_pending_location &&
                   !snapshot.main_ending.empty() && !snapshot.final_summary.empty();
    }
    return false;
}

}  // namespace

std::filesystem::path default_save_path(const std::filesystem::path& application_directory) {
    return application_directory / "saves" / "slot1.sav";
}

bool has_save(const std::filesystem::path& path) {
    // 这里只判断文件存在；内容有效性由 load_session 的版本和字段解析负责。
    return std::filesystem::is_regular_file(path);
}

SaveResult save_session_atomic(const std::filesystem::path& path, const GameSession& session,
                               SaveOverwrite overwrite) {
    // 先写临时文件再替换正式文件，避免中断留下半个可解析存档。
    std::error_code error;
    if (overwrite == SaveOverwrite::fail_if_exists && std::filesystem::exists(path, error)) {
        return {SaveStatus::already_exists, "save already exists; overwrite was not confirmed"};
    }

    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return {SaveStatus::write_failed, "could not create save directory"};
    }

    const std::filesystem::path temporary = path.string() + ".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) {
            return {SaveStatus::write_failed, "could not open temporary save file"};
        }
        output << serialize_session(session);
        output.flush();
        if (!output) {
            return {SaveStatus::write_failed, "could not write temporary save file"};
        }
    }

    if (!replace_file_atomically(temporary, path)) {
        std::filesystem::remove(temporary, error);
        return {SaveStatus::write_failed, "could not atomically replace save file"};
    }

    return {SaveStatus::ok, ""};
}

LoadGameResult load_session(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        return {SaveStatus::not_found, GameSession::new_game(), "save file was not found"};
    }

    std::ifstream input(path);
    if (!input) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file could not be opened"};
    }

    std::unordered_map<std::string, std::string> values;
    std::string line;
    while (std::getline(input, line)) {
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos || separator == 0) {
            return {SaveStatus::corrupt, GameSession::new_game(), "save file has an invalid line"};
        }
        values.emplace(line.substr(0, separator), line.substr(separator + 1));
    }

    int version = 0;
    if (!parse_int(values, "format_version", version)) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file is missing version"};
    }
    if (version != legacy_save_format_version && version != save_format_version) {
        return {SaveStatus::incompatible_version, GameSession::new_game(),
                "save file version is not compatible"};
    }

    GameSessionSnapshot snapshot;
    if (!parse_int(values, "day", snapshot.day) || !parse_uint(values, "seed", snapshot.seed) ||
        !parse_int(values, "next_result_id", snapshot.next_result_id) ||
        !parse_int(values, "active_result_id", snapshot.active_result_id) ||
        !parse_int(values, "player_money", snapshot.player.money) ||
        !parse_int(values, "player_stamina", snapshot.player.stamina) ||
        !parse_int(values, "player_reputation", snapshot.player.reputation) ||
        !parse_int(values, "player_knowledge", snapshot.player.knowledge) ||
        !parse_int(values, "player_mood", snapshot.player.mood) ||
        !parse_bool(values, "has_pending_location", snapshot.has_pending_location) ||
        !parse_bool(values, "location_started", snapshot.location_started) ||
        !parse_bool(values, "day_action_done", snapshot.day_action_done) ||
        !parse_bool(values, "night_action_done", snapshot.night_action_done) ||
        !parse_string(values, "last_summary", snapshot.last_summary) ||
        !parse_string(values, "main_ending", snapshot.main_ending) ||
        !parse_string(values, "final_summary", snapshot.final_summary)) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file is missing fields"};
    }

    const auto phase = values.find("phase");
    const auto pending_location = values.find("pending_location");
    const auto applied_result_ids = values.find("applied_result_ids");
    if (phase == values.end() || pending_location == values.end() ||
        applied_result_ids == values.end() || values.find("store_inventory") == values.end()) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file is missing fields"};
    }
    if (values.find("tavern_wins") != values.end() &&
        !parse_int(values, "tavern_wins", snapshot.tavern_wins)) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file has invalid values"};
    }
    if (values.find("tavern_losses") != values.end() &&
        !parse_int(values, "tavern_losses", snapshot.tavern_losses)) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file has invalid values"};
    }
    if (version == save_format_version &&
        (!parse_int(values, "location_visits_home", snapshot.location_visits.home) ||
         !parse_int(values, "location_visits_restaurant", snapshot.location_visits.restaurant) ||
         !parse_int(values, "location_visits_convenience_store",
                    snapshot.location_visits.convenience_store) ||
         !parse_int(values, "location_visits_library", snapshot.location_visits.library) ||
         !parse_int(values, "location_visits_tavern", snapshot.location_visits.tavern))) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file is missing fields"};
    }
    const auto store_inventory = values.find("store_inventory");
    if (!parse_phase(phase->second, snapshot.phase) ||
        !parse_location(pending_location->second, snapshot.pending_location) ||
        !parse_applied_results(applied_result_ids->second, snapshot.applied_result_ids) ||
        !parse_store_inventory(store_inventory->second, snapshot.store_inventory)) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file has invalid values"};
    }
    if (snapshot.day < 1 || snapshot.day > configured_game_day_limit() ||
        snapshot.next_result_id < 1 ||
        snapshot.active_result_id < 0 || snapshot.tavern_wins < 0 ||
        snapshot.tavern_losses < 0) {
        return {SaveStatus::corrupt, GameSession::new_game(), "save file has invalid values"};
    }
    if (!is_valid_snapshot_combination(snapshot)) {
        return {SaveStatus::corrupt, GameSession::new_game(),
                "save file has an unreachable phase state"};
    }

    return {SaveStatus::ok, GameSession::from_snapshot(snapshot), ""};
}

}  // namespace pixel_town

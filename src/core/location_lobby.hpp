#pragma once

#include <string>
#include <vector>

#include "core/game_session.hpp"

namespace pixel_town {

struct LobbyRect {
    float x{0.0F};
    float y{0.0F};
    float width{0.0F};
    float height{0.0F};
};

struct LocationLobbySpec {
    Location location{Location::home};
    std::string title;
    std::string npc_label;
    std::string action_label;
    LobbyRect npc_hotspot;
    LobbyRect back_button;
    LobbyRect action_button;
};

[[nodiscard]] const LocationLobbySpec* find_location_lobby_spec(Location location);
[[nodiscard]] bool lobby_rects_overlap(LobbyRect left, LobbyRect right) noexcept;
[[nodiscard]] std::vector<std::string> validate_location_lobby_spec(
    const LocationLobbySpec& spec);

}  // namespace pixel_town

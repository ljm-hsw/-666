#include <doctest/doctest.h>

#include <array>

#include "core/location_lobby.hpp"

TEST_CASE("non-tavern scene lobbies expose bounded NPC and action hotspots") {
    const std::array locations{
        pixel_town::Location::restaurant,
        pixel_town::Location::convenience_store,
        pixel_town::Location::library,
        pixel_town::Location::home,
    };

    for (const auto location : locations) {
        const auto* spec = pixel_town::find_location_lobby_spec(location);
        REQUIRE(spec != nullptr);
        CHECK(spec->location == location);
        CHECK_FALSE(spec->title.empty());
        CHECK_FALSE(spec->npc_label.empty());
        CHECK_FALSE(spec->action_label.empty());
        CHECK(pixel_town::validate_location_lobby_spec(*spec).empty());
        CHECK_FALSE(pixel_town::lobby_rects_overlap(
            spec->back_button, spec->action_button));
        CHECK_FALSE(pixel_town::lobby_rects_overlap(
            spec->npc_hotspot, spec->back_button));
        CHECK_FALSE(pixel_town::lobby_rects_overlap(
            spec->npc_hotspot, spec->action_button));
    }
}

TEST_CASE("tavern keeps its dedicated lobby implementation") {
    CHECK(pixel_town::find_location_lobby_spec(
              pixel_town::Location::tavern) == nullptr);
}

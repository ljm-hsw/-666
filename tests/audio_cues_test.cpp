#include <doctest/doctest.h>

#include "app/audio_cues.hpp"

TEST_CASE("map uses the main town music") {
    pixel_town::AudioSceneState state;
    state.phase = pixel_town::GamePhase::day_choice;
    state.weather = "晴天";

    CHECK(pixel_town::music_track_for(state) == pixel_town::MusicTrack::main_map);
}

TEST_CASE("rainy map uses the rainy-day music") {
    pixel_town::AudioSceneState state;
    state.phase = pixel_town::GamePhase::day_choice;
    state.weather = "小雨";

    CHECK(pixel_town::music_track_for(state) == pixel_town::MusicTrack::rainy_day);
}

TEST_CASE("rain does not override summary and ending music") {
    pixel_town::AudioSceneState state;
    state.weather = "小雨";

    state.phase = pixel_town::GamePhase::day_summary;
    CHECK(pixel_town::music_track_for(state) == pixel_town::MusicTrack::main_map);
    state.phase = pixel_town::GamePhase::ending;
    CHECK(pixel_town::music_track_for(state) == pixel_town::MusicTrack::main_map);
}

TEST_CASE("visible location chooses its own music even on a rainy day") {
    using pixel_town::Location;
    using pixel_town::MusicTrack;

    pixel_town::AudioSceneState state;
    state.weather = "小雨";

    const auto check_location = [&](Location location, MusicTrack expected) {
        state.visible_location = location;
        CHECK(pixel_town::music_track_for(state) == expected);
    };

    check_location(Location::restaurant, MusicTrack::restaurant);
    check_location(Location::convenience_store, MusicTrack::convenience_store);
    check_location(Location::library, MusicTrack::library);
    check_location(Location::tavern, MusicTrack::tavern);
    check_location(Location::home, MusicTrack::home);
}

TEST_CASE("entering a work location plays the location-switch cue") {
    pixel_town::AudioCueTracker tracker;
    pixel_town::AudioSceneState map;
    map.weather = "晴天";
    CHECK(tracker.update(map).sound == pixel_town::SoundCue::none);

    auto restaurant = map;
    restaurant.visible_location = pixel_town::Location::restaurant;
    CHECK(tracker.update(restaurant).sound == pixel_town::SoundCue::location_switch);
}

TEST_CASE("entering home plays the return-home cue") {
    pixel_town::AudioCueTracker tracker;
    pixel_town::AudioSceneState map;
    CHECK(tracker.update(map).sound == pixel_town::SoundCue::none);

    auto home = map;
    home.visible_location = pixel_town::Location::home;
    CHECK(tracker.update(home).sound == pixel_town::SoundCue::return_home);
}

TEST_CASE("finishing a location activity plays the success cue") {
    pixel_town::AudioCueTracker tracker;
    pixel_town::AudioSceneState restaurant;
    restaurant.visible_location = pixel_town::Location::restaurant;
    restaurant.location_activity_started = true;
    CHECK(tracker.update(restaurant).sound == pixel_town::SoundCue::none);

    auto map = restaurant;
    map.visible_location.reset();
    map.location_activity_started = false;
    map.visits.restaurant = 1;
    CHECK(tracker.update(map).sound == pixel_town::SoundCue::success);
}

TEST_CASE("abandoning a started location activity plays the failure cue") {
    pixel_town::AudioCueTracker tracker;
    pixel_town::AudioSceneState library;
    library.visible_location = pixel_town::Location::library;
    library.location_activity_started = true;
    CHECK(tracker.update(library).sound == pixel_town::SoundCue::none);

    auto map = library;
    map.visible_location.reset();
    map.location_activity_started = false;
    CHECK(tracker.update(map).sound == pixel_town::SoundCue::failure);
}

TEST_CASE("settling a lost tavern challenge plays the failure cue") {
    pixel_town::AudioCueTracker tracker;
    pixel_town::AudioSceneState tavern;
    tavern.visible_location = pixel_town::Location::tavern;
    tavern.location_activity_started = true;
    CHECK(tracker.update(tavern).sound == pixel_town::SoundCue::none);

    auto map = tavern;
    map.visible_location.reset();
    map.location_activity_started = false;
    map.visits.tavern = 1;
    map.tavern_losses = 1;
    CHECK(tracker.update(map).sound == pixel_town::SoundCue::failure);
}

TEST_CASE("finishing home rest in one frame plays the success cue") {
    pixel_town::AudioCueTracker tracker;
    pixel_town::AudioSceneState home;
    home.visible_location = pixel_town::Location::home;
    CHECK(tracker.update(home).sound == pixel_town::SoundCue::none);

    auto summary = home;
    summary.visible_location.reset();
    summary.visits.home = 1;
    summary.phase = pixel_town::GamePhase::day_summary;
    CHECK(tracker.update(summary).sound == pixel_town::SoundCue::success);
}

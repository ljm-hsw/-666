// 音频场景状态与去重提示计算；将游戏状态变化转换为播放意图。
#include "app/audio_cues.hpp"

namespace pixel_town {
namespace {

int visit_count(const LocationVisitCounts& visits, Location location) {
    switch (location) {
        case Location::home:
            return visits.home;
        case Location::restaurant:
            return visits.restaurant;
        case Location::convenience_store:
            return visits.convenience_store;
        case Location::library:
            return visits.library;
        case Location::tavern:
            return visits.tavern;
    }
    return 0;
}

}  // namespace

MusicTrack music_track_for(const AudioSceneState& state) {
    if (state.visible_location.has_value()) {
        switch (*state.visible_location) {
            case Location::restaurant:
                return MusicTrack::restaurant;
            case Location::convenience_store:
                return MusicTrack::convenience_store;
            case Location::library:
                return MusicTrack::library;
            case Location::tavern:
                return MusicTrack::tavern;
            case Location::home:
                return MusicTrack::home;
        }
    }
    const bool map_phase = state.phase == GamePhase::day_choice ||
                           state.phase == GamePhase::night_choice;
    if (!state.visible_location.has_value() && map_phase &&
        state.weather.find("雨") != std::string::npos) {
        return MusicTrack::rainy_day;
    }
    return MusicTrack::main_map;
}

AudioDecision AudioCueTracker::update(const AudioSceneState& state) {
    SoundCue sound = SoundCue::none;
    if (has_previous_ && previous_.location_activity_started &&
        previous_.visible_location == Location::tavern &&
        previous_.visible_location != state.visible_location &&
        state.tavern_losses > previous_.tavern_losses) {
        sound = SoundCue::failure;
    } else if (has_previous_ && previous_.visible_location.has_value() &&
               previous_.visible_location != state.visible_location &&
               visit_count(state.visits, *previous_.visible_location) >
                   visit_count(previous_.visits, *previous_.visible_location)) {
        sound = SoundCue::success;
    } else if (has_previous_ && previous_.location_activity_started &&
               previous_.visible_location.has_value() &&
               previous_.visible_location != state.visible_location) {
        sound = SoundCue::failure;
    } else if (has_previous_ && previous_.visible_location != state.visible_location) {
        sound = state.visible_location == Location::home ? SoundCue::return_home
                                                         : SoundCue::location_switch;
    }
    previous_ = state;
    has_previous_ = true;
    return AudioDecision{music_track_for(state), sound};
}

}  // namespace pixel_town

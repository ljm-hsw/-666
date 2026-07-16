// 从只读 GameAppState 派生音频场景，不修改会话或地点 Runtime。
#pragma once

#include <optional>
#include <string>

#include "core/game_session.hpp"

namespace pixel_town {

enum class MusicTrack {
    main_map,
    rainy_day,
    restaurant,
    convenience_store,
    library,
    tavern,
    home,
};

enum class SoundCue {
    none,
    location_switch,
    return_home,
    success,
    failure,
};

struct AudioSceneState {
    GamePhase phase{GamePhase::day_choice};
    std::optional<Location> visible_location;
    bool location_activity_started{false};
    std::string weather;
    LocationVisitCounts visits{};
    int tavern_losses{0};
};

struct AudioDecision {
    MusicTrack music{MusicTrack::main_map};
    SoundCue sound{SoundCue::none};
};

[[nodiscard]] MusicTrack music_track_for(const AudioSceneState& state);

class AudioCueTracker {
public:
    [[nodiscard]] AudioDecision update(const AudioSceneState& state);

private:
    bool has_previous_{false};
    AudioSceneState previous_{};
};

}  // namespace pixel_town

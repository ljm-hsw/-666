#pragma once

#include <array>
#include <optional>

#include <raylib.h>

#include "app/audio_cues.hpp"

namespace pixel_town {

class AudioRuntime {
public:
    AudioRuntime() = default;
    AudioRuntime(const AudioRuntime&) = delete;
    AudioRuntime& operator=(const AudioRuntime&) = delete;

    [[nodiscard]] bool initialize();
    void update(const AudioDecision& decision, bool enabled);
    void shutdown();
    [[nodiscard]] bool ready() const noexcept { return ready_; }

private:
    std::array<Music, 7> music_{};
    std::array<bool, 7> music_loaded_{};
    std::array<Sound, 4> sounds_{};
    std::array<bool, 4> sound_loaded_{};
    std::optional<MusicTrack> current_track_;
    bool music_playing_{false};
    bool owns_device_{false};
    bool ready_{false};
};

}  // namespace pixel_town

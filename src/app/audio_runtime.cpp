// 根据 AudioCueTracker 的场景提示加载、切换和更新背景音乐/音效。
#include "app/audio_runtime.hpp"

#include <array>
#include <cstddef>
#include <utility>

namespace pixel_town {
namespace {

constexpr std::array<std::pair<MusicTrack, const char*>, 7> music_files{{
    {MusicTrack::main_map, "assets/audio/bgm_main_map.mp3"},
    {MusicTrack::rainy_day, "assets/audio/bgm_rainy_day.mp3"},
    {MusicTrack::restaurant, "assets/audio/bgm_restaurant.mp3"},
    {MusicTrack::convenience_store, "assets/audio/bgm_store.mp3"},
    {MusicTrack::library, "assets/audio/bgm_library.mp3"},
    {MusicTrack::tavern, "assets/audio/bgm_tavern.mp3"},
    {MusicTrack::home, "assets/audio/bgm_home.mp3"},
}};

constexpr std::array<std::pair<SoundCue, const char*>, 4> sound_files{{
    {SoundCue::location_switch, "assets/audio/sfx_location_switch.mp3"},
    {SoundCue::return_home, "assets/audio/sfx_return_home.mp3"},
    {SoundCue::success, "assets/audio/sfx_success.mp3"},
    {SoundCue::failure, "assets/audio/sfx_failure.mp3"},
}};

std::size_t music_index(MusicTrack track) {
    return static_cast<std::size_t>(track);
}

std::size_t sound_index(SoundCue cue) {
    return static_cast<std::size_t>(cue) - 1U;
}

}  // namespace

bool AudioRuntime::initialize() {
    if (ready_) {
        return true;
    }

    InitAudioDevice();
    owns_device_ = IsAudioDeviceReady();
    if (!owns_device_) {
        TraceLog(LOG_WARNING, "AUDIO: Device initialization failed; continuing muted");
        return false;
    }

    for (const auto& [track, path] : music_files) {
        const std::size_t index = music_index(track);
        music_[index] = LoadMusicStream(path);
        music_loaded_[index] = IsMusicValid(music_[index]);
        if (!music_loaded_[index]) {
            TraceLog(LOG_WARNING, "AUDIO: Could not load music stream: %s", path);
            shutdown();
            return false;
        }
        music_[index].looping = true;
        SetMusicVolume(music_[index], 0.45F);
    }

    for (const auto& [cue, path] : sound_files) {
        const std::size_t index = sound_index(cue);
        sounds_[index] = LoadSound(path);
        sound_loaded_[index] = IsSoundValid(sounds_[index]);
        if (!sound_loaded_[index]) {
            TraceLog(LOG_WARNING, "AUDIO: Could not load sound effect: %s", path);
            shutdown();
            return false;
        }
        SetSoundVolume(sounds_[index], 0.70F);
    }

    ready_ = true;
    TraceLog(LOG_INFO, "AUDIO: Scene music and sound effects loaded");
    return true;
}

void AudioRuntime::update(const AudioDecision& decision, bool enabled) {
    if (!ready_) {
        return;
    }

    if (current_track_ != decision.music) {
        if (current_track_.has_value() && music_playing_) {
            StopMusicStream(music_[music_index(*current_track_)]);
        }
        current_track_ = decision.music;
        music_playing_ = false;
    }

    Music& current_music = music_[music_index(*current_track_)];
    if (!enabled) {
        if (music_playing_) {
            StopMusicStream(current_music);
            music_playing_ = false;
        }
        return;
    }

    if (!music_playing_) {
        PlayMusicStream(current_music);
        music_playing_ = true;
    }
    UpdateMusicStream(current_music);

    if (decision.sound != SoundCue::none) {
        PlaySound(sounds_[sound_index(decision.sound)]);
    }
}

void AudioRuntime::shutdown() {
    if (current_track_.has_value() && music_playing_) {
        StopMusicStream(music_[music_index(*current_track_)]);
    }
    music_playing_ = false;
    current_track_.reset();

    for (std::size_t index = 0; index < sounds_.size(); ++index) {
        if (sound_loaded_[index]) {
            UnloadSound(sounds_[index]);
            sound_loaded_[index] = false;
        }
    }
    for (std::size_t index = 0; index < music_.size(); ++index) {
        if (music_loaded_[index]) {
            UnloadMusicStream(music_[index]);
            music_loaded_[index] = false;
        }
    }

    if (owns_device_) {
        CloseAudioDevice();
        owns_device_ = false;
    }
    ready_ = false;
}

}  // namespace pixel_town

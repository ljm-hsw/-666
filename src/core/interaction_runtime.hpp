// 全局交互策略：暂停、静音、失焦和最小化时决定是否允许游戏更新。
#pragma once

#include <algorithm>

namespace pixel_town {

struct InteractionFrameInput {
    double delta_seconds{0.0};
    bool toggle_pause{false};
    bool toggle_mute{false};
    bool window_focused{true};
    bool window_minimized{false};
};

struct InteractionFrameResult {
    bool game_updates_enabled{true};
    bool mute_toggled{false};
};

class InteractionRuntime {
public:
    [[nodiscard]] bool paused() const noexcept { return paused_; }
    [[nodiscard]] bool muted() const noexcept { return muted_; }
    [[nodiscard]] double active_elapsed_seconds() const noexcept { return active_elapsed_seconds_; }

    void set_muted(bool muted) noexcept { muted_ = muted; }

    [[nodiscard]] InteractionFrameResult update(const InteractionFrameInput& input) noexcept {
        if (input.toggle_pause && input.window_focused && !input.window_minimized) {
            paused_ = !paused_;
        }
        bool mute_toggled = false;
        if (input.toggle_mute) {
            muted_ = !muted_;
            mute_toggled = true;
        }

        const bool updates_enabled = input.window_focused && !input.window_minimized && !paused_;
        if (updates_enabled) {
            active_elapsed_seconds_ += std::max(0.0, input.delta_seconds);
        }
        return InteractionFrameResult{updates_enabled, mute_toggled};
    }

private:
    bool paused_{false};
    bool muted_{false};
    double active_elapsed_seconds_{0.0};
};

}  // namespace pixel_town

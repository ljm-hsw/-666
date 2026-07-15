#pragma once

#include <optional>
#include <string>

#include "app/dialogue_runtime.hpp"
#include "core/game_session.hpp"
#include "core/story_dialogue.hpp"

namespace pixel_town {

enum class StoryLifecycleContext {
    new_game_opening,
    home_rest,
};

enum class StoryLifecycleStepStatus {
    unchanged,
    changed,
    completed,
    rejected,
};

struct StoryLifecycleStepResult {
    StoryLifecycleStepStatus status{StoryLifecycleStepStatus::unchanged};
    StoryLifecycleContext context{StoryLifecycleContext::new_game_opening};
    bool action_applied{false};
    std::string notice;
};

struct StoryLifecyclePresentation {
    bool active{false};
    StoryLifecycleContext context{StoryLifecycleContext::new_game_opening};
    std::optional<DialoguePresentation> dialogue;
};

class StoryLifecycleRuntime {
public:
    [[nodiscard]] bool open(StoryLifecycleContext context);
    [[nodiscard]] StoryLifecycleStepResult step(
        const DialogueFrameInput& input);
    [[nodiscard]] StoryLifecyclePresentation presentation() const;
    [[nodiscard]] bool active() const noexcept { return active_; }

private:
    StoryDialogueCatalog dialogue_catalog_;
    DialogueRuntime dialogue_;
    StoryLifecycleContext context_{StoryLifecycleContext::new_game_opening};
    bool active_{false};
};

[[nodiscard]] StoryLifecycleStepResult step_story_lifecycle(
    GameSession& session, StoryLifecycleRuntime& runtime,
    const DialogueFrameInput& input);
[[nodiscard]] bool should_replay_new_game_opening(
    const GameSession& session);

}  // namespace pixel_town

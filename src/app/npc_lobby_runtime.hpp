// 餐馆/便利店/NPC 大厅通用 Runtime：先完成 NPC 对话，再请求地点活动。
#pragma once

#include <optional>
#include <string>

#include "app/dialogue_runtime.hpp"
#include "core/story_dialogue.hpp"

namespace pixel_town {

struct NpcLobbyInput {
    float elapsed_seconds{0.0F};
    DialogueFrameInput dialogue;
    bool interaction_activated{false};
    bool back_pressed{false};
};

struct NpcLobbyPresentation {
    bool active{false};
    float npc_animation_seconds{0.0F};
    std::optional<DialoguePresentation> dialogue;
};

enum class NpcLobbyStepStatus {
    unchanged,
    changed,
    dialogue_opened,
    activity_requested,
    closed,
    rejected,
};

struct NpcLobbyStepResult {
    NpcLobbyStepStatus status{NpcLobbyStepStatus::unchanged};
    std::string notice;
};

class NpcLobbyRuntime {
public:
    [[nodiscard]] bool open(DialogueTrigger trigger);
    [[nodiscard]] bool open(const DialogueScript& script);
    [[nodiscard]] NpcLobbyStepResult step(const NpcLobbyInput& input);
    [[nodiscard]] NpcLobbyPresentation presentation() const;
    [[nodiscard]] bool active() const noexcept { return active_; }

private:
    StoryDialogueCatalog dialogue_catalog_;
    DialogueRuntime dialogue_;
    std::optional<DialogueScript> script_;
    float npc_animation_seconds_{0.0F};
    bool active_{false};
};

}  // namespace pixel_town

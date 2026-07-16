// 图书馆室内展示 Runtime：负责管理员对话、导航和进入玩法选择的窄接口。
#pragma once

#include <optional>
#include <string>

#include "app/dialogue_runtime.hpp"
#include "core/story_dialogue.hpp"

namespace pixel_town {

struct LibraryRoomInput {
    float elapsed_seconds{0.0F};
    DialogueFrameInput dialogue;
    bool administrator_activated{false};
    bool back_pressed{false};
};

struct LibraryRoomPresentation {
    bool active{false};
    float administrator_animation_seconds{0.0F};
    std::optional<DialoguePresentation> dialogue;
};

enum class LibraryRoomStepStatus {
    unchanged,
    changed,
    dialogue_opened,
    work_requested,
    closed,
    rejected,
};

struct LibraryRoomStepResult {
    LibraryRoomStepStatus status{LibraryRoomStepStatus::unchanged};
    std::string notice;
};

class LibraryRoomRuntime {
public:
    [[nodiscard]] bool open();
    [[nodiscard]] bool open(const DialogueScript& script);
    [[nodiscard]] LibraryRoomStepResult step(const LibraryRoomInput& input);
    [[nodiscard]] LibraryRoomPresentation presentation() const;
    [[nodiscard]] bool active() const noexcept { return active_; }

private:
    StoryDialogueCatalog dialogue_catalog_;
    DialogueRuntime dialogue_;
    std::optional<DialogueScript> script_;
    float administrator_animation_seconds_{0.0F};
    bool active_{false};
};

}  // namespace pixel_town

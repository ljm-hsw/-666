#pragma once

#include <cstddef>
#include <string>

#include "core/story_dialogue.hpp"

namespace pixel_town {

struct DialogueFrameInput {
    bool advance_pressed{false};
    bool skip_pressed{false};
};

struct DialoguePresentation {
    bool active{false};
    std::string speaker;
    std::string text;
    std::size_t current_line{0};
    std::size_t total_lines{0};
};

enum class DialogueStepStatus {
    unchanged,
    advanced,
    closed,
};

class DialogueRuntime {
public:
    [[nodiscard]] bool open(const DialogueScript& script);
    [[nodiscard]] DialogueStepStatus step(const DialogueFrameInput& input);
    [[nodiscard]] DialoguePresentation presentation() const;
    [[nodiscard]] bool active() const noexcept { return active_; }

private:
    DialogueScript script_;
    std::size_t line_index_{0};
    bool active_{false};
};

}  // namespace pixel_town

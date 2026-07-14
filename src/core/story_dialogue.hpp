#pragma once

#include <string>
#include <vector>

namespace pixel_town {

enum class DialogueTrigger {
    tavern_bartender_intro,
    library_administrator_intro,
    restaurant_owner_intro,
};

struct DialogueLine {
    std::string speaker;
    std::string text;
};

struct DialogueScript {
    DialogueTrigger trigger{DialogueTrigger::tavern_bartender_intro};
    std::vector<DialogueLine> lines;
};

class StoryDialogueCatalog {
public:
    [[nodiscard]] const DialogueScript* find(DialogueTrigger trigger) const noexcept;
    [[nodiscard]] std::string glyphs() const;
};

}  // namespace pixel_town

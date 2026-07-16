// 对话领域契约和脚本目录接口；文本消费由 app 层 DialogueRuntime 完成。
#pragma once

#include <string>
#include <vector>

namespace pixel_town {

enum class DialogueTrigger {
    tavern_bartender_intro,
    library_administrator_intro,
    restaurant_owner_intro,
    convenience_store_owner_intro,
    mayor_new_game_intro,
    home_rest_reflection,
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

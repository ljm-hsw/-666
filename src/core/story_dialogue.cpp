#include "core/story_dialogue.hpp"

namespace pixel_town {
namespace {

const DialogueScript bartender_intro{
    DialogueTrigger::tavern_bartender_intro,
    {
        {"酒保", "新面孔？先别急着下注，酒馆的规矩不多。"},
        {"主角", "我先看看。左边是棋桌，右边是骰盅，对吗？"},
        {"酒保", "对。选好玩法和赌注再坐下，输赢都只算今晚这一局。"},
    },
};

}  // namespace

const DialogueScript* StoryDialogueCatalog::find(DialogueTrigger trigger) const noexcept {
    switch (trigger) {
        case DialogueTrigger::tavern_bartender_intro:
            return &bartender_intro;
    }
    return nullptr;
}

std::string StoryDialogueCatalog::glyphs() const {
    std::string result;
    for (const auto& line : bartender_intro.lines) {
        result += line.speaker;
        result += line.text;
    }
    return result;
}

}  // namespace pixel_town

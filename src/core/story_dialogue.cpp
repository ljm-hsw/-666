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

const DialogueScript library_administrator_intro{
    DialogueTrigger::library_administrator_intro,
    {
        {"管理员", "欢迎来到图书馆。先在房间里走走，别碰到书架。"},
        {"主角", "我想帮点忙。今天有什么需要处理的？"},
        {"管理员", "可以帮读者找书，也可以把散落的书整理归位。"},
    },
};

}  // namespace

const DialogueScript* StoryDialogueCatalog::find(DialogueTrigger trigger) const noexcept {
    switch (trigger) {
        case DialogueTrigger::tavern_bartender_intro:
            return &bartender_intro;
        case DialogueTrigger::library_administrator_intro:
            return &library_administrator_intro;
    }
    return nullptr;
}

std::string StoryDialogueCatalog::glyphs() const {
    std::string result;
    for (const auto& line : bartender_intro.lines) {
        result += line.speaker;
        result += line.text;
    }
    for (const auto& line : library_administrator_intro.lines) {
        result += line.speaker;
        result += line.text;
    }
    return result;
}

}  // namespace pixel_town

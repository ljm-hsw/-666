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
        {"管理员", "欢迎来到图书馆。书架按类别排好，先听听读者需要什么。"},
        {"主角", "我想帮点忙。今天有什么需要处理的？"},
        {"管理员", "可以帮读者找书，也可以把散落的书整理归位。"},
    },
};

const DialogueScript restaurant_owner_intro{
    DialogueTrigger::restaurant_owner_intro,
    {
        {"餐馆老板", "围裙在门边。先看清订单，忙起来也别端错桌。"},
        {"主角", "明白。我先记菜名和桌号，再把每份餐送稳。"},
        {"餐馆老板", "好。客人等得起一句招呼，订单可等不起发呆。"},
    },
};

const DialogueScript convenience_store_owner_intro{
    DialogueTrigger::convenience_store_owner_intro,
    {
        {"便利店店主", "先看看今天的天气。下雨和晴天，货架缺的东西不一样。"},
        {"主角", "我会对照库存进货，再决定价格，不让常用商品断档。"},
        {"便利店店主", "好，卖完别只看收入，成本和剩货也要记进账本。"},
    },
};

}  // namespace

const DialogueScript* StoryDialogueCatalog::find(DialogueTrigger trigger) const noexcept {
    switch (trigger) {
        case DialogueTrigger::tavern_bartender_intro:
            return &bartender_intro;
        case DialogueTrigger::library_administrator_intro:
            return &library_administrator_intro;
        case DialogueTrigger::restaurant_owner_intro:
            return &restaurant_owner_intro;
        case DialogueTrigger::convenience_store_owner_intro:
            return &convenience_store_owner_intro;
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
    for (const auto& line : restaurant_owner_intro.lines) {
        result += line.speaker;
        result += line.text;
    }
    for (const auto& line : convenience_store_owner_intro.lines) {
        result += line.speaker;
        result += line.text;
    }
    return result;
}

}  // namespace pixel_town

// 日程、行动结果和结局的静态文案；与 raylib 和存档格式解耦。
#include "core/story_text.hpp"

#include <algorithm>
#include <array>

#include "core/ending_rules.hpp"
namespace pixel_town {
namespace {

constexpr std::array<const char*, 10> daily_prompts{
    "镇长把地图摊在桌上，铅笔圈出几个地点。今天先走一遍也没关系。",
    "餐馆门口的菜单换了新纸，墨还没干透。街上有人比昨天更早出门。",
    "天气晴。便利店门口摆出汽水箱，箱角压着一张旧集市传单。",
    "图书馆窗台晒着借书卡。管理员说，今天适合把书页翻慢一点。",
    "傍晚可能下雨。店主把雨伞往门边挪了挪，又看了一眼账本。",
    "酒馆昨晚的棋盘还没收。有人把一枚黑子落在了桌缝旁。",
    "餐馆门口多摆了一张桌子。老板说只是天气好，适合坐外面。",
    "旧海报被重新贴平了。图书馆那边传来扫地的声音。",
    "小镇路不长，但你已经能记住哪盏灯最晚熄。",
    "镇长说晚上大家会简单聊聊这十天。桌上已经放好了账本和几张便签。",
};

constexpr std::array<const char*, 10> day_closings{
    "第一天的路还不熟，但地图已经被你折出新的痕迹。",
    "街上的声音比昨天近了一点。有人开始记得你经过的方向。",
    "旧集市传单被风吹到脚边，又被人重新压回箱角。",
    "书页翻动的声音很轻，今天的小镇也慢了下来。",
    "傍晚的雨没有把灯吹灭，门边的伞少了几把。",
    "酒馆的桌缝里还留着一枚棋子，夜色从窗边慢慢退开。",
    "餐馆外那张新桌子收了起来，桌面还留着一点温度。",
    "旧海报贴平以后，街口看起来像被认真整理过一次。",
    "你已经能分清哪扇门晚些关，哪盏灯会多亮一会儿。",
    "十天不长，但也够大家把一些事放在桌上说一说了。",
};

const char* completed_location_summary(Location location) {
    switch (location) {
        case Location::restaurant:
            return "你把最后一碗汤端到靠窗的位置，老板只点了点头。\n"
                   "午饭高峰过去，桌面还热着。熟客说今天上菜挺快。";
        case Location::convenience_store:
            return "雨伞少了三把，饭团也卖空一排。\n"
                   "店主把零钱理进抽屉，账本边角多了一个小勾。";
        case Location::library:
            return "你帮孩子找到了考试要用的书，借书卡又多盖了一个章。\n"
                   "管理员把旧地图递给你，说可以先看看，不急着还。";
        case Location::tavern:
            return "棋子落下去的声音不大，旁边的人却都看了过来。\n"
                   "你输了这一局，但椅子还给你留着。";
        case Location::home:
            return "你烧了热水，把今天的事写进日记。\n"
                   "窗灯亮到很晚，街上的声音慢慢退下去。";
    }
    return "行动完成。";
}

}  // namespace

const char* opening_story() {
    return "车在小镇入口停下时，路边的旧海报被风掀起一角。\n"
           "镇长拿着钥匙和地图等在站牌旁，地图折痕很深。\n"
           "“先住十天吧。想去哪里，就从哪里开始。”";
}

const char* daily_prompt(int day) {
    if (is_five_day_showcase_build() && day == configured_game_day_limit()) {
        return daily_prompts.back();
    }
    if (day < 1 || day > static_cast<int>(daily_prompts.size())) {
        return daily_prompts.front();
    }
    return daily_prompts[static_cast<std::size_t>(day - 1)];
}

const char* location_result_summary(Location location, ActionOutcome outcome) {
    if (outcome == ActionOutcome::abandoned) {
        return "你提前离开了这里，今天没有留下太多痕迹。\n"
               "阶段已经过去，小镇把这段空白也算进日子里。";
    }
    return completed_location_summary(location);
}

const char* day_closing_summary(int day) {
    if (is_five_day_showcase_build() && day == configured_game_day_limit()) {
        return day_closings.back();
    }
    if (day < 1 || day > static_cast<int>(day_closings.size())) {
        return day_closings.front();
    }
    return day_closings[static_cast<std::size_t>(day - 1)];
}

const char* council_opening(int final_day) {
    if (final_day == 5) {
        return "第五天晚上，镇长把几张便签摊在桌上。\n"
               "菜单、账本、借书卡和一副棋子被放在旁边。\n"
               "“五天不长，”他说，“但也够大家记住一些事了。”";
    }
    return "第十天晚上，镇长把几张便签摊在桌上。\n"
           "菜单、账本、借书卡和一副棋子被放在旁边。\n"
           "“十天不长，”他说，“但也够大家记住一些事了。”";
}

const char* ending_narrative(MainEnding ending) {
    switch (ending) {
        case MainEnding::town_star:
            return "菜单、账本、借书卡和棋子旁，都有人提起你的名字。";
        case MainEnding::money_machine:
            return "账本最后一页很整齐，店主又在总数旁添了一道线。";
        case MainEnding::popular_resident:
            return "街上已经有人先向你点头，像你在这里住了很久。";
        case MainEnding::library_star:
            return "管理员展开旧地图，在空白处添上了你的名字。";
        case MainEnding::tavern_legend:
            return "常客未必记准每一局，却一直留着你坐过的位置。";
        case MainEnding::ordinary_newcomer:
            return "地图的纸角软了一点，小镇记住了你走过的路。";
        case MainEnding::business_failure:
            return "账本留着几处空白，镇长把折好的地图放回你手边。";
    }
    return "地图的纸角软了一点，小镇记住了你走过的路。";
}

std::string story_text_glyphs() {
    std::string glyphs;
    glyphs += opening_story();
    for (const char* prompt : daily_prompts) {
        glyphs += prompt;
    }
    for (const char* closing : day_closings) {
        glyphs += closing;
    }
    glyphs += location_result_summary(Location::restaurant, ActionOutcome::completed);
    glyphs += location_result_summary(Location::convenience_store, ActionOutcome::completed);
    glyphs += location_result_summary(Location::library, ActionOutcome::completed);
    glyphs += location_result_summary(Location::tavern, ActionOutcome::completed);
    glyphs += location_result_summary(Location::home, ActionOutcome::completed);
    glyphs += location_result_summary(Location::home, ActionOutcome::abandoned);
    glyphs += council_opening(5);
    glyphs += council_opening(10);
    constexpr std::array endings{
        MainEnding::town_star,
        MainEnding::money_machine,
        MainEnding::popular_resident,
        MainEnding::library_star,
        MainEnding::tavern_legend,
        MainEnding::ordinary_newcomer,
        MainEnding::business_failure,
    };
    for (const MainEnding ending : endings) {
        glyphs += ending_narrative(ending);
    }
    glyphs.erase(std::remove(glyphs.begin(), glyphs.end(), '\n'), glyphs.end());
    return glyphs;
}

}  // namespace pixel_town

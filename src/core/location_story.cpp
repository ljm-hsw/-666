#include "core/location_story.hpp"

namespace pixel_town {
namespace {

DialogueTrigger tutorial_trigger_for(Location location) {
    switch (location) {
        case Location::restaurant:
            return DialogueTrigger::restaurant_owner_intro;
        case Location::convenience_store:
            return DialogueTrigger::convenience_store_owner_intro;
        case Location::library:
            return DialogueTrigger::library_administrator_intro;
        case Location::tavern:
            return DialogueTrigger::tavern_bartender_intro;
        case Location::home:
            return DialogueTrigger::home_rest_reflection;
    }
    return DialogueTrigger::home_rest_reflection;
}

const char* location_id(Location location) {
    switch (location) {
        case Location::restaurant:
            return "restaurant";
        case Location::convenience_store:
            return "convenience_store";
        case Location::library:
            return "library";
        case Location::tavern:
            return "tavern";
        case Location::home:
            return "home";
    }
    return "home";
}

DialogueScript finale_script(Location location) {
    const DialogueTrigger trigger = tutorial_trigger_for(location);
    switch (location) {
        case Location::restaurant:
            return {trigger, {{"餐馆老板", "这张菜单我会带去晚上。十天里，总有几碗汤记得你的手。"}}};
        case Location::convenience_store:
            return {trigger, {{"便利店店主", "账本边角的勾都在。晚上聊十天时，这页也带上。"}}};
        case Location::library:
            return {trigger, {{"管理员", "我挑了几张借书卡。晚上大家会看看，这十天留下些什么。"}}};
        case Location::tavern:
            return {trigger, {{"酒保", "棋子和骰盅都收好了。今晚先听镇长说几句。"}}};
        case Location::home:
            return {trigger, {{"主角", "日记先合上。十天的事，晚上总能慢慢说。"}}};
    }
    return {trigger, {}};
}

const char* npc_speaker(Location location) {
    switch (location) {
        case Location::restaurant:
            return "餐馆老板";
        case Location::convenience_store:
            return "便利店店主";
        case Location::library:
            return "管理员";
        case Location::tavern:
            return "酒保";
        case Location::home:
            return "主角";
    }
    return "主角";
}

DialogueScript location_script(Location location, const char* npc_text,
                               const char* protagonist_text) {
    return {tutorial_trigger_for(location),
            {{npc_speaker(location), npc_text}, {"主角", protagonist_text}}};
}

DialogueScript daily_script(Location location, int day) {
    if (location == Location::restaurant && day == 2) {
        return location_script(location, "新菜单刚换好，先把桌号听清。今天街上比昨天早热闹。",
                               "我会先确认桌号，再开始上菜。");
    }
    if (location == Location::restaurant && day == 3) {
        return location_script(location, "旧集市传单夹进了菜单，先替我压平，别让风吹走。",
                               "传单背面像是写着很早以前的日期。");
    }
    if (location == Location::restaurant && day == 4) {
        return location_script(location, "带孩子的客人临时改了订单，先听清再下单。",
                               "今天要比平时更仔细一点。");
    }
    if (location == Location::convenience_store && day == 2) {
        return location_script(location, "早班客人变多了，便当和咖啡要先看一眼库存。",
                               "我会把成本和剩货一起记住。");
    }
    if (location == Location::convenience_store && day == 3) {
        return location_script(location, "汽水箱角压着旧集市传单，收银台旁先别弄乱。",
                               "原来这里以前也办过集市。");
    }
    if (location == Location::convenience_store && day == 4) {
        return location_script(location, "考试周文具和咖啡更受欢迎，价格别只盯着一种商品。",
                               "我会把进货数量和价格档一起考虑。");
    }
    if (location == Location::library && day == 4) {
        return location_script(location, "借书卡在窗边晒着。今天帮读者找书时，慢一点也没关系。",
                               "我会先听清读者要找什么。");
    }
    if (location == Location::library && day == 2) {
        return location_script(location, "新借书卡刚装进盒子，先慢慢熟悉每一类书架。",
                               "我会记住读者常问的方向。");
    }
    if (location == Location::library && day == 3) {
        return location_script(location, "捐来的旧书里夹着褪色价签，先放到整理桌上。",
                               "这张价签也许和旧集市有关。");
    }
    if (location == Location::tavern && day == 2) {
        return location_script(location, "常客给新面孔挪了半张桌子。先看清玩法，再慢慢坐下。",
                               "我先熟悉这里的节奏，不急着下注。");
    }
    if (location == Location::tavern && day == 4) {
        return location_script(location, "有人带来一本旧棋谱，正争论哪一步更稳。",
                               "我会看清局面，也会记得骰盅那边的规矩。");
    }
    if (location == Location::tavern && day == 9) {
        return location_script(location, "常客已经替你留了位置，玩法和赌注还是由你自己选。",
                               "熟悉归熟悉，今晚的选择仍要认真。");
    }
    if (location == Location::home && day == 2) {
        return location_script(location, "地图上还空着几条路，先把今天走过的地方补上。",
                               "确认休息后再合上日记，明天继续。");
    }
    if (location == Location::home && day == 4) {
        return location_script(location, "借来的书放在灯下，屋里终于安静下来。",
                               "今晚休息好，明天再把没读完的页数接上。");
    }
    if (location == Location::home && day == 9) {
        return location_script(location, "已经知道哪盏灯会最晚熄，这间屋也更像住过。",
                               "再休息一晚，就要整理这十天的答案了。");
    }
    if (location == Location::home && day == 7) {
        return {tutorial_trigger_for(location),
                {{"主角", "窗边晾着今天用过的外套。把灯留暖一点，明天再继续。"}}};
    }
    return {};
}

DialogueScript incident_script(Location location, int day, const std::string& weather) {
    if (location == Location::restaurant && day == 5 && weather == "小雨") {
        return location_script(location, "雨里来的客人都想喝热汤，桌号和菜品更别弄混。",
                               "我会先稳住节奏，再把热汤送到位。");
    }
    if (location == Location::convenience_store && day == 5 && weather == "小雨") {
        return location_script(location, "雨伞一下少了不少。先看库存和成本，别急着把价签改乱。",
                               "我会先把紧俏商品和其他货分开看。");
    }
    if (location == Location::library && day == 5 && weather == "小雨") {
        return location_script(location, "窗边有几页书受潮了，先别把它们和干书混在一起。",
                               "我会先把需要整理的书看清楚。");
    }
    if (location == Location::home && day == 5 && weather == "小雨") {
        return location_script(location, "窗缝漏进几滴雨，灯下的日记先往里挪一挪。",
                               "检查好窗边，再确认休息，恢复规则不变。");
    }
    if (location == Location::restaurant && day == 8) {
        return location_script(location, "旧集市用过的大汤锅擦出来了，今天先放在后厨别多问。",
                               "大家好像都记得它，只是暂时不说。");
    }
    if (location == Location::convenience_store && day == 8) {
        return location_script(location, "仓库翻出旧价签和纸杯，先别把它们和今天的货混在一起。",
                               "价签上的摊位编号和传单很像。");
    }
    if (location == Location::library && day == 8) {
        return location_script(location, "旧地图从借书卡盒后滑出来，边缘圈着几个摊位。",
                               "我会先记住这些圈出的地方。");
    }
    if (location == Location::tavern && day == 8) {
        return location_script(location, "旧照片从相框里滑了一点，里面的人都围着棋桌笑。",
                               "我先把相框摆正，再选择今晚的玩法。");
    }
    if (location == Location::home && day == 8) {
        return location_script(location, "旧海报的一角又翘起来了，灯也短暂闪了两下。",
                               "把海报贴平就好，今晚仍只结算一次休息。");
    }
    if (location == Location::tavern && day == 6) {
        return location_script(location, "昨晚有枚黑棋子卡在桌缝。先别急着坐，替我看看有没有别的落下。",
                               "我会先看看桌边，再开始今天的游戏。");
    }
    return {};
}

std::string incident_id(Location location, int day, const std::string& weather) {
    if (day == 5 && weather == "小雨") {
        switch (location) {
            case Location::restaurant:
                return "restaurant_day_5_rainy_guests";
            case Location::convenience_store:
                return "convenience_store_day_5_umbrella_shortage";
            case Location::library:
                return "library_day_5_damp_pages";
            case Location::tavern:
                break;
            case Location::home:
                return "home_day_5_leaking_window";
        }
    }
    if (day == 8) {
        switch (location) {
            case Location::restaurant:
                return "restaurant_day_8_old_market_pot";
            case Location::convenience_store:
                return "convenience_store_day_8_old_price_tags";
            case Location::library:
                return "library_day_8_old_market_map";
            case Location::tavern:
                return "tavern_day_8_old_photo";
            case Location::home:
                return "home_day_8_flickering_lamp";
        }
    }
    if (location == Location::tavern && day == 6) {
        return "tavern_day_6_lost_black_piece";
    }
    return std::string{location_id(location)} + "_incident";
}

void append_script_glyphs(std::string& glyphs, const DialogueScript& script) {
    for (const DialogueLine& line : script.lines) {
        glyphs += line.speaker;
        glyphs += line.text;
    }
}

}  // namespace

LocationStoryContext location_story_context(const GameSession& session,
                                            Location location) {
    const DayContext day_context = session.current_day_context();
    const int completed_visits = session.location_visit_count(location);
    return {location,
            day_context.day,
            day_context.weather,
            day_context.event,
            completed_visits,
            session.location_seed(location,
                                  static_cast<unsigned int>(completed_visits))};
}

LocationStorySelection LocationStoryCatalog::select(
    const LocationStoryContext& context) const {
    const std::string prefix = location_id(context.location);
    const DialogueTrigger trigger = tutorial_trigger_for(context.location);
    const DialogueScript* script = StoryDialogueCatalog{}.find(trigger);
    if (context.completed_visits <= 0 && script != nullptr) {
        return {LocationStoryEventKind::tutorial, prefix + "_tutorial", *script};
    }
    if (context.day == 10) {
        return {LocationStoryEventKind::finale,
                prefix + "_day_10_finale",
                finale_script(context.location)};
    }
    const DialogueScript daily = daily_script(context.location, context.day);
    if (!daily.lines.empty()) {
        return {LocationStoryEventKind::daily,
                prefix + "_day_" + std::to_string(context.day) + "_daily",
                daily};
    }
    const DialogueScript incident =
        incident_script(context.location, context.day, context.weather);
    if (!incident.lines.empty()) {
        return {LocationStoryEventKind::incident,
                incident_id(context.location, context.day, context.weather),
                incident};
    }
    return {LocationStoryEventKind::familiar,
            prefix + "_familiar",
            {trigger, {{"主角", "今天也从这里开始。"}}}};
}

std::string LocationStoryCatalog::glyphs() const {
    std::string glyphs;
    for (const Location location : {Location::home, Location::restaurant,
                                    Location::convenience_store, Location::library,
                                    Location::tavern}) {
        append_script_glyphs(glyphs, finale_script(location));
    }
    for (const Location location : {Location::restaurant, Location::convenience_store,
                                    Location::library}) {
        for (const int day : {2, 3, 4}) {
            append_script_glyphs(glyphs, daily_script(location, day));
        }
        append_script_glyphs(glyphs, incident_script(location, 5, "小雨"));
        append_script_glyphs(glyphs, incident_script(location, 8, "晴天"));
    }
    append_script_glyphs(glyphs, daily_script(Location::home, 7));
    append_script_glyphs(glyphs, incident_script(Location::tavern, 6, "微风"));
    for (const Location location : {Location::tavern, Location::home}) {
        for (const int day : {2, 4, 9}) {
            append_script_glyphs(glyphs, daily_script(location, day));
        }
        append_script_glyphs(glyphs, incident_script(location, 8, "晴天"));
    }
    append_script_glyphs(glyphs, incident_script(Location::home, 5, "小雨"));
    return glyphs;
}

}  // namespace pixel_town

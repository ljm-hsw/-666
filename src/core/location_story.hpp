// 地点剧情选择器。
//
// 选择输入来自稳定的日程上下文、地点、天气、事件和访问次数；输出是只读
// DialogueScript。剧情用于阻塞/引导流程和展示文本，但不会改变行动结果、
// 资源、玩家属性或日期推进。
#pragma once

#include <string>

#include "core/game_session.hpp"
#include "core/story_dialogue.hpp"

namespace pixel_town {

enum class LocationStoryEventKind {
    tutorial,
    daily,
    incident,
    finale,
    familiar,
};

struct LocationStoryContext {
    Location location{Location::home};
    int day{1};
    std::string weather;
    std::string day_event;
    int completed_visits{0};
    unsigned int seed{0};
};

struct LocationStorySelection {
    LocationStoryEventKind kind{LocationStoryEventKind::familiar};
    std::string id;
    DialogueScript script;
};

[[nodiscard]] LocationStoryContext location_story_context(
    const GameSession& session, Location location);

class LocationStoryCatalog {
public:
    [[nodiscard]] LocationStorySelection select(
        const LocationStoryContext& context) const;
    [[nodiscard]] std::string glyphs() const;
};

}  // namespace pixel_town

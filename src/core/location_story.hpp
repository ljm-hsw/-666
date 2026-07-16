// 地点剧情选择器：根据地点、日期、天气和访问次数选择只读对话脚本。
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

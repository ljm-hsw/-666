// 图书馆两种玩法共享的领域结果，之后由 app Adapter 转为 ActionResult。
#pragma once

#include <string>

namespace pixel_town::library {

struct LibraryWorkResult {
    int money_change{0};
    int stamina_change{0};
    int reputation_change{0};
    int knowledge_change{0};
    int mood_change{0};
    std::string summary;
    bool completed{false};
    bool gave_up{false};
    std::string narrative_echo;
    bool plot_triggered{false};
    std::string plot_title;
    std::string plot_description;
};

}  // namespace pixel_town::library

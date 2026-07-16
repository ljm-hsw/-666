// 线性对话游标和输入消费；对话 active 时阻止同帧其他游戏更新。
#include "app/dialogue_runtime.hpp"

namespace pixel_town {

bool DialogueRuntime::open(const DialogueScript& script) {
    if (script.lines.empty()) {
        return false;
    }
    script_ = script;
    line_index_ = 0;
    active_ = true;
    return true;
}

DialogueStepStatus DialogueRuntime::step(const DialogueFrameInput& input) {
    if (!active_) {
        return DialogueStepStatus::unchanged;
    }
    if (input.skip_pressed) {
        active_ = false;
        return DialogueStepStatus::closed;
    }
    if (!input.advance_pressed) {
        return DialogueStepStatus::unchanged;
    }
    if (line_index_ + 1 < script_.lines.size()) {
        ++line_index_;
        return DialogueStepStatus::advanced;
    }
    active_ = false;
    return DialogueStepStatus::closed;
}

DialoguePresentation DialogueRuntime::presentation() const {
    DialoguePresentation view;
    view.active = active_;
    if (!active_ || script_.lines.empty()) {
        return view;
    }
    view.speaker = script_.lines[line_index_].speaker;
    view.text = script_.lines[line_index_].text;
    view.current_line = line_index_ + 1;
    view.total_lines = script_.lines.size();
    return view;
}

}  // namespace pixel_town

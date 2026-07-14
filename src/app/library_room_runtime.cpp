#include "app/library_room_runtime.hpp"

#include <algorithm>

namespace pixel_town {

bool LibraryRoomRuntime::open() {
    dialogue_ = DialogueRuntime{};
    administrator_animation_seconds_ = 0.0F;
    active_ = true;
    return true;
}

LibraryRoomStepResult LibraryRoomRuntime::step(const LibraryRoomInput& input) {
    if (!active_) {
        return {LibraryRoomStepStatus::rejected,
                "图书馆室内场景尚未打开。"};
    }
    if (input.elapsed_seconds < 0.0F) {
        return {LibraryRoomStepStatus::rejected,
                "图书馆场景收到非法帧时间。"};
    }
    administrator_animation_seconds_ +=
        std::min(input.elapsed_seconds, 1.0F);

    if (dialogue_.active()) {
        const DialogueStepStatus dialogue_status = dialogue_.step(input.dialogue);
        if (dialogue_status == DialogueStepStatus::closed) {
            active_ = false;
            return {LibraryRoomStepStatus::work_requested,
                    "管理员对话结束，请选择今天的图书馆工作。"};
        }
        if (dialogue_status == DialogueStepStatus::advanced) {
            return {LibraryRoomStepStatus::changed, {}};
        }
        return {LibraryRoomStepStatus::unchanged, {}};
    }

    if (input.back_pressed) {
        active_ = false;
        return {LibraryRoomStepStatus::closed,
                "已返回地图：白天阶段未消耗。"};
    }

    if (input.administrator_activated) {
        const auto* script = dialogue_catalog_.find(
            DialogueTrigger::library_administrator_intro);
        if (script == nullptr || !dialogue_.open(*script)) {
            return {LibraryRoomStepStatus::rejected,
                    "管理员对话暂时不可用。"};
        }
        return {LibraryRoomStepStatus::dialogue_opened,
                "已开始与管理员交谈。"};
    }
    return {LibraryRoomStepStatus::unchanged, {}};
}

LibraryRoomPresentation LibraryRoomRuntime::presentation() const {
    LibraryRoomPresentation view;
    view.active = active_;
    view.administrator_animation_seconds =
        administrator_animation_seconds_;
    if (dialogue_.active()) {
        view.dialogue = dialogue_.presentation();
    }
    return view;
}

}  // namespace pixel_town

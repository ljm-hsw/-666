#include "app/npc_lobby_runtime.hpp"

#include <algorithm>

namespace pixel_town {

bool NpcLobbyRuntime::open(DialogueTrigger trigger) {
    if (dialogue_catalog_.find(trigger) == nullptr) {
        return false;
    }
    dialogue_ = DialogueRuntime{};
    trigger_ = trigger;
    npc_animation_seconds_ = 0.0F;
    active_ = true;
    return true;
}

NpcLobbyStepResult NpcLobbyRuntime::step(const NpcLobbyInput& input) {
    if (!active_) {
        return {NpcLobbyStepStatus::rejected, "固定 NPC 场景尚未打开。"};
    }
    if (input.elapsed_seconds < 0.0F) {
        return {NpcLobbyStepStatus::rejected, "固定 NPC 场景收到非法帧时间。"};
    }
    npc_animation_seconds_ += std::min(input.elapsed_seconds, 1.0F);

    if (dialogue_.active()) {
        const DialogueStepStatus dialogue_status = dialogue_.step(input.dialogue);
        if (dialogue_status == DialogueStepStatus::closed) {
            active_ = false;
            return {NpcLobbyStepStatus::activity_requested,
                    "对话结束，准备进入地点活动。"};
        }
        if (dialogue_status == DialogueStepStatus::advanced) {
            return {NpcLobbyStepStatus::changed, {}};
        }
        return {NpcLobbyStepStatus::unchanged, {}};
    }

    if (input.back_pressed) {
        active_ = false;
        return {NpcLobbyStepStatus::closed, "已返回地图：阶段未消耗。"};
    }

    if (input.interaction_activated) {
        const DialogueScript* script =
            trigger_.has_value() ? dialogue_catalog_.find(*trigger_) : nullptr;
        if (script == nullptr || !dialogue_.open(*script)) {
            return {NpcLobbyStepStatus::rejected, "NPC 对话暂时不可用。"};
        }
        return {NpcLobbyStepStatus::dialogue_opened, "已开始与 NPC 交谈。"};
    }
    return {NpcLobbyStepStatus::unchanged, {}};
}

NpcLobbyPresentation NpcLobbyRuntime::presentation() const {
    NpcLobbyPresentation view;
    view.active = active_;
    view.npc_animation_seconds = npc_animation_seconds_;
    if (dialogue_.active()) {
        view.dialogue = dialogue_.presentation();
    }
    return view;
}

}  // namespace pixel_town

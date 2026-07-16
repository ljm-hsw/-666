// 把剧情脚本接入日程边界；剧情结束后才允许继续地点或休息流程。
#include "app/story_lifecycle_runtime.hpp"

namespace pixel_town {
namespace {

DialogueTrigger trigger_for(StoryLifecycleContext context) {
    switch (context) {
        case StoryLifecycleContext::new_game_opening:
            return DialogueTrigger::mayor_new_game_intro;
        case StoryLifecycleContext::home_rest:
            return DialogueTrigger::home_rest_reflection;
    }
    return DialogueTrigger::mayor_new_game_intro;
}

}  // namespace

bool StoryLifecycleRuntime::open(StoryLifecycleContext context) {
    const DialogueScript* script = dialogue_catalog_.find(trigger_for(context));
    if (script == nullptr) {
        return false;
    }
    dialogue_ = DialogueRuntime{};
    context_ = context;
    active_ = dialogue_.open(*script);
    return active_;
}

bool StoryLifecycleRuntime::open_home_rest(const GameSession& session) {
    const LocationStorySelection selection = LocationStoryCatalog{}.select(
        location_story_context(session, Location::home));
    if (selection.script.lines.empty()) {
        return false;
    }
    dialogue_ = DialogueRuntime{};
    context_ = StoryLifecycleContext::home_rest;
    active_ = dialogue_.open(selection.script);
    return active_;
}

StoryLifecycleStepResult StoryLifecycleRuntime::step(
    const DialogueFrameInput& input) {
    if (!active_) {
        return {StoryLifecycleStepStatus::rejected, context_, false, {}};
    }
    const DialogueStepStatus dialogue_status = dialogue_.step(input);
    if (dialogue_status == DialogueStepStatus::closed) {
        active_ = false;
        return {StoryLifecycleStepStatus::completed, context_, false, {}};
    }
    if (dialogue_status == DialogueStepStatus::advanced) {
        return {StoryLifecycleStepStatus::changed, context_, false, {}};
    }
    return {StoryLifecycleStepStatus::unchanged, context_, false, {}};
}

StoryLifecyclePresentation StoryLifecycleRuntime::presentation() const {
    StoryLifecyclePresentation view;
    view.active = active_;
    view.context = context_;
    if (active_) {
        view.dialogue = dialogue_.presentation();
    }
    return view;
}

StoryLifecycleStepResult step_story_lifecycle(
    GameSession& session, StoryLifecycleRuntime& runtime,
    const DialogueFrameInput& input) {
    StoryLifecycleStepResult result = runtime.step(input);
    if (result.status != StoryLifecycleStepStatus::completed) {
        return result;
    }
    if (result.context == StoryLifecycleContext::new_game_opening) {
        result.notice = "第 1 天开始：请选择一个白天工作地点。";
        return result;
    }

    const ApplyResult applied =
        session.apply_action_result(session.home_rest_result());
    result.notice = applied.message;
    result.action_applied = applied.accepted;
    if (!applied.accepted) {
        result.status = StoryLifecycleStepStatus::rejected;
    }
    return result;
}

bool should_replay_new_game_opening(const GameSession& session) {
    const GameSessionSnapshot snapshot = session.snapshot();
    return snapshot.day == 1 && snapshot.phase == GamePhase::day_choice &&
           !snapshot.day_action_done;
}

}  // namespace pixel_town

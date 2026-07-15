#include <doctest/doctest.h>

#include "app/story_lifecycle_runtime.hpp"

TEST_CASE("new game opening emits one completion after all dialogue lines") {
    auto session = pixel_town::GameSession::new_game(20260714U);
    const auto session_before_dialogue = session.snapshot();
    pixel_town::StoryLifecycleRuntime runtime;
    REQUIRE(runtime.open(pixel_town::StoryLifecycleContext::new_game_opening));

    auto view = runtime.presentation();
    REQUIRE(view.dialogue.has_value());
    CHECK(view.context == pixel_town::StoryLifecycleContext::new_game_opening);
    CHECK(view.dialogue->speaker == "镇长");
    CHECK(view.dialogue->current_line == 1);
    CHECK(view.dialogue->total_lines == 4);

    pixel_town::DialogueFrameInput input;
    input.advance_pressed = true;
    CHECK(runtime.step(input).status ==
          pixel_town::StoryLifecycleStepStatus::changed);
    CHECK(runtime.step(input).status ==
          pixel_town::StoryLifecycleStepStatus::changed);
    CHECK(runtime.step(input).status ==
          pixel_town::StoryLifecycleStepStatus::changed);

    const auto completed =
        pixel_town::step_story_lifecycle(session, runtime, input);
    CHECK(completed.status ==
          pixel_town::StoryLifecycleStepStatus::completed);
    CHECK(completed.context ==
          pixel_town::StoryLifecycleContext::new_game_opening);
    CHECK_FALSE(runtime.active());
    CHECK(session.snapshot() == session_before_dialogue);
    CHECK(runtime.step(input).status ==
          pixel_town::StoryLifecycleStepStatus::rejected);
}

TEST_CASE("home reflection applies the existing rest result exactly once") {
    auto session = pixel_town::GameSession::new_game(20260714U);
    REQUIRE(session.enter_location(pixel_town::Location::restaurant));
    const int result_id = session.start_location();
    REQUIRE(result_id != 0);
    pixel_town::ActionResult day_result;
    day_result.result_id = result_id;
    day_result.slot = pixel_town::ActionSlot::day;
    day_result.location = pixel_town::Location::restaurant;
    day_result.outcome = pixel_town::ActionOutcome::completed;
    day_result.summary = "完成白天行动。";
    REQUIRE(session.apply_action_result(day_result).accepted);
    REQUIRE(session.phase() == pixel_town::GamePhase::night_choice);

    auto expected = session;
    REQUIRE(expected.apply_action_result(expected.home_rest_result()).accepted);

    pixel_town::StoryLifecycleRuntime runtime;
    REQUIRE(runtime.open(pixel_town::StoryLifecycleContext::home_rest));
    const auto before_dialogue = session.snapshot();

    pixel_town::DialogueFrameInput input;
    input.advance_pressed = true;
    CHECK(pixel_town::step_story_lifecycle(session, runtime, input).status ==
          pixel_town::StoryLifecycleStepStatus::changed);
    CHECK(session.snapshot() == before_dialogue);

    const auto completed =
        pixel_town::step_story_lifecycle(session, runtime, input);
    CHECK(completed.status ==
          pixel_town::StoryLifecycleStepStatus::completed);
    CHECK(completed.action_applied);
    CHECK(session.snapshot() == expected.snapshot());

    const auto settled_snapshot = session.snapshot();
    CHECK(pixel_town::step_story_lifecycle(session, runtime, input).status ==
          pixel_town::StoryLifecycleStepStatus::rejected);
    CHECK(session.snapshot() == settled_snapshot);
}

TEST_CASE("restored first day boundary can replay a side effect free opening") {
    const auto original = pixel_town::GameSession::new_game(20260714U);
    auto restored = pixel_town::GameSession::from_snapshot(original.snapshot());
    const auto restored_snapshot = restored.snapshot();

    REQUIRE(pixel_town::should_replay_new_game_opening(restored));
    pixel_town::StoryLifecycleRuntime runtime;
    REQUIRE(runtime.open(pixel_town::StoryLifecycleContext::new_game_opening));

    pixel_town::DialogueFrameInput input;
    input.skip_pressed = true;
    const auto completed =
        pixel_town::step_story_lifecycle(restored, runtime, input);

    CHECK(completed.status ==
          pixel_town::StoryLifecycleStepStatus::completed);
    CHECK_FALSE(completed.action_applied);
    CHECK(restored.snapshot() == restored_snapshot);

    REQUIRE(restored.enter_location(pixel_town::Location::restaurant));
    CHECK_FALSE(pixel_town::should_replay_new_game_opening(restored));
}

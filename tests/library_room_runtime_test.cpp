#include <doctest/doctest.h>

#include "app/library_room_runtime.hpp"

TEST_CASE("library room loops a fixed administrator and opens dialogue on click") {
    pixel_town::LibraryRoomRuntime room;
    REQUIRE(room.open());
    CHECK(room.presentation().administrator_animation_seconds ==
          doctest::Approx(0.0F));

    pixel_town::LibraryRoomInput input;
    input.elapsed_seconds = 0.45F;
    (void)room.step(input);
    CHECK(room.presentation().administrator_animation_seconds ==
          doctest::Approx(0.45F));

    input = {};
    input.administrator_activated = true;
    CHECK(room.step(input).status ==
          pixel_town::LibraryRoomStepStatus::dialogue_opened);
    REQUIRE(room.presentation().dialogue.has_value());
    CHECK(room.presentation().dialogue->speaker == "管理员");

    const float before_dialogue_frame =
        room.presentation().administrator_animation_seconds;
    input = {};
    input.elapsed_seconds = 0.2F;
    input.administrator_activated = true;
    input.back_pressed = true;
    (void)room.step(input);
    CHECK(room.presentation().administrator_animation_seconds ==
          doctest::Approx(before_dialogue_frame + 0.2F));
    CHECK(room.presentation().dialogue.has_value());

    input = {};
    input.dialogue.advance_pressed = true;
    CHECK(room.step(input).status == pixel_town::LibraryRoomStepStatus::changed);
    CHECK(room.step(input).status == pixel_town::LibraryRoomStepStatus::changed);
    CHECK(room.step(input).status ==
          pixel_town::LibraryRoomStepStatus::work_requested);
    CHECK_FALSE(room.active());
}

TEST_CASE("library room can return without consuming work") {
    pixel_town::LibraryRoomRuntime room;
    REQUIRE(room.open());

    pixel_town::LibraryRoomInput input;
    input.back_pressed = true;
    const auto closed = room.step(input);

    CHECK(closed.status == pixel_town::LibraryRoomStepStatus::closed);
    CHECK_FALSE(room.active());
    CHECK(closed.notice.find("未消耗") != std::string::npos);
}

#include <doctest/doctest.h>

#include "core/interaction_runtime.hpp"

TEST_CASE("pause toggle freezes game updates and runtime clock") {
    pixel_town::InteractionRuntime runtime;

    const auto first = runtime.update(pixel_town::InteractionFrameInput{0.5, true, false, true, false});
    CHECK(first.game_updates_enabled == false);
    CHECK(runtime.paused());
    CHECK(runtime.active_elapsed_seconds() == doctest::Approx(0.0));

    const auto frozen = runtime.update(pixel_town::InteractionFrameInput{1.0, false, false, true, false});
    CHECK(frozen.game_updates_enabled == false);
    CHECK(runtime.active_elapsed_seconds() == doctest::Approx(0.0));

    const auto resumed = runtime.update(pixel_town::InteractionFrameInput{0.25, true, false, true, false});
    CHECK(resumed.game_updates_enabled);
    CHECK_FALSE(runtime.paused());
    CHECK(runtime.active_elapsed_seconds() == doctest::Approx(0.25));
}

TEST_CASE("focus loss and minimization freeze without catch-up") {
    pixel_town::InteractionRuntime runtime;

    CHECK(runtime.update(pixel_town::InteractionFrameInput{0.5, false, false, true, false})
              .game_updates_enabled);
    CHECK(runtime.active_elapsed_seconds() == doctest::Approx(0.5));

    CHECK_FALSE(runtime.update(pixel_town::InteractionFrameInput{2.0, false, false, false, false})
                    .game_updates_enabled);
    CHECK(runtime.active_elapsed_seconds() == doctest::Approx(0.5));

    CHECK_FALSE(runtime.update(pixel_town::InteractionFrameInput{3.0, false, false, true, true})
                    .game_updates_enabled);
    CHECK(runtime.active_elapsed_seconds() == doctest::Approx(0.5));

    CHECK(runtime.update(pixel_town::InteractionFrameInput{0.25, false, false, true, false})
              .game_updates_enabled);
    CHECK(runtime.active_elapsed_seconds() == doctest::Approx(0.75));
}

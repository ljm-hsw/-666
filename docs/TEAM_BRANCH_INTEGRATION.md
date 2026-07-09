# Team Branch Integration Notes

Last updated: 2026-07-09

This document records the current integration state of teammate feature branches on
`codex/integrate-team-branches`.

## Integration baseline

The aggregation branch is based on `722dcce feat: add p1 story shell`.

The following baseline behavior is intentionally preserved:

- title page starts before the map and keeps the new game / continue game split;
- 960×540 logical canvas and current imagegen title/map assets remain authoritative;
- `StoryText` remains the P1 narrative text entry point;
- existing `docs/story/` and `docs/UI_ASSET_STRATEGY.md` are retained;
- location results still flow through `GameSession::apply_action_result()`;
- v1 save files without tavern record fields remain readable, defaulting tavern wins/losses to `0`.

## Merged modules

| Branch | Module | Integration commit | Current state |
|---|---|---:|---|
| `origin/dev-yhr` | Restaurant | `2cd3da6` | Rules, basic UI, CMake target, and tests merged. |
| `origin/dev-ckz` | Tavern shell/rules | `80bd9d3` | Tavern rules, win/loss counters, save fields, night entry, and tests merged. |
| `origin/dev-xhy` | Library | `340b5b2` | Data, rules, NPC/scene/UI files, optional data manifest, map entry, and tests merged. |

## Intentional integration decisions

- `CMakePresets.json` from `dev-xhy` was not kept because its build/test presets defaulted to
  Windows MinGW and would mislead macOS users. Existing build docs remain the source of truth.
- Library data is registered as an optional resource in diagnostics so missing data does not block
  baseline app startup.
- The library engine uses `std::unique_ptr` in `GameAppState`; the branch's raw pointer was not kept.
- Library UI starts only after `GameSession::start_location()` succeeds, so the final library result
  uses the active result id and passes through the unified action-result gate.
- Restaurant result construction now uses explicit field assignment to remain stable after tavern
  fields were added to `ActionResult`.
- Follow-up architecture pass split location runtime responsibilities out of `game_flow.cpp`:
  `location_result_adapter` maps module-local results into core `ActionResult`, while
  `location_runtime` owns restaurant/tavern/library start, update, and completion flow.
- CMake now keeps `pixel_town_locations` as the headless location-rules target; raylib-backed
  library scene/NPC support is isolated in `pixel_town_location_scene`.
- `ui_primitives` centralizes 960×540 logical-canvas drawing helpers and hit testing so scene code
  does not duplicate scaling, text, panel, click, and hover rules.

## Current risks and follow-up

- `src/app/game_flow.cpp` still owns title, map, status, summary, ending, and high-level scene
  routing. The immediate location flow has been moved behind adapter/runtime seams; a later pass
  should split title/map/summary scenes only when new work makes that worthwhile.
- Restaurant and library UI are usable integration shells, not final visual style. P4 should replace
  or normalize final scene visuals.
- Some warnings remain from teammate code:
  - unused parameters/constants in library UI.
- Restaurant, library, and tavern summaries still include module-local text. They should be routed
  through `StoryText` or a location text adaptor before final content acceptance.
- Convenience store is still not integrated; P2 is not complete until Issue 10 is merged and tested.

## Architecture refactor on 2026-07-09

The first post-merge architecture pass applied the `improve-codebase-architecture` deepening rule:
move repeated ordering and mapping knowledge behind small Interfaces.

- `location_result_adapter` is the Seam for translating location-specific results into unified
  `ActionResult` values.
- `location_runtime` is the Seam for app-layer location runtime state and input ordering. It keeps
  restaurant, library, and tavern UI state out of the top-level `GameAppState` Interface.
- `ui_primitives` is the Seam for logical-canvas drawing and input hit testing.
- `game_flow.cpp` now stays closer to scene routing. Its line count dropped from about 1017 lines
  after the team merge to about 760 lines after the first refactor.

This pass did not change `GameSession`, save format, player-state semantics, or location rule
contracts.

## Validation on integration branch

After merging each module, the following checks were run locally on macOS:

```bash
.tools/CMake.app/Contents/bin/cmake --build build --parallel 4
.tools/CMake.app/Contents/bin/ctest --test-dir build --output-on-failure
```

The latest run after all three module merges passed.

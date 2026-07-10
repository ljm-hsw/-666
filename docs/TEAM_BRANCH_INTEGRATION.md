# Team Branch Integration Notes

Last updated: 2026-07-10

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
| `origin/dev-ckz` | Tavern shell/rules/minigames | `80bd9d3` + source `6ebaddd` | Shell contract retained; playable Gomoku, Liar's Dice, candidate lobby/NPC assets, tests, and diagnostic captures integrated through current adapters. |
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
  `location_result_adapter` now owns library result/context mapping, while `location_runtime`
  owns restaurant/library/store UI flow and the lifetime of tavern visual assets.
- CMake now keeps `pixel_town_locations` as the headless location-rules target; raylib-backed
  library scene/NPC support is isolated in `pixel_town_location_scene`.
- `ui_primitives` centralizes 960×540 logical-canvas drawing helpers and hit testing so scene code
  does not duplicate scaling, text, panel, click, and hover rules.
- The rewritten `dev-ckz` branch was not merged wholesale because it was based on an older `main`.
  Gomoku and Liar's Dice rules were moved into the headless locations target. Tavern input is now
  converted into explicit `TavernFrameInput`; the raylib-free `TavernRuntime` exposes only
  `open / step / presentation / active`, while `tavern_view` owns drawing and `TavernVisualAssets`.
- Tavern terminal outcomes pass through the raylib-free `TavernChallengeSettlement` module in
  `src/locations/` before the existing `GameSession::apply_action_result()` gate. Settlement accepts
  the real Gomoku or Liar's Dice state, rejects a non-terminal game, and derives the challenge and
  outcome from that state. Runtime caches the first terminal `ActionResult`; a rejected core result
  keeps that same candidate and the terminal board/dice state visible. Save, story, inventory, seed,
  action-result, and balance contracts remain unchanged.

## Current risks and follow-up

- `src/app/game_flow.cpp` still owns title, map, status, summary, ending, and high-level scene
  routing. The immediate location flow has been moved behind adapter/runtime seams; a later pass
  should split title/map/summary scenes only when new work makes that worthwhile.
- Restaurant, convenience store, and library UI are usable integration shells, not final visual
  style. P4 should replace or normalize final scene visuals.
- Restaurant and tavern summaries still include module-local text. They should be routed through
  `StoryText` or a location text adaptor before final content acceptance.
- Convenience store is integrated with P2 placeholder UI: rule tests, product-specific keyboard
  purchase/price decisions, inventory updates, and save/load round-trip are covered. Mouse-first
  convenience-store controls and final convenience-store visuals remain polish work.
- The current Cozy Tavern lobby is licensed only for non-commercial projects and remains a P3
  candidate. A commercial-capable final release must replace it with project-authored or generated
  art; the bartender candidate has separate Pixel Crawler terms recorded in `CREDITS.md`.

## Architecture refactor on 2026-07-09

The first post-merge architecture pass applied the `improve-codebase-architecture` deepening rule:
move repeated ordering and mapping knowledge behind small Interfaces.

- `location_result_adapter` is the Seam for translating library-specific results and daily context
  into the unified core flow.
- `location_runtime` is the Seam for app-layer daytime location state and input ordering; it also
  owns `TavernVisualAssets` without mixing those raylib resources into gameplay state.
- `TavernRuntime` is the raylib-free Seam for tavern ordering and hidden presentation state;
  `TavernChallengeSettlement` is the raylib-free Seam for proving a real game is terminal, deriving
  its result, and constructing the existing night `ActionResult`. Runtime keeps that candidate
  stable until the core accepts it.
- `ui_primitives` is the Seam for logical-canvas drawing and input hit testing.
- `game_flow.cpp` now stays closer to scene routing. Its line count dropped from about 1017 lines
  after the team merge to about 760 lines after the first refactor.

This pass did not change `GameSession`, save format, `ActionResult`, player-state semantics,
balance values, or location rule contracts.

## Validation on integration branch

After merging each module, the following checks were run locally on macOS:

```bash
.tools/CMake.app/Contents/bin/cmake --build build --parallel 4
.tools/CMake.app/Contents/bin/ctest --test-dir build --output-on-failure
```

The latest run after all three module merges passed.

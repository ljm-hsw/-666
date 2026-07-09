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

## Current risks and follow-up

- `src/app/game_flow.cpp` now owns title, map, restaurant, tavern, and library UI flow. It should be
  split into scene/adaptor modules before more teammate work is merged.
- Restaurant and library UI are usable integration shells, not final visual style. P4 should replace
  or normalize final scene visuals.
- Some warnings remain from teammate code:
  - ignored `[[nodiscard]]` results in restaurant UI/tests;
  - unused parameters/constants in library UI.
- Restaurant, library, and tavern summaries still include module-local text. They should be routed
  through `StoryText` or a location text adaptor before final content acceptance.
- Convenience store is still not integrated; P2 is not complete until Issue 10 is merged and tested.

## Validation on integration branch

After merging each module, the following checks were run locally on macOS:

```bash
.tools/CMake.app/Contents/bin/cmake --build build --parallel 4
.tools/CMake.app/Contents/bin/ctest --test-dir build --output-on-failure
```

The latest run after all three module merges passed.

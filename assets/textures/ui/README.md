# UI Texture Assets

This directory stores runtime UI texture candidates grouped by module.

## Directory Layout

- `tavern/`: tavern lobby, tavern NPC, challenge cards, and tavern-specific UI textures.
  - `tavern/npc/`: tavern NPC sprite sheets and character texture candidates.
  - `tavern/LICENSES/`: local copies of third-party tavern asset terms.
- `restaurant/restaurant_interior.png`: 960×540 restaurant runtime background.
- `restaurant/npc/chef_idle_sheet.png`: four-frame team-provided restaurant NPC candidate.
- `home/home_interior.png`: 960×540 home runtime background used by the rest confirmation scene.
- `library/library_interior.png`: 960×540 library runtime background used behind the existing library flow.
- `library/npc/librarian_idle_sheet.png`: four-frame team-provided librarian candidate.
- `convenience_store/convenience_store_interior.png`: 960×540 convenience-store runtime background.
- `convenience_store/npc/salesclerk_idle_sheet.png`: four-frame team-provided salesclerk candidate.
- `characters/protagonist_idle_sheet.png`: shared four-frame team-provided protagonist candidate used by dialogue portraits.
- `characters/mayor_idle_sheet.png`: shared four-frame team-provided mayor candidate used by opening dialogue portraits.
- `tavern/tavern_lobby.png`: 960×540 team-provided tavern runtime background.
- Sources and processing records for these five team-provided composites live under
  `assets/art_sources/manual_composites/`.
- Future imagegen scene kits place processed runtime pieces under `<module>/pieces/` and manual 960×540 composites under `<module>/composites/`. Raw imagegen sources belong under `assets/art_sources/imagegen/scene_kits/`, not in this runtime directory.

## Naming Rules

- Use lowercase ASCII names with underscores, such as `tavern_lobby_draft.png`.
- Put module-specific textures under their module folder instead of the root `ui/` folder.
- Keep draft assets clearly named with `_draft` or `_candidate` until the team approves them.
- Prefer PNG for pixel-art UI textures.
- For sprite sheets, include the animation role in the name, such as
  `bartender_idle_sheet.png`.

## Provenance

For each asset that may ship with the game, record its source, author, license, and usage in
`CREDITS.md` or the module-level design note before final merge.

Project-authored draft assets can be kept here during development, but final assets should still
have a short note explaining who made them and what tool/source was used.

The production and approval process for tavern, library, and convenience-store scene kits is
defined in `docs/SCENE_ASSET_PRODUCTION_PLAN.md`.

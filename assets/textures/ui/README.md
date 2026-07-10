# UI Texture Assets

This directory stores runtime UI texture candidates grouped by module.

## Directory Layout

- `tavern/`: tavern lobby, tavern NPC, challenge cards, and tavern-specific UI textures.
  - `tavern/npc/`: tavern NPC sprite sheets and character texture candidates.
  - `tavern/LICENSES/`: local copies of third-party tavern asset terms.

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

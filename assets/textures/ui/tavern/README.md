# Tavern UI Textures

This directory stores tavern-specific UI texture candidates.

## Runtime Candidates

- `tavern_lobby.png`: current tavern lobby background candidate, assembled in Tiled by
  the tavern module owner from MangoSeven's Cozy Tavern Interior Pack (free version).
- `npc/bartender_idle_sheet.png`: tavern bartender NPC animation candidate.
  - Source file in the original pack: `Entities/Npc's/Citizen_F/Tavern_A/Walk/Walk_Down-Sheet.png`
  - Size: 384x64
  - Intended runtime interpretation: 6 frames, 64x64 each
  - Current use: idle/down-facing tavern NPC candidate

## License Notes

- `tavern_lobby.png` uses the Cozy Tavern Interior Pack free version by MangoSeven.
  Its bundled terms allow use and editing only in non-commercial projects. The original
  readme is stored at `LICENSES/cozy_tavern_free_readme.txt`.
- `npc/bartender_idle_sheet.png` comes from Pixel Crawler - Free Pack by Anokolisa.
- The copied terms file is stored at `LICENSES/pixel_crawler_free_pack_terms.txt`.
- Do not add more third-party tavern assets without recording source, author, license,
  usage, and any edits in `CREDITS.md`.

## Implementation Notes

- Use point/nearest texture filtering for pixel art.
- Keep UI layout in design coordinates and scale through the existing UI helpers.
- If a texture fails to load, the tavern UI should fall back to code-drawn placeholders.

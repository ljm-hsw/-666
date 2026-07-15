# Tavern UI Textures

This directory stores tavern-specific UI texture candidates.

## Runtime Candidates

- `tavern_lobby.png`: current 960×540 team-provided tavern lobby candidate. Its source,
  processing hash, and unresolved component-license boundary are recorded under
  `assets/art_sources/manual_composites/` and in `CREDITS.md`.
- `npc/bartender_idle_sheet.png`: tavern bartender NPC animation candidate.
  - Source: team-provided transparent PNG; author/tool/license details remain pending.
  - Size: 256x96
  - Intended runtime interpretation: 4 frames, 64x96 each
  - Current use: idle/down-facing tavern NPC candidate

## License Notes

- The previous `tavern_lobby.png` used the Cozy Tavern Interior Pack free version by
  MangoSeven. That exact PNG is preserved under `assets/art_sources/legacy/tavern/`;
  its bundled terms remain in this directory for provenance.
- The previous runtime bartender came from Pixel Crawler - Free Pack by Anokolisa.
- Its copied terms remain at `LICENSES/pixel_crawler_free_pack_terms.txt` only as historical
  provenance; they do not apply to the current team-provided bartender PNG.
- Do not add more third-party tavern assets without recording source, author, license,
  usage, and any edits in `CREDITS.md`.

## Implementation Notes

- Use point/nearest texture filtering for pixel art.
- Keep UI layout in design coordinates and scale through the existing UI helpers.
- If a texture fails to load, the tavern UI should fall back to code-drawn placeholders.

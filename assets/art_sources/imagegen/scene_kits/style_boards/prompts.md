# Imagegen 风格板提示词

## 共用参考图

- `assets/textures/imagegen_backgrounds/title_screen_background.png`
- `assets/textures/imagegen_backgrounds/town_map_full_scene.png`
- C 方案额外参考 `assets/textures/ui/tavern/tavern_lobby.png` 的空间功能分区；不继承其较暗、较粗糙的画面完成度。

三张风格板均为非运行时候选，不要求直接输出 960×540；只要求 16:9 构图和一致的像素视觉语言。

## A：暖色叙事绘本

```text
Use case: stylized-concept.
Create a single 16:9 cross-location pixel-art style board for the game Pixel Town: Ten-Day Business Plan. Show exactly three separate, unlabeled interior vignettes in one image: a cozy small-town tavern, a quiet library, and a friendly convenience store. They must clearly belong to the same town and share one coherent visual system.

Visual direction A: warm storybook town interiors, closely aligned with the supplied title-screen and town-map references. Use cheerful natural colors, warm cream highlights, green accents, medium walnut wood, gentle red and blue accents, crisp dark outlines, chunky readable silhouettes, and moderate handcrafted detail. Keep the mood welcoming and lived-in, but not cluttered.

Camera and composition: fixed-camera light top-down orthographic room view, not an isometric diamond and not a side view. Use the same camera angle, pixel density, outline thickness, furniture scale, and upper-left light direction for all three vignettes. Each room needs clear open floor areas for later character movement and UI overlays.

Location cues: tavern with bar counter, a few wooden tables and warm amber lamps; library with bookcases, circulation desk and reading tables in soft daylight; convenience store with checkout counter, low shelves, cooler and abstract colorful goods in bright neutral warmth.

Pixel-art constraints: crisp hard pixel edges, limited palette, no smooth gradients, no painterly blur, no soft antialiasing, no photorealistic texture, no mixed perspective, no excessive tiny props.

Content constraints: no characters, no UI, no dialogue boxes, no text, no letters, no numbers, no labels, no logos, no brands, no watermark, and no imitation of a recognizable commercial game's assets.
```

## B：清晰模块化

```text
Use case: stylized-concept.
Create a single 16:9 cross-location pixel-art style board for the game Pixel Town: Ten-Day Business Plan. Show exactly three separate, unlabeled interior vignettes in one image: a cozy tavern, a quiet library, and a bright convenience store. All three rooms must look as if they were built from one reusable modular asset kit.

Visual direction B: clean modular readability. Match the supplied title-screen and town-map references in palette and charm, but simplify shapes and reduce decorative noise. Use larger furniture masses, clearer walkable floor zones, stronger value separation between floor, walls and furniture, consistent one-pixel-like dark outlines at the chosen pixel scale, and restrained highlights. This direction must be especially suitable for manual cutting, recomposition, collision rectangles, interaction markers and Chinese UI overlays.

Camera and composition: fixed-camera light top-down orthographic room view, never an isometric diamond. Use identical camera angle, pixel density, outline thickness, object scale, and upper-left lighting across all three vignettes. Give every major furniture item a clean silhouette and enough empty space around it. Avoid overlapping furniture clusters.

Location cues: tavern with a simple straight bar, two game tables and a wide central aisle; library with modular bookcases, checkout desk, reading table and wide aisle; convenience store with modular checkout, two low gondola shelves, wall cooler and open queue space. Goods use abstract color blocks only.

Pixel-art constraints: crisp hard pixel edges, limited warm palette, no smooth gradients, no painterly blur, no soft antialiasing, no photorealism, no strong vanishing-point perspective, no high-frequency floor texture, no tiny unreadable clutter.

Content constraints: no characters, no UI, no dialogue boxes, no text, no letters, no numbers, no labels, no logos, no brands, no watermark, and no imitation of a recognizable commercial game's assets.
```

## C：温暖夜色对比

```text
Use case: stylized-concept.
Create a single 16:9 cross-location pixel-art style board for the game Pixel Town: Ten-Day Business Plan. Show exactly three separate, unlabeled interior vignettes in one image: a tavern, a library, and a convenience store. They must share the same town architecture, camera, pixel scale, outlines and material language while allowing stronger location-specific atmosphere.

Visual direction C: cozy evening contrast. Preserve the approachable color language and crisp pixel craft of the supplied title-screen and town-map references. Use deeper shadows and richer warm pools of light than direction A, but keep all room shapes and walkable paths clearly readable. The tavern may be the darkest with amber and muted burgundy; the library uses warm late-afternoon daylight with calm green accents; the convenience store uses soft cream practical lighting with restrained red and blue product accents. Avoid neon cyberpunk color.

Camera and composition: fixed-camera light top-down orthographic room view, not isometric and not side-on. Use the same camera angle, pixel density, outline thickness, furniture scale and upper-left light direction in all three vignettes. Keep a clear central circulation path in every room and leave calm background zones for future dialogue UI.

Location cues: tavern with bar counter, bottle cabinet shapes, two tabletop-game areas and a door path; library with tall and low bookcases, circulation desk, reading area and old-map display; convenience store with checkout, wall shelving, low center shelves, cooler and umbrella stand. All packaging and signs must remain abstract and unreadable.

Pixel-art constraints: crisp hard pixel edges, limited palette, strong readable silhouettes, controlled contrast, no smooth gradients, no painterly blur, no soft antialiasing, no photorealistic texture, no mixed perspective, no excessive micro-detail.

Content constraints: no characters, no UI, no dialogue boxes, no text, no letters, no numbers, no labels, no logos, no brands, no watermark, and no imitation of a recognizable commercial game's assets.
```

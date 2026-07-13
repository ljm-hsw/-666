# C 方向首轮分件生成契约

## 共用要求

每张图都必须把 `style_boards/style_board_c_source.png` 作为风格参考，并满足以下约束：

```text
Use case: stylized-concept.
Create one modular opaque pixel-art asset sheet for manual game-scene compositing, matching the supplied approved Style Board C exactly in pixel density, warm evening palette, dark outline language, material rendering, light top-down orthographic camera, object scale, and upper-left lighting.

Layout: arrange all requested pieces as separate, non-overlapping objects in a clean grid with generous padding. Every piece must be fully visible and easy to cut out. Do not build a complete room scene.

Background: perfectly flat solid #ff00ff chroma-key background, with no gradient, texture, floor plane, border, lighting variation, shadow or reflection. Never use #ff00ff inside an asset.

Pixel constraints: crisp hard pixel edges, limited palette, no smooth gradients, no painterly blur, no soft antialiasing, no photorealism, no mixed camera angles, no isometric diamond perspective, no tiny noisy decoration.

Content constraints: no characters, no UI, no dialogue boxes, no readable text, no letters, no numbers, no labels, no logos, no brands, no watermark. Any books, bottles, packaging or signs use abstract unreadable pixel blocks only.
```

每次调用只生成下列九个 section 中的一张图，不把不同地点或不同 section 合并到同一张源图。

## 酒馆建筑结构

```text
Asset type: modular tavern architecture sheet.
Generate: straight interior wall segments in three useful lengths; inner and outer wall corners; wooden wainscot segment; dark timber beam; warm plank-floor square swatches in two variants; stone hearth-floor patch; plain wooden entrance door in closed and open states; arched night window; rectangular curtained window; simple doorway frame; baseboard and threshold pieces.
Keep every structural piece tileable or clean-ended where appropriate. The arched window and wood treatment should match the tavern vignette in approved Style Board C.
```

## 酒馆大型家具

```text
Asset type: modular tavern large-furniture sheet.
Generate: straight bar counter; short bar-counter end piece; back-bar bottle cabinet with abstract bottles; round tavern table; square tabletop-game table; long table; matching wooden chairs facing four useful directions; bar stools; fireplace; large barrel; stacked small barrels; compact storage cabinet.
Keep furniture silhouettes clear and walkable collision footprints easy to infer. No cast shadows and no floor baked beneath the objects.
```

## 酒馆小型道具

```text
Asset type: modular tavern small-prop sheet.
Generate: tankard; drinking glass; bottle cluster; candle lantern; wall lantern; dice cup; several dice without readable values; abstract board-game box; loose game tokens; small potted plant; folded cloth; blank framed picture; plate with simple food; small flower vase; firewood bundle.
Make props large enough to remain readable in a 960x540 pixel-art game and keep all items separate.
```

## 图书馆建筑结构

```text
Asset type: modular library architecture sheet.
Generate: straight cream plaster wall segments in three useful lengths; walnut lower-wall panel segments; inner and outer wall corners; calm stone-tile floor square swatches in two variants; warm wood-floor square swatch; plain entrance door in closed and open states; tall arched daylight window with green curtains; rectangular daylight window; doorway frame; baseboard and threshold pieces.
Keep pieces tileable or clean-ended. Match the bright late-afternoon library lighting and materials from approved Style Board C without baking the sunlight onto the magenta background.
```

## 图书馆大型家具

```text
Asset type: modular library large-furniture sheet.
Generate: tall bookcase; low bookcase; narrow bookcase; inner-corner bookcase; circulation desk; rectangular reading table; compact side table; matching wooden chairs facing four useful directions; rolling book cart; card-catalog cabinet; old-map display table; upholstered reading armchair; floor lamp base without glow.
Use abstract colored book spines with no readable text. Keep furniture separate, with clear silhouettes and collision footprints.
```

## 图书馆小型道具

```text
Asset type: modular library small-prop sheet.
Generate: closed book stacks in several colors; open book; loose paper stack; borrowing-card box; ink stamp; desk lamp without glow; bookends; small potted plants; rolled old map; globe; blank category plaque; reading glasses; bookmark bundle; pencil cup.
Use no readable text or symbols. Make each prop readable at the target game's scale and leave generous separation.
```

## 便利店建筑结构

```text
Asset type: modular convenience-store architecture sheet.
Generate: straight warm cream wall segments in three useful lengths; simple wood trim; inner and outer wall corners; clean beige square-tile floor swatches in two variants; warm stone-floor swatch; glass entrance door in closed and open states; large rectangular window; small curtained side window; doorway frame; baseboard and threshold pieces; restrained red-and-cream fabric awning piece.
Keep the shop friendly and small-town rather than modern neon. Make structural pieces tileable or clean-ended and match approved Style Board C.
```

## 便利店大型家具

```text
Asset type: modular convenience-store large-furniture sheet.
Generate: checkout counter; short counter end piece; cash register with blank display; low double-sided gondola shelf; low single-sided shelf; wall shelving cabinet; glass-door drink cooler with abstract bottles; compact freezer chest; umbrella stand; shopping-basket rack; produce crate stand; stack of cardboard boxes.
Use abstract packaging blocks only. Keep all objects separate, with clean silhouettes and easy-to-read collision footprints.
```

## 便利店小型道具

```text
Asset type: modular convenience-store small-prop sheet.
Generate: umbrella variants; shopping basket; paper bag; small cardboard box; abstract snack bag variants; abstract drink cans and bottles; takeaway coffee cup; lunch box; receipt roll; blank price-tag plaque; mop bucket; small cleaning sign with no symbol or text; potted plant; fruit-and-vegetable crate inserts.
No brands, readable labels or generated lettering. Keep pieces readable at game scale and fully separated.
```

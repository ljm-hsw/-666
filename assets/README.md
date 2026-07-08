# 运行时资源

P0 基线包含资源诊断与视觉原型所需的最小资源：

- `fonts/fusion-pixel-12px-proportional-zh_hans.ttf`：Issue 03 待审简体中文像素字体候选，当前用于 P0 视觉原型和 P1 game-flow UI，许可证见同目录 `OFL.txt` 和 `LICENSES/`。
- `textures/town_marker.png`：代码生成的 8×8 小镇标记，用于证明贴图验证和加载路径。
- `textures/kenney_tiny_town/`：Kenney Tiny Town 1.1，CC0，16×16 tile 视觉原型参考素材。当前保留 `Tilemap/`、`Tiles/`、`License.txt` 和 `Tilesheet.txt`，原型优先使用 `Tilemap/tilemap_packed.png`。
- `textures/kenney_tiny_farm/`：Kenney Tiny Farm 1.0，CC0，16×16 tile 视觉原型参考素材。当前保留 `Tilemap/`、`Tiles/`、`License.txt` 和 `Tilesheet.txt`，作为 Tiny Town 缺失时的备用参考；两者都缺失时回退到代码绘制形状。
- `textures/imagegen_buildings/p1_building_candidates.png`：使用内置 imagegen 生成的 P1 地图建筑候选原图，仅用于保留生成来源。
- `textures/imagegen_buildings/p1_building_sprites.png`：由候选原图本地处理得到的 5-cell 透明建筑 sprite sheet，当前用于 P1 地图房屋 UI 替换尝试。该资源未获得最终美术批准。
- `textures/imagegen_backgrounds/title_screen_background.png`：使用内置 imagegen 生成并处理得到的 P1 标题/进入页背景，当前已接入运行时。该资源未获得最终美术批准。
- `textures/imagegen_backgrounds/town_map_full_scene.png`：使用内置 imagegen 生成并处理得到的 P1 地图页完整场景背景，当前已接入运行时。该资源未获得最终美术批准。
- `data/baseline.txt`：启动数据标记，用于证明数据资源验证路径。
- `audio/theme.ogg`：可选路径，P0 默认不提供；缺失时记录诊断并静音继续。

Fusion Pixel Font、Kenney Tiny Town、Kenney Tiny Farm、imagegen 建筑候选、当前贴图、调色板和尺寸规范仍需 P0 Issue 03 人工批准，不把这里的候选资源视为最终美术。第三方资源来源和摘要见根目录 `CREDITS.md`。

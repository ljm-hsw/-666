# 第三方资源署名

## 团队提供的餐馆、家、图书馆、酒馆与便利店场景候选

- 接入日期：2026-07-13
- 来源：团队成员提供的人工合成 PNG；仓库内源图位于 [`assets/art_sources/manual_composites/`](assets/art_sources/manual_composites/)
- 运行时路径：[`assets/textures/ui/restaurant/restaurant_interior.png`](assets/textures/ui/restaurant/restaurant_interior.png)、[`assets/textures/ui/home/home_interior.png`](assets/textures/ui/home/home_interior.png)、[`assets/textures/ui/library/library_interior.png`](assets/textures/ui/library/library_interior.png)、[`assets/textures/ui/tavern/tavern_lobby.png`](assets/textures/ui/tavern/tavern_lobby.png)、[`assets/textures/ui/convenience_store/convenience_store_interior.png`](assets/textures/ui/convenience_store/convenience_store_interior.png)
- 处理：垂直居中裁切；480×272 源图使用最近邻放大到 960×540；图书馆裁为 960×540
- 完整尺寸、SHA-256 与转换记录：[`assets/art_sources/manual_composites/README.md`](assets/art_sources/manual_composites/README.md)
- 许可状态：组成素材的作者、来源与许可证尚未随图片提供；当前仅作为内部开发和教学演示候选，不作商业可用结论

进入最终发布包前，必须补齐五张合成稿所用素材的来源与许可，或替换为许可证明确的项目自制/imagegen 资源。本节记录的是接入事实，不是第三方授权声明。

## Fusion Pixel Font

- 名称：Fusion Pixel Font 12px Proportional zh_hans
- 作者：TakWolf 及项目贡献者
- 版本：2026.07.01
- 来源：`https://github.com/TakWolf/fusion-pixel-font/releases/tag/2026.07.01`
- 许可证：SIL Open Font License 1.1
- 上游归档：`fusion-pixel-font-12px-proportional-ttf-v2026.07.01.zip`
- 上游归档 SHA-256：`917888b28d3cf854d6b29b9c1e38d3c88ee9a0d0339a790024adfeb77bcc8e9e`
- 仓库字体 SHA-256：`5b27e9eb9d9dd93cff727d8919ddd2e7a482b19314b62991cb1e7806852e8734`
- 许可证文件：[`assets/fonts/OFL.txt`](assets/fonts/OFL.txt) 及 [`assets/fonts/LICENSES/`](assets/fonts/LICENSES/)

该字体融合多个 OFL 字形来源，因此随仓库保留上游提供的完整许可证集合。它当前用于 P0 视觉原型和 P1 game-flow 中文 UI；运行时会从集中 UI 文案生成必需字形清单并逐字检查，用于阻止缺字或替代字形进入评审。最终 UI 字体是否沿用仍需 Issue 03 人工批准。

## Kenney Tiny Farm

- 名称：Tiny Farm
- 作者：Kenney
- 版本：1.0
- 来源：`https://www.kenney.nl/assets/tiny-farm`
- 下载包：`https://www.kenney.nl/media/pages/assets/tiny-farm/dfded1ae3e-1782913588/kenney_tiny-farm.zip`
- 许可证：Creative Commons Zero，CC0 1.0
- 上游页面标注：16×16 tile，130 个素材文件，CC0；包内 `Tilesheet.txt` 标注 tilesheet 为 12×11，共 132 个 tile。
- 上游 ZIP SHA-256：`a06f75f312c27eff15a2288475612e6f6699411be7259d408323cd15a790decc`
- 仓库路径：[`assets/textures/kenney_tiny_farm/`](assets/textures/kenney_tiny_farm/)
- 许可证文件 SHA-256：`9c4b5fc75e585a3495f6aa55b62172504780a51b7b4394066e1dc1a45403c548`
- `Tilemap/tilemap_packed.png` SHA-256：`0c4b3b4058cacf6a9bf021666838156d46da9ce27500d4c13e30cd5a92b722f4`
- `Tilemap/tilemap.png` SHA-256：`af7608d355329f0ca3b61981eca963ffdf3509a00983f7c424b28f2ee4fb0f11`

当前仓库保留 `Tilemap/`、`Tiles/`、`License.txt` 和 `Tilesheet.txt`；未保留上游 `.url` 快捷方式、预览图和下载 ZIP。Tiny Farm 作为 P0 视觉原型参考素材，用于验证 16×16 tile 规格、小镇地图质感和 CC0 第三方素材接入流程。正式美术仍需 Issue 03 人工批准后锁定。

## Kenney Tiny Town

- 名称：Tiny Town
- 作者：Kenney
- 版本：1.1
- 来源：`https://www.kenney.nl/assets/tiny-town`
- 下载包：`https://www.kenney.nl/media/pages/assets/tiny-town/a415fbeb49-1735736916/kenney_tiny-town.zip`
- 许可证：Creative Commons Zero，CC0 1.0
- 上游页面标注：16×16 tile，130 个素材文件，CC0；包内 `Tilesheet.txt` 标注 tilesheet 为 12×11，共 132 个 tile。
- 上游 ZIP SHA-256：`9768692dccff1d706408a5aedd6ca4f6cd1409506cbc84cb2f862919764be977`
- 仓库路径：[`assets/textures/kenney_tiny_town/`](assets/textures/kenney_tiny_town/)
- 许可证文件 SHA-256：`c483c167cc3bdfe17c15f957ff847d409f64d81b410b28783636cf3b2b7814c5`
- `Tilemap/tilemap_packed.png` SHA-256：`3a54d99ecde790d4fdea207a3644cf130fc56fa838f1beb1507c185a95b8e902`
- `Tilemap/tilemap.png` SHA-256：`b9e9fb80b95d0b90b52f12d914f000cf16f8b22c1faa815aa94500abfa57bc50`

当前仓库保留 `Tilemap/`、`Tiles/`、`License.txt` 和 `Tilesheet.txt`；未保留上游 `.url` 快捷方式、预览图和下载 ZIP。Tiny Town 与当前小镇地图语义更匹配，P0 原型优先使用它绘制草地、道路、房体纹理、门窗和地点图标；最终是否作为正式小镇美术仍需 Issue 03 人工批准。

## Imagegen P1 Town Map Background

- 名称：P1 town map background
- 生成方式：内置 imagegen，按项目 960×540 地图页主背景需求生成
- 生成日期：2026-07-07
- 候选原图：[`assets/textures/imagegen_backgrounds/town_map_background_source.png`](assets/textures/imagegen_backgrounds/town_map_background_source.png)
- 候选原图 SHA-256：`935bffd470845cd17a397191385a6633033009d453c6f730f906e1355706781d`
- 处理后运行时背景：[`assets/textures/imagegen_backgrounds/town_map_background.png`](assets/textures/imagegen_backgrounds/town_map_background.png)
- 处理后运行时背景 SHA-256：`386c7087135e21ba2cf7ad03f322567d8359965383c32e907977a7a96ebdf770`
- 生成源已归档为仓库内候选原图；交付记录以仓库路径和 SHA-256 为准。

该资源是 P1 地图页的分层背景候选。当前运行时代码优先加载完整 imagegen 场景图；完整场景图缺失时才回退到该背景与独立建筑 sprite 的组合方案。它仍属于候选美术资产，是否进入最终交付需后续美术验收确认。

## Imagegen P1 Town Map Full Scene

- 名称：P1 town map full scene
- 生成方式：内置 imagegen，按“背景与五个地点房屋一起生成”的可直接使用地图方案生成
- 生成日期：2026-07-07
- 候选原图：[`assets/textures/imagegen_backgrounds/town_map_full_scene_source.png`](assets/textures/imagegen_backgrounds/town_map_full_scene_source.png)
- 候选原图 SHA-256：`112b546f9d85d581ecf6482959c964f6bf79bc7b95943d807d5aaecd442ccf15`
- 处理后运行时背景：[`assets/textures/imagegen_backgrounds/town_map_full_scene.png`](assets/textures/imagegen_backgrounds/town_map_full_scene.png)
- 处理后运行时背景 SHA-256：`fc78b733e0847c71cb11dad53c247d39dd789ad4aaa4efd05c42b6ded3b5ceef`
- 生成源已归档为仓库内候选原图；交付记录以仓库路径和 SHA-256 为准。

该资源是当前 P1 地图页的优先视觉方案。五个地点建筑已经和地图背景一起生成，运行时代码只叠加交互标签与点击热区，以避免独立房屋 sprite 和地块错位。该图仍属于候选美术资产，是否进入最终交付需后续美术验收确认。

## Imagegen P1 Title Screen Background

- 名称：P1 title screen background
- 生成方式：内置 imagegen，按项目 960×540 进入/标题页背景需求生成
- 生成日期：2026-07-08
- 候选原图：[`assets/textures/imagegen_backgrounds/title_screen_background_source.png`](assets/textures/imagegen_backgrounds/title_screen_background_source.png)
- 候选原图 SHA-256：`6c36104f59decdf4d4faa39fd3705d2d79f9ae94b19bc2d6681af74a09afb7f6`
- 处理后运行时背景：[`assets/textures/imagegen_backgrounds/title_screen_background.png`](assets/textures/imagegen_backgrounds/title_screen_background.png)
- 处理后运行时背景 SHA-256：`f5d3df2f5700511c6747c7591a9b86273744ce73cd0ff84d445a2c009922c42d`
- 生成源已归档为仓库内候选原图；交付记录以仓库路径和 SHA-256 为准。

该资源是当前 P1 标题/进入页的优先视觉方案。画面采用小镇入口广场构图，中央留出标题、开始按钮和提示文本空间，运行时代码叠加中文标题、开始按钮和提示。该图仍属于候选美术资产，是否进入最终交付需 Issue 03 或后续美术验收确认。

## Imagegen P1 Building Candidates

- 名称：P1 building candidates
- 生成方式：内置 imagegen，按项目当前 P1 地图房屋 UI 替换需求生成
- 生成日期：2026-07-07
- 候选原图：[`assets/textures/imagegen_buildings/p1_building_candidates.png`](assets/textures/imagegen_buildings/p1_building_candidates.png)
- 候选原图 SHA-256：`98d97717d4c2f1e0a1a2a72c3094e8ebfa07b188afb785d0174cd83ef5910a1b`
- 处理后运行时 sprite sheet：[`assets/textures/imagegen_buildings/p1_building_sprites.png`](assets/textures/imagegen_buildings/p1_building_sprites.png)
- 处理后 sprite sheet SHA-256：`836388b286b25b946d93b587765ede67adf80f3b9de7eb8a98219ce014ae3e37`
- 生成源已归档为仓库内候选原图；交付记录以仓库路径和 SHA-256 为准。

该资源是项目内 P1 地图 UI 的候选生成资产，不是第三方素材，也不是最终美术批准结果。当前代码把处理后的 sprite sheet 作为地图页主体建筑资源优先显示；缺失时仍回退到 Kenney/代码绘制方案。是否进入最终交付仍需 Issue 03 或后续美术验收确认。

## Pixel Crawler Free Pack Tavern NPC Candidate

- 名称：Pixel Crawler - Free Pack / Citizen_F Tavern_A Walk Down Sheet
- 作者：Anokolisa
- 版本：Pixel Crawler - Free Pack 2.1（上游页面于 2026-07-07 更新）
- 来源：`https://anokolisa.itch.io/free-pixel-art-asset-pack-topdown-tileset-rpg-16x16-sprites`
- 原始包内路径：`Entities/Npc's/Citizen_F/Tavern_A/Walk/Walk_Down-Sheet.png`
- 许可证/条款：允许用于商业、学习或其他功能性项目；允许修改；不得将素材本身作为最终产品单独销售
- 条款文件：[`assets/textures/ui/tavern/LICENSES/pixel_crawler_free_pack_terms.txt`](assets/textures/ui/tavern/LICENSES/pixel_crawler_free_pack_terms.txt)
- 条款文件 SHA-256：`6eea461769c5c89126b3b566cd9e11dea9e56a386b6f9cf5e134939c6d53b4d1`
- 仓库路径：[`assets/textures/ui/tavern/npc/bartender_idle_sheet.png`](assets/textures/ui/tavern/npc/bartender_idle_sheet.png)
- 仓库文件 SHA-256：`d54f1d1c859a0481efc21e8e3029e097238c160808bc409524fc578be97f7dba`
- 当前用途：酒馆老板 NPC 的 6 帧、每帧 64×64 向下动画候选

该资源仍需 P4 视觉验收。若替换或重新绘制，应同步更新本节和文件哈希。

## Cozy Tavern Interior Pack Free Tavern Lobby Candidate

- 名称：Cozy Tavern Interior Pack（Free Version）
- 作者：MangoSeven
- 来源：`https://mangoseven.itch.io/cozy-tavern-interior-pack`
- 场景制作：酒馆模块负责人使用 Tiled 组合并导出为 480×272 PNG
- 许可证/条款：仅允许在非商业项目中使用和修改
- 条款文件：[`assets/textures/ui/tavern/LICENSES/cozy_tavern_free_readme.txt`](assets/textures/ui/tavern/LICENSES/cozy_tavern_free_readme.txt)
- 条款文件 SHA-256：`9a94205492bd7f8dd66f1c83c5a358b0529cf5f14c8807d808f1ade223866873`
- 历史归档路径：[`assets/art_sources/legacy/tavern/tavern_lobby_cozy_tavern_source.png`](assets/art_sources/legacy/tavern/tavern_lobby_cozy_tavern_source.png)
- 仓库文件 SHA-256：`2669847c3f89ee4a9d2c6393c634563c25ac2d3c889c97f1947cce11534a8227`
- 当前用途：已退出运行时，仅作为旧酒馆背景与许可证据归档；运行时已替换为团队提供的新合成稿

该背景适用于当前非商业教学项目，但不具备商业项目使用许可，现已退出运行时并仅作历史归档。新酒馆合成稿的组成素材许可仍需按本文件首节补齐。

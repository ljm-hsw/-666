# 第三方资源署名

## 团队提供的场景音乐与反馈音效候选

- 接入日期：2026-07-15
- 来源：团队提供的 11 个 MP3 文件；仓库不记录聊天工具或个人目录路径
- 运行时路径、场景映射、时长与 SHA-256：[`assets/audio/README.md`](assets/audio/README.md)
- 处理：仅把空格、中文和场景名称统一为小写 `snake_case`；未转码、剪切或调整增益
- 当前用途：标题/地图、雨天、餐馆、便利店、图书馆、酒馆和家使用各自循环 BGM；地点切换、回家、成功和失败使用关键音效
- 未采用候选：团队原始目录中的 `sfx_loaction_switch.mp3` 与已采用文件语义重合且名称错拼，未进入仓库，等待人工 A/B 试听
- 许可状态：作者、创作/生成工具、原始来源和可再分发/商用许可尚未随文件提供

本批音频已经完成资源诊断和运行时接线，但仍只能作为内部开发与教学演示候选。Issue 17 在
人工试听、循环接缝/响度验收和授权证据补齐前不得标记最终音频交付完成。

## 团队提供的餐馆、家、图书馆、酒馆与便利店场景候选

- 接入日期：2026-07-13
- 来源：团队成员提供的人工合成 PNG；仓库内源图位于 [`assets/art_sources/manual_composites/`](assets/art_sources/manual_composites/)
- 运行时路径：[`assets/textures/ui/restaurant/restaurant_interior.png`](assets/textures/ui/restaurant/restaurant_interior.png)、[`assets/textures/ui/home/home_interior.png`](assets/textures/ui/home/home_interior.png)、[`assets/textures/ui/library/library_interior.png`](assets/textures/ui/library/library_interior.png)、[`assets/textures/ui/tavern/tavern_lobby.png`](assets/textures/ui/tavern/tavern_lobby.png)、[`assets/textures/ui/convenience_store/convenience_store_interior.png`](assets/textures/ui/convenience_store/convenience_store_interior.png)
- 处理：垂直居中裁切；480×272 源图使用最近邻放大到 960×540；图书馆裁为 960×540
- 完整尺寸、SHA-256 与转换记录：[`assets/art_sources/manual_composites/README.md`](assets/art_sources/manual_composites/README.md)
- 许可状态：组成素材的作者、来源与许可证尚未随图片提供；当前仅作为内部开发和教学演示候选，不作商业可用结论

进入最终发布包前，必须补齐五张合成稿所用素材的来源与许可，或替换为许可证明确的项目自制/imagegen 资源。本节记录的是接入事实，不是第三方授权声明。

## Imagegen 图书馆整理书本候选

- 生成方式：内置 imagegen，使用仓库已批准的 C 风格板生成图书馆小型道具 sheet，再经仓库脚本去背和独立分件裁切
- 生成日期：2026-07-11；运行时接入日期：2026-07-14
- 源分件：`assets/art_sources/imagegen/scene_kits/library/pieces/props/library_props_piece_001_candidate_v01.png` 至 `library_props_piece_006_candidate_v01.png`
- 运行时路径：`assets/textures/ui/library/organizing_books/`
- 分类映射和逐文件 SHA-256：[`assets/textures/ui/library/organizing_books/README.md`](assets/textures/ui/library/organizing_books/README.md)
- 生成、透明处理与裁切记录：[`assets/art_sources/imagegen/scene_kits/component_batch_c_manifest.md`](assets/art_sources/imagegen/scene_kits/component_batch_c_manifest.md)

这六张书堆用于图书馆整理模式的散落书、错架书和手持槽显示，不包含第三方素材声明。它们仍是项目自制候选美术，最终交付前需完成人工视觉验收。

## Imagegen 图书馆旧集市地图候选

- 生成方式：内置 imagegen，按图书馆旧地图剧情页面的暖色像素风要求生成；画面不包含生成文字或 UI
- 生成与接入日期：2026-07-16
- 候选源图：[`assets/art_sources/imagegen/library/old_town_map_source.png`](assets/art_sources/imagegen/library/old_town_map_source.png)
- 运行时路径：[`assets/textures/ui/library/old_town_map.png`](assets/textures/ui/library/old_town_map.png)
- 尺寸、处理和 SHA-256：[`assets/art_sources/imagegen/library/README.md`](assets/art_sources/imagegen/library/README.md)

该地图替换读者咨询模式中原有的程序线段占位图；中文标题和剧情说明仍由程序字体绘制。资源属于项目自制候选美术，最终交付前需完成人工视觉验收。

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

## Team-Provided Fixed Character Sprite Candidates

- 提供方式：团队于 2026-07-15 提供的透明 PNG；仓库不记录聊天工具或个人目录路径。
- 统一规格：`256×96` RGBA，横排 4 帧，每帧 `64×96`，最近邻采样，运行时每帧 `0.18s`。
- 便利店售货员：[`assets/textures/ui/convenience_store/npc/salesclerk_idle_sheet.png`](assets/textures/ui/convenience_store/npc/salesclerk_idle_sheet.png)，SHA-256 `689b93b54bbc80bd8ef083dc2700677f3e92b1fdc97e97cce7da655db88ed101`。
- 酒保：[`assets/textures/ui/tavern/npc/bartender_idle_sheet.png`](assets/textures/ui/tavern/npc/bartender_idle_sheet.png)，SHA-256 `aadc190dc24c5103484a3498ec107a2c8f22aab172deee228888e3a11ea73757`。
- 餐馆厨师：[`assets/textures/ui/restaurant/npc/chef_idle_sheet.png`](assets/textures/ui/restaurant/npc/chef_idle_sheet.png)，SHA-256 `26a7adcfe4d2c25c6f8b78c07f274743debdd6fa17a0229cd7f31e0a5acd90d8`。
- 图书管理员：[`assets/textures/ui/library/npc/librarian_idle_sheet.png`](assets/textures/ui/library/npc/librarian_idle_sheet.png)，SHA-256 `643980db5e2e2f8d3282add90a4e6a00fd5106793b407ba7fc4aaabf9dd61446`。
- 主角：[`assets/textures/ui/characters/protagonist_idle_sheet.png`](assets/textures/ui/characters/protagonist_idle_sheet.png)，SHA-256 `fc8871c4512429a948f39d2fc24a6303f7a49e5515d6a9ca207bd4473bc24e05`。
- 镇长：[`assets/textures/ui/characters/mayor_idle_sheet.png`](assets/textures/ui/characters/mayor_idle_sheet.png)，SHA-256 `1b13443dbc0b3a92ff4a9bab9a0e22c9971961677906e6ea77a343e276bfa788`。
- 当前用途：餐馆、便利店、图书馆和酒馆的固定热点待机小人，以及各地点、开场和回家剧情的共享对话框人物槽；资源缺失时仍回退到程序绘制人物。
- imagegen 决策：六张表的 24 个单帧均已满足当前四帧规格，本批没有为凑帧数生成风格不一致的中间帧。
- 许可边界：原作者、生成工具和可再分发/商用许可仍待团队补齐；在 Issue 17 完成人工验收和授权确认前只能作为项目候选资源，不能据此宣称可商用最终美术。

旧 Pixel Crawler 酒保候选已退出运行时。其条款文件仍保留在
[`assets/textures/ui/tavern/LICENSES/pixel_crawler_free_pack_terms.txt`](assets/textures/ui/tavern/LICENSES/pixel_crawler_free_pack_terms.txt)
作为历史来源记录，不再描述当前 `bartender_idle_sheet.png`。

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

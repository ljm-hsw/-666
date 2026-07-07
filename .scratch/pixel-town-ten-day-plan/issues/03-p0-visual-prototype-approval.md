# P0：确认像素视觉原型与资源规范

Status: ready-for-human
Milestone: P0
Type: HITL
User stories: 56, 58, 63, 67

## What to build

制作一个可运行视觉原型，展示静态小镇地图、状态栏、地点热点、按钮、中文文本和典型弹窗。原型用于人工确认像素缩放、字体可读性、调色板、tile/sprite 网格和资源许可规范，不承载真实游戏规则。

## Acceptance criteria

- [x] 原型在 960×540 逻辑画布和默认 960×540 窗口中保持像素锐利。
- [ ] 窗口调整后使用整数倍缩放与留黑边，不拉伸画面。
- [ ] 简体中文标题、正文、状态值和按钮在目标 Windows 机器上可读。
- [ ] 小镇地图热点、状态栏和弹窗的视觉层级清晰。
- [ ] 团队确认并记录调色板、像素网格、tile/sprite 尺寸和资源命名规则。
- [x] 选定字体和所有原型第三方资源具有允许再分发的许可证，并登记来源。
- [ ] 人工评审结果和需要修改的项目记录在本 issue 的 `Comments` 中。
- [ ] 获得明确人工批准后，后续 UI issue 才可依赖该视觉规格。

## Blocked by

- [01 P0：建立可离线复现的应用基线](01-p0-offline-app-baseline.md)

## Comments

<!-- 在此行下方追加人工评审记录。 -->

### 2026-07-07 Agent 交付候选，等待人工评审

- 已提供 A 地图优先、B 日程侧栏、C 地点卡片三套可切换方案；点击地点显示典型说明弹窗。
- 默认 960×540 窗口使用 960×540 RenderTexture 和 1:1 点采样；旧 640×360 原型 UI 坐标按 1.5× 迁移显示，正文等效放大以提高中文可读性。
- 窗口缩放实现限定为整数倍并居中留黑边，但仍需人工拖动窗口确认视觉结果。
- 候选字体为 Fusion Pixel Font 12px Proportional zh_hans 2026.07.01；运行时从集中 UI 文案生成原型必需字形清单并逐字检查，SIL OFL 1.1 许可证、上游摘要和子资源许可证已登记在 `CREDITS.md`。
- 已按用户确认接入 Kenney Tiny Town 1.1 与 Kenney Tiny Farm 1.0 作为 16×16 tile 视觉参考；官方页面均标注 CC0，仓库保留各自 `Tilemap/`、`Tiles/`、`License.txt` 和 `Tilesheet.txt`，来源与 SHA-256 已登记在 `CREDITS.md`。
- 原型运行时优先使用 Tiny Town 的 `Tilemap/tilemap_packed.png` 铺设草地、道路参考、房体纹理、地图装饰和地点图标；Tiny Town 缺失时尝试 Tiny Farm；两者都缺失时回退到代码绘制形状，不阻塞启动。
- 候选调色板、16×16 tile、16/32 px sprite、4 px UI 网格和资源命名规则见 `docs/VISUAL_PROTOTYPE.md`，尚未获得团队批准。
- macOS Debug 构建和 CTest 通过。可运行 `./build/pixel_town --capture-prototype` 生成评审截图，或直接启动应用进行窗口缩放、中文可读性和热点层级评审。

人工评审需明确记录：选择 A、B、C 或组合方案；中文字体是否可读；窗口缩放是否锐利；是否批准 Kenney Tiny Town / Tiny Farm 作为 P0 参考素材；是否批准调色板、尺寸和命名规则。未得到这些结论前，本 issue 保持 `ready-for-human`。

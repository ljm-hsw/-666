# Imagegen 视觉资产规划

本文记录从 P1 开始的视觉方向调整：页面背景和主要场景图逐步改为 imagegen 生成的统一像素风资产，Kenney Tiny Town / Tiny Farm 退为 tile 点缀、道路、树木和临时装饰素材。本文是规划与生成约束；已落地资产仍需 Issue 03 或后续美术验收记录确认是否进入最终交付。

## 画布与窗口基线

| 项目 | 当前目标 |
| --- | --- |
| 逻辑画布 | 960×540 |
| 默认窗口 | 1920×1080 |
| 缩放策略 | 仍使用整数倍点采样缩放，窗口变化时居中留黑边 |
| UI 迁移方式 | 旧 640×360 坐标按 1.5× 迁移到 960×540，避免中文小字不可读 |

迁移原因：

- 640×360 下中文像素字体和状态栏密度过高，长句易产生可读性问题。
- 960×540 仍保持 16:9 和 2× 默认窗口缩放，同时给地图、结算和弹窗留出更多排版空间。
- 后续 imagegen 背景更适合按 960×540 页面级构图，再由代码绘制交互热点和文字。

## 统一风格方向

- 主题：温暖、清晰、轻量的小镇经营像素风。
- 构图：主要页面使用完整背景图或大面积场景图，交互层使用清晰卡片/按钮覆盖。
- 像素规则：硬边、有限色板、清楚轮廓，避免照片感、油画感、软阴影和高频噪点。
- 文字规则：生成图中不包含文字；所有中文、数字和按钮文案由程序字体绘制，避免生成文本不可控。
- 透明规则：地点建筑、道具、按钮装饰等独立资产优先使用 chroma-key 或本地处理后的透明 PNG。
- Kenney 用法：只作为草地、道路、树木、小图标、临时 tile 点缀；不再作为主页面背景和主要建筑风格来源。

## 当前落地状态

| 资产 | 状态 | 仓库路径 | 运行时策略 |
| --- | --- | --- | --- |
| 小镇地图背景 | 已生成并接入 P1 game-flow 地图页 | `assets/textures/imagegen_backgrounds/town_map_background.png` | 优先加载 imagegen 背景；缺失时回退到 Kenney/代码拼接地图 |
| 五个地点建筑 sprite sheet | 已生成并接入 P1 game-flow 地图页 | `assets/textures/imagegen_buildings/p1_building_sprites.png` | 优先加载 imagegen 建筑；缺失时回退到 Kenney/代码绘制地点卡片 |
| 标题页/总结页/结局页背景 | 待生成 | 待定 | 继续使用程序绘制背景，后续逐页替换 |

## 资产生成清单

| 优先级 | 资产 | 建议尺寸 | 用途 | 生成约束 |
| --- | --- | --- | --- | --- |
| P0 | 小镇地图背景 | 960×540 | 地图页主背景 | 不含地点文字；预留 5 个热点区域；道路和地块清晰 |
| P0 | 五个地点建筑 sprite sheet | 每格约 192×144 或 256×192 | 餐馆、便利店、图书馆、酒馆、家 | 透明背景；强轮廓；类型一眼可辨 |
| P1 | 标题页背景 | 960×540 | 标题页 | 夜晚/黄昏小镇氛围；中央留标题和按钮空间 |
| P1 | 每日总结背景 | 960×540 | 每日总结页 | 温暖桌面/日记/账本感；中间留文本面板 |
| P1 | 结局页背景 | 960×540 | 第十日结局 | 小镇远景或夕阳；中心留结局卡片 |
| P2 | 餐馆室内背景 | 960×540 | 餐馆玩法 | 吧台、餐桌、上菜区；不含文字 |
| P2 | 便利店室内背景 | 960×540 | 便利店玩法 | 货架、收银台、商品区；不含文字 |
| P2 | 图书馆室内背景 | 960×540 | 图书馆玩法 | 书架、借阅台、分类区；不含文字 |
| P3 | 酒馆室内背景 | 960×540 | 酒馆选择/小游戏入口 | 木质暖光、桌游桌面；不含文字 |
| P4 | UI 面板/按钮装饰套件 | 透明 PNG 或小 spritesheet | 状态栏、弹窗、按钮边框 | 只做装饰，不承载文本 |

## Prompt 模板

### 页面背景

```text
Use case: stylized-concept
Asset type: 960x540 pixel-art background for a desktop game page
Primary request: Generate a warm cozy pixel-art <PAGE_NAME> background for Pixel Town: Ten-Day Plan.
Style: crisp 2D pixel art, limited palette, hard pixel edges, no photorealism, no painterly blur.
Composition: 16:9, 960x540, leave clear empty UI-safe areas for program-rendered Chinese text and buttons.
Text policy: no text, no labels, no watermark, no signs with readable letters.
Kenney compatibility: compatible with simple 16x16 tile accents, but original imagegen background is the main style.
Avoid: soft shadows, gradients that look non-pixel, tiny unreadable details, realistic perspective.
```

### 独立建筑 / UI sprite

```text
Use case: stylized-concept
Asset type: transparent/chroma-key pixel-art sprite sheet for game UI
Primary request: Generate five small-town building sprites: restaurant, convenience store, library, tavern, home.
Style: chunky readable pixel art, strong dark outline, flat colors, clear silhouette.
Layout: one row, equal cells, generous padding, chroma-key background for removal.
Text policy: no text, no labels, no watermark.
Runtime target: each sprite must remain readable when drawn as a map hotspot on a 960x540 logical canvas.
```

## 验收标准

- 960×540 截图中文字不出现替代字形，状态栏和提示栏可读。
- 页面背景不包含生成文字；所有可读文本来自程序字体。
- 小镇地图在不看文字的情况下也能区分餐馆、便利店、图书馆、酒馆和家。
- 背景和热点不会遮挡状态栏、按钮和结算文本。
- 资源必须进入 `assets/`，来源、生成方式、SHA-256 和候选/最终状态记录在 `CREDITS.md` 或资产说明中。
- 任何 imagegen 资产进入正式交付前，必须保留原始生成图、处理后图和验收截图。

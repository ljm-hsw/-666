# Imagegen 视觉资产规划

本文记录从 P1 开始的视觉方向调整：页面背景和主要场景图逐步改为 imagegen 生成的统一像素风资产，Kenney Tiny Town / Tiny Farm 退为 tile 点缀、道路、树木和临时装饰素材。本文是规划与生成约束；已落地资产仍需 Issue 03 或后续美术验收记录确认是否进入最终交付。

协作策略见 [`UI_ASSET_STRATEGY.md`](UI_ASSET_STRATEGY.md)：P2/P3 期间 imagegen 用于统一风格和候选资产，地点模块允许使用占位 UI 先完成逻辑、测试和统一行动结果；P4 再统一替换最终 NPC、场景、面板和教程表现。

## 画布与窗口基线

| 项目 | 当前目标 |
| --- | --- |
| 逻辑画布 | 960×540 |
| 默认窗口 | 960×540 |
| 缩放策略 | 仍使用整数倍点采样缩放，窗口变化时居中留黑边 |
| UI 迁移方式 | P1 game-flow 已直接在 960×540 逻辑画布绘制；P0 visual prototype 保留独立候选缩放路径 |

迁移原因：

- 640×360 下中文像素字体和状态栏密度过高，长句易产生可读性问题。
- 960×540 仍保持 16:9，并直接作为默认窗口尺寸，减少全屏截图和小屏设备上的缩放风险。
- 后续 imagegen 背景更适合按 960×540 页面级构图，再由代码绘制交互热点和文字。
- P1 game-flow 字体按 24px 栅格加载，并将常用字号吸附到稳定档位，避免旧 1.5× camera 放大造成的中文边缘发虚。

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
| 小镇完整地图场景 | 已生成并接入 P1 game-flow 地图页 | `assets/textures/imagegen_backgrounds/town_map_full_scene.png` | 第一优先级；背景和五个地点建筑在同一张图中，代码只叠加标签/热区 |
| 标题页背景 | 已生成并接入 P1 标题页 | `assets/textures/imagegen_backgrounds/title_screen_background.png` | 第一优先级；代码叠加标题、开始按钮和提示文本 |
| 小镇地图背景 | 已生成，作为 fallback | `assets/textures/imagegen_backgrounds/town_map_background.png` | 完整场景缺失时使用，再叠加独立 imagegen 建筑 |
| 五个地点建筑 sprite sheet | 已生成，作为 fallback | `assets/textures/imagegen_buildings/p1_building_sprites.png` | 完整场景缺失时使用；再缺失时回退到 Kenney/代码绘制地点卡片 |
| 总结页/结局页背景 | 待生成 | 待定 | 继续使用程序绘制背景，后续逐页替换 |

## 资产生成清单

| 优先级 | 资产 | 建议尺寸 | 用途 | 生成约束 |
| --- | --- | --- | --- | --- |
| P0 | 小镇地图背景 | 960×540 | 地图页主背景 | 不含地点文字；预留 5 个热点区域；道路和地块清晰 |
| P0 | 五个地点建筑 sprite sheet | 每格约 192×144 或 256×192 | 餐馆、便利店、图书馆、酒馆、家 | 透明背景；强轮廓；类型一眼可辨 |
| P1 | 标题页背景 | 960×540 | 标题页 | 已生成小镇入口广场版本；中央留标题和按钮空间 |
| P1 | 每日总结背景 | 960×540 | 每日总结页 | 温暖桌面/日记/账本感；中间留文本面板 |
| P1 | 结局页背景 | 960×540 | 第十日结局 | 小镇远景或夕阳；中心留结局卡片 |
| P2 | 餐馆室内背景 | 960×540 | 餐馆玩法 | 吧台、餐桌、上菜区；不含文字 |
| P2 | 餐馆食物图标 | 5 个小图标或 spritesheet | 餐馆菜品选择 | 炒饭、面条、汤、饺子、沙拉；小尺寸轮廓可区分；不含文字 |
| P2 | 餐馆 UI 装饰件 | 透明 PNG 或程序绘制规格 | 订单票据、倒计时条、反馈标记 | 文字由程序绘制；必须有纯程序 fallback |
| P2 | 便利店室内背景 | 960×540 | 便利店玩法 | 货架、收银台、商品区；不含文字 |
| P2 | 图书馆室内背景 | 960×540 | 图书馆玩法 | 书架、借阅台、分类区；不含文字 |
| P3 | 酒馆室内背景 | 960×540 | 酒馆选择/小游戏入口 | 木质暖光、桌游桌面；不含文字 |
| P4 | UI 面板/按钮装饰套件 | 透明 PNG 或小 spritesheet | 状态栏、弹窗、按钮边框 | 只做装饰，不承载文本 |

P2/P3 的地点 issue 不以最终 imagegen 素材为完成前提。地点负责人可以用程序绘制面板、占位头像框、临时色块和 Kenney 点缀素材完成可操作 UI；但必须保持 960×540 逻辑画布、布局可替换、图片缺失不影响规则测试、剧情长文案不写死在绘制代码中。

## 餐馆 P2 视觉拆分要求

餐馆是白天反应型玩法的主要展示场景，P2 收口时至少需要把以下信息可视化，而不是只用文本列表表达：

- 背景 UI：960×540 餐馆室内或程序绘制等价布局，预留状态栏、订单区、菜品区、反馈区和结算区。
- 食物：炒饭、面条、汤、饺子、沙拉五个菜品必须有图标槽位；最终图标缺失时使用文字按钮或简单程序图形 fallback。
- 时间：等待时间必须同时用数字和条形/颜色变化表现，剩余时间低时有明显警示。
- 说明：首次进入说明卡要解释目标、快捷键、鼠标点击、错单、超时、放弃和结算。
- 反馈：正确、错单、超时和完成结算需要独立视觉状态，便于截图验收和课堂演示。

餐馆视觉资源的规划 issue 为 `.scratch/pixel-town-ten-day-plan/issues/21-p2-restaurant-visual-assets-and-layout.md`；实际 UI 反馈收口 issue 为 `.scratch/pixel-town-ten-day-plan/issues/20-p2-restaurant-ui-feedback.md`。详细坐标、槽位、fallback 和资产记录要求见 [`RESTAURANT_VISUAL_SPEC.md`](RESTAURANT_VISUAL_SPEC.md)。

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

### 餐馆室内背景

```text
Use case: stylized-concept
Asset type: 960x540 pixel-art restaurant interior background for a desktop game page
Primary request: Generate a warm cozy pixel-art small-town restaurant interior for Pixel Town: Ten-Day Plan.
Scene content: wooden counter, serving window, simple dining tables, kitchen hints, warm wall decor, readable silhouettes.
Composition: 16:9, 960x540, leave the top 84px clear for program UI, leave a left-center safe area for an order panel.
Text policy: no text, no menu labels, no signs with readable letters, no watermark.
Style: crisp 2D pixel art, limited warm palette, hard pixel edges, no photorealism, no painterly blur.
Avoid: realistic perspective, high-frequency clutter behind UI panels, baked-in buttons, baked-in Chinese text.
```

### 餐馆食物图标

```text
Use case: stylized-concept
Asset type: transparent pixel-art food icon sprite sheet for game UI
Primary request: Generate five readable food icons: fried rice, noodles, soup, dumplings, salad.
Layout: one row, equal cells, 48x48 or 64x64 each, generous padding, transparent or chroma-key background.
Style: chunky readable pixel art, strong outline, flat colors, clear silhouettes at small size.
Text policy: no text, no numbers, no labels, no watermark.
Runtime target: icons will be drawn inside restaurant dish buttons on a 960x540 logical canvas.
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

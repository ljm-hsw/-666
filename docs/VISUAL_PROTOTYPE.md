# P0 视觉原型候选规范

本文记录 Issue 03 的待审视觉候选，不是已批准的最终规范。原型运行时使用左右方向键或底部切换条查看三个结构方案：

- A 地图优先：状态栏固定在顶部，大地图承担主要导航。
- B 日程侧栏：左侧展示日程与状态，右侧保留地图氛围。
- C 地点卡片：地点能力和收益预期优先，地图退居次要位置。

## 待审共同基线

| 项目 | 候选值 |
| --- | --- |
| 逻辑画布 | 960×540 |
| 默认窗口 | 960×540 |
| 缩放 | 仅 1×、2×、3×等整数倍；剩余区域留黑边 |
| 基础 tile | 16×16 px |
| 常规 sprite | 16×16 或 32×32 px |
| UI 间距 | 4 px 基线网格 |
| 中文字体 | Fusion Pixel Font 12px Proportional zh_hans |
| 正文基准 | 12 px；小型状态文字不得低于 9 px |
| 参考 tileset | Kenney Tiny Town 1.1 优先，Kenney Tiny Farm 1.0 备用，均为 16×16、CC0 |

`--capture-prototype` 与 `--capture-game-flow` 导出的是 960×540 逻辑画布图片，不依赖当前机器显示器是否能完整容纳更大窗口。

## 待审视觉素材参考

当前原型优先接入 Kenney Tiny Town，并保留 Kenney Tiny Farm 作为备用参考素材：

- Tiny Town 官方页面：`https://www.kenney.nl/assets/tiny-town`
- Tiny Farm 官方页面：`https://www.kenney.nl/assets/tiny-farm`
- 仓库路径：`assets/textures/kenney_tiny_town/`、`assets/textures/kenney_tiny_farm/`
- 使用方式：运行时优先加载 Tiny Town 的 `Tilemap/tilemap_packed.png` 铺设草地、道路参考、房体纹理、地图装饰和地点图标；Tiny Town 缺失时尝试 Tiny Farm；两者都缺失时回退到代码绘制形状。
- 保留范围：`Tilemap/`、`Tiles/`、`License.txt`、`Tilesheet.txt`。
- 剔除范围：上游 `.url` 快捷方式、预览图、下载 ZIP。

Tiny Town 的优势是与当前 16×16 tile 基线一致、许可证简单、素材体量小，且比 Tiny Farm 更贴近小镇地图、建筑、道路和门窗语义；Tiny Farm 继续作为农场/草地风格参考。两套素材仍不能完整表达餐馆、便利店、图书馆、酒馆的最终专用建筑、页面背景与室内图标。当前 P1 game-flow 地图页已按 [`IMAGEGEN_VISUAL_PLAN.md`](IMAGEGEN_VISUAL_PLAN.md) 优先接入“背景和房屋在同一张图中生成”的 imagegen 完整场景图，Kenney 退为 tile 点缀、参考和缺失资源回退。Issue 03 评审时只确认它们是否作为视觉参考和临时素材，不把它们直接视为最终美术全集。

## 待审调色板

| 用途 | 色值 |
| --- | --- |
| 主墨色 | `#2D3436` |
| 纸张底色 | `#FAEECB` |
| 浅面板 | `#FFF8E2` |
| 主绿色 | `#52895C` |
| 草地 | `#90BE77` |
| 河流蓝 | `#5899BA` |
| 强调红 | `#B75348` |
| 金色 | `#E0A94A` |

## 待审资源命名

- 全部使用小写 `snake_case`，不含空格、中文或个人姓名。
- 贴图：`<场景>_<对象>_<状态>.png`，例如 `town_store_open.png`。
- sprite sheet：`<角色>_<动作>_<方向>_<帧数>f.png`。
- 音频：`bgm_<场景>.ogg`、`sfx_<动作>.ogg`。
- 数据：`<模块>_<内容>.txt`。
- 第三方资源必须在 `CREDITS.md` 登记版本、来源、许可证和摘要。

## 人工评审要求

请在目标 Windows 机器上重点检查：中文正文与 9 px 状态文字是否可读、窗口调整后是否保持像素锐利、热点是否易发现、弹窗是否遮挡关键信息。选定方案或组合后，将结论写入 Issue 03 的 `Comments`，再删除落选变体并把获胜方案吸收到正式 UI。

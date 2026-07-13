# 酒馆、图书馆与便利店分件式视觉素材生产计划

## 目的与当前决定

本计划把三处地点的视觉生产方式从“让 imagegen 一次生成可直接运行的完整背景”调整为：

```text
统一风格板
  -> 分别生成空房壳、主要家具和小型道具候选
  -> 人工挑选、抠图、像素整理和拼接
  -> 形成 960×540 场景合成稿
  -> 为图书馆和酒馆登记阻挡物与互动区
  -> 人工批准
  -> 最后接入程序替换现有背景
```

优先地点固定为酒馆、图书馆和便利店。餐馆继续遵循 [`RESTAURANT_VISUAL_SPEC.md`](RESTAURANT_VISUAL_SPEC.md)，不加入 imagegen 首批生成；2026-07-13 团队另行提供的餐馆、家、图书馆、酒馆和便利店合成稿作为补充接入批次，不改变 imagegen 分件生产路线。

本轮只准备候选素材和人工合成底稿，不改变地点规则、UI 热区、对话逻辑、行动结果或 v1 存档。现有代码绘制和旧背景继续作为 fallback，直到新合成稿通过人工验收。

## 已接入的团队合成稿（2026-07-13）

团队提供的餐馆、家、图书馆、酒馆和便利店 PNG 已归档到 `assets/art_sources/manual_composites/`，并转换为 960×540 运行时图：

| 场景 | 运行时路径 | 当前接入 | 碰撞切分 |
| --- | --- | --- | --- |
| 餐馆 | `assets/textures/ui/restaurant/restaurant_interior.png` | 先显示餐馆大厅与老板预留热点，再进入原有准备、订单和结算 UI | 柜台、果蔬台、木桶、冷柜、两组餐桌、伞架、墙体和门口通道 |
| 家 | `assets/textures/ui/home/home_interior.png` | 先显示家中大厅与互动预留热点，再进入休息确认；两次返回都不消耗阶段 | 床、衣柜、壁炉、扶手椅、圆桌、书架、书桌、右侧桌椅、沙发、墙体和门口通道 |
| 图书馆 | `assets/textures/ui/library/library_interior.png` | 先显示图书馆大厅与管理员预留热点，再进入原有引导、答题、反馈及房间视图 | 借阅台、书车、三组书架、地图桌、两组阅读桌、植物、墙体和门口通道 |
| 酒馆 | `assets/textures/ui/tavern/tavern_lobby.png` | 替换旧酒馆背景；酒保、五子棋桌和骗子骰子桌热点已重新对齐 | 吧台、酒桶、酒柜、四组圆桌、两张桌游桌、墙体和门口通道 |
| 便利店 | `assets/textures/ui/convenience_store/convenience_store_interior.png` | 先显示便利店大厅与店主预留热点，再进入原有进货、定价、预算和结算 UI | 货架、纸箱、购物篮、冷柜、饮料柜、收银台、伞架、墙体和门口通道 |

碰撞数据的唯一代码事实源为 `src/core/scene_collision.cpp`，使用 960×540 场景源坐标；
运行时与 F3 覆盖层再统一映射到室内视口。无窗口测试覆盖布局合法性、出生点、门口通道、
越界和轴向滑动。运行时按 F3 可显示：绿色可行走边界、红色静态碰撞体、蓝色出口触发区
和黄色 24×24 出生占位。

这次接入不包含主角移动或 NPC 路点逻辑。它冻结可供后续 Issue 27/28 使用的背景、静态碰撞和出生/出口基础契约；酒馆现有酒保与桌游点击热点已按新画面调整，但不改变其结算或 v1 存档格式。便利店碰撞数据当前只用于 F3 视觉审查与后续扩展，不代表便利店开放人物移动。图片组成素材的授权信息仍待团队补齐，不能据此视为可商用最终美术。

## 临时执行顺序

- Issue 23 已完成，不回退其共享对话能力。
- Issue 24-26 暂时保持 `needs-info`；Issue 27 和 28 已因团队提供图书馆/酒馆合成稿与静态碰撞清单而恢复为 `ready-for-agent`。Issue 28 仍受 Issue 27 依赖约束，不会提前自动执行。
- 当前执行入口改为本计划的视觉生产和人工拼接流程。
- 只有满足“恢复 Issue 的门槛”并得到用户明确确认后，才继续餐馆/便利店/镇长对话和图书馆/酒馆室内移动接入。

## 统一风格锁

三个地点必须像同一座小镇内的不同房间，而不是三套互不相关的素材包。

| 项目 | 统一要求 |
| --- | --- |
| 画面类型 | 2D 像素风、小镇生活模拟、固定镜头、轻度俯视的正交房间 |
| 运行画布 | 最终合成稿固定为 960×540 PNG |
| 视觉密度 | 中低密度；主要家具轮廓清楚，小道具不形成噪点墙 |
| 像素边缘 | 硬边、最近邻、有限色板；禁止照片纹理、柔焦和连续油画笔触 |
| 轮廓 | 深色清晰轮廓，和当前 `#2D3436` 主墨色兼容 |
| 光照 | 主光方向统一从左上方进入；允许各地点改变色温，不改变投影方向 |
| 透视 | 所有家具保持同一轻度俯视角；禁止鱼眼、强消失点和混合等距视角 |
| 文字 | 生成图不得包含中文、英文、数字、品牌、菜单或可读招牌 |
| 人物 | 场景套件不生成人物；NPC 和主角保持独立 sprite 管线 |
| UI | 不烘焙状态栏、按钮、对话框、热点光圈或教程文字 |
| 许可 | 不模仿可识别商业游戏资产，不使用商标或品牌包装 |

地点可以拥有自己的次级色温：

- 酒馆：深木色、琥珀暖光、暗红和黄铜点缀。
- 图书馆：胡桃木、纸张米色、安静绿色和柔和日光。
- 便利店：浅木/米白、生活绿色、少量红蓝商品色块，保持明亮但不现代霓虹化。

## 首批 imagegen 生成批次

内置 imagegen 是默认生成路径。不同素材必须分别调用，不使用一张大图替代多个独立 prompt，也不在本阶段切换到需要 API Key 的 CLI 路径。

Prompt 中的 960×540 是最终构图目标，不假设内置工具必然直接输出该像素尺寸。原始候选若为其他分辨率，选中后必须按 16:9 安全区裁切，并用最近邻方式整理为 960×540 合成底稿；不得通过拉伸改变家具比例或透视。

### A：跨地点风格校准

先生成 3 张不进入运行时的跨地点风格板。每张风格板都同时展示酒馆、图书馆和便利店的局部小场景，并保持相同像素颗粒、轮廓、俯视角和光照方向。

| 批次 | 数量 | 目的 |
| --- | ---: | --- |
| `style_board_a-c` | 3 | 在大量生成前选择统一像素密度、透视、轮廓和色板 |

未经人工选定一个风格板，不开始 B-D 批次。

### 已批准方向

2026-07-11，用户明确选择 `style_board_c_source.png` 作为后续场景分件的统一视觉基准。A、B 继续保留为过程记录，但后续生成不得混用其轮廓、色板或光照风格。

首轮获批方向分件先收敛为每个地点各 3 张：建筑结构、主要家具、小型道具，共 9 张。建筑结构 sheet 承接墙段、转角、地板样片、门和窗；它们替代原计划中“先生成完整空房壳”的第一轮探索，完整空房壳延后到人工挑选分件之后作为合成验证稿。这样可以先检查素材是否真的可拆、可重复拼接，再消耗调用生成整屋候选。

上述 9 张分件已完成生成、去背和首轮检查。原图、透明候选、哈希、透明度检查和联系表记录在 [`../assets/art_sources/imagegen/scene_kits/component_batch_c_manifest.md`](../assets/art_sources/imagegen/scene_kits/component_batch_c_manifest.md)。它们仍停留在 `assets/art_sources/`，尚未进入运行时资源目录。

9 张透明 sheet 已进一步裁切为 204 张带 4 像素安全边的独立 RGBA PNG，并保留源坐标、尺寸与 SHA-256 清单。该步骤只形成可供人工挑选的源分件；在完成语义命名、比例统一和场景拼接之前，不把编号文件直接复制到运行时目录。

### B：空房壳

每个地点生成 2 个空房壳候选，共 6 张。空房壳只包含墙体、地板、门窗和固定光照，不包含可移动家具、人物、文字或 UI。

| 地点 | 候选 | 必须体现 |
| --- | ---: | --- |
| 酒馆 | 2 | 木地板、墙面、门窗、吧台预留位、桌游区预留位 |
| 图书馆 | 2 | 安静木地板、书架预留带、借阅台预留位、阅读区通道 |
| 便利店 | 2 | 明亮地面、墙面货架预留带、收银区和商品区留白 |

空房壳最终不是“越空越好”，而是先锁定一致的房间透视和光线，让后续家具能够被人工放置而不发生透视冲突。

### C：大型家具套件

每个地点生成 2 张大型家具候选 sheet，共 6 张。每件物体之间保留足够间距，使用纯色 `#ff00ff` chroma-key 背景，不产生地面、墙面、人物、文字、接触阴影或反射。

| 地点 | 大型家具候选 |
| --- | --- |
| 酒馆 | 直吧台、L 型吧台、酒瓶墙柜、圆桌、方桌、成套椅子、壁炉、木桶、棋桌、骰子桌 |
| 图书馆 | 高书架、矮书架、转角书架、借阅台、阅读桌、椅子、书车、卡片柜、旧地图展示台 |
| 便利店 | 收银台、单面/双面货架、墙柜、冰柜、饮料柜、雨伞架、商品端架、纸箱堆、购物篮架 |

同一 sheet 内物体必须共享比例和俯视角，但不要连接成不可拆分的完整房间。

### D：小型道具套件

每个地点生成 1 张小道具 sheet，共 3 张。仍使用纯色 `#ff00ff` chroma-key 背景。

| 地点 | 小型道具候选 |
| --- | --- |
| 酒馆 | 酒杯、杯垫、骰盅、骰子、棋子盒、桌灯、空白相框、烛台、小木牌、挂布 |
| 图书馆 | 成摞书、展开书、借书卡盒、台灯、盆栽、书挡、纸张、印章、卷起旧地图、空白分类牌 |
| 便利店 | 雨伞、便当盒、汽水罐、零食袋、咖啡杯、价签底板、收据卷、纸袋、拖把桶、小纸箱 |

商品包装只使用抽象色块，不出现真实品牌、可读文字或生成乱码。

### 数量预算

- 风格板：3 次。
- 获批 C 方向第一轮建筑结构 sheet：3 次。
- 获批 C 方向第一轮大型家具 sheet：3 次。
- 获批 C 方向第一轮小型道具 sheet：3 次。
- 已执行/计划中的第一轮合计：12 次独立 imagegen 调用。
- 空房壳与第二候选延后到第一轮分件人工筛选后按缺口生成，不再预先固定消耗 6 次。
- 人工拼出三张场景草稿后，最多再开 6 次定向补缺调用；未列出缺口前不盲目继续生成变体。

## Prompt 契约

### 风格板基础模板

```text
Use case: stylized-concept
Asset type: cross-location pixel-art style board for a 960x540 desktop game
Primary request: Create one coherent visual direction for three interiors in the same small town: a cozy tavern, a quiet library, and a bright convenience store.
Composition: three separate room vignettes in one image, no labels, same fixed-camera light top-down orthographic perspective, same pixel scale and outline language.
Style: crisp 2D pixel art, limited warm palette, chunky readable silhouettes, hard pixel edges, low visual noise.
Lighting: shared upper-left light direction; tavern warm amber, library soft daylight, convenience store bright neutral warmth.
Constraints: no characters, no UI, no text, no logos, no brands, no watermark; do not imitate a recognizable commercial game.
Avoid: photorealism, painterly blur, soft antialiasing, mixed perspective, isometric diamond rooms, excessive clutter, tiny unreadable props.
```

### 空房壳基础模板

```text
Use case: stylized-concept
Asset type: empty 960x540 pixel-art interior shell for manual game-scene compositing
Primary request: Create an empty <LOCATION> room shell matching the approved Pixel Town style board.
Scene content: walls, floor, door, windows, fixed architectural trim and lighting only.
Composition: fixed-camera light top-down orthographic perspective, 16:9, keep the top 84 pixels free of important details for program UI, leave broad open zones for manually placed furniture.
Style: crisp 2D pixel art, hard edges, limited palette, clear dark outlines, consistent upper-left light.
Constraints: no furniture that cannot be removed, no characters, no readable signs, no text, no UI, no watermark.
Avoid: strong vanishing perspective, fisheye, isometric diamond layout, gradients that look non-pixel, high-frequency floor texture.
```

### 分件 sheet 基础模板

```text
Use case: stylized-concept
Asset type: modular opaque pixel-art <LOCATION> furniture and prop sheet for manual compositing
Primary request: Generate the listed standalone objects as separate, non-overlapping pieces, matching the approved Pixel Town style board.
Layout: evenly spaced objects in a clean grid with generous padding; every object fully visible; consistent scale and light top-down orthographic perspective.
Background: perfectly flat solid #ff00ff chroma-key background with no shadows, gradients, texture, floor plane or lighting variation.
Style: crisp 2D pixel art, hard pixel edges, limited palette, strong readable silhouette, shared upper-left lighting.
Constraints: do not use #ff00ff inside any object; no characters, no readable text, no logos, no brands, no watermark, no cast shadow, no reflection.
Avoid: connected room scenes, overlapping objects, mixed camera angles, photorealism, painterly blur, tiny decorative noise.
```

实际调用时必须把 `<LOCATION>` 和物体清单替换为本计划中对应地点的明确内容。风格板一旦批准，后续 prompt 都要注明“matching the approved style board”，并把批准图片作为风格参考，而不是每批重新发明风格。

## 原始素材与运行时目录

生成结果先作为可追溯源文件保存，不直接放进运行时路径：

```text
assets/art_sources/imagegen/scene_kits/
  style_boards/
  tavern/raw/
  tavern/selected/
  library/raw/
  library/selected/
  convenience_store/raw/
  convenience_store/selected/

assets/textures/ui/
  tavern/pieces/
  tavern/composites/
  library/pieces/
  library/composites/
  convenience_store/pieces/
  convenience_store/composites/
```

命名规则：

- 原始生成图：`<location>_<kind>_candidate_v01_source.png`。
- 处理后分件：`<location>_<object>_candidate_v01.png`。
- 人工拼接源：`<location>_interior_composite_v01_source.png`；如使用分层编辑文件，可额外保留 `.aseprite`、`.xcf` 或等价源文件，但不能只保留无法预览的专有文件。
- 运行时候选：`<location>_interior_candidate_v01.png`。
- 最终获批后才去掉 `_candidate_vNN`，并由接入变更更新资源清单和 CREDITS。

内置 imagegen 默认产物位于 Codex 的生成目录。凡是进入本项目的候选，都必须复制到上述仓库目录并记录 prompt；不能让文档或代码引用只存在于 Codex 用户目录中的文件。

## 手工处理与拼接流程

1. 只从获批风格板对应批次挑选素材；落选原图可以保留在 `raw/`，不得进入运行时清单。
2. 对 `#ff00ff` 分件使用 imagegen skill 附带的 chroma-key 去背流程，输出带 alpha 的 PNG；检查透明角、边缘色溢出和主体覆盖率。
3. 将分件裁切到紧凑边界，保留 2-4 个透明像素安全边；禁止双线性缩放。
4. 以 960×540 空房壳为底，在整数像素坐标上人工放置大型家具，再添加小道具。
5. 所有物件保持统一俯视角、光照方向、轮廓宽度和像素颗粒；不一致的物件宁可弃用，不用模糊、旋转插值或软阴影掩盖。
6. 背景源图不得烘焙状态栏；运行时会把完整 960×540 场景等比例放入顶部 60 像素
   独立状态栏下方的居中视口，因此顶部窗户、墙饰等画面信息可以保留，但互动目标仍需
   与场景坐标/碰撞清单共同登记。
7. 不在背景中烘焙人物、中文、按钮、对话框、玩法提示和可变数值。
8. 导出一张未叠加程序 UI 的 960×540 runtime candidate，并另外导出一张带 UI/热点/碰撞辅助线的 review PNG。
9. 将选用物件、来源 candidate、缩放、裁切、坐标、层级和 SHA-256 写入场景 composition manifest。
10. 只有 review PNG 通过人工批准后，才允许程序加载新背景；原背景和程序 fallback 在替换验证结束前不删除。

## 场景专用布局约束

### 酒馆

- 保留酒保岗位、五子棋桌、骗子骰子桌、返回入口和至少一条连续通道。
- 对话框出现时，酒保和主要桌面仍应能从遮罩后辨认，但不与正文形成高频干扰。
- 吧台、桌椅、木桶、壁炉和墙体属于潜在阻挡物，必须能从合成稿中提取清楚矩形范围。
- 不把五子棋棋盘、骰面或赌注数值烘焙成玩法状态；运行时棋局仍由程序绘制。

### 图书馆

- 管理员岗位、答题入口、返回入口和阅读区之间必须有连续通道。
- 高书架、矮书架、借阅台、阅读桌和墙体属于阻挡物；后续增加书架时必须同步更新碰撞数据。
- 旧地图、借书卡盒和书车是叙事道具，但不能遮挡分类题目和反馈面板。
- 书脊不得生成可读乱码；使用抽象色条和像素块。

### 便利店

- 便利店首版仍是静态经营 UI，不开放人物自由移动，因此不因背景拼接引入导航系统。
- 收银台、货架和冰柜用于强化经营语义，不能遮挡商品行、进货数量、价格档、预算、锁定方案和销售结果。
- 商品图只做生活品类暗示，不承诺和需求模型一一对应；真实商品名与数值仍由程序文字表达。
- 保留明亮清晰的经营面板安全区，避免满屏包装色块造成文字对比不足。

## 碰撞与互动交接

图书馆和酒馆在人工合成后必须各自提供一张碰撞/互动审查图和一份坐标清单。2026-07-13 两者均已提供静态碰撞、出生点和出口；图书馆互动区/NPC 路点留给 Issue 27，酒馆现有热点已对齐而移动邻近区/NPC 路点留给 Issue 28：

- `walkable_bounds`：角色允许活动的整体边界。
- `static_colliders`：墙、书架、吧台、桌椅、柜台等不可进入区域。
- `spawn_points`：主角和 NPC 的确定出生点。
- `interaction_zones`：管理员、酒保、桌游桌等互动范围。
- `npc_waypoints`：管理员或常客的候选巡逻路点。

坐标清单以程序当前 640×360 设计网格为主，并附 960×540 画布换算结果。任何阻挡物视觉位置发生变化时，碰撞清单必须和合成稿在同一评审批次更新。便利店只提供 F3 视觉审查/后续预留用的静态碰撞清单，不在当前 MVP 接入移动运行期。

## 资产记录

每个候选至少记录：

- 内置 imagegen 或手工来源。
- 最终 prompt 和引用的风格板。
- 生成日期、用途、候选/选中/运行时状态。
- 原始图、去背图、裁切图、合成图和 review 图的仓库路径。
- SHA-256。
- 手工处理说明和使用的物件列表。
- 人工验收结论。

候选阶段可以使用同目录 manifest；只有进入发布包的资产才更新 `CREDITS.md` 正式条目和必要资源清单。

## 人工验收清单

- 三个场景的透视、轮廓、像素颗粒和主光方向一致。
- 酒馆、图书馆和便利店在不看文字时也能被辨认。
- 960×540 下背景无生成乱码、水印、品牌或人物残影。
- 顶部状态栏和主要程序面板区域有足够对比度。
- 分件透明边缘无明显 chroma-key 色边。
- 图书馆和酒馆存在连续通道，主角/NPC 不需要穿过家具才能抵达互动点。
- 所有阻挡性视觉物件均能在碰撞审查图中找到对应范围。
- 便利店背景不会削弱进货、定价、预算和结算反馈的可读性。
- 旧背景或程序 fallback 仍能在候选资源缺失时正常运行。

## 恢复 Issue 的门槛

除已按用户确认单独恢复的 Issue 27/28 外，同时满足以下条件后，才批量恢复仍暂停的视觉相关 Issue：

1. 三张跨地点风格板中已有一张获得明确人工批准。
2. 酒馆、图书馆和便利店各完成至少一张 960×540 人工拼接草稿。
3. 三张草稿均有无程序文字的 runtime candidate 和带辅助线的 review PNG。
4. 图书馆、酒馆的碰撞与互动坐标草案完成；家具/书架/吧台等阻挡物一一对应。
5. 原始生成图、prompt、选用分件、处理记录和 hash 可以追溯。
6. 用户明确确认恢复 Issue 实施。

恢复后优先接入单场景候选并保留 fallback；不要一次替换三个地点后再统一排查问题。

## 明确不做

- 不在本阶段生成完整角色动画、NPC 头像或对话表情套件。
- 不生成带文字的菜单、分类牌、价格牌或商品品牌包装。
- 不把图书馆或酒馆扩展为滚动地图、开放世界或通用寻路场景。
- 不让 imagegen 输出直接决定碰撞和热区；坐标由人工合成稿与程序审查共同确定。
- 不删除现有 Kenney、第三方候选或程序 fallback，直到最终候选通过接入和缺失资源测试。
- 不在候选未批准时修改规则层、存档格式或地点收益。

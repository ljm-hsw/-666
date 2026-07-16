# 《像素小镇：十日经营计划》

四川大学计算机科学实训项目：一款桌面端、单人、像素风生活模拟经营游戏。

玩家作为初到小镇的新居民，在十个游戏日内安排白天工作和夜晚活动，管理金钱、体力、声望、知识和心情，并根据最终经营表现触发一个主结局。

## 当前状态

项目已完成 **P3 夜晚玩法与正式结局的代码闭环**，进入 P4 人工试玩、视觉与发布验收阶段：

- 产品范围、领域语言和核心设计已确认。
- P0-P4 已拆分为 33 个本地实施 issue。
- C++17/CMake 应用、raylib 6.0、doctest 2.5.2 和跨平台 CI 已初始化。
- Issue 01 的 macOS 本地配置、构建、CTest，以及 GitHub Actions 上的 Windows 2022/MSVC 与 macOS/Apple Clang 矩阵验证已通过。
- Issue 02 已接入启动资源诊断、本地 `logs/latest.log` 和可选音频静音降级。
- Issue 04 已跑通从标题创建新游戏、第一日地图、模拟白天工作、回家休息、每日总结到第二日开始的核心闭环。
- Issue 05 已扩展为固定十日周期；Issue 15 在第十日完整结算后执行一次库存清算，并从七种正式结局中选择唯一主结局。
- Issue 06 已实现阶段边界自动存档与恢复，正式槽位位于应用目录旁 `saves/slot1.sav`。
- Issue 07 的窗口缩放、逻辑输入、暂停/失焦冻结和静音设置外壳已实现并通过自动化测试；最终视觉批准转入 Issue 03/17。
- Issue 17 的候选音频已完成运行时接线：地图/雨天与五个地点使用对应循环 BGM，地点切换、回家、成功和失败使用关键音效，`M` 覆盖全部音频；最终试听和再分发许可仍待人工验收。
- Issue 08 已实现显式演示预设加载，演示模式不读取也不覆盖正式自动存档。
- Issue 19 已接入 P1 MVP 主线剧情骨架：标题开场、十天每日提示、地点摘要和第十天评议会；Issue 15 已用七种正式主结局替换早期占位结局。
- 聚合分支 `codex/integrate-team-branches` 已合入餐馆、图书馆和酒馆模块，保留标题页、960×540 画布、StoryText 和可迁移的 v2 自动存档。集成说明见 [`docs/TEAM_BRANCH_INTEGRATION.md`](docs/TEAM_BRANCH_INTEGRATION.md)。
- Issue 13/14 已接入可完整游玩的五子棋和 1v1 骗子骰子：两套规则支持固定状态/种子测试，酒馆 UI 完成玩法与赌注选择、真实胜负和统一夜晚结算。
- Issue 22 已将酒馆流程收口为显式帧输入、只读 presentation、窄 `TavernRuntime` Interface 和独立 Settlement Module；玩法逻辑可无窗口测试，Settlement 从真实终局推导胜负，电脑骰在揭晓前不会进入展示数据，核心拒绝时保留同一候选结果供稳定重试。
- Issue 15 已完成库存折价清算、七种正式主结局、固定冲突优先级、单项路线归一化比较、结局存档恢复和第十日日初端到端测试；当前阈值仍是 P3 基线，等待 Issue 16 人工试玩批准。
- Issues 23-29 已完成共享对话、固定 NPC 待机/热点和图书馆新增整理模式；Issue 33 在不恢复 NPC 巡逻或寻路的前提下，为餐馆、便利店、图书馆、酒馆和家接入有限室内 WASD/方向键移动、静态碰撞及邻近互动。
- Issue 23 已按 TDD 完成共享线性对话运行期和酒保端到端切片：对话输入模态拦截、UTF-8 三行布局、字形清单、角色缺图 fallback 和四张诊断截图均已验证。
- Issue 24/25 已按 TDD 完成餐馆老板和便利店店主的固定热点、三句主线初识对话与原玩法交接；对话关闭前不会推进订单、计时或经营输入。
- Issue 26 已按 TDD 完成镇长/主角四句新游戏开场和回家休息生命周期基线；Issue 32 已将休息前独白扩展为确定性的每次回家访客剧情。对话关闭后仍只提交一次既有休息结果，第一日存档边界可无奖励重播，支线剧情继续延期。
- 2026-07-15 Issue 30-32 已完成：五个地点以完成访问次数区分首访教程和后续事件，按游戏日、天气与确定性种子选择日常/意外/第 10 天回收文本；白天地点关闭对话后进入原玩法，酒馆关闭对话后恢复桌游热点，家关闭或跳过后只应用一次既有休息结果。首批事件不暗改玩法数值。
- 2026-07-13 团队提供的餐馆、家、图书馆、酒馆和便利店合成稿均已接入运行时。2026-07-14 Issue 27/28 按固定 NPC 待机与点击热点方案收口；2026-07-16 新增 Issue 33，以独立 raylib-free 导航模块恢复主角有限室内移动和静态碰撞，既有固定 NPC、对话及地点玩法契约保持不变。
- 餐馆、便利店、图书馆和家现统一为“地图 → 场景大厅 → NPC/剧情互动或返回 → 进入原玩法”的入口；五个地点均已接首访与后续地点事件。进入大厅或阅读事件不消耗行动且不写入奖励，酒馆保留专用大厅。
- 图书馆行动现提供并列的“读者咨询 / 书籍整理”模式：整理任务和书架由数据文件提供，错误归类可保留手中书重试，两种模式都通过核心统一行动结果结算；固定管理员点击对话已接到该模式选择入口。

当前不应把文档中的临时数值视为最终平衡承诺。

## 核心循环

```text
进入新的一天
  → 查看状态与当日提示
  → 选择一个白天工作
  → 应用行动结果
  → 选择回家或一次酒馆挑战
  → 每日结算并保存
  → 推进天数
  → 第十日完成库存清算和主结局判定
```

白天地点包括餐馆、便利店和图书馆；夜晚可以回家，或在酒馆选择五子棋与骗子骰子之一。

## 文档入口

| 文档 | 用途 |
| --- | --- |
| [`AGENTS.md`](AGENTS.md) | 仓库级工作流、安全边界和验证规则 |
| [`DEVELOPMENT.md`](DEVELOPMENT.md) | 五人团队协作、分支、评审、测试和交付规范 |
| [`CONTEXT.md`](CONTEXT.md) | 统一领域术语、关系和不变量 |
| [`docs/README.md`](docs/README.md) | 文档地图与事实来源顺序 |
| [`docs/BUILD_AND_RUN.md`](docs/BUILD_AND_RUN.md) | Windows/macOS 从零编译、测试和运行指南 |
| [`docs/PROJECT_PLAN.md`](docs/PROJECT_PLAN.md) | P0-P4 阶段门槛、依赖图和 issue 索引 |
| [`docs/DESIGN.md`](docs/DESIGN.md) | 核心架构、状态机、模块契约和数据流 |
| [`docs/CHARACTER_DIALOGUE_PLAN.md`](docs/CHARACTER_DIALOGUE_PLAN.md) | 人物小人占位规格、共享对话、固定 NPC、室内移动/邻近互动与 P4 实施切片 |
| [`docs/SCENE_ASSET_PRODUCTION_PLAN.md`](docs/SCENE_ASSET_PRODUCTION_PLAN.md) | 三地点 imagegen 分件生成、人工拼接和视觉替换门槛 |
| [`docs/VISUAL_PROTOTYPE.md`](docs/VISUAL_PROTOTYPE.md) | P0 三套视觉原型、候选调色板和资源规范 |
| [PRD](.scratch/pixel-town-ten-day-plan/PRD.md) | 产品范围、75 条用户故事和验收方向 |
| [实施 Issues](.scratch/pixel-town-ten-day-plan/issues/) | 33 个依赖有序的本地 Markdown issue |

## 技术基线

- 语言：C++17
- 构建系统：CMake
- Windows 标准环境：Visual Studio 2022 / MSVC
- 图形、输入和音频：raylib 6.0
- 自动化测试：doctest 2.5.2 + CTest
- 目标平台：Windows 10/11 x64
- 开发支持：macOS

raylib 和 doctest 以锁定版本随仓库保存在 [`third_party/`](third_party/)，确保首次构建不依赖网络或用户全局安装。具体约束见 [ADR-0001](docs/adr/0001-adopt-raylib-and-doctest.md)；酒馆运行期与结算边界见 [ADR-0002](docs/adr/0002-use-explicit-tavern-runtime-and-settlement-modules.md)。

## 开始协作

1. 阅读 `AGENTS.md`。
2. 阅读 `DEVELOPMENT.md`。
3. 从 `docs/README.md` 定位当前任务需要的文档。
4. 阅读 `CONTEXT.md` 和目标 issue。
5. 检查 issue 的 `Blocked by`；只有真实阻塞项完成后才能开始。
6. 使用短期 feature branch，完成验收标准和相关测试后再请求合并。

issue 和 PRD 使用仓库内 `.scratch/` Markdown 文件管理，不发布为 GitHub Issues。代码通过私有 GitHub 仓库协作。

## 构建说明

前提：CMake 3.25 或更高版本，以及支持 C++17 的编译器。raylib 与 doctest 已随仓库提供；配置和构建过程不需要网络，也不要求全局安装这两个依赖。

完整的 Windows/macOS 从零配置、构建、测试、运行和故障排查流程见 [`docs/BUILD_AND_RUN.md`](docs/BUILD_AND_RUN.md)。常用命令如下。

macOS（Apple Clang）：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/pixel_town
```

Windows 10/11 x64（Visual Studio 2022 Developer PowerShell）：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --parallel
ctest --test-dir build -C Debug --output-on-failure
.\build\Debug\pixel_town.exe
```

五日展示版使用独立构建目录，第五个完整昼夜后进入同一套库存清算和正式结局；默认十日版不受影响。macOS：

```bash
cmake -S . -B build-five-day -DCMAKE_BUILD_TYPE=Release -DPIXEL_TOWN_DAY_LIMIT=5 -DPIXEL_TOWN_BUILD_TESTS=OFF
cmake --build build-five-day --target pixel_town --parallel
./build-five-day/pixel_town_five_day
```

Windows：

```powershell
cmake -S . -B build-five-day -G "Visual Studio 17 2022" -A x64 -DPIXEL_TOWN_DAY_LIMIT=5 -DPIXEL_TOWN_BUILD_TESTS=OFF
cmake --build build-five-day --target pixel_town --config Release --parallel
.\build-five-day\Release\pixel_town_five_day.exe
```

Windows Release 交付目录可通过安装目标生成；该目录包含可执行文件和运行时 `assets/`，可直接复制到其他 Windows 10/11 x64 机器：

```powershell
cmake --install build --config Release --prefix dist\pixel-town-ten-day
cmake --install build-five-day --config Release --prefix dist\pixel-town-five-day
```

应用默认创建 960×540 窗口，并以 1:1 点采样显示 960×540 逻辑画布。删除 `build/` 后重复以上命令即可验证干净构建；构建目录和本机工具目录 `.tools/` 均不会提交。

普通构建进入当前十日完整循环：标题页点击“新游戏”进入第一日地图，选择一个白天地点完成工作；夜晚可以回家，也可以在酒馆完成一局五子棋或一场骗子骰子淘汰赛；第十日总结后完成库存清算并进入唯一正式主结局。五日展示构建复用相同玩法与结局，只把最终日压缩为第 5 天。

当前快捷键：室内大厅使用 `WASD` 或方向键移动，靠近 NPC、桌面、床铺或出口后按 `E`/`Space` 互动；`Enter` 确认/继续，`Esc` 返回、取消、跳过对话或放弃当前玩法（不会关闭整个游戏），`P` 暂停/继续，`M` 切换全局静音，`F3` 显示/隐藏当前可行走边界、静态碰撞体、出口和主角碰撞框。鼠标热点和按钮仍提供等价路径；窗口缩放使用整数倍显示，黑边区域点击不会触发控件。退出程序请使用系统窗口关闭按钮，静音设置保存到应用目录旁的 `saves/settings.ini`。

便利店可直接点击商品行、进货 `- / +` 和“低/标/高”价格按钮；键盘仍可使用 `1-4` 选择商品、`A/D` 调进货、`Q/W/E` 调价格。锁定方案前会持续显示预算与操作反馈，锁定后点击“结算销售”。

P0 视觉原型仍可通过 `./build/pixel_town --capture-prototype` 生成评审截图。原型已接入 Kenney Tiny Town 和 Tiny Farm 作为 CC0 视觉参考素材，来源见 [`CREDITS.md`](CREDITS.md)。原型仍需人工批准，不代表最终 UI。

原型评审截图可通过 `./build/pixel_town --capture-prototype` 生成到被 Git 忽略的 `prototype-captures/`。
P1 核心闭环截图可通过 `./build/pixel_town --capture-game-flow` 生成到被 Git 忽略的 `game-flow-captures/`，用于复查标题、地图和结局页的文字可读性。
地点 UI 诊断截图可通过 `./build/pixel_town --capture-ui-diagnostics` 生成到被 Git 忽略的 `ui-diagnostics-captures/`；当前共 60 张，其中包含四个通用场景大厅、人物待机/对话槽、五套碰撞覆盖图、地点剧情、三类家中访客头像、酒馆玩法，以及六张主角靠近餐馆老板、便利店店主、图书管理员、酒馆双桌和家中床铺的移动/邻近互动证据。

演示预设只能通过显式命令行参数加载，不出现在普通玩家菜单中，也不会读取或覆盖正式自动存档：

```bash
./build/pixel_town --demo-preset midgame
./build/pixel_town --demo-preset ending-eve
```

预设文件位于 [`assets/data/demo_presets/`](assets/data/demo_presets/)，使用与自动存档相同的版本化文本格式；缺失、损坏或版本不兼容会在标题页提示，并写入 `logs/latest.log`。

## 启动资源与日志

应用从仓库根目录启动时验证 [`assets/`](assets/) 下的基线资源。必要贴图或数据缺失、为空或签名无效时，只显示资源错误页；可选音频包任一文件缺失、签名无效或音频设备初始化失败时，以整包静音模式继续并记录到 `logs/latest.log`。

每次启动覆盖 `logs/latest.log`，只记录应用版本、启动阶段、资源状态和相对资源路径。`logs/` 是本地运行目录，不提交、不保留历史文件，也不会发送网络请求或遥测。

## 范围边界

MVP 只包含五个固定室内场景的有限主角移动和静态矩形碰撞；不包含小镇地图/开放世界移动、NPC 巡逻、通用寻路、动态避障、复杂 NPC 关系、联网、多玩家、背包、成就、多语言、多存档槽、第三种酒馆游戏或运行时数值编辑器。完整范围见 PRD。

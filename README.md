# 《像素小镇：十日经营计划》

四川大学计算机科学实训项目：一款桌面端、单人、像素风生活模拟经营游戏。

玩家作为初到小镇的新居民，在十个游戏日内安排白天工作和夜晚活动，管理金钱、体力、声望、知识和心情，并根据最终经营表现触发一个主结局。

## 当前状态

项目处于 **P1 核心闭环实现阶段**：

- 产品范围、领域语言和核心设计已确认。
- P0-P4 已拆分为 19 个本地实施 issue。
- C++17/CMake 应用、raylib 6.0、doctest 2.5.2 和跨平台 CI 已初始化。
- Issue 01 的 macOS 本地配置、构建、CTest，以及 GitHub Actions 上的 Windows 2022/MSVC 与 macOS/Apple Clang 矩阵验证已通过。
- Issue 02 已接入启动资源诊断、本地 `logs/latest.log` 和可选音频静音降级。
- Issue 04 已跑通从标题创建新游戏、第一日地图、模拟白天工作、回家休息、每日总结到第二日开始的核心闭环。
- Issue 05 已扩展为固定十日周期，第十日完整结算后进入占位主结局和最终状态总结。
- Issue 06 已实现阶段边界自动存档与恢复，正式槽位位于应用目录旁 `saves/slot1.sav`。
- Issue 07 的窗口缩放、逻辑输入、暂停/失焦冻结和静音设置外壳已实现并通过自动化测试；最终视觉批准转入 Issue 03/17。
- Issue 08 已实现显式演示预设加载，演示模式不读取也不覆盖正式自动存档。
- Issue 19 已接入 P1 MVP 主线剧情骨架：标题开场、十天每日提示、占位地点摘要、第十天评议会和占位结局文案；P2/P3 地点逻辑由组员在占位 UI 下并行推进。

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
| [`docs/VISUAL_PROTOTYPE.md`](docs/VISUAL_PROTOTYPE.md) | P0 三套视觉原型、候选调色板和资源规范 |
| [PRD](.scratch/pixel-town-ten-day-plan/PRD.md) | 产品范围、70 条用户故事和验收方向 |
| [实施 Issues](.scratch/pixel-town-ten-day-plan/issues/) | 19 个依赖有序的本地 Markdown issue |

## 技术基线

- 语言：C++17
- 构建系统：CMake
- Windows 标准环境：Visual Studio 2022 / MSVC
- 图形、输入和音频：raylib 6.0
- 自动化测试：doctest 2.5.2 + CTest
- 目标平台：Windows 10/11 x64
- 开发支持：macOS

raylib 和 doctest 以锁定版本随仓库保存在 [`third_party/`](third_party/)，确保首次构建不依赖网络或用户全局安装。具体约束见 [ADR-0001](docs/adr/0001-adopt-raylib-and-doctest.md)。

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

应用默认创建 960×540 窗口，并以 1:1 点采样显示 960×540 逻辑画布。删除 `build/` 后重复以上命令即可验证干净构建；构建目录和本机工具目录 `.tools/` 均不会提交。

普通启动进入 P1 核心闭环界面：标题页点击“新游戏”进入第一日地图，选择白天地点后可完成模拟工作或主动放弃，夜晚回家休息后进入每日总结；第十日总结后进入占位主结局，不进入第十一日。

当前快捷键：`Enter` 确认/继续，`Esc` 返回或取消覆盖确认，`Space` 开始/完成地点模拟，`P` 暂停/继续，`M` 切换全局静音。鼠标仍是主要操作方式；窗口缩放使用整数倍显示，黑边区域点击不会触发控件。静音设置保存到应用目录旁的 `saves/settings.ini`。

P0 视觉原型仍可通过 `./build/pixel_town --capture-prototype` 生成评审截图。原型已接入 Kenney Tiny Town 和 Tiny Farm 作为 CC0 视觉参考素材，来源见 [`CREDITS.md`](CREDITS.md)。原型仍需人工批准，不代表最终 UI。

原型评审截图可通过 `./build/pixel_town --capture-prototype` 生成到被 Git 忽略的 `prototype-captures/`。
P1 核心闭环截图可通过 `./build/pixel_town --capture-game-flow` 生成到被 Git 忽略的 `game-flow-captures/`，用于复查标题、地图和结局页的文字可读性。

演示预设只能通过显式命令行参数加载，不出现在普通玩家菜单中，也不会读取或覆盖正式自动存档：

```bash
./build/pixel_town --demo-preset midgame
./build/pixel_town --demo-preset ending-eve
```

预设文件位于 [`assets/data/demo_presets/`](assets/data/demo_presets/)，使用与自动存档相同的版本化文本格式；缺失、损坏或版本不兼容会在标题页提示，并写入 `logs/latest.log`。

## 启动资源与日志

应用从仓库根目录启动时验证 [`assets/`](assets/) 下的基线资源。必要贴图或数据缺失、为空或签名无效时，只显示资源错误页；可选音频缺失时以静音模式继续。

每次启动覆盖 `logs/latest.log`，只记录应用版本、启动阶段、资源状态和相对资源路径。`logs/` 是本地运行目录，不提交、不保留历史文件，也不会发送网络请求或遥测。

## 范围边界

MVP 不包含开放世界移动、复杂 NPC 关系、联网、多玩家、背包、成就、多语言、多存档槽、第三种酒馆游戏或运行时数值编辑器。完整范围见 PRD。

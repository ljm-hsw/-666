# 《像素小镇：十日经营计划》

四川大学计算机科学实训项目：一款桌面端、单人、像素风生活模拟经营游戏。

玩家作为初到小镇的新居民，在十个游戏日内安排白天工作和夜晚活动，管理金钱、体力、声望、知识和心情，并根据最终经营表现触发一个主结局。

## 当前状态

项目处于 **P0 工程基线验证阶段**：

- 产品范围、领域语言和核心设计已确认。
- P0-P4 已拆分为 18 个本地实施 issue。
- C++17/CMake 应用、raylib 6.0、doctest 2.5.2 和跨平台 CI 已初始化。
- macOS 本地配置、构建和 CTest 已通过；Windows 基线由 GitHub Actions 验证。

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
| [`docs/PROJECT_PLAN.md`](docs/PROJECT_PLAN.md) | P0-P4 阶段门槛、依赖图和 issue 索引 |
| [`docs/DESIGN.md`](docs/DESIGN.md) | 核心架构、状态机、模块契约和数据流 |
| [PRD](.scratch/pixel-town-ten-day-plan/PRD.md) | 产品范围、70 条用户故事和验收方向 |
| [实施 Issues](.scratch/pixel-town-ten-day-plan/issues/) | 18 个依赖有序的本地 Markdown issue |

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

应用默认创建 1280×720 窗口，并用点采样放大 640×360 逻辑画布。删除 `build/` 后重复以上命令即可验证干净构建；构建目录和本机工具目录 `.tools/` 均不会提交。

## 范围边界

MVP 不包含开放世界移动、复杂 NPC 关系、联网、多玩家、背包、成就、多语言、多存档槽、第三种酒馆游戏或运行时数值编辑器。完整范围见 PRD。

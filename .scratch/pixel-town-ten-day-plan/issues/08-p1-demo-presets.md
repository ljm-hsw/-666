# P1：实现隔离的演示预设加载

Status: done
Milestone: P1
Type: AFK
User stories: 51, 52, 66

## What to build

允许现场演示者通过显式命令行参数加载仓库内的只读演示预设，直接进入指定游戏日、阶段或结局前状态。演示预设不出现在普通菜单，不读取、不覆盖也不更新玩家自动存档。

## Acceptance criteria

- [x] 合法参数可加载至少一个中期状态和一个结局前状态。
- [x] 未提供演示参数时，应用只使用正常新游戏或继续游戏流程。
- [x] 演示预设不出现在普通玩家菜单，也不提供隐藏作弊按钮。
- [x] 加载和游玩演示预设前后，正式自动存档内容保持不变。
- [x] 演示预设使用固定种子并能稳定复现当日上下文。
- [x] 不存在、损坏或版本不兼容的预设会产生明确错误和日志记录。
- [x] 自动化测试比较演示运行前后的正式存档，证明二者隔离。
- [x] 构建说明记录演示参数用法，但明确其不属于普通游戏流程。

## Blocked by

- [05 P1：扩展为十日周期与占位主结局](05-p1-ten-day-cycle.md)
- [06 P1：实现阶段边界自动存档与恢复](06-p1-save-and-resume.md)

## Verification record

2026-07-07 在 macOS 15.7.4 / Apple Clang 17.0.0 上完成：

- 新增 `--demo-preset <id>` 显式参数；未提供该参数时仍走普通新游戏/继续游戏流程。
- 新增仓库内只读预设 `assets/data/demo_presets/midgame.sav` 和 `assets/data/demo_presets/ending-eve.sav`，均使用固定种子 `20260707`。
- 演示模式禁用正式自动存档读取与写入；自动化测试比较正式 `saves/slot1.sav` 内容，确认加载和游玩演示预设不会覆盖玩家进度。
- 不存在、损坏或版本不兼容的演示预设返回明确错误；启动日志支持记录 `launch_note`。
- README 记录演示参数用法，并说明该入口不属于普通菜单流程。
- `.tools/CMake.app/Contents/bin/cmake --build build --parallel 4` 通过。
- `.tools/CMake.app/Contents/bin/ctest --test-dir build --output-on-failure` 通过，1/1 测试成功。
- `./build/pixel_town --capture-game-flow` 通过，确认普通 P1 截图路径未受影响。

# P1：跑通一个完整游戏日

Status: done
Milestone: P1
Type: AFK
User stories: 1-10, 28, 35, 53, 54

## What to build

交付第一个真正的游戏纵向闭环：玩家从标题开始新游戏，在小镇地图查看状态，选择一个返回模拟行动结果的白天工作，完成回家休息，并看到当日总结后进入下一游戏日。该切片建立游戏会话、阶段状态机、玩家状态、当日上下文和行动结果的稳定契约。

地点内容仍是模拟结果，但必须经过与真实地点相同的进入、完成、结果应用和场景返回路径。

## Acceptance criteria

- [x] 玩家可以从标题创建新游戏并进入第一日地图。
- [x] 地图持续展示天数、阶段、金钱、体力、声望、知识和心情。
- [x] 当前阶段只开放合法地点，非法地点给出原因。
- [x] 模拟白天工作生成一个行动结果，由核心系统应用后切换到夜晚选择。
- [x] 玩家可以回家休息，完成夜晚结果和每日结算并进入第二日。
- [x] 同一白天或夜晚行动不能重复，重复提交同一行动结果会被拒绝。
- [x] 进入地点前返回不消耗阶段；地点开始后主动放弃消耗阶段且无收益。
- [x] 领域状态机和行动结果测试不依赖 raylib 窗口。
- [x] 一条集成测试覆盖新游戏到第二日开始的完整路径。

## Blocked by

- [01 P0：建立可离线复现的应用基线](01-p0-offline-app-baseline.md)

## Verification record

2026-07-07 在 macOS 15.7.4 / Apple Clang 17.0.0 上完成：

- 新增 raylib-free `GameSession` 领域核心，覆盖新游戏、阶段合法入口、地点开始、模拟行动结果、重复结果拒绝、回家休息、每日总结和进入第二日。
- 普通应用启动进入 P1 标题/地图/地点/总结流程；`--capture-prototype` 继续保留 P0 视觉原型截图路径。
- `.tools/CMake.app/Contents/bin/cmake --build build --parallel 4` 通过。
- `.tools/CMake.app/Contents/bin/ctest --test-dir build --output-on-failure` 通过，1/1 测试成功；`game_session_test.cpp` 包含一条新游戏到第二日开始的完整路径。

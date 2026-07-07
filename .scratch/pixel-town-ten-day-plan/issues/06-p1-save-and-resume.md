# P1：实现阶段边界自动存档与恢复

Status: done
Milestone: P1
Type: AFK
User stories: 39-41, 55, 65

## What to build

为游戏会话建立单槽位、带版本号的行式纯文本存档。应用在已确认的阶段边界原子保存；重新启动后恢复最近完成的边界。新游戏覆盖前必须确认，损坏或不兼容存档必须保留原文件并显示明确错误。

## Acceptance criteria

- [x] 存档包含格式版本、随机种子、游戏日、阶段、玩家状态、店铺库存和酒馆战绩所需字段。
- [x] 日初、白天结果应用后、夜晚结果应用后和每日结算后产生可恢复快照。
- [x] 写入中断不会用半写文件覆盖最后一个有效存档。
- [x] 重新启动后恢复到最近阶段边界，且同一随机种子生成相同未完成内容。
- [x] 新游戏检测到现有进度时明确要求确认，取消后原进度不变。
- [x] 损坏、缺字段或版本不兼容时保留原文件并展示可理解错误。
- [x] 存档位于发布目录旁的便携用户数据位置，不写入系统级目录。
- [x] 自动化测试覆盖往返、覆盖取消、原子替换、损坏输入和版本错误。

## Blocked by

- [04 P1：跑通一个完整游戏日](04-p1-one-day-vertical-loop.md)

## Evidence

- Added `src/io/save_game.hpp` and `src/io/save_game.cpp`.
- Added `tests/save_game_test.cpp` covering round trip, future-module placeholder fields, overwrite cancellation, corrupt input, incompatible version, and portable default path.
- Application startup now restores `saves/slot1.sav` when valid, shows a readable title-page error when invalid, and saves only on confirmed phase boundaries.
- Save format currently stores `store_inventory`, `tavern_wins`, and `tavern_losses` as versioned placeholder fields; Issue 10 and Issue 12 can replace these placeholders with real module data without changing the single-slot contract.
- Validation: `.tools/CMake.app/Contents/bin/cmake --build build --parallel 4` and `.tools/CMake.app/Contents/bin/ctest --test-dir build --output-on-failure` passed on 2026-07-07.

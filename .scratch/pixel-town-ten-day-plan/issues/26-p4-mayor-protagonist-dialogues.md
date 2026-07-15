# P4：接入镇长与主角生命周期对话

Status: done
Milestone: P4
Type: AFK
User stories: 1, 7, 9, 10, 28, 42, 57, 58

## What to build

使用共享对话系统接入新游戏时镇长与主角的十日约定，以及选择回家休息时的主角短独白。新游戏对话结束后进入第一日地图；回家独白与既有休息结果、每日总结和保存边界组合成一个可解释流程。第十日评议会和主结局继续使用现有结局文本入口，不新增阶段或可领取奖励。

镇长和主角先使用 32×32 占位小人和统一 fallback；最终角色资产、表情和对话框装饰留给 Issue 17 人工批准。

## Acceptance criteria

- [x] 新游戏确认后展示镇长与主角的 3-4 句线性对话，继续或跳过后只进入一次第一日地图。
- [x] 开场对话不绕过已有存档覆盖确认，不会提前创建重复行动或重复保存。
- [x] 夜晚选择回家时先展示 1-2 句主角独白，完成或跳过后只提交一次既有回家结果，并保持体力、心情、夜晚槽和每日总结语义。
- [x] 对话打开时优先消费鼠标、Enter/Space 和 Esc，不同时触发地图地点或总结按钮。
- [x] 对话前后除原本的新游戏创建或回家行动外，不产生额外属性、阶段、库存或战绩变化。
- [x] 第十日评议会和唯一主结局仍由现有结局流程生成，不新增 `GamePhase`。
- [x] 首版不保存当前对话行号或已读标记；阶段边界恢复后允许重播无奖励对话，且结果确定。
- [x] 镇长、主角和回家独白文本遵循温暖、日常、不评判的剧情风格。
- [x] 人物素材缺失时显示统一 fallback；所有台词进入字形清单并满足三行/分页限制。
- [x] 自动化测试覆盖新游戏到地图、回家到每日总结、存档恢复和第十日结局未回归。
- [x] 诊断截图覆盖镇长/主角交替说话、回家独白和 fallback。

## Blocked by

- [05 P1：扩展为十日周期与占位主结局](05-p1-ten-day-cycle.md)
- [15 P3：完成库存清算与七种主结局](15-p3-endings.md)
- [19 P1：接入 MVP 主线剧情骨架](19-p1-mvp-story-shell.md)
- [23 P4：用酒保切片建立共享人物对话运行期](23-p4-shared-dialogue-bartender-slice.md)

## Comments

<!-- 在此行下方追加实现与验证记录。 -->

- 2026-07-11：按用户决定随 Issue 24-28 一并暂缓，先完成三地点视觉素材生产和人工拼接。用户明确恢复后改回 `ready-for-agent`；生命周期对话范围和 v1 存档边界不变。
- 2026-07-14：用户明确要求继续下一个 TDD Issue，恢复本切片；对话沿用现有主线规划，支线、好感度、任务链和额外持久化状态继续延期。
- 2026-07-14：新增 raylib-free `StoryLifecycleRuntime`。镇长与主角四句开场覆盖在第一日地图上；主角两句回家独白覆盖在家中背景上。对话 active 时输入优先返回，不会触发地图、休息或总结控件。
- 2026-07-14：开场完成只关闭 overlay，不修改 `GameSession`；第一日未执行白天行动的恢复边界允许无奖励重播。回家独白完成或跳过后才调用一次既有 `home_rest_result()`，快照与直接调用旧规则完全一致，重复输入被拒绝。
- 2026-07-14：人物继续使用共享程序 fallback，最终镇长/主角纹理和表情由 Issue 17 人工批准。诊断截图 `mayor-dialogue.png`、`protagonist-opening-dialogue.png`、`home-reflection-dialogue.png` 已逐张复查，无缺字、重叠或越框。
- TDD RED：生命周期文本测试先因缺少 `mayor_new_game_intro` / `home_rest_reflection` 编译失败；运行期测试再因缺少 `story_lifecycle_runtime.hpp` 失败；回家结算测试因缺少 `step_story_lifecycle` 失败；恢复测试最后因缺少 `should_replay_new_game_opening` 失败。
- TDD GREEN：`./build/pixel_town_tests` 共 240 个测试、2627 个断言全部通过；`ctest --test-dir build --output-on-failure` 通过 1/1；`./build/pixel_town --capture-ui-diagnostics` 成功生成全部 38 张诊断截图。
- 2026-07-15：镇长开场保持新游戏主线入口；现有回家独白作为首访基线，后续游戏日的家庭日记、天气和回收事件由 Issue 30/32 扩展，休息只结算一次的契约不变。

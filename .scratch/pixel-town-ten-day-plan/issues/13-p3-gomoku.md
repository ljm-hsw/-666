# P3：接入自由五子棋和启发式电脑对手

Status: done
Milestone: P3
Type: AFK
User stories: 30, 31, 34, 38, 54, 61

## What to build

把酒馆中的模拟五子棋结果替换为可完成的玩家对电脑自由五子棋。规则引擎负责棋盘、合法落子、轮次、横竖斜五连、和局和确定性电脑决策；UI 负责显示与输入，酒馆外壳负责赌注和全局行动结果。

电脑依次选择立即获胜、阻止玩家立即获胜和位置评分最佳落点，不实现禁手、高深度 Minimax 或多个难度等级。

## Acceptance criteria

- [x] 玩家从酒馆选择五子棋后完成一局，结束后返回统一酒馆结算。
- [x] 已占用、越界或非当前回合落子被拒绝且不改变棋盘。
- [x] 横、竖和两种斜线五连均正确判胜，棋盘满且无人获胜时判平局。
- [x] 不实现禁手，玩家和电脑遵循同一落子合法性规则。
- [x] 电脑优先立即获胜，其次立即防守，否则使用稳定位置评分。
- [x] 相同棋盘状态与配置产生相同电脑落点。
- [x] 暂停、失焦和最小化不推进电脑回合或动画。
- [x] 棋局规则不直接修改金钱、心情或酒馆战绩。
- [x] 自动化测试覆盖合法性、所有胜线、和局及电脑决策优先级。
- [x] 集成测试覆盖酒馆进入、完整棋局、胜负结果和阶段结算。

## Blocked by

- [12 P3：接入酒馆选择、赌注与夜晚结算](12-p3-tavern-shell.md)

## Verification notes

- 2026-07-10：以 `origin/dev-ckz@6ebaddd` 的玩法实现为输入，规则移植到无 raylib 的 `src/locations/gomoku_rules.*`，运行期接入现有 `LocationRuntimeState` 和统一 `ActionResult` 结算。
- 使用当前 640×360 设计网格映射到 960×540 逻辑画布，修正队友分支背景与热点缩放不一致的问题。
- macOS Debug 构建、CTest、规则/集成测试和 `--capture-ui-diagnostics` 酒馆截图通过；Windows 结果由后续 CI 验证。
- 2026-07-10：Issue 22 通过 `TavernRuntime` 的 public Interface 驱动完整棋局，并由独立 Settlement Module 生成既有 `ActionResult`；五子棋规则、电脑决策和数值行为未改变。

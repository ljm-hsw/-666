# P3：深化酒馆运行期与挑战结算架构

Status: done
Milestone: P3
Type: AFK
User stories: 28-34, 36, 38, 54, 61

## What to build

在不改变统一 `ActionResult`、v1 存档格式和现有赌注/胜负数值的前提下，收口酒馆挑战的运行期状态、输入、只读展示和夜晚结算责任。

`TavernRuntime` 使用 `open / step / presentation / active` 的窄 Interface 管理酒馆大厅、挑战选择、五子棋和骗子骰子流程；raylib 输入先转换为显式 `TavernFrameInput`，展示层只读取 `TavernPresentation`。酒馆挑战结算由 `src/locations/` 下的 raylib-free Module 生成行动结果，最终仍由 `GameSession` 唯一校验和应用全局状态。

本 issue 不引入全项目通用地点或挑战扩展系统；MVP 保持已经确认的两种酒馆挑战。

## Acceptance criteria

- [x] `TavernRuntime` 的 public Interface 收敛为 `open / step / presentation / active`，运行期可写状态保持私有。
- [x] `TavernFrameInput` 显式表达帧时间、暂停/更新开关、鼠标位置和键盘意图，Runtime 不直接调用 raylib 输入或计时函数。
- [x] `TavernPresentation` 只提供绘制需要的只读数据，骗子骰子揭晓前不暴露电脑骰面。
- [x] `pixel_town_tavern_logic` 目标不链接 raylib，可在无窗口测试中运行酒馆流程。
- [x] `TavernChallengeSettlement` 位于 `src/locations/`，分别接收真实五子棋或骗子骰子状态；未终局状态被拒绝，挑战类型和胜负由真实终局推导，配置、结果 ID、赌注和现金集中校验。
- [x] `GameSession` 仍是玩家状态、酒馆战绩和阶段推进的唯一写入者；Settlement 和 Runtime 不绕过 `apply_action_result()`。
- [x] 五子棋和骗子骰子继续使用现有规则 Module、固定种子和数值平衡，不改变玩法语义。
- [x] 酒馆结算被核心拒绝时，Runtime 保持 active，保留终局棋盘/骰局与反馈，不退回大厅或丢失终局。
- [x] 终局首次构造的候选 `ActionResult` 缓存在 Runtime；核心接受前重试不重新读取玩家状态或重建结果。
- [x] 开始前返回地图不消耗夜晚；开始后主动放弃只有在核心接受放弃结果后才结束酒馆会话。
- [x] `TavernVisualAssets` 与玩法状态分离，由 `LocationRuntimeState` 持有并通过 `tavern_view` 加载、卸载和绘制。
- [x] 诊断截图通过 `open / step` 构造大厅、选择、五子棋和骗子骰子状态，不直接写 Runtime 私有字段。
- [x] 自动化测试覆盖 Runtime 进入/开始、核心拒绝后终局保留、电脑骰隐藏和固定种子骗子骰子完整夜晚。
- [x] 自动化测试覆盖真实游戏终局推导、未终局拒绝、成功构造、资金不足、非法结果 ID、非法配置和非法赌注；骗子骰子的真实终局另由 Runtime 完整夜晚集成测试覆盖。
- [x] CMake 构建、CTest、`git diff --check` 和酒馆诊断截图验证通过。

## Blocked by

- [12 P3：接入酒馆选择、赌注与夜晚结算](12-p3-tavern-shell.md)
- [13 P3：接入自由五子棋和启发式电脑对手](13-p3-gomoku.md)
- [14 P3：接入 1v1 骗子骰子](14-p3-liars-dice.md)

## Verification notes

- 2026-07-10：新增 `src/locations/tavern_challenge_settlement.*` 和对应无窗口测试；Settlement 从真实五子棋/骗子骰子终局推导胜负并拒绝进行中状态，原有 `ActionResult`、v1 存档字段与 `TavernChallengeConfig` 数值未修改。
- 2026-07-10：`TavernRuntime` 改为显式帧输入和只读 presentation，五子棋/骗子骰子完整流程通过 public Interface 驱动。
- 2026-07-10：新增独立 `pixel_town_tavern_logic` 目标；raylib 纹理迁移到 `TavernVisualAssets`，由 app 展示层管理。
- 2026-07-10：诊断截图不再直接写酒馆运行期字段；终局候选 `ActionResult` 会缓存到核心接受为止，拒绝结算时终局和原候选结果保持可见、可重试、可诊断。

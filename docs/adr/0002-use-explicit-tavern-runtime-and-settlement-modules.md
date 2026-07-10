---
status: accepted
---

# 酒馆采用显式运行期 Interface 与独立结算 Module

Issue 13/14 接入真实五子棋和骗子骰子后，早期酒馆运行期同时公开页面、棋局、骰局、计时、反馈和纹理句柄，raylib 输入与玩法状态推进位于同一 Implementation。酒馆胜负到统一 `ActionResult` 的转换还经过只做转发的 helper，核心拒绝结果时运行期可能提前丢失终局状态。这些做法降低了 Runtime Interface 的 Depth，也让无窗口集成测试和诊断 fixture 依赖内部字段。

## Decision

- 使用专用 `TavernRuntime` Module 管理酒馆夜晚活动，其 public Interface 固定为 `open / step / presentation / active`。
- Runtime 接收 raylib-free 的 `TavernFrameInput`，把大厅、挑战选择、五子棋、骗子骰子、主动放弃和终局确认的 ordering knowledge 隐藏在同一 seam 后。
- Runtime 直接持有当前两种已确认挑战的规则状态，不为尚未发生的变化新增通用挑战扩展 seam。
- `TavernPresentation` 是展示 Adapter 的只读输入；它只暴露允许玩家看见的信息，骗子骰子揭晓前不包含电脑骰面。
- 酒馆挑战结算由 `src/locations/tavern_challenge_settlement.*` 的 raylib-free Module 负责。它分别接收真实 `GomokuGame` 或 `LiarsDiceGame`，拒绝未终局状态，从真实终局推导挑战类型与胜负，再使用既有 `TavernChallengeConfig` 构造统一夜晚 `ActionResult`，但不应用全局状态。
- `GameSession` 保持玩家状态、酒馆战绩和阶段推进的唯一写入者。Runtime 在终局首次构造后缓存候选 `ActionResult`，只有在 `apply_action_result()` 被接受后才结束酒馆会话；拒绝时保留同一候选结果、终局和错误反馈。
- 酒馆规则与 Settlement 编入无窗口地点规则目标；Runtime 与布局编入不链接 raylib 的 `pixel_town_tavern_logic`，raylib 输入转换和绘制留在 app 展示层。
- `TavernVisualAssets` 与玩法状态分离，由 `LocationRuntimeState` 持有，`tavern_view` 负责加载、卸载、fallback 和绘制。
- 诊断 fixture 必须通过 `open / step` 建立状态，不直接写 Runtime 私有字段。
- 本决定不改变 `ActionResult`、v1 存档格式、阶段边界存档策略或数值平衡参数。进行中的酒馆挑战仍不保存，异常关闭恢复最近阶段边界并使用相同地点种子重新开始。

## Consequences

- Runtime Interface 变小，真实输入、诊断和无窗口测试共享同一测试 surface；结算、放弃、候选结果身份和核心拒绝的行为具有更高 Locality。
- 酒馆纹理生命周期不再污染玩法状态，规则和运行期逻辑可以独立于 raylib 验证。
- 扩展酒馆挑战范围时需要显式扩展 Runtime、Presentation 和 View。这是当前产品范围下有意接受的取舍，比维护尚无第二种 Adapter 的抽象 seam 更简单。
- 不建立全项目 `ILocation`。其他地点只有在出现相同且真实变化的 Adapter 后，才单独评估是否需要类似 Interface。

## Rejected alternatives

- **全项目 `ILocation`**：四个地点的生命周期和展示差异较大，当前会形成只有单一 Adapter 的 hypothetical seam。
- **通用挑战扩展 seam**：产品边界明确只有五子棋和骗子骰子；额外扩展系统会扩大 Interface，而当前没有另一个生产 Adapter 提供 Leverage。
- **在 Runtime 中继续持有纹理**：会把资源生命周期、复制语义和玩法状态绑在一起，降低 Locality，并迫使无窗口测试链接 raylib。
- **由调用者直接提交挑战类型和胜负**：无法证明结果来自真实终局，也会让进行中的挑战绕过规则；Settlement 必须从规则状态推导。
- **核心拒绝后退出终局或重建候选结果**：前者会丢失可诊断状态，后者可能因玩家状态变化产生不同结算；二者都破坏重试的稳定身份。

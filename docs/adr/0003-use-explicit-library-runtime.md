---
status: accepted
---

# 图书馆采用专用地点运行期与共享工作结果

图书馆同时拥有读者咨询和书籍整理后，一次行动的顺序知识分散在 `LocationRuntimeState` 的多个公开字段、UI helper、两套规则对象和两套结果 Adapter 中。调用者必须自行维持模式互斥、日种子、完成条件、主动放弃和核心结果 ID；“最后一本归位后自动结束”还由 raylib UI 推断。该 Interface 暴露了过多 Implementation，诊断 fixture 和测试也依赖内部裸状态。

## Decision

- 使用专用 `LibraryRuntime` Module 管理一次图书馆白天行动，public Interface 固定为 `open / step / presentation / active`。
- Runtime 接收 raylib-free 的语义 `LibraryIntent`，隐藏模式选择、读者咨询、书籍整理、确定性上下文、反馈、完成、放弃和结果提交的 ordering knowledge。
- `LibraryPresentation` 是 UI Adapter 的只读输入；raylib 层只负责输入命中、布局和绘制，不直接推进规则或判断玩法完成。
- `LibraryRuleEngine` 与 `LibraryOrganizingSession` 都产生共享 `LibraryWorkResult`，只保留一处到核心 `ActionResult` 的字段映射。
- 整理规则在最后一本正确归位时返回显式 `completed` 状态，Runtime 当次自动提交结果。UI 不再维护第二套完成 helper。
- 主动放弃是两种规则中的显式状态；即使已有部分答题或整理进度，也返回零收益的 abandoned 结果。
- `GameSession` 继续作为玩家状态、阶段和结果 ID 的唯一写入者；本决定不改变 `ActionResult`、v1 存档格式、数值参数或阶段边界存档策略。
- 诊断 fixture 和无窗口生命周期测试必须通过 Runtime Interface 建立状态，不直接写读者或整理规则对象。

## Consequences

- 图书馆模式增加、完成语义、放弃语义和结算修复集中在一个 seam，调用者不再了解规则对象的互斥关系。
- 读者咨询与书籍整理仍保留各自深层规则 Module，只共享能够稳定复用的生命周期结果契约。
- Presentation 当前会复制本次小型任务与类别集合；在现有固定规模下优先保证所有权安全和简单调用，只有取得性能证据后才引入引用生命周期或缓存。
- 扩展图书馆模式需要显式扩展 Intent、Presentation 与 Runtime。这是有意接受的 Locality 取舍。
- 不建立全项目 `ILocation`。餐馆、便利店和酒馆保持各自已经验证的生命周期与展示边界。

## Rejected alternatives

- **继续由 `LocationRuntimeState` 公开两套规则对象**：模式互斥、完成和清理顺序会继续泄漏到输入、诊断与测试调用者。
- **由 UI 判断整理完成并返回 `finish`**：领域终态会依赖 raylib Adapter，规则测试无法独立证明自动结束。
- **为所有地点建立统一 `ILocation`**：各地点输入、阶段和结算差异仍然很大，会形成没有多个真实 Adapter 支撑的 hypothetical seam。
- **继续维护咨询与整理两套结果 Adapter**：只复制相同字段，不隐藏复杂度；新增模式还会复制第三份映射。

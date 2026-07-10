# 项目文档索引

本目录是《像素小镇：十日经营计划》的工程文档入口。开始规划、实现或审查前，只读取与当前任务直接相关的文档。

## 事实来源顺序

出现冲突时按以下顺序处理，并明确报告冲突：

1. 当前可执行代码、自动化测试和构建配置。
2. 已接受的 ADR。
3. 当前阶段对应的本地 issue 和验收标准。
4. PRD、核心设计和领域上下文。
5. 阶段计划、运行说明和历史记录。

当前仓库已完成 P3 代码闭环：P1 核心循环、P2 白天地点、P3 酒馆双玩法、最终库存清算和七种正式结局已有代码与自动化测试；P4 的数值、视觉、音频、发布包与实机体验仍待人工验收。已实现部分以代码、自动化测试和构建配置为准，未实现与待人工验收部分仍以 PRD、`CONTEXT.md`、ADR 和已批准 issue 共同构成执行契约。

## 核心文档

- [`../CONTEXT.md`](../CONTEXT.md)：领域语言、关系、不变量及已澄清歧义。
- [`../.scratch/pixel-town-ten-day-plan/PRD.md`](../.scratch/pixel-town-ten-day-plan/PRD.md)：产品范围、用户故事、实现决策和测试决策。
- [`PROJECT_PLAN.md`](PROJECT_PLAN.md)：P0-P4 阶段目标、门槛、依赖和 issue 总览。
- [`BUILD_AND_RUN.md`](BUILD_AND_RUN.md)：Windows/macOS 从零编译、测试、运行、截图和常见问题。
- [`DESIGN.md`](DESIGN.md)：核心架构、状态机、模块契约、存档、随机性和测试边界。
- [`UI_ASSET_STRATEGY.md`](UI_ASSET_STRATEGY.md)：P2/P3 占位 UI、imagegen 候选资产和 P4 统一视觉收尾策略。
- [`CHARACTER_DIALOGUE_PLAN.md`](CHARACTER_DIALOGUE_PLAN.md)：P4 人物小人占位规格、共享对话、图书馆/酒馆室内导航、静态碰撞、NPC 轨迹和 Issues 23-28 实施顺序。
- [`RESTAURANT_VISUAL_SPEC.md`](RESTAURANT_VISUAL_SPEC.md)：餐馆 960×540 布局、背景、食物图标、倒计时和反馈资源槽位。
- [`story/README.md`](story/README.md)：主线剧情、文案风格、MVP 文本草案、地点/NPC、结局回声和剧情接入规划。
- [`VISUAL_PROTOTYPE.md`](VISUAL_PROTOTYPE.md)：P0 待审视觉方案、调色板、像素尺寸与资源命名候选。
- [`IMAGEGEN_VISUAL_PLAN.md`](IMAGEGEN_VISUAL_PLAN.md)：960×540 画布迁移与 imagegen 页面背景/建筑资产规划。
- [`TEAM_BRANCH_INTEGRATION.md`](TEAM_BRANCH_INTEGRATION.md)：同学分支聚合、冲突处理、保留基线和后续风险。
- [`adr/0001-adopt-raylib-and-doctest.md`](adr/0001-adopt-raylib-and-doctest.md)：raylib、doctest 与离线依赖策略。
- [`adr/0002-use-explicit-tavern-runtime-and-settlement-modules.md`](adr/0002-use-explicit-tavern-runtime-and-settlement-modules.md)：酒馆显式运行期 Interface、隐藏信息 presentation 与独立结算边界。
- [`../CREDITS.md`](../CREDITS.md)：第三方字体、贴图和许可证来源登记。

## 工作流文档

- [`../DEVELOPMENT.md`](../DEVELOPMENT.md)：五人团队的协作、分支、评审、测试与交付规范。
- [`agents/issue-tracker.md`](agents/issue-tracker.md)：本地 Markdown issue 规则。
- [`agents/triage-labels.md`](agents/triage-labels.md)：issue 状态映射。
- [`agents/domain.md`](agents/domain.md)：领域文档读取规则。

## 实施入口

实施 issue 位于 [`../.scratch/pixel-town-ten-day-plan/issues/`](../.scratch/pixel-town-ten-day-plan/issues/)，按编号和依赖关系执行。

- AFK issue 使用 `Status: ready-for-agent`，可由 Agent 按验收标准独立完成。
- HITL issue 使用 `Status: ready-for-human`，必须等待人工评审、试玩或视觉确认。
- issue 是否可启动以其 `Blocked by` 为准；阶段门槛未满足时，不得把该阶段标记为完成或交付。

## 文档维护规则

- 产品范围变化：先更新 PRD，再更新受影响 issue。
- 领域术语或关系变化：更新 `CONTEXT.md`。
- 难以逆转且存在真实取舍的架构决定：新增 ADR。
- 阶段边界或依赖变化：更新 `PROJECT_PLAN.md` 和对应 issue。
- 实现细节变化但契约不变：优先通过代码和测试表达，不扩写稳定文档。
- 数值平衡参数在 P4 固化；此前只记录可运行基线，不把临时值写成产品承诺。

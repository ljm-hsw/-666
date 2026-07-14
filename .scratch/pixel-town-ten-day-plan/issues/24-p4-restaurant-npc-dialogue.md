# P4：接入餐馆老板对话切片

Status: done
Milestone: P4
Type: AFK
User stories: 11, 12, 54, 57, 58, 61

## What to build

在共享对话运行期上接入餐馆老板。玩家进入餐馆后，先看到老板与主角的短对话，再进入现有说明和接待玩法；对话可以继续或跳过，不改变订单规则、奖励、随机种子或行动结果。

人物先使用 32×32 占位小人和统一 fallback。本 issue 不接入角色移动；便利店店主由 Issue 25 独立接入，图书馆固定管理员与点击交互由 Issue 27 端到端完成。

## Acceptance criteria

- [x] 餐馆老板有一段围绕围裙、订单和服务节奏的主角短对话。
- [x] 对话发生在餐馆倒计时开始前，关闭前顾客、订单和计时均不推进。
- [x] 餐馆复用共享剧情目录、对话运行期、对话框和人物 fallback，不创建餐馆专用对话组件。
- [x] 餐馆既有教程、返回地图、开始、结算和主动放弃行为保持不变。
- [x] 新增对话路径不使用按字节遍历 `std::string` 的中文换行；文本最多三行或分页。
- [x] 对话继续/跳过输入不会泄漏给菜品、提交上菜或返回按钮。
- [x] 对话前后玩家属性、阶段、库存、随机种子和行动结果保持不变。
- [x] 餐馆老板素材缺失时显示统一 fallback，不阻断无窗口规则测试。
- [x] 字形清单覆盖所有 NPC 与主角台词，诊断截图无缺字、重叠或越框。
- [x] 自动化测试覆盖餐馆从进入、对话、关闭到首个订单可操作的端到端路径。

## Blocked by

- [09 P2：接入完整餐馆白天工作](09-p2-restaurant-work.md)
- [23 P4：用酒保切片建立共享人物对话运行期](23-p4-shared-dialogue-bartender-slice.md)

## Comments

<!-- 在此行下方追加实现与验证记录。 -->

- 2026-07-11：按用户决定暂缓实施。当前先完成 `docs/SCENE_ASSET_PRODUCTION_PLAN.md` 的三地点风格板、分件素材与人工拼接；用户明确恢复 Issue 后再改回 `ready-for-agent`。本 issue 的范围和验收标准不变。
- 2026-07-13：餐馆已增加不消耗行动的场景大厅、餐馆老板占位热点和“进入餐馆工作”入口；点击热点目前只显示接口预留提示。正式线性台词、共享对话运行期接入与输入冻结仍由本 Issue 完成，状态暂不提前改动。
- 2026-07-14：用户确认以现有主线规划为本轮基准，允许在不新增分支状态的前提下延伸地点初识对话；NPC 支线、好感度、任务链和存档字段继续延期。
- 2026-07-14：按当前“基础功能先使用程序人物 fallback、最终人物由 Issue 17 统一替换”的视觉策略，将本 Issue 的人物验收限定为复用共享 fallback；不为仅有占位人物的餐馆提前建立空的纹理注册表。
- 2026-07-14：按 TDD 接入通用 `NpcLobbyRuntime` 和餐馆垂直切片。点击老板、主按钮或键盘入口先打开三句线性对话，关闭或跳过后才进入既有餐馆说明页；对话期间阶段、玩家快照、订单和计时不变。
- 2026-07-14：诊断截图新增 `restaurant-dialogue-first.png` 与 `restaurant-dialogue-last.png`。逐张复查首句/末句页码、继续/关闭按钮、背景、状态栏、中文字形和三行边界均正常；批量捕获在每张图前清空离屏画布，消除末图残帧/黑帧。
- TDD RED：剧情目录测试先因缺少 `DialogueTrigger::restaurant_owner_intro` 编译失败；固定 NPC 运行期测试再因缺少 `app/npc_lobby_runtime.hpp` 编译失败；餐馆交接测试最后因缺少 `step_restaurant_lobby` 和 `LocationRuntimeState::npc_lobby` 编译失败。
- TDD GREEN：`./build/pixel_town_tests` 共 234 个测试、2534 个断言全部通过；`ctest --test-dir build --output-on-failure` 通过 1/1；`./build/pixel_town --capture-ui-diagnostics` 成功生成全部 33 张诊断截图。

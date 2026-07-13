# P4：用酒保切片建立共享人物对话运行期

Status: done
Milestone: P4
Type: AFK
User stories: 29, 54, 57, 58

## What to build

以酒馆大厅现有酒保热点和候选小人为第一条垂直切片，建立可复用的线性剧情目录、对话运行期、统一对话框和人物资源回退。玩家点击酒保后看到酒保与主角的短对话，可使用鼠标或键盘逐句继续或跳过，再返回同一酒馆大厅；对话期间不得推进酒馆玩法或修改全局状态。

人物先使用已确认的 32×32 占位接口或现有候选酒保素材，最终人物造型不属于本 issue 的完成前提。

## Acceptance criteria

- [x] 剧情目录与对话运行期不依赖 raylib，使用显式输入并提供只读 presentation。
- [x] 酒保与主角至少有一段符合剧情风格规范的线性短对话，台词不再硬编码在酒馆绘制代码中。
- [x] 统一对话框展示说话人、人物槽、正文、当前行/总行数和继续/跳过提示。
- [x] 鼠标点击、Enter/Space 继续，Esc 跳过；同一输入不会同时触发酒馆返回、玩法选择或挑战操作。
- [x] 对话 active 时酒馆运行期不推进挑战、计时或动画；关闭后回到原大厅状态。
- [x] 对话前后玩家属性、阶段、库存、酒馆战绩和行动结果保持不变。
- [x] 人物纹理缺失时显示程序绘制轮廓和姓名，不出现空白页或阻塞启动。
- [x] 对话正文使用 UTF-8 安全的限行布局，最多三行且不越出 960×540 逻辑画布。
- [x] 对话目录自动贡献字体字形，截图中无 A 替代字形。
- [x] 无窗口测试覆盖打开、逐句推进、重复输入、跳过、关闭、状态不变和稳定 presentation。
- [x] 诊断截图覆盖酒保第一句、中间句、最后一句和素材缺失 fallback。

## Blocked by

- [12 P3：接入酒馆选择、赌注与夜晚结算](12-p3-tavern-shell.md)
- [19 P1：接入 MVP 主线剧情骨架](19-p1-mvp-story-shell.md)
- [22 P3：深化酒馆运行期与挑战结算架构](22-p3-tavern-runtime-and-settlement-architecture.md)

## Comments

<!-- 在此行下方追加实现与验证记录。 -->

- 2026-07-10：按 TDD 新增 `StoryDialogueCatalog`、raylib-free `DialogueRuntime`、只读 presentation 和酒保三句线性脚本；酒馆对话 active 时优先消费输入并冻结动画与挑战入口。
- 2026-07-10：统一对话框接入角色槽、说话人、UTF-8 三行布局、行进度、继续/跳过提示和缺图程序轮廓；脚本及共享标签自动汇入字体字形清单。
- 验证：`.tools/CMake.app/Contents/bin/cmake --build build --parallel 4` 通过；`./build/pixel_town_tests` 共 200 个测试、2098 个断言全部通过。
- 诊断：`./build/pixel_town --capture-ui-diagnostics` 成功生成 `tavern-dialogue-first.png`、`tavern-dialogue-middle.png`、`tavern-dialogue-last.png` 和 `tavern-dialogue-fallback.png`；逐张复查无缺字、重叠或越框，末句“关闭”及缺图 fallback 正常。

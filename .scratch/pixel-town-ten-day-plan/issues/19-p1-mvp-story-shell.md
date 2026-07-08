# P1：接入 MVP 主线剧情骨架

Status: done
Milestone: P1
Type: AFK
User stories: 2, 9, 10, 36, 42, 43, 49, 50, 61

## What to build

把 `docs/story/` 中确认的 S 级剧情文本接入当前十日核心闭环，让标题、新游戏、每日提示、模拟地点结果、每日总结和占位结局不再只显示功能性占位文案。

本 issue 只做 MVP 主线剧情骨架，不接入复杂 NPC、好感度、对话树、任务链或旧夏日集市完整暗线。后续 P2/P3 地点和结局 issue 基于本 issue 建立的剧情文本入口继续扩展。

## Acceptance criteria

- [x] 普通启动始终先显示标题页；若存在有效存档，标题页提供继续游戏和新游戏入口，不自动跳过标题页。
- [x] 新游戏路径能展示开场剧情文本，内容来自 `docs/story/MVP_TEXT_PACK.md`。
- [x] 地图页按第 1-10 天展示主线每日提示，而不是只显示随机天气和事件。
- [x] 模拟餐馆、便利店、图书馆、回家和占位酒馆摘要使用 MVP 地点短结果摘要或同风格压缩版。
- [x] 每日总结页展示行动摘要和生活收束，不再只像调试状态页。
- [x] 第十天进入结局前展示第十天评议会文本；若不新增阶段，也必须在结局页顶部可见。
- [x] 占位结局 `平凡小镇新人` 使用剧情文档中的生活画像文案，并保留最终状态数值展示。
- [x] 新增 raylib-free 剧情文本入口，UI 不直接硬编码长剧情正文。
- [x] 新增自动化测试覆盖每日提示、地点摘要、占位结局文本和非法天数回退。
- [x] 新增或更新字体字形覆盖，确保新增剧情文本不会出现缺字或误报可用。
- [x] 存档恢复后显示与当前阶段一致的剧情文本，不重复触发已结算剧情。

## Blocked by

- [05 P1：扩展为十日周期与占位主结局](05-p1-ten-day-cycle.md)
- [06 P1：实现阶段边界自动存档与恢复](06-p1-save-and-resume.md)
- [07 P1：完成窗口、输入、暂停与静音外壳](07-p1-interaction-shell.md)
- [08 P1：实现隔离的演示预设加载](08-p1-demo-presets.md)

## Reference docs

- [`docs/story/NARRATIVE_OVERVIEW.md`](../../../docs/story/NARRATIVE_OVERVIEW.md)
- [`docs/story/MVP_TEXT_PACK.md`](../../../docs/story/MVP_TEXT_PACK.md)
- [`docs/story/STYLE_GUIDE.md`](../../../docs/story/STYLE_GUIDE.md)
- [`docs/story/STORY_INTEGRATION_PLAN.md`](../../../docs/story/STORY_INTEGRATION_PLAN.md)

## Out of scope

- NPC 状态机。
- 好感度或对话树。
- 任务链。
- 完整旧夏日集市线。
- 七种主结局正式判定。
- 需要改变存档格式的路线统计。

## Verification record

2026-07-08 在 macOS / Apple Clang 本地完成：

- 新增 `src/core/story_text.hpp` 和 `src/core/story_text.cpp`，集中提供开场、每日提示、地点摘要、每日收束、第十天评议会和占位结局文本。
- 新增 `tests/story_text_test.cpp`，覆盖每日提示、地点摘要、非法天数回退、评议会和占位结局文案。
- `GameSession` 的占位行动摘要和占位结局改为读取 `StoryText`；最终状态数值仍由 UI 单独展示。
- 普通启动加载到有效存档时不再直接进入地图，而是在标题页提供“继续游戏”和“新游戏”入口。
- `game_flow_glyphs()` 纳入剧情文本字形集合，并排除换行符，避免 U+000A 被误报为缺字。
- `.tools/CMake.app/Contents/bin/cmake --build build --parallel 4` 通过。
- `.tools/CMake.app/Contents/bin/ctest --test-dir build --output-on-failure` 通过。
- `./build/pixel_town --capture-game-flow` 通过，标题、地图和结局截图已生成；标题开场、每日提示和结局评议会文本可见。

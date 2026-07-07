# P0：提供启动资源诊断与本地日志

Status: ready-for-agent
Milestone: P0
Type: AFK
User stories: 64, 70

## What to build

在应用启动路径中加入资源清单验证和本地诊断日志。必要字体、贴图或数据缺失时，玩家应看到可理解的错误页，开发者应能从本次启动日志定位资源和阶段；音频缺失只导致静音降级，不阻塞核心游戏。

## Acceptance criteria

- [x] 启动时验证当前基线所需的必要字体、贴图和数据资源。
- [x] 必要资源完整时进入正常基线场景，不显示误报。
- [x] 任一必要资源缺失或无效时进入明确错误页，不进入半可用主循环。
- [x] 音频资源缺失时记录错误并静音继续，不阻塞核心场景。
- [x] 每次启动覆盖本地 `latest.log`，记录应用版本、资源验证结果和当前启动阶段。
- [x] 日志不包含遥测、网络请求、用户个人信息或无界历史文件。
- [x] 自动化测试覆盖资源完整、必要资源缺失和可选音频缺失三条路径。

## Blocked by

- [01 P0：建立可离线复现的应用基线](01-p0-offline-app-baseline.md)

## Verification record

2026-07-06 在 macOS 15.7.4 / Apple Clang 17.0.0 上完成：

- Debug 完整构建通过，CTest 1/1 通过；同一测试程序覆盖资源完整、必要资源缺失、必要资源无效、可选音频缺失和日志覆盖写入。
- 正常资源启动持续运行到主动终止，raylib 成功加载 8×8 PNG 必要贴图；`latest.log` 记录 `startup_stage=baseline_scene`、`resource_status=ready` 和 `audio_status=muted`。
- 从临时工作目录移除必要数据后启动持续停留在错误页，日志记录 `startup_stage=resource_error` 和缺失资源相对路径。
- Issue 03 已加入待审 Fusion Pixel Font 简体中文候选；启动清单现在将该字体作为必要资源，并在实际加载后检查原型所需字形是否完整。字体仍需 HITL 批准才能成为最终资源。

# P0：建立可离线复现的应用基线

Status: done
Milestone: P0
Type: AFK
User stories: 60, 68, 69

## What to build

建立项目的首条可运行垂直切片：从干净检出开始，在没有网络和用户全局游戏依赖的环境中配置并构建应用，启动一个使用 raylib 的像素画布窗口，同时通过统一测试入口运行一个不需要图形窗口的 doctest。Windows 是发布基线，macOS 是受支持的开发环境，两者由同一套 CMake 配置驱动。

该切片必须落实 ADR-0001：锁定 raylib 6.0 和 doctest 2.5.2，保留许可证，并防止领域代码依赖 raylib 类型。

## Acceptance criteria

- [x] Windows 10/11 x64 使用 Visual Studio 2022/MSVC 和 CMake 可从干净检出完成配置与构建。
- [x] macOS 使用同一 CMake 入口可完成配置与构建。
- [x] 首次配置和构建在依赖已随仓库存在的情况下不访问网络，也不要求全局安装 raylib 或 doctest。
- [x] 应用启动单一窗口，以 960×540 逻辑画布清晰显示一个可识别的基线场景，并默认呈现在 960×540 窗口中。
- [x] CTest 能发现并运行至少一个不创建图形窗口的 doctest 测试。
- [x] Windows 与 macOS GitHub Actions 执行配置、构建和 CTest。
- [x] 第三方版本与许可证可从仓库直接确认。
- [x] 构建说明记录标准命令、支持环境和离线前提。

## Blocked by

None - can start immediately.

## Verification record

2026-07-06 在 macOS 15.7.4 / Apple Clang 17.0.0 上完成：

- 从独立空构建目录执行 Release 配置、完整构建和 CTest，1/1 测试通过。
- CMake 配置只使用仓库 `third_party/`，未定义 `FetchContent` 或 `ExternalProject` 下载路径。
- 原生应用持续运行 3 秒，raylib 6.0 成功创建当时的 1280×720 窗口和 640×360 RenderTexture；该尺寸已在 2026-07-07 迁移为当前 960×540 窗口和 960×540 RenderTexture。

2026-07-07 在 GitHub Actions run [28836570723](https://github.com/ljm-hsw/-666/actions/runs/28836570723) 上完成提交 `551e62b9aff8104ee00d34ed669d39241e810444` 的远端验证：

- [Windows 2022 / MSVC](https://github.com/ljm-hsw/-666/actions/runs/28836570723/job/85521523988) 完成 `cmake -S . -B build -G "Visual Studio 17 2022" -A x64`、`cmake --build build --config Release --parallel` 和 `ctest --test-dir build -C Release --output-on-failure`，结论为 `success`。
- [macOS / Apple Clang](https://github.com/ljm-hsw/-666/actions/runs/28836570723/job/85521524005) 完成 Release 配置、构建和 CTest，结论为 `success`。

2026-07-07 当前显示基线迁移为 960×540 逻辑画布和 960×540 默认窗口，后续 UI 和 imagegen 背景规划以新尺寸为准。

2026-07-08 P1 game-flow 已从旧 640×360 UI + 1.5× camera 放大迁移为 960×540 逻辑画布原生绘制；P0 visual prototype 仍保留独立候选缩放路径用于人工评审。

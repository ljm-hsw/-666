# P0：建立可离线复现的应用基线

Status: ready-for-agent
Milestone: P0
Type: AFK
User stories: 60, 68, 69

## What to build

建立项目的首条可运行垂直切片：从干净检出开始，在没有网络和用户全局游戏依赖的环境中配置并构建应用，启动一个使用 raylib 的像素画布窗口，同时通过统一测试入口运行一个不需要图形窗口的 doctest。Windows 是发布基线，macOS 是受支持的开发环境，两者由同一套 CMake 配置驱动。

该切片必须落实 ADR-0001：锁定 raylib 6.0 和 doctest 2.5.2，保留许可证，并防止领域代码依赖 raylib 类型。

## Acceptance criteria

- [ ] Windows 10/11 x64 使用 Visual Studio 2022/MSVC 和 CMake 可从干净检出完成配置与构建。
- [x] macOS 使用同一 CMake 入口可完成配置与构建。
- [x] 首次配置和构建在依赖已随仓库存在的情况下不访问网络，也不要求全局安装 raylib 或 doctest。
- [x] 应用启动单一窗口，以 640×360 逻辑画布清晰显示一个可识别的基线场景，并默认呈现在 1280×720 窗口中。
- [x] CTest 能发现并运行至少一个不创建图形窗口的 doctest 测试。
- [ ] Windows 与 macOS GitHub Actions 执行配置、构建和 CTest。
- [x] 第三方版本与许可证可从仓库直接确认。
- [x] 构建说明记录标准命令、支持环境和离线前提。

## Blocked by

None - can start immediately.

## Verification record

2026-07-06 在 macOS 15.7.4 / Apple Clang 17.0.0 上完成：

- 从独立空构建目录执行 Release 配置、完整构建和 CTest，1/1 测试通过。
- CMake 配置只使用仓库 `third_party/`，未定义 `FetchContent` 或 `ExternalProject` 下载路径。
- 原生应用持续运行 3 秒，raylib 6.0 成功创建 1280×720 窗口和 640×360 RenderTexture。
- Windows/MSVC 构建与两平台 GitHub Actions 实际运行仍需远端 CI 证据，因此对应验收项保持未勾选。

---
status: accepted
---

# 采用 raylib 6.0 与 doctest 2.5.2

本项目选择 raylib 6.0 统一提供窗口、2D 渲染、输入和音频，选择 doctest 2.5.2 提供 C++17 规则测试，并将锁定版本放在仓库 `third_party/` 中。相比 SDL3，raylib 为本项目所需的 2D 游戏能力提供了更直接的接口；相比 SFML，它同时符合 C/C++ 实训、CMake 跨平台开发和较低接入成本的要求；doctest 的单头文件形式适合小型团队维护。依赖随仓库保存是为了保证 Windows 演示和 macOS 开发都能离线复现构建，而不是依赖每名成员的全局环境。

## Consequences

- 领域规则和核心状态机不得依赖 raylib 类型，raylib 只进入展示、输入、音频和应用装配边界。
- 不再引入 SDL、SFML、raygui、Catch2 或其他重叠框架。
- 升级依赖必须显式修改锁定版本、检查许可证，并在 Windows 与 macOS 重新验证构建。

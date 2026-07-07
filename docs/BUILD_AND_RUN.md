# Windows 与 macOS 从零编译运行指南

本文说明团队成员如何从一台新机器或干净目录开始，编译、测试并运行《像素小镇：十日经营计划》。

## 0. 基本前提

- 仓库已包含锁定版本的 raylib 6.0 和 doctest 2.5.2，位于 `third_party/`。
- 配置和构建过程不需要下载 raylib 或 doctest，也不应访问网络获取运行时资源。
- 运行时资源位于 `assets/`；必要资源缺失时应用会显示资源错误页，并写入 `logs/latest.log`。
- 构建产物写入 `build/`；本地运行状态写入 `logs/`、`saves/`、`prototype-captures/` 或 `game-flow-captures/`，这些目录不提交。

## 1. 获取代码

Windows PowerShell 或 macOS Terminal 均可使用：

```bash
git clone https://github.com/ljm-hsw/-666.git
cd -666
```

如果你已经拿到的是压缩包，解压后进入项目根目录即可。项目根目录应能看到：

```text
CMakeLists.txt
assets/
src/
tests/
third_party/
```

## 2. Windows 10/11 x64

### 2.1 安装工具

推荐环境：

- Visual Studio 2022 或 Build Tools for Visual Studio 2022
- CMake 3.25 或更高版本
- Git for Windows

Visual Studio 安装时至少选择：

- Desktop development with C++
- MSVC v143 x64/x86 build tools
- Windows 10/11 SDK
- CMake tools for Windows（可选；也可以单独安装 CMake）

### 2.2 打开正确终端

使用开始菜单中的：

```text
x64 Native Tools Command Prompt for VS 2022
```

或：

```text
Developer PowerShell for VS 2022
```

然后进入仓库目录。路径有空格时请加引号：

```powershell
cd "C:\path\to\-666"
```

### 2.3 配置、构建、测试

配置：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

Debug 构建：

```powershell
cmake --build build --config Debug --parallel
```

Debug 测试：

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

Release 构建与测试：

```powershell
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

预期测试结果：

```text
100% tests passed
```

### 2.4 运行

Debug：

```powershell
.\build\Debug\pixel_town.exe
```

Release：

```powershell
.\build\Release\pixel_town.exe
```

应用默认打开 960×540 窗口，使用 960×540 逻辑画布。

### 2.5 Windows 常用检查命令

生成 P1 核心流程截图：

```powershell
.\build\Debug\pixel_town.exe --capture-game-flow
```

生成 P0 视觉原型截图：

```powershell
.\build\Debug\pixel_town.exe --capture-prototype
```

加载演示预设：

```powershell
.\build\Debug\pixel_town.exe --demo-preset midgame
.\build\Debug\pixel_town.exe --demo-preset ending-eve
```

## 3. macOS

### 3.1 安装工具

推荐环境：

- Xcode Command Line Tools
- CMake 3.25 或更高版本
- Git

安装 Xcode Command Line Tools：

```bash
xcode-select --install
```

确认工具可用：

```bash
clang++ --version
cmake --version
git --version
```

### 3.2 进入仓库

路径有空格时请加引号：

```bash
cd "/Users/your-name/path/to/-666"
```

### 3.3 配置、构建、测试

Debug：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Release：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

预期测试结果：

```text
100% tests passed
```

### 3.4 运行

```bash
./build/pixel_town
```

应用默认打开 960×540 窗口，使用 960×540 逻辑画布。

### 3.5 macOS 常用检查命令

生成 P1 核心流程截图：

```bash
./build/pixel_town --capture-game-flow
```

生成 P0 视觉原型截图：

```bash
./build/pixel_town --capture-prototype
```

加载演示预设：

```bash
./build/pixel_town --demo-preset midgame
./build/pixel_town --demo-preset ending-eve
```

## 4. 运行时操作

当前快捷键：

| 操作 | 快捷键 |
| --- | --- |
| 确认 / 继续 | `Enter` |
| 返回 / 取消覆盖确认 | `Esc` |
| 开始 / 完成地点模拟 | `Space` |
| 暂停 / 继续 | `P` |
| 切换全局静音 | `M` |

鼠标是主要操作方式。窗口缩放使用整数倍显示，黑边区域点击不会触发控件。

## 5. 本地产物位置

| 路径 | 用途 | 是否提交 |
| --- | --- | --- |
| `build/` | CMake 构建产物 | 否 |
| `logs/latest.log` | 最近一次启动诊断日志 | 否 |
| `saves/slot1.sav` | 正式自动存档 | 否 |
| `saves/settings.ini` | 静音等本地设置 | 否 |
| `game-flow-captures/` | P1 截图导出 | 否 |
| `prototype-captures/` | P0 原型截图导出 | 否 |

## 6. 干净重建

如果配置或构建状态异常，删除 `build/` 后重新配置。

Windows PowerShell：

```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --parallel
ctest --test-dir build -C Debug --output-on-failure
```

macOS：

```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## 7. 常见问题

### CMake 找不到编译器

Windows：

- 确认使用的是 Visual Studio Developer PowerShell 或 x64 Native Tools Command Prompt。
- 确认 Visual Studio 安装了 C++ 桌面开发组件。

macOS：

- 执行 `xcode-select --install`。
- 执行 `clang++ --version` 确认命令行工具可用。

### 应用打开后显示资源错误页

检查 `assets/` 是否完整，并查看：

```text
logs/latest.log
```

错误页会列出缺失或无效的必要资源。可选音频缺失只会静音降级，不阻塞游戏启动。

### 中文显示异常

确认字体文件存在：

```text
assets/fonts/fusion-pixel-12px-proportional-zh_hans.ttf
```

应用启动时会校验当前 UI 所需字形；缺字会进入资源错误路径。

### Windows 上运行位置不对导致找不到资源

当前应用从进程工作目录读取 `assets/`。开发阶段请从仓库根目录运行可执行文件，或确保运行目录能访问仓库内 `assets/`。

### 构建过程中访问网络

这不符合当前工程基线。raylib 和 doctest 应从 `third_party/` 使用仓库内锁定版本；不要改成 FetchContent、ExternalProject 或全局依赖。

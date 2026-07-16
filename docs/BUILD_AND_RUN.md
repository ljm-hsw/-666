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

构建五日展示版（独立 Release 目录）：

```powershell
cmake -S . -B build-five-day -G "Visual Studio 17 2022" -A x64 -DPIXEL_TOWN_DAY_LIMIT=5 -DPIXEL_TOWN_BUILD_TESTS=OFF
cmake --build build-five-day --target pixel_town --config Release --parallel
.\build-five-day\Release\pixel_town_five_day.exe
```

该变体在第五个完整昼夜后进入库存清算与正式结局。输出名和构建目录均与十日版不同，因此便携存档位于 `build-five-day\Release\saves\`。完整回归套件仍由默认十日构建执行，五日边界另有核心单元测试；尚未在目标 Windows 机器完成实机验收。

生成 P1 核心流程截图：

```powershell
.\build\Debug\pixel_town.exe --capture-game-flow
```

生成 P0 视觉原型截图：

```powershell
.\build\Debug\pixel_town.exe --capture-prototype
```

生成餐馆、家、便利店、图书馆和酒馆地点 UI 诊断截图（包含五张可选室内美术诊断覆盖图；不代表存在移动或运行时碰撞）：

```powershell
.\build\Debug\pixel_town.exe --capture-ui-diagnostics
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

构建五日展示版（已在 macOS Release 构建验证）：

```bash
cmake -S . -B build-five-day -DCMAKE_BUILD_TYPE=Release -DPIXEL_TOWN_DAY_LIMIT=5 -DPIXEL_TOWN_BUILD_TESTS=OFF
cmake --build build-five-day --target pixel_town --parallel
./build-five-day/pixel_town_five_day
```

该命令不会修改默认 `build/` 的十日版。五日版在第五个完整昼夜后进入同一套库存清算和正式结局，并把存档写入 `build-five-day/saves/`。完整回归套件仍由默认十日构建执行，五日边界另有核心单元测试。五日版不要加载为十日边界准备的 `ending-eve` 预设。

生成 P1 核心流程截图：

```bash
./build/pixel_town --capture-game-flow
```

生成 P0 视觉原型截图：

```bash
./build/pixel_town --capture-prototype
```

生成餐馆、家、便利店、图书馆和酒馆地点 UI 诊断截图（包含五张可选室内美术诊断覆盖图；不代表存在移动或运行时碰撞）：

```bash
./build/pixel_town --capture-ui-diagnostics
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
| 室内移动 | `WASD` 或方向键 |
| 靠近 NPC / 桌面 / 床铺 / 出口后互动 | `E` 或 `Space` |
| 确认 / 继续 | `Enter` |
| 返回 / 取消覆盖确认 / 跳过对话 / 放弃当前玩法 | `Esc`（不会关闭程序） |
| 暂停 / 继续 | `P` |
| 切换全局静音 | `M` |
| 显示 / 隐藏可行走边界、碰撞体、出口和主角碰撞框 | `F3` |

鼠标热点与按钮保留为等价操作路径。窗口缩放使用整数倍显示，黑边区域点击不会触发控件。

音频会随标题/地图、天气和当前地点自动切换。`M` 同时静音背景音乐和反馈音效；取消静音后从当前场景重新开始对应 BGM。若 11 个候选音频中任一文件缺失、签名无效或设备初始化失败，程序会整包静音继续运行，原因写入 `logs/latest.log`。

从地图点击餐馆、便利店、图书馆、酒馆或家后，会先进入不消耗行动的固定室内场景。使用 `WASD`/方向键绕开家具，靠近固定 NPC、酒馆桌面、家中床铺或门口后按 `E`/`Space` 互动；也可直接点击对应人物、热点或按钮。对话关闭或跳过后才进入原有玩法，大厅返回地图不消耗阶段。NPC 位置固定，不会巡逻或寻路；主角位置不写入存档，重新进入场景时回到门口出生点。

人工验证室内移动时，建议按以下最短路径逐项检查：进入任一场景后先按住单一方向键确认主角位移；对着家具继续移动确认被阻挡；沿家具边缘斜向移动确认可滑动；靠近目标观察底部提示变为具体互动文案；按 `E` 打开对话/玩法；对话打开后继续按移动键，确认主角不移动且输入不泄漏。餐馆、便利店、图书馆、酒馆和家应各完成一次。

确认新游戏后会在第一日地图上显示镇长与主角的四句开场；`Enter`、`Space`、鼠标点击继续，`Esc` 跳过。第一日未执行白天行动的存档恢复允许无奖励重播。夜晚进入家并确认休息后会显示两句当日晚访客剧情：首次由镇长说明休息规则，后续按日期由餐馆老板、便利店店主、管理员或镇长来访。关闭或跳过后才提交一次休息结果并进入每日总结；选择酒馆的夜晚不会触发家中访客。

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

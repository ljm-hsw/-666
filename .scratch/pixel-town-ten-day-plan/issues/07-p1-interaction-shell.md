# P1：完成窗口、输入、暂停与静音外壳

Status: ready-for-human
Milestone: P1
Type: HITL
User stories: 56, 57, 59

## What to build

完成所有地点共享的表现外壳：整数倍像素缩放、逻辑坐标输入、鼠标与键盘操作、暂停、窗口失焦冻结和全局静音。该切片只提供稳定的场景与交互能力，不加入新的产品页面或地点规则。

## Acceptance criteria

- [x] 960×540 逻辑画布在支持的窗口尺寸中使用整数倍缩放与留黑边。
- [x] 鼠标输入正确转换到逻辑画布坐标，黑边点击不会触发控件。
- [x] 主要操作可由鼠标完成，常用确认、返回和暂停具有键盘快捷键。
- [x] 暂停菜单打开时，地点计时、动画推进和规则更新冻结。
- [x] 窗口失焦或最小化时同样冻结，恢复焦点后继续而不补算失去时间。
- [x] 全局静音开关即时生效并可持久保存。
- [ ] 交互控件遵循已批准的视觉原型和中文字体规范。
- [x] 坐标转换、暂停时钟和设置往返具有自动化测试；像素清晰度由人工检查。

## Blocked by

- [01 P0：建立可离线复现的应用基线](01-p0-offline-app-baseline.md)
- [03 P0：确认像素视觉原型与资源规范](03-p0-visual-prototype-approval.md)

## Verification record

2026-07-07 在 macOS 15.7.4 / Apple Clang 17.0.0 上完成：

- 新增无窗口测试覆盖整数倍 viewport、黑边输入过滤、暂停冻结、失焦/最小化冻结和静音设置往返。
- 应用主循环改为使用可测试 viewport 进行逻辑坐标转换；黑边区域输入映射为无效坐标。
- 新增 `P` 暂停/继续、`M` 切换全局静音；静音设置保存到应用目录旁 `saves/settings.ini`。
- 暂停、窗口失焦或最小化时跳过游戏流程更新；恢复后不补算冻结期间时间。
- `.tools/CMake.app/Contents/bin/cmake --build build --parallel 4` 通过。
- `.tools/CMake.app/Contents/bin/ctest --test-dir build --output-on-failure` 通过，1/1 测试成功。
- `./build/pixel_town --capture-game-flow` 通过，输出标题、地图和结局截图。

### HITL pending

- 交互外壳的自动化实现已完成，但 Issue 03 视觉原型仍未获得人工批准；本 issue 保持 `ready-for-human`，等待团队确认中文字体、窗口缩放、热点层级和视觉规格后再关闭。

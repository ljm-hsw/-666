# P4：完成发布包、演示和最终验收

Status: ready-for-human
Milestone: P4
Type: HITL
User stories: 51, 52, 57, 60, 64-70, 75

## What to build

生成并验证最终 Windows 便携发布包，整理可复现构建说明、测试结果、演示预设、演示脚本和 PPT。发布包必须在目标 Windows 机器无网络运行，资源、存档、日志、许可证和演示路径都达到交付标准。

## Acceptance criteria

- [ ] 从干净检出可以使用文档化命令完成 Windows Release 配置、构建和测试。
- [ ] GitHub Actions 的 Windows 与 macOS 配置、构建和 CTest 全部通过。
- [ ] 完整自动化测试覆盖核心状态机、地点规则、存档、随机性、资源验证、酒馆和结局。
- [ ] Windows 发布包不要求安装 raylib、doctest、开发工具或联网下载运行时资源。
- [ ] 发布包从可写目录运行，自动存档、设置和日志只写入便携用户数据位置。
- [ ] 缺失必要资源、损坏存档和非法演示预设产生明确错误，不崩溃或静默重置。
- [ ] 正常游戏可从新游戏运行至第十日唯一主结局。
- [ ] Windows 发布包中五个室内场景的 WASD/方向键移动、静态碰撞、E/Space 邻近互动和鼠标等价路径均通过实际键盘烟雾测试。
- [ ] 演示预设可展示中期经营、酒馆和结局前状态，且不修改正式自动存档。
- [ ] 发布包包含 `CREDITS.md` 和所有要求的第三方许可证。
- [ ] 演示脚本和 PPT 覆盖产品定位、核心循环、四个地点、统一属性、测试证据和主结局。
- [ ] 在最终目标 Windows 机器完成一次无网络现场烟雾测试，并记录机器、构建版本和结果。
- [ ] 人工发布批准及任何已知限制记录在本 issue 的 `Comments` 中。

## Blocked by

- [02 P0：提供启动资源诊断与本地日志](02-p0-resource-diagnostics-and-log.md)
- [08 P1：实现隔离的演示预设加载](08-p1-demo-presets.md)
- [15 P3：完成库存清算与七种主结局](15-p3-endings.md)
- [16 P4：完成内容扩充与数值平衡](16-p4-content-and-balance.md)
- [17 P4：完成像素美术、音频、教程与署名](17-p4-presentation-and-credits.md)
- [33 P4：接入五地点室内移动、静态碰撞与邻近互动](33-p4-indoor-navigation-and-collision.md)

## Comments

<!-- 在此行下方追加发布验收记录。 -->

- 2026-07-16：新增独立五日展示构建入口 `-DPIXEL_TOWN_DAY_LIMIT=5`。macOS Release 已生成 `build-five-day/pixel_town_five_day`；第五个完整昼夜后复用既有库存清算和正式结局。默认值仍为十日，展示版使用独立构建目录、输出名和便携存档目录。Windows 命令已文档化，目标 Windows 实机仍属于本 issue 的人工门禁。

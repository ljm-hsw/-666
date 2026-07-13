# 跨地点风格板

本目录保存酒馆、图书馆与便利店的 imagegen 跨地点风格板源图。它们只用于人工选定统一视觉方向，不进入运行时资源清单。

- `style_board_a_source.png`：暖色叙事绘本方向，最接近当前标题页和小镇地图。
- `style_board_b_source.png`：清晰模块化方向，减少噪声，便于后续拆件、碰撞和 UI 叠加。
- `style_board_c_source.png`：温暖夜色对比方向，强化地点氛围，同时保持三场景属于同一小镇。
- `style_board_contact_sheet.png`：A/B/C 纵向对比图，仅供人工评审。
- `prompts.md`：本批次使用的完整生成提示词和参考图。

2026-07-11，用户已明确批准 C 方向。后续建筑结构、家具和道具分件必须引用 `style_board_c_source.png`；A、B 仅作为过程记录，不得混入同一运行时场景。

## 生成记录

生成日期：2026-07-11。三张源图均由内置 imagegen 独立调用生成；联系表由源图无插值拼接，不是新的生成图。

| 文件 | 尺寸 | SHA-256 |
| --- | --- | --- |
| `style_board_a_source.png` | 1672×941 RGB PNG | `853947909ba591471b47cde7f11dc4f3ea87bb4a1c8e375212eea49c80d6692f` |
| `style_board_b_source.png` | 1672×941 RGB PNG | `ddae9306eb0f3c00190026a5b34154a9b38d375448dd5e28c712739fc09403ea` |
| `style_board_c_source.png` | 1672×941 RGB PNG | `6a0b4a24439943f7ec014e99d6b8a609ba7c4ea627b91514214fce398d168c75` |
| `style_board_contact_sheet.png` | 1672×3019 RGB PNG | `0302422e74b617596849434875539af344ea5bebb59fc46b8615a3d3d3a6955c` |

## 初步机器与人工检查

- 三张源图尺寸一致，均接近 16:9，可作为后续 prompt 的参考图。
- 三张都包含可区分的酒馆、图书馆和便利店，且保持同一轻度俯视正交视角。
- 未观察到人物、程序 UI、对话框、商标、水印或可读文字污染。
- 这些结论只是候选素材检查，不等于视觉批准，也不等于已经满足 960×540 运行时布局和碰撞要求。

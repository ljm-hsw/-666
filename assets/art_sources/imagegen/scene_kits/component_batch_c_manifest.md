# C 方向首轮分件清单

生成日期：2026-07-11。

本批次使用内置 imagegen，以 `style_boards/style_board_c_source.png` 为唯一视觉参考，共执行 9 次独立生成。完整提示词见 [`component_batch_c_prompts.md`](component_batch_c_prompts.md)。所有文件仍是候选源素材，不在运行时加载。

## 原始生成图

| 地点 | 类型 | 尺寸 | SHA-256 |
| --- | --- | --- | --- |
| 酒馆 | 建筑结构 | 1536×1024 | `64e63e3889ae9a99400eeb559f6efbc2572c56296cee5267064daa7ac9021c52` |
| 酒馆 | 大型家具 | 1672×940 | `b120757866321467f63432cb64018ff57aa3f100a5f905dd041759e2e2c7534e` |
| 酒馆 | 小型道具 | 1672×941 | `eee5129962da35c0b06dfe7178b2b5ac5c3ed0710bacc9699b83ff8c167a92fc` |
| 图书馆 | 建筑结构 | 1672×941 | `e29ff60c573313d3d2ddacb7a77212e8a576340e4cc431e4b296cc82ad230161` |
| 图书馆 | 大型家具 | 1024×1536 | `8c7e619bc72ab876e79d2e2be9ea72c56cefda2a7039a1ea248965c10bae4ff2` |
| 图书馆 | 小型道具 | 1448×1086 | `d35576db2cac038888c3650fbc27ef0219b1a6b6b9afb51a957e65118651d6e9` |
| 便利店 | 建筑结构 | 1672×941 | `fd11364f2f864613807f7b0c2fb9e4becde4f884bfb1ba03c5aa9ced86484206` |
| 便利店 | 大型家具 | 1672×941 | `355f8eb662323c3b08e5ccf37e03d2ffc31f6396a21e30ad62ce1ef0e5ca3435` |
| 便利店 | 小型道具 | 1672×941 | `ec584a3c9cb40a437c34a6875e50d49a82b63b4e0e658e0a5c0a43ad5054740d` |

原图保存在各地点的 `raw/` 目录。视觉上接近洋红背景，但实际包含轻微色差和渐变，因此不得直接当作纯色键控图使用，也不得覆盖删除。

## 透明候选

使用 imagegen skill 自带的 `remove_chroma_key.py`，以边框自动采样、soft matte、despill、透明阈值 12、实色阈值 220 生成 `selected/` 下的 RGBA PNG。

| 地点 | 类型 | 透明像素 | 半透明边缘 | SHA-256 |
| --- | --- | ---: | ---: | --- |
| 酒馆 | 建筑结构 | 60.32% | 1.05% | `81d8a707328c8a9d84fe905f9273c17dcaa98f96acd78bad4d90aada29641dc4` |
| 酒馆 | 大型家具 | 57.20% | 1.19% | `45fd32b79d7d93d5d8614e2a59bd79eb82e4e1644145649ffaaac77f8e2c9e39` |
| 酒馆 | 小型道具 | 67.26% | 1.73% | `72d0a45471c4aa55ba3888a4d3033636b8722514fbd244609c70051fd44a8cd3` |
| 图书馆 | 建筑结构 | 58.63% | 1.51% | `a212b2b60d5f52444b3eecb3ae6f6674617ea90ff6769b5899819389a07d80c4` |
| 图书馆 | 大型家具 | 58.20% | 1.11% | `6237c08c0603743ed641a92826a1dc7611b2a8457f64c2a2d09a10978fa4f043` |
| 图书馆 | 小型道具 | 70.57% | 2.53% | `3cec7c417c2e25c5209d5d067045956a6213ec7c2c5a11d78dece76d83e7d639` |
| 便利店 | 建筑结构 | 61.17% | 1.59% | `506125295f4f9172fd1c0ced8746ed15072a2a816a8e0ad733140b5778722510` |
| 便利店 | 大型家具 | 55.58% | 1.23% | `f7776a9fa815888b3bf1d12a8c95503c7e4e98be3e8b54f92905aad462868576` |
| 便利店 | 小型道具 | 62.33% | 3.18% | `8f567e2ccde5d1b888d08d0022b8e4af67a256d8eae1c3dbaba1488003d9bb9f` |

九张透明候选均通过以下机器检查：

- 文件模式为 RGBA。
- 四角透明，整张图片边框的最大 alpha 为 0。
- 主体覆盖率合理，没有整图被误删或背景大面积残留。
- 在灰白棋盘格评审图上未观察到明显紫边，因此本轮未使用 `--edge-contract 1` 二次收缩。

## 评审联系表

| 地点 | 文件 | SHA-256 |
| --- | --- | --- |
| 酒馆 | `tavern/selected/tavern_kit_contact_sheet.png` | `789e9e9d3e0cfcb10140a2c1bdc52a99b8caaacc457807f1d7e38a1f36d0f9e1` |
| 图书馆 | `library/selected/library_kit_contact_sheet.png` | `3ded6b5018f03991243ce7420bf10327329c4b035d3c26533f374ff57ee565f6` |
| 便利店 | `convenience_store/selected/convenience_store_kit_contact_sheet.png` | `43e46f87fdf6800f20d19fccfe61291860b6f6acbc937eca082789f15968cc13` |

联系表使用最近邻缩放和灰白棋盘格，仅用于检查透明边缘与整体风格，不进入运行时。

## 独立 PNG 裁切

使用 [`extract_individual_pieces.py`](extract_individual_pieces.py) 按 alpha 连通区域裁切整张透明 sheet。裁切结果保存在：

```text
<location>/pieces/architecture/
<location>/pieces/furniture/
<location>/pieces/props/
```

每张输出保留 4 像素透明安全边，并使用稳定编号命名。当前编号只代表源图中的阅读顺序，不代表最终物件语义；人工选中后再使用 `tavern_bar_counter`、`library_bookcase` 等语义名称复制到运行时目录。

| 地点 | 建筑结构 | 家具 | 道具 | 合计 |
| --- | ---: | ---: | ---: | ---: |
| 酒馆 | 21 | 18 | 25 | 64 |
| 图书馆 | 21 | 18 | 24 | 63 |
| 便利店 | 26 | 12 | 39 | 77 |
| 总计 | 68 | 48 | 88 | 204 |

可追溯清单：

- `individual_pieces_manifest.json`：适合后续自动处理，记录源图、原坐标、裁切尺寸和 SHA-256。
- `individual_pieces_manifest.csv`：适合人工筛选和表格标注。
- 每个 `pieces/<kind>/` 下的 `*_pieces_contact_sheet.png`：灰白棋盘格评审图，不进入运行时。

复核结果：204 张独立文件全部为 RGBA PNG，边框最大 alpha 为 0，文件 SHA-256 与清单一致。图书馆阅读桌及其两个凳子最初因矩形包围盒产生重复，现已改为按当前连通区域掩码清除包围盒内其他物件，并分别输出三个 PNG。

复现命令：

```bash
conda run -n lab python assets/art_sources/imagegen/scene_kits/extract_individual_pieces.py
```

默认拒绝覆盖已有 `pieces/` 内容；仅在确认重建本脚本的确定性产物时使用 `--overwrite`。

## 初步视觉检查

- 三个地点的木材、描边、俯视角和光照语言与 C 风格板基本一致。
- 建筑结构包含可重复墙段、墙角、地板样片、门窗和收边；家具与道具保持独立排布，具备继续裁切的基础。
- 未出现人物、程序 UI、品牌、水印或可读生成文字。书页、包装与瓶身存在抽象像素纹理，后续单件裁切时仍需逐个复核。
- 当前 sheet 中同类物件比例并非全部严格统一；它们仍是人工筛选候选，不应整张直接作为 sprite atlas 接入。
- 下一步应先从透明候选中列出实际采用的单件、裁切边界和目标比例，再拼三张 960×540 场景草稿；不要先改程序坐标。

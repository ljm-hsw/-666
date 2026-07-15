# 音频候选资源清单

本目录保存 2026-07-15 由团队提供并接入运行时的 MP3 候选资源。文件名已统一为小写
`snake_case`，没有对音频内容进行转码、剪切或增益处理。

| 文件 | 运行时用途 | 时长 | SHA-256 |
| --- | --- | ---: | --- |
| `bgm_main_map.mp3` | 标题、小镇地图与普通结算页循环 BGM | 60.024 s | `e07c131365bf9446b5d02944a8809d2449ba13aadd26f20f5d50c74724de9d60` |
| `bgm_rainy_day.mp3` | 雨天小镇地图循环 BGM | 84.192 s | `9d00642fbb596fb06cfe95efa725d037885922ade920ffb1ae93fb34ebe16dec` |
| `bgm_restaurant.mp3` | 餐馆大厅、对话与玩法循环 BGM | 120.163 s | `7f30a6915179e758a8b5bb0b31c1c58381ce48609e01c61e740170ffe9beabe8` |
| `bgm_store.mp3` | 便利店大厅、对话与玩法循环 BGM | 144.196 s | `464ba4ee230f05c5332b4af2c0205c566684d06328a642e96b81c839a44407ff` |
| `bgm_library.mp3` | 图书馆大厅、对话与两种玩法循环 BGM | 98.038 s | `42461483eb6be0534a4b57ae4bc0224860590444e066c58eda573bcef5065c95` |
| `bgm_tavern.mp3` | 酒馆大厅与挑战循环 BGM | 130.104 s | `06113d80545438843340a354d34d4833b9a69d2cc87e40a1639c42af68db7280` |
| `bgm_home.mp3` | 家中预览与回家独白循环 BGM | 585.091 s | `3648b875a5f6d99f0cb1b1e447d80ff63b1e9f13b97b406f81e5211bfa2bf962` |
| `sfx_location_switch.mp3` | 地图与非家地点之间切换 | 1.776 s | `748e6d03a559ac909d04b9fa907fca3424c6bf00aa45bcad3f6a07238e58f6cb` |
| `sfx_return_home.mp3` | 进入家的场景 | 1.944 s | `702b60fd411afe8050295d38a63397e15bcf5909f4e96265ebd2a0bfcaebdb29` |
| `sfx_success.mp3` | 地点玩法成功完成与成功结算 | 3.360 s | `698d9f57279045824ef4fbe4c7ec7eb0438b92c75a272fe2c839fdddf226cdc6` |
| `sfx_failure.mp3` | 放弃地点玩法或酒馆失败结算 | 3.624 s | `7a9f4b7bd33323ec3ffbfcf18d9a243ef5b35323591bfccc9e3475f5173496c6` |

原始目录还包含一个错拼文件 `sfx_loaction_switch.mp3`，它与
`sfx_location switch.mp3` 内容不同但名称表达同一语义。当前只采用后者，未擅自为错拼文件
创造第二种用途；如需替换，应先完成人工 A/B 试听并更新本清单。

## 许可与验收边界

团队尚未提供作者、创作/生成工具、原始来源和可再分发许可证。以上文件只能作为内部开发和
教学演示候选，不能据此宣称可再分发或可商用。进入最终发布包前，必须补齐授权证据或替换为
许可明确的音频，并完成循环接缝、响度、场景匹配和 Windows/macOS 实机试听。

# 团队提供的室内场景合成稿

本目录保存团队成员提供、由项目接入流程归档的室内场景源图。源图与运行时图分开保存，便于追踪裁切、缩放和碰撞坐标。

## 2026-07-13 接入批次

| 场景 | 源图 | 原始尺寸 | 源图 SHA-256 | 运行时图 | 运行时 SHA-256 |
| --- | --- | ---: | --- | --- | --- |
| 餐馆 | `restaurant/restaurant_interior_source.png` | 480×272 | `c99725d91448d8a1350ad454fc19a07cd5db34309cb3bf8f8b7a3cdaca2bf721` | `../../textures/ui/restaurant/restaurant_interior.png` | `47a7fd7685b720d962e1e07f1b93cfb431708ead3de63ae9f3e3e33dc377f228` |
| 家 | `home/home_interior_source.png` | 480×272 | `e78d9d1e38e316d12faaea6ccd1e743ed389b66668ab6ac1dc4dc630fa446d70` | `../../textures/ui/home/home_interior.png` | `29934d2377c3db7082f82978617d6dc9d4e7760929080b7381f8ce59be4e49c4` |
| 图书馆 | `library/library_interior_source.png` | 960×544 | `a14da09ea46a8dcba958c4f8efd5f84273cd0e80f101b622a655b225ddaaf1b8` | `../../textures/ui/library/library_interior.png` | `95f9a2648b623f861c4479680cc0c186cfdf54745ed325a22519217232ab1043` |
| 酒馆 | `tavern/tavern_interior_source.png` | 480×272 | `9e4a11f5f3e43c0f8e4c110b422b24c7ee6f29cb02502c7207d53a33156703da` | `../../textures/ui/tavern/tavern_lobby.png` | `5e89ec3399f11ae64f2ce4d50c2fa4a78612d9e0a71c6ebd0b0ce64d6465c101` |
| 便利店 | `convenience_store/convenience_store_interior_source.png` | 480×272 | `265740198f5271eb7c55a100e6851122ea81a644a4dad621fa806fb52c7dd939` | `../../textures/ui/convenience_store/convenience_store_interior.png` | `1687b5a84d9df8120327feb2cc5f48de4ed2f49c520cc15536875d4cc2720ca2` |

处理规则：

- 餐馆、家、酒馆和便利店从垂直方向居中裁成 480×270，再使用最近邻放大到 960×540。
- 图书馆从垂直方向居中裁成 960×540，不做比例拉伸。
- 不对源图做重绘、柔化、插值或文字烘焙。
- 对应 960×540 碰撞布局位于 `src/core/scene_collision.cpp`；家具位置改变时必须同步更新布局和测试。

## 授权边界

这些图片由团队提供，但当前没有随图附带组成素材清单、作者信息或许可证证明。它们可以用于当前项目的内部开发与教学演示候选，不据此作商业可用结论。进入最终发布包前，团队必须补齐上游素材来源与许可，或替换为许可证明确的项目自制/imagegen 素材。

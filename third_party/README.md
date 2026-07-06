# 第三方依赖

本目录保存构建所需的锁定源码，项目配置和构建不会下载依赖，也不要求用户全局安装 raylib 或 doctest。

| 依赖 | 版本 | 上游归档 SHA-256 | 许可证 | 仓库内容 |
| --- | --- | --- | --- | --- |
| raylib | 6.0 | `2b3ee1e2120c7a0796b33062c7e9a694dd8a8caa56a96319ac8c8ecf54a90d0b` | zlib/libpng | 构建所需的根 CMake/打包模板、`cmake/`、`src/`、`README.md` 和 `LICENSE` |
| doctest | 2.5.2 | `9189960c2bbbc4f3382ce0773b2bb5f13e3afd8fed47f55f193e11e85a4f9854` | MIT | `doctest/` 单头文件/实现和 `LICENSE.txt` |

来源：

- `https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz`
- `https://github.com/doctest/doctest/archive/refs/tags/v2.5.2.tar.gz`

升级依赖必须更新版本、归档摘要、许可证，并重新验证 Windows 与 macOS 构建。不要在此目录混入本机构建产物。

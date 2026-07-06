# Issue Tracker: Local Markdown

本仓库的 PRD 与 issue 使用 `.scratch/` 下的本地 Markdown 文件管理，不默认发布到 GitHub、GitLab 或其他外部平台。

代码协作可以使用私有 GitHub 远程，但这不会改变 issue tracker 的本地 Markdown 约定。除非用户明确要求迁移，禁止因为检测到 GitHub remote 就自动调用 `gh issue`。

## 约定

- 每个功能使用一个目录：`.scratch/<feature-slug>/`。
- PRD 路径：`.scratch/<feature-slug>/PRD.md`。
- 实施 issue 路径：`.scratch/<feature-slug>/issues/<NN>-<slug>.md`，从 `01` 开始编号。
- 每个 issue 在文件开头附近使用 `Status:` 记录 triage 状态；可用状态见 `triage-labels.md`。
- 评论和讨论记录追加到文件末尾的 `## Comments`。

## 当技能要求“发布到 issue tracker”时

在 `.scratch/<feature-slug>/` 下创建对应 Markdown 文件；目录不存在时可以创建。

## 当技能要求“获取相关 ticket”时

读取用户指定的文件路径或 issue 编号对应的本地 Markdown 文件。

## 外部发布边界

除非用户明确要求，不调用外部 issue tracker、不创建远程 issue，也不把 `.scratch/` 内容单独发布到其他系统。`.scratch/` 是代码仓库的一部分，可以随正常 Git 版本控制进入已确认的私有代码远程；任何 commit 或 push 操作仍必须获得用户明确授权。

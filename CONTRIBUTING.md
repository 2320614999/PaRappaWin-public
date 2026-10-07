# Contributing to PaRappaWin

[Home](README.md) · [中文首页](README.zh-CN.md) · [Status](STATUS.md) · [Roadmap](ROADMAP.md)

English and Chinese reports and patches are welcome. **中英文均可。**
The current implementation and evidence scope are described in [STATUS.md](STATUS.md).
These guidelines organize submissions; they do not promise a review date or
change the release criteria.

## Start with the evidence

Check the current [status](STATUS.md), [roadmap](ROADMAP.md) and existing issues
before reporting a problem. A known limitation is still a limitation, not a
successful test. Useful contributions include reproducible build/runtime
reports, documentation corrections and narrowly scoped fixes.

For major runtime changes, explain the proposed behavior and scope in an issue
before undertaking a large rewrite. Preserve the project's native-port direction;
do not substitute an embedded emulator or call a visual approximation verified
parity. Keep Windows-specific additions intact unless a change is explicitly
part of the proposal.

## Report a reproducible problem

Use the [issue forms](https://github.com/2320614999/PaRappaWin-public/issues/new/choose).
Include the public commit SHA, Windows/toolchain information where relevant,
exact steps, expected vs. actual behavior, and a small sanitized log excerpt.
Specify physical keyboard/controller vs. injected PAD input, and same-process
return vs. a new-process readback when those distinctions matter.

Do not attach game archives, disc images, memory dumps, personal save cards,
credentials or private research artifacts. Redact user names and local paths
from diagnostics. For save-related investigation, use a backed-up, disposable
test card and describe the result rather than uploading the card.

## Prepare a focused patch

Fork the public repository and work on a branch. Keep each pull request to one
coherent change; avoid unrelated reformatting, generated compiler output or
private-history imports. Describe the behavior being changed and the evidence
supporting it. Do not include extracted game code/data or private reverse-
engineering artifacts as evidence attachments.

When a runtime change adds/removes a product source file, keep both `build.ps1`
and `CMakeLists.txt` consistent. Update the relevant public documentation when
behavior or build instructions change. Keep the English and Chinese homepages
aligned, and preserve the qualification of existing test results.

## State exactly what was tested

Follow [BUILDING.md](BUILDING.md). Use a full build rather than `-Fast` for
verification; close the checkout's game and do not run tests/gameplay concurrently
with compilation. Run appropriate tests after the build has finished.

In the PR, list commands, toolchain, results and limitations separately. Mark
checks **not run** when they were not executed, with the reason. A Linux-side
static check, a published test target, or a successful compile does not prove
Windows gameplay parity. Documentation-only patches do not need to invent a
Windows build result; check links, formatting and factual consistency instead.

## Publication and licensing

Submit only material you have the right to contribute under the repository's
applicable license. Keep [LICENSE](LICENSE), [NOTICE](NOTICE),
[THIRD_PARTY.md](THIRD_PARTY.md) and [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md)
intact. Project-owned licensing does not grant rights to third-party game data.
Do not add official art/audio/video, user saves, secrets or private Git ancestry.

## 中文速览

请先核对现有状态和问题列表，再提交可复现的反馈或单一范围的补丁。报告中注明
公开提交 SHA、环境、步骤、期望与实际结果；区分实体输入与 PAD 注入、同进程
返回与新进程读卡。未运行的检查写明“未运行”，不要把编译通过当作游戏验收。

仅提交有权公开的内容；不要上传原版资源、内存转储、个人存档、凭据或私有研究
资料。保存测试使用隔离卡并备份，不接触原有存档。新增源码需同步两套构建入口，
中英文首页与公共进度口径保持一致。具体验收范围以当前状态页为准。

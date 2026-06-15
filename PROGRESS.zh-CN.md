# PaRappaWin 移植进度

[README](README.md) | [English](PROGRESS.en-US.md) | [公开边界](PUBLIC_BOUNDARY.md)

![Stage1 overlay](https://img.shields.io/badge/Stage1_overlay-96%25-brightgreen?style=flat-square)
![Stage1 gameplay](https://img.shields.io/badge/Stage1_gameplay-100%25-brightgreen?style=flat-square)
![S0 direct runtime](https://img.shields.io/badge/S0_direct_runtime-70%25-yellowgreen?style=flat-square)
![Full build source](https://img.shields.io/badge/full_build_source-not_complete-lightgrey?style=flat-square)

> 这是 PaRappaWin 私有移植工作区的公开进度展示与阶段性源码快照。
>
> 当前仓库**不包含所有编译所需源码**。完整入口、构建编排、S0/SS0
> 与后续 stage 的部分源码仍在私有工作区；等移植完成到可公开边界后，会再分批整理并推送。

## 进度快照

下方百分比表示当前私有工作区的移植成熟度，不表示本仓库已经公开的源码比例。

| 模块 | 进度 | 状态 | 说明 |
|---|---:|---|---|
| Stage1 整体覆盖层移植 | 96% | 接近完成 | Stage1 overlay 主体已迁移。剩余主要是长尾 parity、清理、公开发布边界和打包工作。 |
| Stage1 核心 gameplay | 100% | 已完成 | 核心 gameplay 行为在私有工作区按当前标准已完成。 |
| S0 / SS0 direct runtime | 70% | 活跃推进，尚未 cutover | title、main menu、stage-select、practice、options、card/replay 等路径已有大量 direct-runtime 证据和实现切片；card/save/replay authority、`word_800916F0`、可见/音频 parity、最终 runtime cutover 仍未关闭。 |
| 公开仓可独立编译性 | 未完成 | 有意保留 | 当前公开仓是 curated snapshot，不是完整源码镜像。部分编译入口和场景编排源码暂不公开。 |

## 当前已公开

- Stage1 相关源码、文档和辅助工具。
- Windows 底层适配层：平台、媒体、解码器、PSX adapter 等已筛选源码。
- `bin/config.ini`。
- Stage1 字幕文件。
- Stage1 高清贴图映射表，不包含图片资产。
- 公开边界、路线图和同步清单。

## 当前未公开

- 所有编译所需源码的完整集合。
- S0/SS0 实现、验证工具和相关私有证据。
- Stage2 以及后续 stage 内容。
- 图片、截图、二进制、视频、PDB/IDA 文件、日志、存档和专有游戏资产。
- 仍引用未公开边界的私有入口、构建编排和完整场景调度代码。

## 后续公开节奏

1. Stage1 长尾 parity 和公开发布边界继续收敛。
2. S0/SS0 完成 card/save/replay、F0、可见/音频 parity 与 cutover blocker 后，再进入公开整理。
3. 完成移植后，编译所需源码会按可审查、可维护的批次陆续推送，而不是直接镜像私有工作区。

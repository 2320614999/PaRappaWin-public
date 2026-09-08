<p align="center">
  <img src=".github/assets/hero.svg" width="1280" alt="PaRappaWin：经典节奏，原生新生。Windows 原生移植与保存项目。">
</p>

<p align="center">
  <a href="README.md">English</a> · <strong>简体中文</strong><br>
  <a href="#看看实际运行">试玩演示</a> · <a href="BUILDING.md">编译</a> · <a href="PROGRESS.zh-CN.md">中文进度</a> · <a href="STATUS.md">项目状态</a> · <a href="ROADMAP.md">路线图</a> · <a href="CONTRIBUTING.md">参与贡献</a>
</p>

**经典节奏，原生新生。** PaRappaWin 以原程序行为为依据，将 *PaRappa the Rapper*（PSX）移植到原生 Windows 运行环境，**不是内嵌模拟器**。这里发布当前全量构建所需的源码快照，保留 Windows 专属功能与增强。

<table>
<tr>
<td width="33%" valign="top"><strong>01 / 原生运行</strong><br><br>Windows 入口、渲染、媒体、音频与输入，由移植工程自身的运行环境承载。</td>
<td width="33%" valign="top"><strong>02 / 行为保存</strong><br><br>以原版行为为参照。源码公开是里程碑，不是“已完全还原”的宣言。</td>
<td width="33%" valign="top"><strong>03 / 源码可构建</strong><br><br>当前产品源码与两套构建入口均已公开；运行所需的合法游戏数据由用户自行提供。</td>
</tr>
</table>

## 看看实际运行

<p align="center">
  <a href="https://www.youtube.com/watch?v=jLRAnNM8XWU">
    <img src="https://img.youtube.com/vi/jLRAnNM8XWU/maxresdefault.jpg" width="880" alt="在 YouTube 观看已有的 Stage 1 中文化试玩演示">
  </a><br>
  <strong><a href="https://www.youtube.com/watch?v=jLRAnNM8XWU">▶ Stage 1 · 中文化试玩演示</a></strong><br>
  <sub>PaRappaWin 原生 Windows 程序画面，非模拟器录像。此为已有演示，不代表当前 HEAD 的新增验收结果。</sub>
</p>

## 一眼看懂当前检查点

**源码快照：2026-09-08 · [c43ef46](https://github.com/2320614999/PaRappaWin-public/commit/c43ef463090d1c549143f45616bbf7d4321a42c5)**

| 439 个源码 / 头文件 / 表文件 | 153 个产品 C++ 编译单元 | 2 套构建入口 |
| :---: | :---: | :---: |
| 已公开的源码快照清单 | 已记录 PowerShell 全量编译与链接通过 | `build.ps1` + `CMakeLists.txt` |

这些数字来自[既有发布记录](BUILDING.md)，不是实时 CI 状态。记录使用 MSVC 14.44.35207 x64 / Windows SDK 10.0.19041.0；不代表 CMake 或所有源码测试均已验证。

| 范围 | 当前记录支持的结论 |
| :--- | :--- |
| **源码与 Windows 增强** | 全量构建源码已公开，平台专属功能保留，没有为发布回退实现。 |
| **S0 / SS0 / Stage1** | 实现已包含在公开源码中，原版行为对齐尚未全部完成。 |
| **Stage1 视频跳过** | 原版 Cross 链路经 PAD 注入观察到同时结束视频、字幕和音轨；实体手柄验证仍待完成。 |
| **返回 → LOAD → 高分 → 通关进度** | **仍有阻塞。** 同进程返回已发起目录资源加载，但停在底层 CD 完成反馈；已有隔离保存/读回观察不能代替此链路闭环。 |
| **Stage2 及之后** | 包含已有框架，**不代表完成移植或可以正常游玩**。 |

> [!IMPORTANT]
> **源码可编译 ≠ 游戏已完成。** 本轮实现工作仍按维护者要求暂停。此次首页改版不代表恢复开发，没有修改运行时代码，也没有新增编译或游戏实测结果。[状态与证据范围](STATUS.md) · [恢复后的验收条件](ROADMAP.md)

## 在 Windows 上构建

准备 64 位 Windows、Visual Studio 2022 C++ x64 构建工具、Windows 10/11 SDK，以及 PowerShell 5.1 或更高版本。在仓库根目录执行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File build.ps1
```

默认是**全量重编译**，不要用 `-Fast` 代替验证。编译前关闭此工作副本的游戏程序，编译时不要同时运行游戏或测试。

**产物：** `build/product/PaRappaWin.exe`，并复制到 `bin/PaRappaWin.exe`。

编译不需要游戏镜像或私有研究目录；运行需要自行提供合法持有的游戏数据。可执行文件不是完整游戏发行包。工具链覆盖参数和 CMake 替代方案见 [BUILDING.md](BUILDING.md)。

## 从这里继续了解

| 快速入口 | 详细资料 |
| :--- | :--- |
| [中文进度](PROGRESS.zh-CN.md) · [English progress](PROGRESS.en-US.md) | [状态与验证范围](STATUS.md) · [路线图](ROADMAP.md) |
| [全量编译说明](BUILDING.md) · [贡献指南](CONTRIBUTING.md) | [公开源码清单](PUBLIC_SYNC_FILELIST.txt) · [Windows 层清单](PUBLIC_WIN_LAYER_FILELIST.txt) |
| [反馈可复现的问题](https://github.com/2320614999/PaRappaWin-public/issues/new/choose) | [公开边界](PUBLIC_BOUNDARY.md) · [第三方素材说明](THIRD_PARTY.md) |

## 常见问题

<details>
<summary><strong>这是模拟器吗？</strong></summary>

不是。目标是将原程序行为移植到原生 Windows 运行环境，而非内嵌 PSX 模拟器。当前快照仍存在已记录的行为差异和未闭环流程。

</details>

<details>
<summary><strong>现在能编译吗？能玩完整游戏吗？</strong></summary>

公开版 PowerShell 全量编译通过记录见 [BUILDING.md](BUILDING.md)，但不能将其理解为全测试、全关卡通过。S0/SS0 和 Stage1 实现已公开，同进程返回目录仍阻塞，Stage2 及之后尚未完成。不用完成百分比代替端到端验收。

</details>

<details>
<summary><strong>仓库包含游戏数据吗？</strong></summary>

这不是完整游戏数据发行包，运行资源需由用户合法提供。公开范围保留此前已发布的字幕、映射、项目自制 PR2 轨道纹理与既有开机 Logo 字节头文件；本次没有新增提取的游戏素材。第三方字节数据不因公开而成为项目原创作品。详见 [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md)、[THIRD_PARTY.md](THIRD_PARTY.md) 和 [NOTICE](NOTICE)。

</details>

<details>
<summary><strong>恢复开发后最重要的下一步是什么？</strong></summary>

先修复 Stage1 返回目录的底层 CD 完成反馈，再在**同一进程**内验收返回、LOAD、高分与通关进度。新开进程读卡成功不能代替返回链路验收。详见 [ROADMAP.md](ROADMAP.md)。

</details>

<details>
<summary><strong>我能怎样参与？</strong></summary>

可复现的问题反馈、文档修正与小范围补丁都有帮助。中英文均可，请附提交版本与实际验证结果，删除个人信息，不上传游戏数据转储或个人存档卡。先阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。新增协作模板不代表开发恢复，也不承诺审核时限。

</details>

---

**许可与公开范围。** 项目原创材料采用 [Apache-2.0](LICENSE)，不授予第三方游戏数据、名称、商标、音视频或美术作品的权利。公开仓库保留独立历史；私有研究资料、凭据、配置与个人存档不属于本次发布。[NOTICE](NOTICE) · [THIRD_PARTY.md](THIRD_PARTY.md) · [PUBLIC_BOUNDARY.md](PUBLIC_BOUNDARY.md)。

<sub>页首为原创装饰性项目视觉，不是游戏截图或官方封面。[展示素材说明](.github/assets/README.md)</sub>

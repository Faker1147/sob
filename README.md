# SOB — Counter-Strike: Source Offensive 第三方 Bot

> **English:** SOB is a rebranded, custom build of the open-source
> [RCBot2](https://github.com/APGRoboCop/rcbot2) bot plugin, packaged for
> **Counter-Strike: Source Offensive (CS:SO) 2.0** (64-bit engine).
> Prebuilt Windows packages are attached to [Releases](../../releases).
> Licensed under AGPLv3 — see [License](#license).

SOB (Source Offensive Bot) 是一个**独立的第三方 bot**，不依赖游戏内置的 ZBot，
也不需要改 `botprofile.db`。它是 RCBot2 的再命名 + 适配衍生版，安装目录
`addons/sob`、命令前缀 `sob`、cvar 前缀 `sob_`。

## 特性

- **独立的第三方 Bot**：自带 AI、导航、航点系统，与内置 ZBot 无关
- **支持 64 位 CS:SO 2.0**：CS:SO 上报的引擎 ID 是 `28`（独立引擎），本仓库的
  loader 已适配，映射到 CSS 模块（Metamod 自身也是这么回退的）
- **航点导航**：自动寻路、可手动录制航点
- **武器 / 技能 / 难度配置**：`sob/config/*.ini`
- **可选 SourceMod 集成**：向 SourcePawn 暴露 `SOB_CreateBot` 等 native

## 下载

- **[Releases](../../releases)** —— 编译好的 Windows 包（推荐）
- 或者到 [Actions](../../actions) 里下载最新一次构建的 artifact

包内结构（同时含 32 位与 64 位）：

```
addons/metamod/sob.vdf          Metamod 自动加载配置
addons/sob/bin/SOBMeta.dll      loader（32 位）
addons/sob/bin/SOBMeta.x64.dll  loader（64 位）
addons/sob/bin/x64/sob.2.css.dll  64 位 Bot 本体
addons/sob/bin/sob.2.*.dll        32 位 Bot 本体（各 mod）
addons/sob/config/*.ini           配置文件
```

## 安装

**前置条件：[Metamod:Source](https://www.metamodsource.net/) 2.0**（CS:SO 必须装 64 位版本）

1. 解压下载到的包
2. 把 `addons` 文件夹覆盖进游戏根目录，例如
   `D:\Counter-Strike Source\csso\addons\`
3. 重启服务端 / 游戏
4. 控制台输入 `meta list` 验证，应当看到：

   ```
   [01] SOB    RUNNING
   ```

> 64 位下 Metamod 会把插件路径解析为 `SOBMeta.x64.dll`，所以两个 DLL 都要保留。
> 加载失败时请看 `addons/sob/sob_loader.log`，里面有明确的失败原因。

## 使用

```
bot_quota 0       // 关掉游戏内置的 ZBot（可选，避免混淆）
sob addbot        // 加一个 SOB
sob kickbot       // 踢掉一个 SOB
sob               // 显示完整命令帮助
```

其他命令：`sob config` / `sob debug` / `sob user` / `sob util` / `sob waypoint` /
`sob pathwaypoint` / `sob control` / `sob print`

### 航点（重要！）

SOB **不附带 CS:SO 的航点文件**，所以刚装好时 bot 会站着不动 —— 必须自己录：

```
sob waypoint on       // 打开航点编辑模式
sob waypoint add      // 添加航点（走到目标位置再执行）
sob waypoint save     // 保存到 addons/sob/waypoints/<地图名>.rcw
sob waypoint off      // 退出编辑模式
```

其他子命令：`sob waypoint delete|info|load|clear`

> 一个地图至少要有一条连通的航点路线，bot 才会寻路移动。
> 录之前的 `bot_quota 0` 可以避免 ZBot 干扰。

### SourceMod 集成（可选）

`scripting/` 里提供了 native 接口，让你能从 SourcePawn 里控制 SOB：

| 文件 | 放置位置 |
|---|---|
| `scripting/include/sob.inc` | `addons/sourcemod/scripting/include/` |
| `scripting/sob_sample.sp` | `addons/sourcemod/scripting/` |
| 编译产物 `sob_sample.smx` | `addons/sourcemod/plugins/` |

编译（用游戏自带的编译器即可）：

```bat
cd "D:\Counter-Strike Source\csso\addons\sourcemod\scripting"
spcomp64.exe sob_sample.sp
```

然后在游戏里：

```
sm exts list        // 应当能看到 SOB 扩展
sm plugins list     // 应当能看到 sob_sample.smx
sm_addsob TestBot   // 用 SourceMod 命令加 bot（需要 root admin）
```

## 构建

仓库自带 GitHub Actions（`.github/workflows/build.yml`），push 到 `main` 就会
在 Windows 上编译 x86 + x64 两个版本并打包成 artifact。本地要编译的话需要
MSVC + Python 3 + AMBuild，详见上游 [Building SourceMod]。

> **注意**：编译前必须对 HL2SDK 打一次兼容补丁，否则 64 位插件在 CS:SO 里会
> 加载失败（`ERROR_PROC_NOT_FOUND` / 127）。CI 会自动执行这一步，本地编译请先
> 手动运行：
>
> ```
> powershell -ExecutionPolicy Bypass -File tools\patch_tier0_threadtools.ps1 -Root alliedmodders
> ```
>
> 原因见脚本开头的注释：CS:SO 引擎的 `tier0.dll` 用的是 32 位线程 ID
> （`CThreadFastMutex::Lock(unsigned int, unsigned)`），而旧版 HL2SDK 在 64 位
> 下会去要 `Lock(unsigned __int64, unsigned)`；两个类的内存布局也不同，所以不能
> 只改名字（直接改名无法链接、运行时也会操作错偏移）。补丁的做法是把这段慢路径
> 实现**内联**到自己模块里，彻底不再从引擎导入该函数。

## 常见问题

| 现象 | 原因 / 解决 |
|---|---|
| `meta list` 里 SOB 显示失败 | 看 `addons/sob/sob_loader.log`，里面会写明原因 |
| 日志停在 `Unknown source engine id: N` | 新引擎 ID 未适配，需要在 `loader/loader.cpp` 里加分支 |
| 日志写着 `FAILED to open: 找不到指定的程序` | 就是上面那个 tier0 兼容补丁没打（错误码 127） |
| bot 加了但不动 | 没录航点，见上面「航点」一节 |
| 命令没反应 | 监听服务器里第一个进入的客户端自动获得全部权限；专用服务器请从服务端控制台输入 |

## 目录结构

```
loader/        Metamod loader shim（决定加载哪个 sob.2.<mod>.dll）
utils/SOB_meta/  Bot 主体（AI、导航、命令）
sob_subcmds/   命令子模块（waypoint / path 等）
sm_ext/        SourceMod 扩展接口（编译进插件本身）
scripting/     SourcePawn native 声明与示例插件
package/       打包配置（metamod vdf、config ini）
tools/         构建期兼容补丁（patch_tier0_threadtools.ps1）
```

## License

SOB 是 [RCBot2](https://github.com/APGRoboCop/rcbot2) 的衍生版，遵循上游的
**GNU Affero General Public License v3**（见 [LICENSE](LICENSE)）。
其中 `loader/` 与 `sm_ext/` 来自 SourceMod，为 GPLv3。
详见 [NOTICE.md](NOTICE.md)。修改后的源码必须同样公开。

## Credits

- 原始项目 **RCBot2** 与 bot AI / 导航 / 航点逻辑：Paul Murphy ("Botman")、
  [APG]RoboCop[CL]、nosoop 及各位贡献者
- Bot 基础代码：Botman's HPB Template
- SourceMod / AMBuild 构建支持：nosoop
- TF2 / MvM / CSS 等支持：Anonymous Player (caxanga334)、Ducky1231/Pongo 等
- 完整名单见上游仓库

[Building SourceMod]: https://wiki.alliedmods.net/Building_SourceMod

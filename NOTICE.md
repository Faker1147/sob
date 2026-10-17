# SOB (Source Offensive Bot)

SOB is a **rebranded, custom-built derivative** of the open-source bot project
**RCBot2** (https://github.com/APGRoboCop/rcbot2), which is licensed under the
GNU Affero General Public License v3 (see `LICENSE`).

All bot AI, navigation, waypoint and weapon logic originates from RCBot2 and
remains the work of its authors (Paul Murphy / "Botman", [APG]RoboCop[CL],
nosoop and contributors). This repository only renames the plugin identity
(plugin name, install folder `addons/sob`, console prefix `sob`, cvars
`sob_*`), adapts the Metamod loader for the CS:SO engine, and packages it as
"SOB" for use with Counter-Strike: Source Offensive (CS:SO) 2.0.

## Changes made by this repository

1. **Renamed** the plugin from RCBot2 to SOB (plugin identity, install folder
   `addons/sob`, command prefix `sob`, cvar prefix `sob_*`, loader name
   `SOBMeta`).
2. **Adapted the Metamod loader** (`loader/loader.cpp`) to accept the engine id
   (`28`) that CS:SO reports to Metamod 2.0, mapping it to the CSS bot module.
   Metamod itself ships no csso-specific core and falls back to its CSS one.
3. **Added a `csso` entry** to `package/config/bot_mods.ini` so the bot module
   picks up the right mod configuration when `gamedir` is `csso`.
4. **Fixed upstream build breakage** (missing `CBotFortress::chooseClass`
   declaration, missing FF_HudLua plugin members) and restricted CI to Windows.

## Licensing of individual parts

| Part | License |
|---|---|
| Bot core (`utils/SOB_meta`, `sob_subcmds`, `compat`, ...) | AGPLv3 (RCBot2) |
| `loader/` | GPLv3 (derived from SourceMod's loader) |
| `sm_ext/` | GPLv3 (derived from SourceMod's extension SDK) |
| `sob/logging.{h,cpp}` | 0BSD |

Because SOB is distributed under the AGPLv3, any modifications must be made
available under the same license to the players of the server it runs on.
This repository is the corresponding source for the prebuilt binaries.

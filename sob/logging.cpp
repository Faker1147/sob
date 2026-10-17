// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/**
 * Copyright 2021 nosoop
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "logging.h"

#if defined(_WIN64) || defined(_WIN32)
// for SetConsoleTextAttribute and co.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <ctime>

#include "convar.h"
#include "icvar.h"

#include "engine_wrappers.h"

#include "tier0/dbg.h"
#include "tier0/icommandline.h"

static CBotLogger s_Logger;
CBotLogger *logger = &s_Logger;

ConVar sob_loglevel("sob_loglevel", "2", 0,
                      "Display logging messages with increasing verbosity (higher number = more messages)");

const char *LOGLEVEL_STRINGS[]     = { "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE" };

const char *LOGLEVEL_ANSI_COLORS[] = { "\x1B[1;31m", "\x1B[1;91m", "\x1B[1;33m",
	                                   "\x1B[1;92m", "\x1B[1;94m", "\x1B[1;96m" };

#if defined(_WIN64) || defined(_WIN32)
#define FOREGROUND_YELLOW (FOREGROUND_GREEN | FOREGROUND_RED)
#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
//Supposed to be `WORD` not `DWORD`? [APG]RoboCop[CL]
constexpr WORD LOGLEVEL_WINCON_COLORS[] = { FOREGROUND_RED,
	                                     FOREGROUND_RED | FOREGROUND_INTENSITY,
	                                     FOREGROUND_YELLOW,
	                                     FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	                                     FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	                                     FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY };
#endif

// colors based on https://www.codeproject.com/Tips/5255355/How-to-Put-Color-on-Windows-Console
const Color LOGLEVEL_CONSOLE_COLORS[] = {
	{ 197, 15, 31, 255 }, { 231, 72, 75, 255 },  { 193, 156, 0, 255 },
	{ 19, 198, 13, 255 }, { 59, 120, 255, 255 }, { 97, 214, 214, 255 },
};

enum MessageColorizationMode : std::uint8_t
{
	Colorize_None,

	// ANSI escapes
	Colorize_ANSI,

#if defined(_WIN64)	|| defined(_WIN32)
	// windows console -- uses console text attributes
	Colorize_WinConsole,
#endif

	Colorize_ClientConsole,
};

MessageColorizationMode GetMessageColorizationMode()
{
#ifdef _LINUX
	if (!engine->IsDedicatedServer())
	{
		return Colorize_ClientConsole;
	}
	else
	{
		return Colorize_ANSI;
	}
#elif defined(_WIN64) || defined(_WIN32)
	if (!engine->IsDedicatedServer())
	{
		return Colorize_ClientConsole;
	}
	if (CommandLine()->CheckParm("-console") != nullptr)
	{
		return Colorize_WinConsole;
	}
#endif
	return Colorize_None;
}

#if defined(_WIN64) || defined(_WIN32)
// SOB: derive "<gamedir>\addons\sob" from this module's own path, e.g.
//   D:\Counter-Strike Source\csso\addons\sob\bin\x64\sob.2.css.dll
// so that paths never depend on the working directory or on the VDF file.
static bool SOB_BuildAddonFolder(char *szOut, const std::size_t iOutSize)
{
	HMODULE hSelf = nullptr;

	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCSTR>(&SOB_BuildAddonFolder), &hSelf))
	{
		return false;
	}

	char szModule[512];

	if (GetModuleFileNameA(hSelf, szModule, sizeof(szModule)) == 0)
		return false;

	for (char *p = szModule; *p; p++)
	{
		if ((*p == '\\' || *p == '/') && _strnicmp(p, "\\addons\\", 8) == 0)
		{
			snprintf(szOut, iOutSize, "%.*s\\addons\\sob", static_cast<int>(p - szModule), szModule);
			return szOut[0] != '\0';
		}
	}

	return false;
}

const char *SOB_GetAddonFolder()
{
	static char s_szFolder[512];
	static bool s_bTried = false;
	static bool s_bFound = false;

	if (!s_bTried)
	{
		s_bTried = true;
		s_bFound = SOB_BuildAddonFolder(s_szFolder, sizeof(s_szFolder));
	}

	return s_bFound ? s_szFolder : nullptr;
}

// Also write messages to "<addons/sob>/logs/sob.log" so that load failures can
// be diagnosed without access to the game console.
static void SOB_WriteLogFile(const LogLevel level, const char *szMessage)
{
	static char s_szLog[512];
	static bool s_bTried = false;
	static bool s_bReady = false;

	if (level > LogLevel::DEBUG)
		return;

	if (!s_bTried)
	{
		s_bTried = true;

		if (const char *szFolder = SOB_GetAddonFolder())
		{
			char szDir[512];

			snprintf(szDir, sizeof(szDir), "%s\\logs", szFolder);
			CreateDirectoryA(szDir, nullptr);
			snprintf(s_szLog, sizeof(s_szLog), "%s\\sob.log", szDir);
			s_bReady = true;
		}
	}

	if (!s_bReady)
		return;

	if (FILE *fp = fopen(s_szLog, "a"))
	{
		char szTime[32] = "";

		const std::time_t now = std::time(nullptr);

		if (const std::tm *pNow = std::localtime(&now))
			std::strftime(szTime, sizeof(szTime), "%Y-%m-%d %H:%M:%S", pNow);

		fprintf(fp, "%s\t[%s] %s\n", szTime, LOGLEVEL_STRINGS[level], szMessage);
		fclose(fp);
	}
}
#endif

void CBotLogger::Log(const LogLevel level, const char *fmt, ...)
{
	char buf[1024];

	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(buf, sizeof(buf), fmt, argptr);
	va_end(argptr);

#if defined(_WIN64) || defined(_WIN32)
	// Written to the log file before the console verbosity check below, because
	// the sob_loglevel cvar does not exist yet while the plugin is loading.
	SOB_WriteLogFile(level, buf);
#endif

	if (level > static_cast<LogLevel>(sob_loglevel.GetInt()))
	{
		return;
	}

	switch (GetMessageColorizationMode())
	{
	case Colorize_ANSI:
		if (level <= LogLevel::WARN)
		{
			Warning("%s[SOB] %s: %s\x1B[0m\n", LOGLEVEL_ANSI_COLORS[level], LOGLEVEL_STRINGS[level], buf);
		}
		else
		{
			Msg("%s[SOB] %s: %s\x1B[0m\n", LOGLEVEL_ANSI_COLORS[level], LOGLEVEL_STRINGS[level], buf);
		}
		break;
#if defined(_WIN64) || defined(_WIN32)
	case Colorize_WinConsole:
		HANDLE hConsoleHandle;
		hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		SetConsoleTextAttribute(hConsoleHandle, LOGLEVEL_WINCON_COLORS[level]);

		Msg("[SOB] %s: %s\n", LOGLEVEL_STRINGS[level], buf);
		SetConsoleTextAttribute(hConsoleHandle, FOREGROUND_WHITE);
		break;
#endif
	case Colorize_ClientConsole:
		extern ICvar *icvar;
		icvar->ConsoleColorPrintf(LOGLEVEL_CONSOLE_COLORS[level], "[SOB] %s: %s\n", LOGLEVEL_STRINGS[level], buf);
		break;
	case Colorize_None:
	//default:
		if (level <= LogLevel::WARN)
		{
			Warning("[SOB] %s: %s\n", LOGLEVEL_STRINGS[level], buf);
		}
		else
		{
			Msg("[SOB] %s: %s\n", LOGLEVEL_STRINGS[level], buf);
		}
		break;
	}
}

// Temporary ABI probe for the CS:SO ConVar help-text corruption.
//
// CS:SO's engine prints machine code when a SOB ConVar is typed into the console.
// That points at a ConVar layout mismatch between the pinned HL2SDK and the one
// the engine was built with: the engine reads value/help members at its own
// offsets and ends up following a code pointer.
//
// This dumps, for a SOB ConVar and for engine-owned ConVars, the raw object plus
// the vtable, so the two layouts can be compared field by field.
//
// Delete this file once the offsets are known.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "convar.h"
#include "icvar.h"

#include "bot_cvars.h"

extern ICvar *g_pCVar;

#ifdef _WIN32

#include <Windows.h>

namespace
{
	// Classifies an address: which module owns it.
	void ModuleOf(const void *addr, char *out, std::size_t outLen)
	{
		MEMORY_BASIC_INFORMATION mbi{};
		if (addr == nullptr || VirtualQuery(addr, &mbi, sizeof(mbi)) == 0)
		{
			snprintf(out, outLen, "?");
			return;
		}

		char path[MAX_PATH]{};
		if (GetModuleFileNameA(static_cast<HMODULE>(mbi.AllocationBase), path, MAX_PATH) != 0)
		{
			const char *leaf = strrchr(path, '\\');
			snprintf(out, outLen, "%s+0x%llx", leaf ? leaf + 1 : path,
				static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(addr) - reinterpret_cast<std::uintptr_t>(mbi.AllocationBase)));
			return;
		}

		snprintf(out, outLen, "0x%p", addr);
	}

	// Returns true and copies the text when p looks like a NUL terminated ASCII string.
	bool PeekString(const void *p, char *out, std::size_t outLen)
	{
		out[0] = 0;
		if (p == nullptr)
			return false;

		MEMORY_BASIC_INFORMATION mbi{};
		if (VirtualQuery(p, &mbi, sizeof(mbi)) == 0)
			return false;
		if (mbi.State != MEM_COMMIT)
			return false;
		if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
			return false;
		if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
			return false;

		const unsigned char *b = static_cast<const unsigned char *>(p);
		std::size_t i = 0;
		for (; i + 1 < outLen && i < 48; ++i)
		{
			const unsigned char c = b[i];
			if (c == 0)
				break;
			if (c < 0x20 || c > 0x7e)
				return false;
			out[i] = static_cast<char>(c);
		}
		if (i == 0)
			return false;
		out[i] = 0;
		return true;
	}

	void DumpObject(FILE *f, const char *tag, void *obj)
	{
		fprintf(f, "\n=== %s  object=0x%p  sizeof(ConCommandBase)=%zu sizeof(ConVar)=%zu sizeof(ConCommand)=%zu\n",
			tag, obj, sizeof(ConCommandBase), sizeof(ConVar), sizeof(ConCommand));
		if (obj == nullptr)
			return;

		const unsigned char *b = static_cast<const unsigned char *>(obj);
		for (int off = 0; off <= 112; off += 8)
		{
			void *q = *reinterpret_cast<void *const *>(b + off);
			char mod[160];
			ModuleOf(q, mod, sizeof(mod));

			char text[64];
			const bool isText = PeekString(q, text, sizeof(text));

			fprintf(f, "  +%-3d 0x%016llx  %-46s %s%s%s\n", off,
				static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(q)),
				mod,
				isText ? "\"" : "", isText ? text : "", isText ? "\"" : "");
		}

		void **vt = *reinterpret_cast<void ***>(obj);
		fprintf(f, "  vtable=0x%p", static_cast<void *>(vt));
		if (vt != nullptr)
		{
			for (int slot = 0; slot < 12; ++slot)
			{
				char mod[160];
				ModuleOf(vt[slot], mod, sizeof(mod));
				fprintf(f, "\n    slot%-2d 0x%016llx  %s", slot,
					static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(vt[slot])), mod);
			}
		}
		fprintf(f, "\n");
	}

	void ProbeOne(FILE *f, const char *name)
	{
		ConVar *p = (g_pCVar != nullptr) ? g_pCVar->FindVar(name) : nullptr;
		fprintf(f, "\n--- %s -> FindVar 0x%p\n", name, static_cast<void *>(p));
		if (p == nullptr)
			return;

		// Virtual calls go through the vtable of the object that answered FindVar.
		// For engine cvars that is the engine's own ConVar class, so a wrong slot
		// index shows up here as a bogus string instead of a correct one.
		const char *nameStr = p->GetName();
		fprintf(f, "  virtual GetName()     = \"%s\"\n", nameStr ? nameStr : "(null)");

		const char *help = p->GetHelpText();
		char text[64];
		const bool helpIsText = PeekString(help, text, sizeof(text));
		fprintf(f, "  virtual GetHelpText() = 0x%p %s%s%s\n", static_cast<const void *>(help),
			helpIsText ? "\"" : "([not text]) ", helpIsText ? text : "", helpIsText ? "\"" : "");

		fprintf(f, "  IsCommand()=%d IsRegistered()=%d GetDLLIdentifier()=%d\n",
			static_cast<int>(p->IsCommand()), static_cast<int>(p->IsRegistered()), static_cast<int>(p->GetDLLIdentifier()));

		DumpObject(f, name, p);
	}
}

void SOB_CvarProbe(const char *baseDir)
{
	if (baseDir == nullptr)
		return;

	char path[512];
	snprintf(path, sizeof(path), "%s/addons/sob/cvar_probe.log", baseDir);

	FILE *f = fopen(path, "w");
	if (f == nullptr)
		return;

	fprintf(f, "SOB ConVar ABI probe, build " __DATE__ " " __TIME__ "\n");

	ProbeOne(f, "sv_cheats");
	ProbeOne(f, "mp_timelimit");
	ProbeOne(f, "bot_quota");
	ProbeOne(f, "sob_addbottime");
	ProbeOne(f, "sob_visrevs");

	// Our own global: the object we registered, independent of what FindVar returns.
	DumpObject(f, "our &sob_addbottime", &sob_addbottime);

	fclose(f);
}

#else

void SOB_CvarProbe(const char *baseDir)
{
	(void)baseDir;
}

#endif

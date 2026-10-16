// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "bot_sm_ext.h"
#include "bot_sm_natives.h"

//SourceMod::IBinTools *sm_bintools = nullptr;
SourceMod::ISDKTools *sm_sdktools = nullptr;
//SourceMod::ISDKHooks *sm_sdkhooks = nullptr;

using namespace SourceMod;

SOBSourceModExt g_SOBSourceMod;

bool SOBSourceModExt::OnExtensionLoad(IExtension *me, IShareSys *sys, char *error,
		const std::size_t maxlength, bool late) {
	sharesys = sys;
	myself = me;

	/* Get the default interfaces from our configured SDK header */
	if (!SM_AcquireInterfaces(error, maxlength)) {
		return false;
	}

	sharesys->RegisterLibrary(myself, "SOB");
	sharesys->AddNatives(myself, g_SOBNatives);
	return true;
}

void SOBSourceModExt::OnExtensionUnload() {
	SM_UnsetInterfaces();
}

void SOBSourceModExt::OnExtensionsAllLoaded() {
}

void SOBSourceModExt::OnExtensionPauseChange(bool pause) {
	
}

bool SOBSourceModExt::QueryRunning(char *error, std::size_t maxlength) {
	return true;
}

bool SOBSourceModExt::IsMetamodExtension() {
	return false;
}

/// @brief caxanga334: Trying to get the interface for other SM extensions on SOBSourceModExt::OnExtensionsAllLoaded() seems to always fail.
/// My guess is since SOB is loaded from MM first, it loads before every other SM extension.
/// So we call this function from SOBPluginMeta::Hook_LevelInit
void SOBSourceModExt::LateLoadExtensions() {
	//SM_FIND_IFACE(BINTOOLS, sm_bintools);
	SM_FIND_IFACE(SDKTOOLS, sm_sdktools)
	//SM_FIND_IFACE(SDKHOOKS, sm_sdkhooks);

	if (sm_sdktools)
	{
		sm_main->LogMessage(myself, "Loaded extensions interface");
	}
}

const char *SOBSourceModExt::GetExtensionName() {
	return g_SOBPluginMeta.GetName();
}
const char *SOBSourceModExt::GetExtensionURL() {
	return g_SOBPluginMeta.GetURL();
}
const char *SOBSourceModExt::GetExtensionTag() {
	return g_SOBPluginMeta.GetLogTag();
}
const char *SOBSourceModExt::GetExtensionAuthor() {
	return g_SOBPluginMeta.GetAuthor();
}
const char *SOBSourceModExt::GetExtensionVerString() {
	return g_SOBPluginMeta.GetVersion();
}
const char *SOBSourceModExt::GetExtensionDescription() {
	return g_SOBPluginMeta.GetDescription();
}
const char *SOBSourceModExt::GetExtensionDateString() {
	return g_SOBPluginMeta.GetDate();
}

bool SM_LoadExtension(char *error, const std::size_t maxlength) {
	if ((smexts = static_cast<IExtensionManager*>(g_SMAPI->
		MetaFactory(SOURCEMOD_INTERFACE_EXTENSIONS, nullptr, nullptr))) == nullptr)
	{
		if (error && maxlength) {
			snprintf(error, maxlength, SOURCEMOD_INTERFACE_EXTENSIONS " interface not found");
		}
		return false;
	}

	/* This could be more dynamic */
	char path[256];
	g_SMAPI->PathFormat(path, sizeof(path),  "addons/sob/bin/SOBMeta%s",
#ifdef __linux__
		"_i486.so"
#else
		".dll"
#endif	
	);

	if ((myself = smexts->LoadExternal(&g_SOBSourceMod, path, "sob.ext", error, maxlength))
			== nullptr) {
		SM_UnsetInterfaces();
		return false;
	}
	return true;
}

void SM_UnloadExtension() {
	smexts->UnloadExtension(myself);
}

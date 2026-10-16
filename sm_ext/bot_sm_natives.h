#pragma once

#include <IExtensionSys.h>

using namespace SourceMod;
using namespace SourcePawn;

cell_t sm_SOBIsWaypointAvailable(IPluginContext *pContext, const cell_t *params);

cell_t sm_SOBCreate(IPluginContext *pContext, const cell_t *params);
cell_t sm_SOBIsClientBot(IPluginContext *pContext, const cell_t *params);

cell_t sm_SOBSetProfileInt(IPluginContext *pContext, const cell_t *params);
cell_t sm_SOBGetProfileInt(IPluginContext *pContext, const cell_t *params);

cell_t sm_SOBSetProfileFloat(IPluginContext *pContext, const cell_t *params);
cell_t sm_SOBGetProfileFloat(IPluginContext *pContext, const cell_t *params);

const sp_nativeinfo_t g_SOBNatives[] = {
	{ "SOB_IsWaypointAvailable", sm_SOBIsWaypointAvailable },
	
	{ "SOB_CreateBot", sm_SOBCreate },
	{ "IsSOBClient", sm_SOBIsClientBot },
	
	{ "SOB_SetProfileInt", sm_SOBSetProfileInt },
	{ "SOB_GetProfileInt", sm_SOBGetProfileInt },
	
	{ "SOB_SetProfileFloat", sm_SOBSetProfileFloat },
	{ "SOB_GetProfileFloat", sm_SOBGetProfileFloat },

	{nullptr, nullptr},
};

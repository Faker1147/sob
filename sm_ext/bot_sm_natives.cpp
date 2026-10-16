// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "bot_sm_natives.h"
#include "bot_sm_ext.h"

#include "bot.h"
#include "bot_profile.h"
#include "bot_waypoint.h"

enum SOBProfileVar : std::uint8_t {
	SOBProfile_iVisionTicks,
	SOBProfile_iPathTicks,
	SOBProfile_iVisionTicksClients,
	SOBProfile_iSensitivity,
	SOBProfile_fBraveness,
	SOBProfile_fAimSkill,
	SOBProfile_iClass,
};

int* GetIntProperty(CBotProfile* profile, SOBProfileVar profileVar);
float* GetFloatProperty(CBotProfile* profile, SOBProfileVar profileVar);

cell_t sm_SOBIsWaypointAvailable(IPluginContext *pContext, const cell_t *params) {
	return CWaypoints::numWaypoints() > 0;
}

cell_t sm_SOBCreate(IPluginContext *pContext, const cell_t *params) {
	char *name;
	pContext->LocalToString(params[1], &name);

	const int slot = CBots::createDefaultBot(name);
	
	// player slots are off-by-one (though this calculation is performed in the function)
	return (slot != -1)? slot + 1 : -1;
}

/* native void SOB_SetProfileInt(int client, SOBProfileVar property, int value); */
cell_t sm_SOBSetProfileInt(IPluginContext* pContext, const cell_t* params) {
	const int client = params[1];
	const SOBProfileVar profileVar = static_cast<SOBProfileVar>(params[2]);
	if (client < 1 || client > gpGlobals->maxClients) {
		pContext->ThrowNativeError("Invalid client index %d", client);
		return 0; // Ensure the function exits
	}

	const CBot* bot = CBots::getBot(client - 1);
	if (!bot) {
		pContext->ThrowNativeError("Client index %d is not a SOB", client);
		return 0; // Ensure the function exits
	}

	CBotProfile* profile = bot->getProfile();
	int* value = GetIntProperty(profile, profileVar);
	if (!value) {
		pContext->ThrowNativeError("SOB property %d is not an integer property", profileVar);
		return 0;
	}
	*value = params[3];

	return 0;
}

/* native void SOB_SetProfileFloat(int client, SOBProfileVar property, float value); */
cell_t sm_SOBSetProfileFloat(IPluginContext *pContext, const cell_t *params) {
	const int client = params[1];
	const SOBProfileVar profileVar = static_cast<SOBProfileVar>(params[2]);
	if (client < 1 || client > gpGlobals->maxClients) {
		pContext->ThrowNativeError("Invalid client index %d", client);
	}

	const CBot* bot = CBots::getBot(client - 1);
	if (!bot) {
		return pContext->ThrowNativeError("Client index %d is not a SOB", client);
	}

	CBotProfile* profile = bot->getProfile();
	float* value = GetFloatProperty(profile, profileVar);
	if (!value) {
		pContext->ThrowNativeError("SOB property %d is not a float property", profileVar);
		return 0;
	}
	*value = sp_ctof(params[3]);
	
	return 0;
}

/* native int SOB_GetProfileInt(int client, SOBProfileVar property); */
cell_t sm_SOBGetProfileInt(IPluginContext *pContext, const cell_t *params) {
	const int client = params[1];
	const SOBProfileVar profileVar = static_cast<SOBProfileVar>(params[2]);
	if (client < 1 || client > gpGlobals->maxClients) {
		pContext->ThrowNativeError("Invalid client index %d", client);
		return 0;
	}

	const CBot* bot = CBots::getBot(client - 1);
	if (!bot) {
		pContext->ThrowNativeError("Client index %d is not a SOB", client);
		return 0;
	}
	
	CBotProfile* profile = bot->getProfile();
	const int* value = GetIntProperty(profile, profileVar);
	if (!value) {
		pContext->ThrowNativeError("SOB property %d is not an integer property", profileVar);
		return 0;
	}
	return *value;
}

/* native float SOB_GetProfileFloat(int client, SOBProfileVar property); */
cell_t sm_SOBGetProfileFloat(IPluginContext* pContext, const cell_t* params) {
	const int client = params[1];
	const SOBProfileVar profileVar = static_cast<SOBProfileVar>(params[2]);
	if (client < 1 || client > gpGlobals->maxClients) {
		pContext->ThrowNativeError("Invalid client index %d", client);
		return 0;
	}

	const CBot* bot = CBots::getBot(client - 1);
	if (!bot) {
		pContext->ThrowNativeError("Client index %d is not a SOB", client);
		return 0;
	}

	CBotProfile* profile = bot->getProfile();
	const float* value = GetFloatProperty(profile, profileVar);
	if (!value) {
		pContext->ThrowNativeError("SOB property %d is not a float property", profileVar);
		return 0;
	}
	return sp_ftoc(*value);
}

cell_t sm_SOBIsClientBot(IPluginContext *pContext, const cell_t *params) {
	const int client = params[1];
	if (client < 1 || client > gpGlobals->maxClients) {
		pContext->ThrowNativeError("Invalid client index %d", client);
		return 0;
	}
	return CBots::getBot(client - 1) != nullptr;
}

int* GetIntProperty(CBotProfile* profile, const SOBProfileVar profileVar) {
	switch (profileVar) {
		case SOBProfile_iVisionTicks:
			return &profile->m_iVisionTicks;
		case SOBProfile_iPathTicks:
			return &profile->m_iPathTicks;
		case SOBProfile_iClass:
			return &profile->m_iClass;
		case SOBProfile_iVisionTicksClients:
			return &profile->m_iVisionTicksClients;
		case SOBProfile_iSensitivity:
			return &profile->m_iSensitivity;
		default:
			return nullptr;
	}
}

float* GetFloatProperty(CBotProfile* profile, const SOBProfileVar profileVar) {
	switch (profileVar) {
		case SOBProfile_fBraveness:
			return &profile->m_fBraveness;
		case SOBProfile_fAimSkill:
			return &profile->m_fAimSkill;
		default:
			return nullptr;
	}
}

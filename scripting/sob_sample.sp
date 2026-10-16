/**
 * Sample plugin for native calls to SOB from SourceMod.
 */
#pragma semicolon 1
#include <sourcemod>

#pragma newdecls required

#include <sob>

public void OnPluginStart() {
	RegAdminCmd("sm_addsob", AddSOB, ADMFLAG_ROOT);
	RegAdminCmd("sm_sob_supported", TestSOBSupport, ADMFLAG_ROOT);
}

public Action TestSOBSupport(int client, int argc) {
	ReplyToCommand(client, "SOB waypoint available: %b", SOB_IsWaypointAvailable());
	return Plugin_Handled;
}

public Action AddSOB(int client, int argc) {
	char name[64];
	if (argc) {
		GetCmdArg(1, name, sizeof(name));
	}
	
	int bot = SOB_CreateBot(name);
	if (bot == -1) {
		ReplyToCommand(client, "Failed to create SOB.");
	} else {
		ReplyToCommand(client, "Created SOB '%N', slot %d", bot, bot);
		PrintToServer("%N->AimSkill = %.2f", bot, SOB_GetProfileFloat(bot, SOBProfile_fAimSkill));
		PrintToServer("%N->Braveness = %.2f", bot, SOB_GetProfileFloat(bot, SOBProfile_fBraveness));
		PrintToServer("%N->PathTicks: %d", bot, SOB_GetProfileInt(bot, SOBProfile_iPathTicks));
		PrintToServer("%N->Sensitivity: %d", bot, SOB_GetProfileInt(bot, SOBProfile_iSensitivity));
		PrintToServer("%N->VisionTicks: %d", bot, SOB_GetProfileInt(bot, SOBProfile_iVisionTicks));
		PrintToServer("%N->VisionTicksClients: %d", bot, SOB_GetProfileInt(bot, SOBProfile_iVisionTicksClients));
	}
	return Plugin_Handled;
}

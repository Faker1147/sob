#if defined __sob_included
	#endinput
#endif

#define __sob_included

enum SOBProfileVar {
	// speed of recognizing non-player entities (NPCs / teleporters)
	SOBProfile_iVisionTicks,
	
	// speed of finding a path
	SOBProfile_iPathTicks,
	
	// speed of recognizing other players
	SOBProfile_iVisionTicksClients,
	
	// sensitivity of bot's "mouse"; integer between 1 and 20
	SOBProfile_iSensitivity,
	
	// sensitivity to danger (brave = less sensitive); float between 0.0 to 1.0
	SOBProfile_fBraveness,
	
	// ability to predict player movements; float between 0.0 to 1.0
	SOBProfile_fAimSkill,
	
	// preferred player class
	SOBProfile_iClass,
};

/**
 * Returns true if a waypoint is available for the current map.
 */
native bool SOB_IsWaypointAvailable();

/**
 * Instantiates an SOB bot for the current mod with a copy of the default profile.
 * 
 * @return SOB client index, or -1 on failure
 */
native int SOB_CreateBot(const char[] name);

/**
 * Returns true if the client index is an SOB bot.  This does not currently true until after
 * the bot is in the game.
 * 
 * @error Client not in game or invalid client index.
 */
native int IsSOBClient(int client);

/**
 * Sets the profile property to the given integer value.
 * 
 * @noreturn
 * @error Client is not an SOB bot or specified property is not an integer value
 */
native void SOB_SetProfileInt(int client, SOBProfileVar property, int value);

/**
 * Gets the float value for a given bot's profile property.
 * 
 * @error Client is not an SOB bot or specified property is not an integer value
 */
native int SOB_GetProfileInt(int client, SOBProfileVar property);

/**
 * Sets the profile property to the given float value.
 * 
 * @noreturn
 * @error Client is not an SOB bot or specified property is not a float value
 */
native void SOB_SetProfileFloat(int client, SOBProfileVar property, float value);

/**
 * Gets the float value for a given bot's profile property.
 * 
 * @error Client is not an SOB bot or specified property is not a float value
 */
native float SOB_GetProfileFloat(int client, SOBProfileVar property);

public Extension __ext_sob = {
	name = "SOB",
	file = "sob.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_EXTENSIONS
public void __ext_sob_SetNTVOptional() {
	MarkNativeAsOptional("SOB_IsWaypointAvailable");
	MarkNativeAsOptional("SOB_CreateBot");
	MarkNativeAsOptional("IsSOBClient");
	MarkNativeAsOptional("SOB_SetProfileInt");
	MarkNativeAsOptional("SOB_GetProfileInt");
	MarkNativeAsOptional("SOB_SetProfileFloat");
	MarkNativeAsOptional("SOB_GetProfileFloat");
}
#endif

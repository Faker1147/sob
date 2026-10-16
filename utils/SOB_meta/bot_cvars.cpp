// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <string>

#include "icvar.h"
//#include "iconvar.h"
#include "convar.h"

#include "bot_cvars.h"
#include "bot_const.h"

static ICvar* s_pCVar; //TODO: Unused? [APG]RoboCop[CL]

ConVar sob_tf2_debug_spies_cloakdisguise("sob_tf2_debug_spies_cloakdisguise", "1", 0, "Debug command : allow spy bots to cloak and disguise");
ConVar sob_tf2_medic_letgotime("sob_tf2_medic_letgotime", "0.5", 0, "Time for medic to let go of medigun to switch players");
ConVar sob_tf2_pyro_airblast("sob_tf2_pyro_airblast_ammo", "50", 0, "Ammo must be above this to airblast -- if 200 airblast will be disabled");
ConVar sob_projectile_tweak("sob_projtweak", "0.05", 0, "Tweaks the bots knowledge of projectiles and gravity");

// Fortress Forever [APG]RoboCop[CL]
ConVar sob_ff_grenades("sob_ff_grenades", "1", 0, "Fortress Forever bots throw primary (frag) hand grenades at enemies");
ConVar sob_ff_grenade_cook("sob_ff_grenade_cook", "1.8", 0, "FF bot grenade cook/hold time (s) before throwing; clamped 0.6..3.0 (fuse is 3.81s)");
ConVar sob_ff_grenade_chance("sob_ff_grenade_chance", "0.5", 0, "Chance 0..1 an FF bot throws a grenade when a valid opportunity arises");
ConVar sob_ff_sniper_charge("sob_ff_sniper_charge", "2.0", 0, "FF sniper-rifle charge/hold time (s) before the shot is released; clamped 0.5..5.0 (full charge is 5.0s)");

ConVar sob_ranged_strafe("sob_ranged_strafe", "1", 0, "FF/TF2/HL2DM bots strafe side-to-side during ranged firefights to be harder to hit (0 = stand still). Not applied to Counter-Strike (recoil widens with movement).");
ConVar sob_melee_fallback_dist("sob_melee_fallback_dist", "150", 0, "Max distance a bot charges to melee when it has no usable ranged weapon (lower = less melee-rushing)");
// FF HWGuy assault cannon scatter: aim wander per unit of target distance
// (0.05 = ~30u of wander at 600u range). 0 restores pinpoint tracking. [APG]RoboCop[CL]
ConVar sob_ff_ac_spread("sob_ff_ac_spread", "0.05", 0, "FF assault cannon aim scatter per unit of target distance (0 = pinpoint)");

// FF pogo-hops while travelling: keeps a bot at full speed instead of bleeding it to ground
// friction (FF only damps speed above 1.7x maxspeed). [APG]RoboCop[CL]
ConVar sob_ff_bunnyhop("sob_ff_bunnyhop", "1", 0, "FF bots bunnyhop while travelling out of combat (0 = always run)");

ConVar bot_cmd_enable_wpt_sounds("sob_enable_wpt_sounds", "1", 0, "Enable/disable sound effects when editing waypoints");
//ConVar bot_general_difficulty("sob_skill", "0.8", 0, "General difficulty of the bots. 0.5 = stock, < 0.5 easier, > 0.5 = harder");//TODO: Broken! [APG]RoboCop[CL]
ConVar bot_visrevs_clients("sob_visrevs_clients", "3", 0, "how many revs the bot searches for visible players and enemies, lower to reduce cpu usage");
ConVar bot_spyknifefov("sob_spyknifefov", "80", 0, "the FOV from the enemy that spies must backstab from");
ConVar bot_visrevs("sob_visrevs", "6", 0, "how many revs the bot searches for visible monsters, lower to reduce cpu usage min:5");
ConVar bot_pathrevs("sob_pathrevs", "30", 0, "how many revs the bot searches for a path each frame, lower to reduce cpu usage, but causes bots to stand still more");
ConVar bot_command("sob_cmd", "", 0, "issues a command to all bots");
ConVar bot_attack("sob_flipout", "0", 0, "Rcbots all attack");
ConVar bot_scoutdj("sob_scoutdj", "0.5", 0, "time scout uses to double jump");
ConVar bot_anglespeed("sob_anglespeed", "0.25", 0, "smaller number will make bots turn slower (1 = instant turn but may overshoot)");
ConVar bot_stop("sob_stop", "0", 0, "Make bots stop thinking!");
ConVar bot_waypointpathdist("sob_wpt_pathdist", "400", 0, "Length for waypoints to automatically add paths at");

ConVar bot_rj("sob_rj", "0.01", 0, "time for soldier to fire rocket after jumping");
ConVar bot_defrate("sob_defrate", "0.2", 0, "rate for bots to defend");
ConVar bot_beliefmulti("sob_beliefmulti", "20.0", 0, "multiplier for increasing bot belief");
ConVar bot_belief_fade("sob_belief_fade", "0.75", 0, "the multiplayer rate bot belief decreases");
ConVar bot_change_class("sob_change_classes", "0", 0, "bots change classes at random intervals");
ConVar bot_use_vc_commands("sob_voice_cmds", "1", 0, "bots use voice commands e.g. medic/spy etc");
ConVar bot_use_disp_dist("sob_disp_dist", "800.0", 0, "distance that bots will go back to use a dispenser");
ConVar bot_max_cc_time("sob_max_cc_time", "240", 0, "maximum time for bots to consider changing class <seconds>");
ConVar bot_min_cc_time("sob_min_cc_time", "60", 0, "minimum time for bots to consider changing class <seconds>");
ConVar bot_avoid_radius("sob_avoid_radius", "80", 0, "radius in units for bots to avoid things");
ConVar bot_avoid_strength("sob_avoid_strength", "100", 0, "strength of avoidance (0 = disable)");
ConVar bot_messaround("sob_messaround", "1", 0, "bots mess around at start up");
ConVar bot_heavyaimoffset("sob_heavyaimoffset", "0.1", 0, "fraction of how much the heavy aims at a diagonal offset");
ConVar bot_aimsmoothing("sob_aimsmoothing", "0", 0, "(0 = no smoothing)");//TODO: Broken! [APG]RoboCop[CL]
ConVar bot_bossattackfactor("sob_bossattackfactor", "1.0", 0, "the higher the more often the bots will shoot the boss");

ConVar sob_enemyshootfov("sob_enemyshootfov", "0.97", 0, "the fov dot product before the bot shoots an enemy 0.7 = 45 degrees");
ConVar sob_enemyshoot_gravgun_fov("sob_enemyshoot_gravgun_fov", "0.98", 0, "the fov dot product before the bot shoots an enemy 0.98 = 11 degrees");
ConVar sob_wptplace_width("sob_wpt_width", "48", 0, "width of the player, automatic paths won't connect unless there is enough space for a player");
ConVar sob_wpt_autoradius("sob_wpt_autoradius", "0", 0, "every waypoint added is given this radius, 0 = no radius");
ConVar sob_wpt_autotype("sob_wpt_autotype", "1", 0, "If 1, types will be automatically added to waypoints when they are added (only for resupply/health/capture/flag etc)\nIf 2: types will autoamtically be added even if the waypoint is cut/paste");
ConVar sob_wpt_autotype_detection_range("sob_wpt_autotype_detection_range", "80", 0, "Maximum distance between entities and a waypoint for automatic type detection.");
ConVar sob_move_sentry_time("sob_move_sentry_time", "120", 0, "seconds for bots to start thinking about moving sentries");
ConVar sob_move_sentry_kpm("sob_move_sentry_kpm", "1", 0, "kpm = kills per minute, if less than this, bots will think about moving the sentry");
ConVar sob_smoke_time("sob_smoke_time", "10", 0, "seconds a smoke grenade stays active");
ConVar sob_move_disp_time("sob_move_disp_time", "120", 0, "seconds for bots to start thinking about moving dispensers");
ConVar sob_move_disp_healamount("sob_move_disp_healamount", "100", 0, "if dispenser heals less than this per minute, bot will move the disp");
ConVar sob_demo_runup_dist("sob_demo_runup", "99.0", 0, "Distance the demo bot will take to run up for a pipe/sticky bomb jump");
ConVar sob_demo_jump("sob_enable_pipejump", "1", 0, "Enable experimental pipe jumping at rocket jump waypoints");
ConVar sob_move_tele_time("sob_move_tele_time", "120", 0, "seconds for bots to start thinking about moving teleporters");
ConVar sob_move_tele_tpm("sob_move_tele_tpm", "1", 0, "if no of players teleported per minute is less than this, bot will move the teleport");
ConVar sob_tf2_protect_cap_time("sob_tf2_prot_cap_time", "12.5", 0, "time that the bots will spend more attention to the cap point if attacked");
ConVar sob_tf2_protect_cap_percent("sob_tf2_protect_cap_percent", "0.25", 0, "the percentage that bots defend the capture point by standing on the point");
ConVar sob_tf2_spy_kill_on_cap_dist("sob_tf2_spy_kill_on_cap_dist", "200.0", 0, "the distance for spy bots to attack players capturing a point");

ConVar sob_move_dist("sob_move_dist", "800", 0, "minimum distance to move objects to");
ConVar sob_shoot_breakables("sob_shoot_breakables", "1", 0, "if 1, bots will shoot breakable objects");
ConVar sob_shoot_breakable_dist("sob_shoot_breakable_dist", "128.0", 0, "The distance bots will shoot breakables at");
ConVar sob_shoot_breakable_cos("sob_shoot_breakable_cos", "0.9", 0, "The cosine of the angle bots should worry about breaking objects at (default = 0.9) ~= 25 degrees");
ConVar sob_move_obj("sob_move_obj", "1", 0, "if 1 sob engineers will move objects around");
ConVar sob_taunt("sob_taunt", "0", 0, "enable/disable bots taunting");
ConVar sob_notarget("sob_notarget", "0", 0, "bots don't shoot the host!");
ConVar sob_nocapturing("sob_dontcapture", "0", 0, "bots don't capture flags in DOD:S");
ConVar sob_jump_obst_dist("sob_jump_obst_dist", "80", 0, "the distance from an obstacle the bot will jump");
//ConVar sob_jump_obst_speed("sob_jump_obst_speed", "100", 0, "the speed of the bot for the bot to jump an obstacle");
ConVar sob_speed_boost("sob_speed_boost", "1", 0, "multiplier for bots speed");
ConVar sob_melee_only("sob_melee_only", "0", 0, "if 1 bots will only use melee weapons");
ConVar sob_debug_iglev("sob_debug_iglev", "0", 0, "bot think ignores functions to test cpu speed"); //Not referenced properly? [APG]RoboCop[CL]
ConVar sob_dont_move("sob_dont_move", "0", 0, "if 1 , bots will all move forward");

ConVar sob_runplayercmd_dods("sob_runplayer_cmd_dods", "417", 0, "offset of the DOD:S PlayerRunCommand function");
ConVar sob_ladder_offs("sob_ladder_offs", "42", 0, "difference in height for bot to think it has touched the ladder waypoint");
ConVar sob_ffa("sob_ffa", "0", 0, "Free for all mode -- bots shoot everyone");
ConVar sob_prone_enemy_only("sob_prone_enemy_only", "1", 0, "if 1 bots only prone in DOD:S when they have an enemy");
ConVar sob_menu_update_time1("sob_menu_update_time1", "0.04", 0, "time to update menus [displaying message]");
ConVar sob_menu_update_time2("sob_menu_update_time2", "0.2", 0, "time to update menus [interval]");
ConVar sob_autowaypoint_dist("sob_autowpt_dist", "150.0", 0, "distance for autowaypoints to be placed");
ConVar sob_stats_inrange_dist("sob_stats_inrange_dist", "320.0", 0, "distance for bots to realise they have other players in range (for particular radio commands in DOD:S)");
ConVar sob_squad_idle_time("sob_squad_idle_time", "3.0", 0, "time for bots to do other things if squad leader is idle for a short time");
ConVar sob_bots_form_squads("sob_bots_form_squads", "1", 0, "if 1, bots will form their own squads via voice commands");
ConVar sob_listen_dist("sob_listen_dist", "512", 0, "the distance for bots to hear things");
ConVar sob_footstep_speed("sob_footstep_speed", "250", 0, "the speed players can go when you first hear them make footsteps");
ConVar sob_bot_squads_percent("sob_bot_squads_percent", "50", 0, "the percentage of time bots make squads with other bots");
ConVar sob_tooltips("sob_tooltips", "1", 0, "Enables/disables help tooltips");

ConVar sob_debug_notasks("sob_debug_notasks", "0", 0, "Debug command, stops bots from doing tasks by themselves");
ConVar sob_debug_dont_shoot("sob_debug_dont_shoot", "0", 0, "Debug command, stops bots from shooting everyone");
ConVar sob_debug_show_route("sob_debug_show_route", "0", 0, "Debug command, shows waypoint route to host");
ConVar sob_tf2_autoupdate_point_time("sob_tf2_autoupdate_point_time", "60", 0, "Time to automatically update points in TF2 for any changes");
ConVar sob_tf2_payload_dist_retreat("sob_tf2_payload_dist_retreat", "512.0", 0, "Distance for payload bomb to be greater than at cap before defend team retreats");
ConVar sob_spy_runaway_health("sob_spy_runaway_health", "70", 0, "health which spies run away after attacking");
ConVar sob_supermode("sob_supermode", "0", 0, "If 1 will make every bot skill and reaction much higher");
ConVar sob_addbottime("sob_addbottime", "5", 0, "The time in seconds for bots to be added after another");
ConVar sob_gamerules_offset("sob_gamerules_offset", "5", 0, "offset for gamerules object");
ConVar sob_datamap_offset("sob_datamap_offset", "0", 0, "offset for datamaps");
ConVar sob_bot_quota_interval("sob_bot_quota_interval", "10", 0, "Interval between bot quota checks, 0 or lower to disable");
ConVar sob_show_welcome_msg("sob_show_welcome_msg", "1", 0, "Show welcome message on player connect");//Not referenced properly? [APG]RoboCop[CL]
ConVar sob_force_class("sob_force_class", "0", 0, "Force bots to choose specified class, kills alive bots on change (1 - 9, set to 0 for none)");
ConVar sob_ignore_spectators("sob_ignore_spectators", "0", 0, "Ignore spectators when calculating target number of bots");
ConVar sob_nonrandom_kicking("sob_nonrandom_kicking", "0", 0, "Choose newest bot from largest team when kicking bots");
ConVar sob_nonrandom_profile("sob_nonrandom_profile", "0", 0, "Choose first free bot from profile list when creating bots");

// Synergy CVars
ConVar sob_runplayercmd_syn("sob_runplayer_cmd_syn", "424", 0, "offset of the Synergy PlayerRunCommand function");
ConVar sob_syn_use_search_range("sob_syn_use_search_range", "256", 0, "Sets the maximum button search range.", true, 150.0f, true, 1024.0f);

// Counter-Strike:Source Cvars
ConVar sob_css_economy_eco_limit("sob_css_economy_eco_limit", "2000", 0, "If the bot money is less than this, it won't purchase anything.");

ConVarRef sv_gravity("sv_gravity");
ConVarRef mp_teamplay("mp_teamplay");
ConVarRef sv_tags("sv_tags");
ConVarRef mp_friendlyfire("mp_friendlyfire");
ConVarRef mp_stalemate_enable("mp_stalemate_enable");
ConVarRef mp_stalemate_meleeonly("mp_stalemate_meleeonly");

// For CS:S
ConVarRef mp_roundtime("mp_roundtime");
ConVarRef mp_c4timer("mp_c4timer");

void SOB_Cvar_setup(ICvar* cvar) //'cvar' hides global declaration from /public/icvar.h [APG]RoboCop[CL]
{
    if (cvar == nullptr)
    {
        return; // Handle null pointer gracefully
    }

    ConVar* svTagsVar = cvar->FindVar("sv_tags");

    if (svTagsVar == nullptr)
    {
        return; // Handle missing variable gracefully
    }

    std::string sv_tags = svTagsVar->GetString();
    if (sv_tags.find("sob") == std::string::npos)
    {
        sv_tags += ",";
        sv_tags += "sob";
        svTagsVar->SetValue(sv_tags.c_str());
    }
}

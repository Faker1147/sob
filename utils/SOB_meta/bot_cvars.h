#ifndef __BOT_CVARS_H__
#define __BOT_CVARS_H__

extern ConVar sob_tf2_debug_spies_cloakdisguise;
extern ConVar sob_tf2_medic_letgotime;
extern ConVar sob_const_point_master_offset;
extern ConVar sob_tf2_pyro_airblast;
extern ConVar sob_projectile_tweak;
extern ConVar sob_ff_grenades;
extern ConVar sob_ff_grenade_cook;
extern ConVar sob_ff_grenade_chance;
extern ConVar sob_ff_sniper_charge;
extern ConVar sob_ranged_strafe;
extern ConVar sob_melee_fallback_dist;
extern ConVar sob_ff_ac_spread;
extern ConVar sob_ff_bunnyhop;
extern ConVar bot_cmd_enable_wpt_sounds;
//extern ConVar bot_general_difficulty;
extern ConVar bot_visrevs_clients;
extern ConVar bot_spyknifefov;
extern ConVar bot_visrevs;
extern ConVar bot_pathrevs;
extern ConVar bot_command;
extern ConVar bot_attack;
extern ConVar bot_scoutdj;
extern ConVar bot_anglespeed;
extern ConVar bot_stop;
extern ConVar bot_waypointpathdist;
extern ConVar bot_rj;
extern ConVar bot_defrate;
extern ConVar bot_beliefmulti;
extern ConVar bot_belief_fade;
extern ConVar bot_change_class;
extern ConVar bot_use_vc_commands;
extern ConVar bot_use_disp_dist;
extern ConVar bot_max_cc_time;
extern ConVar bot_min_cc_time;
extern ConVar bot_avoid_radius;
extern ConVar bot_avoid_strength;
extern ConVar bot_messaround;
extern ConVar bot_heavyaimoffset;
extern ConVar bot_aimsmoothing;
extern ConVar bot_bossattackfactor;
extern ConVar sob_enemyshootfov;
extern ConVar sob_enemyshoot_gravgun_fov;
extern ConVar sob_wptplace_width;
extern ConVar sob_wpt_autoradius;
extern ConVar sob_wpt_autotype;
extern ConVar sob_wpt_autotype_detection_range;
extern ConVar sob_move_sentry_time;
extern ConVar sob_move_sentry_kpm;
extern ConVar sob_smoke_time;
extern ConVar sob_move_disp_time;
extern ConVar sob_move_disp_healamount;
extern ConVar sob_demo_runup_dist;
extern ConVar sob_demo_jump;
extern ConVar sob_move_tele_time;
extern ConVar sob_move_tele_tpm;
extern ConVar sob_tf2_protect_cap_time;
extern ConVar sob_tf2_protect_cap_percent;
extern ConVar sob_tf2_spy_kill_on_cap_dist;
extern ConVar sob_move_dist;
extern ConVar sob_shoot_breakables;
extern ConVar sob_shoot_breakable_dist;
extern ConVar sob_shoot_breakable_cos;
extern ConVar sob_move_obj;
extern ConVar sob_taunt;
extern ConVar sob_notarget;
extern ConVar sob_nocapturing;
extern ConVar sob_jump_obst_dist;
extern ConVar sob_jump_obst_speed;
extern ConVar sob_speed_boost;
extern ConVar sob_melee_only;
extern ConVar sob_debug_iglev;
extern ConVar sob_dont_move;
extern ConVar sob_runplayercmd_dods;
extern ConVar sob_ladder_offs;
extern ConVar sob_ffa;
extern ConVar sob_prone_enemy_only;
extern ConVar sob_menu_update_time1;
extern ConVar sob_menu_update_time2;
extern ConVar sob_autowaypoint_dist;
extern ConVar sob_stats_inrange_dist;
extern ConVar sob_squad_idle_time;
extern ConVar sob_bots_form_squads;
extern ConVar sob_listen_dist;
extern ConVar sob_footstep_speed;
extern ConVar sob_bot_squads_percent;
extern ConVar sob_tooltips;
extern ConVar sob_debug_notasks;
extern ConVar sob_debug_dont_shoot;
extern ConVar sob_debug_show_route;
extern ConVar sob_tf2_autoupdate_point_time;
extern ConVar sob_tf2_payload_dist_retreat;
extern ConVar sob_spy_runaway_health;
extern ConVar sob_supermode;
extern ConVar sob_addbottime;
extern ConVar sob_gamerules_offset;
extern ConVar sob_datamap_offset;
extern ConVar sob_bot_quota_interval;
//extern ConVar sob_const_point_master_offset;
extern ConVar sob_util_learning;

//Synergy Cvars
extern ConVar sob_runplayercmd_syn;
extern ConVar sob_syn_use_search_range;

// Counter-Strike: Source Cvars
extern ConVar sob_css_economy_eco_limit;

/** Additional convars by pongo1231 **/
extern ConVar sob_show_welcome_msg;
extern ConVar sob_force_class;

extern ConVar sob_ignore_spectators;
extern ConVar sob_nonrandom_kicking;
extern ConVar sob_nonrandom_profile;

extern ConVarRef sv_gravity;
extern ConVarRef mp_teamplay;
extern ConVarRef sv_tags;
extern ConVarRef mp_friendlyfire;
extern ConVarRef mp_stalemate_enable;

// For CS:S
extern ConVarRef mp_roundtime;
extern ConVarRef mp_c4timer;

void SOB_Cvar_setup (ICvar* cvar);

#endif

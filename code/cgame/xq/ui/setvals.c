#include "../../cg_local.h"

void xqui_SetVals(centity_t *cent, centity_t *tcent, playerState_t *ps) {
    int level = ps->xq_level;
    int class = ps->xq_class;
    int race = ps->xq_race;
    int hp = ps->stats[STAT_HEALTH];
	int mana = ps->xq_mana;
	int hp_max = ps->xq_maxhp;
	int mana_max = ps->xq_maxmana;
	int endurance_max = ps->xq_maxendurance;
	int endurance = ps->xq_endurance;
	int energy_max = ps->xq_maxenergy;
	int energy = ps->ammo[0];
    int check_group_disband = 0;


	static char group_m1[XQ_MAX_CHAR_NAME+1] = {0};
	static char group_m2[XQ_MAX_CHAR_NAME+1] = {0};
	static char group_m3[XQ_MAX_CHAR_NAME+1] = {0};
	static char group_m4[XQ_MAX_CHAR_NAME+1] = {0};
	static char group_m5[XQ_MAX_CHAR_NAME+1] = {0};


	xqst->screenw = 640 * cgs.screenXScale;
	xqst->screenh = 480 * cgs.screenYScale;

	xqst->ui_hp_perc = ((hp_max / 100.0f) == 0 ? 0 : (hp / (hp_max / 100.0f)));
	if (hp == hp_max) xqst->ui_hp_perc = 100;

	xqst->ui_mana_perc = ((mana_max / 100.0f) == 0 ? 0 : (mana / (mana_max / 100.0f)));
	if (mana == mana_max) xqst->ui_mana_perc = 100;
	if (mana == 0) xqst->ui_mana_perc = 0;

	xqst->ui_endurance_perc = ((endurance_max / 100.0f) == 0 ? 0 : (endurance / (endurance_max / 100.0f)));
	if (endurance == endurance_max) xqst->ui_endurance_perc = 100;

	xqst->ui_energy_perc = ((energy_max / 100.0f) == 0 ? 0 : (energy / (energy_max / 100.0f)));
	if (energy == energy_max) xqst->ui_energy_perc = 100;

	int l = QW_SMALLTEXT;
	snprintf(xqst->ui_strtxt, l, "%i", ps->xq_str);
	snprintf(xqst->ui_weighttxt, l, "%s%i%s / %i", (ps->xq_weight > ps->xq_weight_max ? "^1" : ""), ps->xq_weight, (ps->xq_weight > ps->xq_weight_max ? "^7" : ""), ps->xq_weight_max);

	snprintf(xqst->ui_actxt, l, "%i", ps->xq_ac);
	snprintf(xqst->ui_atktxt, l, "%i", ps->xq_atk);

	snprintf(xqst->ui_mrtxt, l,  "  MAGIC: %i", ps->xq_mr);
	snprintf(xqst->ui_frtxt, l,  "   FIRE: %i", ps->xq_fr);
	snprintf(xqst->ui_crtxt, l,  "   COLD: %i", ps->xq_cr);
	snprintf(xqst->ui_prtxt, l,  " POISON: %i", ps->xq_pr);
	snprintf(xqst->ui_drtxt, l,  "DISEASE: %i", ps->xq_dr);
	snprintf(xqst->ui_psrtxt, l, "    PSY: %i", ps->xq_psr);

	l = QW_MEDTEXT;
	snprintf(xqst->ui_skill_tailoring, l, 		"%s: %i / %i", xq_skills_literal(XQ_SKILL_TAILORING,		0, 13), ps->xq_skill_tailoring, ps->xq_skill_tailoring_cap);
	snprintf(xqst->ui_skill_cooking, l, 		"%s: %i / %i", xq_skills_literal(XQ_SKILL_COOKING,			0, 13), ps->xq_skill_cooking, ps->xq_skill_cooking_cap);
	snprintf(xqst->ui_skill_blacksmithing, l, 	"%s: %i / %i", xq_skills_literal(XQ_SKILL_BLACKSMITHING,	0, 13), ps->xq_skill_blacksmithing, ps->xq_skill_blacksmithing_cap);
	snprintf(xqst->ui_skill_swimming, l, 		"%s: %i / %i", xq_skills_literal(XQ_SKILL_SWIMMING,			0, 13), ps->xq_skill_swimming, ps->xq_skill_swimming_cap);
	snprintf(xqst->ui_skill_channelling, l,		"%s: %i / %i", xq_skills_literal(XQ_SKILL_CHANNELLING,		0, 13), ps->xq_skill_channelling, ps->xq_skill_channelling_cap);
	snprintf(xqst->ui_skill_melee, l,			"%s: %i / %i", xq_skills_literal(XQ_SKILL_MELEE,			0, 13), ps->xq_skill_melee, ps->xq_skill_melee_cap);
	snprintf(xqst->ui_skill_fire, l,			"%s: %i / %i", xq_skills_literal(XQ_SKILL_FIRE,				0, 13), ps->xq_skill_fire, ps->xq_skill_fire_cap);
	snprintf(xqst->ui_skill_cold, l,			"%s: %i / %i", xq_skills_literal(XQ_SKILL_COLD,				0, 13), ps->xq_skill_cold, ps->xq_skill_cold_cap);
	snprintf(xqst->ui_skill_poison, l,			"%s: %i / %i", xq_skills_literal(XQ_SKILL_POISON,			0, 13), ps->xq_skill_poison, ps->xq_skill_poison_cap);
	snprintf(xqst->ui_skill_disease, l,			"%s: %i / %i", xq_skills_literal(XQ_SKILL_DISEASE,			0, 13), ps->xq_skill_disease, ps->xq_skill_disease_cap);
	snprintf(xqst->ui_skill_psy, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_PSY,				0, 13), ps->xq_skill_psy, ps->xq_skill_psy_cap);
	snprintf(xqst->ui_skill_h2h, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_H2H,				0, 13), ps->xq_skill_h2h, ps->xq_skill_h2h_cap);
	snprintf(xqst->ui_skill_w2, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_W2,				0, 13), ps->xq_skill_w2, ps->xq_skill_w2_cap);
	snprintf(xqst->ui_skill_w3, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_W3,				0, 13), ps->xq_skill_w3, ps->xq_skill_w3_cap);
	snprintf(xqst->ui_skill_w4, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_W4,				0, 13), ps->xq_skill_w4, ps->xq_skill_w4_cap);
	snprintf(xqst->ui_skill_w5, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_W5,				0, 13), ps->xq_skill_w5, ps->xq_skill_w5_cap);
	snprintf(xqst->ui_skill_w6, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_W6,				0, 13), ps->xq_skill_w6, ps->xq_skill_w6_cap);
	snprintf(xqst->ui_skill_w7, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_W7,				0, 13), ps->xq_skill_w7, ps->xq_skill_w7_cap);
	snprintf(xqst->ui_skill_w8, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_W8,				0, 13), ps->xq_skill_w8, ps->xq_skill_w8_cap);
	snprintf(xqst->ui_skill_w9, l,				"%s: %i / %i", xq_skills_literal(XQ_SKILL_W9,				0, 13), ps->xq_skill_w9, ps->xq_skill_w9_cap);
	snprintf(xqst->ui_skill_capups, l, "CapUp points: %i", ps->xq_skill_capups);



	Q_strncpyz((char *)&xqst->ui_charname, xq_getname(cent, 1), XQ_MAX_NAME);
	xqst->ui_charname[0] = toupper(xqst->ui_charname[0]);
	snprintf((char *)&xqst->ui_level, sizeof(xqst->ui_level), "Level %i", level);
	snprintf((char *)&xqst->ui_classrace, sizeof(xqst->ui_classrace), "%s %s", xq_race_lit(race, 0, 0), xq_class_lit(class, 0, 0));
	snprintf((char *)&xqst->ui_hptxt, sizeof(xqst->ui_hptxt), "%i/%i", hp, hp_max);
	snprintf((char *)&xqst->ui_hponlytxt, sizeof(xqst->ui_hponlytxt), "%i", hp);
	snprintf((char *)&xqst->ui_manaonlytxt, sizeof(xqst->ui_manaonlytxt), "%i", mana);
	snprintf((char *)&xqst->ui_enduranceonlytxt, sizeof(xqst->ui_enduranceonlytxt), "%i", endurance);
	snprintf((char *)&xqst->ui_energyonlytxt, sizeof(xqst->ui_energyonlytxt), "%i", energy);

	snprintf((char *)&xqst->ui_copper, sizeof(xqst->ui_copper), "%i", ps->xq_copper);
	snprintf((char *)&xqst->ui_silver, sizeof(xqst->ui_silver), "%i", ps->xq_silver);
	snprintf((char *)&xqst->ui_gold, sizeof(xqst->ui_gold), "%i", ps->xq_gold);
	snprintf((char *)&xqst->ui_platinum, sizeof(xqst->ui_platinum), "%i", ps->xq_platinum);


	// XP bar
	xqst->ui_xp_perc = ps->xq_xpperc;


	// Target name and HP percentage.
	xqst->ui_target_hp_perc = 0;
	xqst->ui_target_name[0] = 0;
	if (tcent && ps->xq_target > 0) {
		if (tcent->currentState.xq_enttype == XQ_ENTTYPE_PC || tcent->currentState.xq_enttype == XQ_ENTTYPE_NPC) {
			xqst->ui_target_hp_perc = ps->xq_hpperc;
		} else if (tcent->currentState.xq_enttype == XQ_ENTTYPE_PCCORPSE || tcent->currentState.xq_enttype == XQ_ENTTYPE_NPCCORPSE) {
			xqst->ui_target_hp_perc = 0.0f;
		}
	}

	Q_strncpyz((char *)&xqst->ui_target_name, xq_TargetName(), XQ_MAX_NAME);

	// Group members
	Q_strncpyz(xqst->ui_group_member_1_name, xq_GroupName(ps, 1), XQ_MAX_CHAR_NAME);
	Q_strncpyz(xqst->ui_group_member_2_name, xq_GroupName(ps, 2), XQ_MAX_CHAR_NAME);
	Q_strncpyz(xqst->ui_group_member_3_name, xq_GroupName(ps, 3), XQ_MAX_CHAR_NAME);
	Q_strncpyz(xqst->ui_group_member_4_name, xq_GroupName(ps, 4), XQ_MAX_CHAR_NAME);
	Q_strncpyz(xqst->ui_group_member_5_name, xq_GroupName(ps, 5), XQ_MAX_CHAR_NAME);

	xqst->ui_group_member_1_hp_perc = ps->xq_group_member1_hp_perc;
	xqst->ui_group_member_2_hp_perc = ps->xq_group_member2_hp_perc;
	xqst->ui_group_member_3_hp_perc = ps->xq_group_member3_hp_perc;
	xqst->ui_group_member_4_hp_perc = ps->xq_group_member4_hp_perc;
	xqst->ui_group_member_5_hp_perc = ps->xq_group_member5_hp_perc;

	qws->obj[qws->objhandles[XQ_UI_OBJID_GROUP_MEMBER_1_NAME]].group_leader = (ps->xq_group_leader == 1 ? 1 : 0);
	qws->obj[qws->objhandles[XQ_UI_OBJID_GROUP_MEMBER_2_NAME]].group_leader = (ps->xq_group_leader == 2 ? 1 : 0);
	qws->obj[qws->objhandles[XQ_UI_OBJID_GROUP_MEMBER_3_NAME]].group_leader = (ps->xq_group_leader == 3 ? 1 : 0);
	qws->obj[qws->objhandles[XQ_UI_OBJID_GROUP_MEMBER_4_NAME]].group_leader = (ps->xq_group_leader == 4 ? 1 : 0);
	qws->obj[qws->objhandles[XQ_UI_OBJID_GROUP_MEMBER_5_NAME]].group_leader = (ps->xq_group_leader == 5 ? 1 : 0);


	// See if someone joined the group
	char *cptr;
	for (cptr = xqst->ui_group_member_1_name;  cptr <= xqst->ui_group_member_5_name;  cptr += (XQ_MAX_CHAR_NAME+1)) {
		if (
			!xq_seq(cptr, group_m1) &&
			!xq_seq(cptr, group_m2) &&
			!xq_seq(cptr, group_m3) &&
			!xq_seq(cptr, group_m4) &&
			!xq_seq(cptr, group_m5)
		) {
			cptr[0] = toupper(cptr[0]);
			xq_clog(COLOR_WHITE, "%s has joined the group.", cptr);
			cptr[0] = tolower(cptr[0]);
		}
	}

	// See if someone just left the group
	for (cptr = group_m1;  cptr <= group_m5;  cptr += (XQ_MAX_CHAR_NAME+1)) {
		if (
			!xq_seq(cptr, xqst->ui_group_member_1_name) &&
			!xq_seq(cptr, xqst->ui_group_member_2_name) &&
			!xq_seq(cptr, xqst->ui_group_member_3_name) &&
			!xq_seq(cptr, xqst->ui_group_member_4_name) &&
			!xq_seq(cptr, xqst->ui_group_member_5_name)
		) {
			cptr[0] = toupper(cptr[0]);
			xq_clog(COLOR_WHITE, "%s has left the group.", cptr);
			cptr[0] = tolower(cptr[0]);
			check_group_disband = 1;
		}
	}


	if (check_group_disband == 1) {
		if (
			!xqst->ui_group_member_1_name[0] &&
			!xqst->ui_group_member_2_name[0] &&
			!xqst->ui_group_member_3_name[0] &&
			!xqst->ui_group_member_4_name[0] &&
			!xqst->ui_group_member_5_name[0]
		) {
			xq_clog(COLOR_WHITE, "Your group has been disbanded.");
		}
	}


	Q_strncpyz(group_m1, xqst->ui_group_member_1_name, XQ_MAX_CHAR_NAME);
	Q_strncpyz(group_m2, xqst->ui_group_member_2_name, XQ_MAX_CHAR_NAME);
	Q_strncpyz(group_m3, xqst->ui_group_member_3_name, XQ_MAX_CHAR_NAME);
	Q_strncpyz(group_m4, xqst->ui_group_member_4_name, XQ_MAX_CHAR_NAME);
	Q_strncpyz(group_m5, xqst->ui_group_member_5_name, XQ_MAX_CHAR_NAME);
}

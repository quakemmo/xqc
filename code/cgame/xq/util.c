/*	DESCRIPTION

	Some utility functions.
*/

#include "../cg_local.h"


qhandle_t xq_UtilModel(char *name) {
	for (int i = 0;  i < xqst->utilmodels_registered;  i++) {
		if (xq_seq(name, xqst->utilmodels[i].name)) {
			return xqst->utilmodels[i].qhandle;
		}
	}
	return 0;
}
qhandle_t xq_GfxShader(char *name, char *dfl) {
	qhandle_t ret = trap_R_RegisterShaderNoMip(name);
	if (!ret) {
		if (!dfl) {
			return 0;
		} else {
			return trap_R_RegisterShaderNoMip(dfl);
		}
	}
	return ret;
}

int xq_cnum2cname(char *cname) {
	// Returns the clientNum of the supplied PC name or -1 if not found.
	// Doesn't not include LD players.

	for (int i = 0;  i < cgs.maxclients;  i++) {
		centity_t *cent = &cg_entities[i];
		if (cent->currentState.xq_enttype == XQ_ENTTYPE_PC) {
			char *name = xq_getname(cent, 1);
			if (name[0]) {
				cname[0] = toupper(cname[0]);
				if (xq_seq(name, cname)) {
					return i;
				}
			}
		}
	}
	return -1;
}
qboolean xq_1second(void) {
    static int old_time = 0;
	if ((cg.time / 1000) > old_time) {
		old_time = cg.time / 1000;
		return qtrue;
    }
	return qfalse;
}
float *xq_Color2Vec(int color) {
	static vec4_t col;

	col[0] = ((color & 0xff0000) >> 16) / 255.0;
	col[1] = ((color & 0xff00) >> 8) / 255.0;
	col[2] = (color & 0xff) / 255.0;
	col[3] = 1.0;

	return col;
}
float *xq_AlphaPulse(int col, int speed) {
	// higher speed val = slower pulsing
	static vec4_t ret;
	if (speed < 1) speed = 1; // avoid zerodiv

	VectorCopy(xq_Color2Vec(col), ret);

    ret[3] = (((cg.clientFrame % (speed*2)) - speed)) * (1.0 / (speed*4));
	return ret;
}
void xq_DrawZoningText() {
	// In case we want to draw something while zoning..
	//UI_DrawProportionalString(320, 300, "Loading, please wait...", UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite);
}
int xq_WeaponTypeSlot(int slot) {
    xq_item_t *iinf = NULL;
	int64_t inst = 0;
	inst = xq_InvSlotItem(slot, 0, 0);
	if (!inst) {
		return 0;
	}

	iinf = xq_ItemInfo(inst);
	if (iinf) {
		return xq_weapon_skill_to_weapon(iinf->weapon_skill);
	}
	return 0;
}
int xq_WeaponActiveSlot() {
    playerState_t *ps = &cg.snap->ps;

	if (ps->weapon == WP_GAUNTLET) {
		return XQ_ITEM_SLOT_PRIMARY1;
	} else {

		if (!(ps->xq_flags & XQ_ZONE_ARENA)) {
			if (		 ps->weapon == xq_WeaponTypeSlot(XQ_ITEM_SLOT_PRIMARY2))  return XQ_ITEM_SLOT_PRIMARY2;
				else if (ps->weapon == xq_WeaponTypeSlot(XQ_ITEM_SLOT_PRIMARY3))  return XQ_ITEM_SLOT_PRIMARY3;
				else if (ps->weapon == xq_WeaponTypeSlot(XQ_ITEM_SLOT_PRIMARY4))  return XQ_ITEM_SLOT_PRIMARY4;
				else if (ps->weapon == xq_WeaponTypeSlot(XQ_ITEM_SLOT_PRIMARY5))  return XQ_ITEM_SLOT_PRIMARY5;
				else if (ps->weapon == xq_WeaponTypeSlot(XQ_ITEM_SLOT_PRIMARY6))  return XQ_ITEM_SLOT_PRIMARY6;
				else if (ps->weapon == xq_WeaponTypeSlot(XQ_ITEM_SLOT_PRIMARY7))  return XQ_ITEM_SLOT_PRIMARY7;
				else if (ps->weapon == xq_WeaponTypeSlot(XQ_ITEM_SLOT_PRIMARY8))  return XQ_ITEM_SLOT_PRIMARY8;
				else if (ps->weapon == xq_WeaponTypeSlot(XQ_ITEM_SLOT_PRIMARY9))  return XQ_ITEM_SLOT_PRIMARY9;
		} else {
			switch (ps->weapon) {
				case WP_MACHINEGUN:			return XQ_ITEM_SLOT_PRIMARY2;
				case WP_SHOTGUN:			return XQ_ITEM_SLOT_PRIMARY3;
				case WP_GRENADE_LAUNCHER:	return XQ_ITEM_SLOT_PRIMARY4;
				case WP_ROCKET_LAUNCHER:	return XQ_ITEM_SLOT_PRIMARY5;
				case WP_LIGHTNING:			return XQ_ITEM_SLOT_PRIMARY6;
				case WP_RAILGUN:			return XQ_ITEM_SLOT_PRIMARY7;
				case WP_PLASMAGUN:			return XQ_ITEM_SLOT_PRIMARY8;
				case WP_BFG:				return XQ_ITEM_SLOT_PRIMARY9;
			}
		}
	}
	return 0;
}
void xq_OptSound(vec3_t origin, int entnum, int chan, char *name, sfxHandle_t handle) {
	// Plays a custom sound file name if it's there, nothing otherwise. If name is NULL, handle will be played instead.
	sfxHandle_t h = 0;
	if (name) {
		h = CG_CustomSound(entnum, name);
	} else {
		h = handle;
	}

	if (h) {
		trap_S_StartSound(origin, entnum, chan, h);
	}
}
void xq_CmdCookie_Add(int cookie) {
	if (cookie == 0) return;

	// find LRU slot
	int lru_slot = -1;
	int lru_ts = INT_MAX;
	xq_cmdCookie_t *slot;
	for (int i = 0;  i < XQ_CMDCOOKIES_MAX;  i++) {
		slot = &xq_CmdCookies[i];
		if (lru_ts > slot->ts) {
			lru_slot = i;
			lru_ts = slot->ts;
		}
	}


	if (lru_slot == -1) {
		xq_clog(COLOR_RED, "xq_CmdCookie: unable to find an LRU slot");
		return;
	}

	slot = &xq_CmdCookies[lru_slot];
	slot->cookie = cookie;
	slot->ts = time(NULL);
}
int xq_CmdCookie_Check(int cookie) {
	if (cookie == 0) return 1;

	for (int i = 0;  i < XQ_CMDCOOKIES_MAX;  i++) {
		if (xq_CmdCookies[i].cookie == cookie) {
			return 1;
		}
	}
	return 0;
}

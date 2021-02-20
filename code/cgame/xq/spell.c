/*	DESCRIPTION

	Spell-related code.
*/

#include "../cg_local.h"
static void BardSing(playerState_t *ps) {
	// See if we're a bard playing a song and we need to recast it
	if (ps->xq_class == XQ_BARD) {
		if (xqst->autocast_gem > 0 && !ps->xq_casting) {
			if (((cg.time - ps->xq_last_bard_song_success) > 5000) || ((cg.time - xqst->autocast_gem_change_ts) < 500)) {
				if (xq_CanCastGem(xqst->autocast_gem)) {
					static uint64_t bard_recast_last_check_ms = 0;
					if ((xq_msec() - bard_recast_last_check_ms) > 100) { // At most request a recast twice per second
						bard_recast_last_check_ms = xq_msec();
						xq_CastGem(xqst->autocast_gem);
					}
				}
			}
		}
	}
}

int xq_CanCastGem(int slot) {
	// Is the slot valid?
	if (slot < 1 || slot > XQ_SPELL_SLOTS) {
		return 0;
	}


	// Is anything memmed in the specified slot?
	int spid = xqst->memmed_spells[slot-1];
	if (spid == 0) {
		return 0;
	}


	// Is the slot's casting disabled for some reason?
	if (xqst->memmed_spells_down[slot -1] == 1) {
		return 0;
	}

	
	// Do we have spell info?
	xq_spell_t *sp = xq_SpellInfo(spid);
	if (!sp) {
		return 0;
	}


	// Cannot cast if we're PvP protected
    playerState_t *ps = &cg.snap->ps;
	if (ps->xq_flags & XQ_ZONED_PROTECTED) {
		return 0;
	}


	// Validate target and range
	switch (sp->target) {
		case XQ_SPELL_TARGET_NONE:
		case XQ_SPELL_TARGET_SELF:
		case XQ_SPELL_TARGET_PBAE:
			break;

		case XQ_SPELL_TARGET_SINGLE:
		case XQ_SPELL_TARGET_TGAE:
			if (ps->xq_target == 0) {
				xq_clog(COLOR_RED, "You need a target for this spell.");
				return 0;
			}
			centity_t *cent = &cg_entities[ps->xq_target];
    		centity_t *selfcent = &cg_entities[ps->clientNum];
			if (Distance(cent->lerpOrigin, selfcent->lerpOrigin) > sp->maxrange) {
				xq_clog(COLOR_RED, "Your target is out of range.");
				return 0;
			}
			break;
	}


	// Can cast
	return 1;
}
void xq_DrawSpellIcon(int x, int y, int icon) {
	if (icon > 0) {
		xq_DrawPic(
			x,
			y,
			QW_OBJ_SPELL_GEM_WIDTH,
			QW_OBJ_SPELL_GEM_HEIGHT,
			xq_GfxShader(va("gfx/spells/%i.png", icon), "gfx/spells/default.png")
		);
	}
}
void xq_Spell_Frame(playerState_t *ps) {
	// Perform magic-related checks that must be ran
	// each client frame.


	// Autosing bard songs
	BardSing(ps);


	// Casting time bar status
	static int current_casting_hash = 0;
	if (current_casting_hash != ps->xq_casting_hash) {
		if (ps->xq_casting_hash > 0) {
			xqui_StartCastingBar(ps->xq_casting_time);
		} else {
			xqui_StopCastingBar();
		}
		current_casting_hash = ps->xq_casting_hash;
	}


	// Set some values according to info received from the server
	int i;
	for (i = 0;  i < XQ_SPELL_SLOTS;  i++) {
		xqst->memmed_spells[i] = *(&ps->xq_spell_slot_1 + i * 2);
		xqst->memmed_spells_down[i] = *(&ps->xq_spell_slot_1_down + i * 2);
	}
	xqst->casting_down = ps->xq_casting_down;


	for (i = 0;  i < XQ_SPFX_SLOTS;  i++) {
		if (*(&ps->xq_spfx_1_hash + i) != xqst->spfx_hash[i]) {
			// Spell effect for the slot changed
			xqst->spfx_hash[i] = *(&ps->xq_spfx_1_hash + i);
			xqst->spfx[i] = *(&ps->xq_spfx_1 + i);
			if (xq_debugSpells.integer) {
				xq_clog(COLOR_WHITE, "Spells: slot %i, hash %i",
					i + 1, xqst->spfx_hash[i]);
			}
		}
		xqst->spfx_remain[i] = *(&ps->xq_spfx_1_remain + i);

		if (xqst->spfx[i]) {
			xq_spell_t *sp = xq_SpellInfo(xqst->spfx[i]);
			if (sp) {
				int remain = xqst->spfx_remain[i] / 1000;

				int h = remain / 3600;
				int m = (remain - (h * 3600)) / 60;
				int s = (remain - (h * 3600) - (m * 60));

				Q_strncpyz(
					xqst->spfx_tooltips[i], 
					va(	
						"%s (%.2i:%.2i:%.2i)",
						sp->name,
						h,
						m,
						s
					),
					sizeof(xqst->spfx_tooltips[i])
				);

				qws->obj[qws->objhandles[XQ_UI_OBJID_SPFX_SLOT_1 +i]].hidden = 0;
			}
		} else {
			xqst->spfx_remain[i] = 0;
			xqst->spfx_tooltips[i][0] = 0;
			qws->obj[qws->objhandles[XQ_UI_OBJID_SPFX_SLOT_1 +i]].hidden = 1;
		}
	}
}

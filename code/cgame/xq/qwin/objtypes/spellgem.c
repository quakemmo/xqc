#include "../../../cg_local.h"

static void DrawGem(int x, int y, int icon, int slotnum) {
	slotnum--;
	int *curval = &qws->gem_fade[slotnum].curval;


	// Draw the spell gem bitmap
	if (icon > 0) {

		// Set the alpha value for the bitmap according to gem fade-in/out status
		float col[4] = {1, 1, 1, 1};
		col[3] = 1.0 - ((*curval / 100.0) * 0.66);
		trap_R_SetColor(col);

		xq_DrawPic(x, y, QW_OBJ_SPELL_GEM_WIDTH, QW_OBJ_SPELL_GEM_HEIGHT,
			xq_GfxShader(va("gfx/spells/%i.png", icon), "gfx/spells/default.png"));

		trap_R_SetColor(NULL);
	} else {
		return;
	}
}

void qw_Obj_SpellGem_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	int spid = 0;
	qw_obj_t *obj = &qws->obj[att->num];
	int brdw = obj->borderwidth;

	int x = att->x;
	int y = att->y;
	qw_AttachWithBorder(win, att, &x, &y);

	// Draw the spell gem's background
	xq_FillRect(
		x,
		y,
		QW_OBJ_SPELL_GEM_WIDTH + brdw * 2,
		QW_OBJ_SPELL_GEM_HEIGHT + brdw * 2,
		xq_Color2Vec(qw_Hover(obj))
	);


	// Draw the actual gem, if a spell is memmed in that slot
	spid = xqst->memmed_spells[obj->slot - 1];
	if (spid) {
		xq_spell_t *sp = xq_SpellInfo(spid);
		if (sp) {
			DrawGem(x + brdw, y + brdw, sp->gem_icon, obj->slot);
		}
	}
}
void qw_Obj_SpellGem_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt) {
	qw_window_t *w = &qws->win[win];
    qw_obj_t *o = &qws->obj[w->obj[obj].num];
	playerState_t *ps = &cg.snap->ps;

	// LMB click on spell gem slot
	if (button == 0) {
		if (!shift && !ctrl && !alt) {
			if (xqst->memmed_spells[o->slot - 1]) {
				// go ahead and cast the spell if something is memmed in that slot

				if (!ps->xq_sitting && !qws->buying && !qws->looting && !qws->banking && !qws->trading && ps->xq_class == XQ_BARD) {
					// for bards only
					xqst->autocast_gem_change_ts = cg.time;
					if (xqst->autocast_gem == 0) {
						xqst->autocast_gem = o->slot;
					} else {
						xqst->autocast_gem = 0;
						xq_scmd("/cast_interrupt");
					}
				} else {
					// for all other spell casters
					if (xq_CanCastGem(o->slot)) {
						xq_CastGem(o->slot);
					}
				}
			} else {
				// nothing is memmed in that slot, maybe we're trying to mem a spell?
				if (qws->mouse_spell > 0) {
					xq_scmd(va("/mem %i %i", o->slot, qws->mouse_spell));
				}
			}
			return;
		} else if (!shift && !ctrl && alt) {
			if (xqst->memmed_spells[o->slot - 1]) {
				xqui_OpenInspector(0, xqst->memmed_spells[o->slot - 1]);
				return;
			}
		}
	}

	// RMB - unmem the spell if there's one
	if (button == 1) {
		if (xqst->memmed_spells[o->slot - 1] > 0) {
			xq_scmd(va("/mem %i 0", o->slot));
		}
	}
}

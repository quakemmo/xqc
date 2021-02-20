#include "../../../cg_local.h"

//#define DBG

void qw_Obj_SpellBookSlot_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt) {
	qw_window_t *w = &qws->win[win];
    qw_obj_t *o = &qws->obj[w->obj[obj].num];
	playerState_t *ps = &cg.snap->ps;


	// Don't do anything if mouse cursor isn't empty
	if (XQ64(ps->xq_inv_mouse1_1, ps->xq_inv_mouse1_2) != 0) return;
	if (ps->xq_mouse_money_amount != 0 || ps->xq_mouse_money_type != 0) return;


	// LMB
	if (button == 0) {
		if (!shift && !ctrl && !alt) {
			// no keys - put the spell on the mouse cursor
    		qws->mouse_spell = *(&ps->xq_spellbook_slot_1 + o->slot - 1);
			return;
		} else if (!shift && !ctrl && alt) {
			// alt + LMB - inspect the spell
			xqui_OpenInspector(0, *(&ps->xq_spellbook_slot_1 + o->slot - 1));
			return;
		}
	}


	// RMB - move spells around in the spell book
	if (button == 1) {
		// Swap scribed spell position
		int pos = ((qws->spellbook_page - 1) * XQ_SPELL_BOOK_PAGESPELLS) + o->slot;
		if (qws->swap_spell == 0) {
			qws->swap_spell = pos;
			#ifdef DBG
				xq_clog(COLOR_WHITE, "Select position %i for swapping", qws->swap_spell);
			#endif
		} else {
			if (qws->swap_spell == pos) {
				xq_clog(COLOR_WHITE, "Cancelling spell position swap.");
			} else {
				xq_scmd(va("/spellbookswap %i %i", qws->swap_spell, pos));
			}
			qws->swap_spell = 0;
		}
	}
}
void qw_Obj_SpellBookSlot_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	playerState_t *ps = &cg.snap->ps;
	int pos;
	qw_obj_t *obj = &qws->obj[att->num];

	if (obj->slot < 1 || obj->slot > XQ_SPELL_BOOK_PAGESPELLS) return;

	pos = (ps->xq_spellbook_page - 1 ) * XQ_SPELL_BOOK_PAGESPELLS + obj->slot;

	obj->w = QW_OBJ_SPELL_ICON_WIDTH + 2;
	obj->h = QW_OBJ_SPELL_ICON_HEIGHT + 2;

	int x = att->x;
	int y = att->y;
	if (att->x_centered == 0) {
		if (x < 0) x = (win->w - obj->w) - QW_WIN_BORDER_WIDTH + x;
	} else {
		x = (win->w / 2) - obj->w / 2;
	}
	if (att->y_centered == 0) {
		if (y < 0) y = win->h - (TINYCHAR_HEIGHT + QW_OBJ_SPELL_ICON_HEIGHT + 2) - QW_WIN_BORDER_WIDTH + y;
	} else {
		y = (win->h / 2) - (TINYCHAR_HEIGHT + QW_OBJ_SPELL_ICON_HEIGHT + 2) / 2;
	}

	qw_AttachWithBorder(win, att, &x, &y);

	// Draw spell background
	xq_DrawRect(
		x,
		y,
		obj->w,
		obj->h,
		1,
		xq_Color2Vec(0x666666)
	);


	// Draw the spell
	int64_t val = *(&ps->xq_spellbook_slot_1 + obj->slot - 1);
	if (val) {
		xq_spell_t *sp = xq_SpellInfo(val);
		if (sp) {
			// spell icon
			xq_DrawSpellIcon(x + 1, y + 1, sp->effect_icon);

			// spell name
			int len = CG_DrawStrlen(sp->name);
			CG_DrawTinyString(
				win,
				x + ((QW_OBJ_SPELL_ICON_WIDTH + 1) / 2) - (len / 2 * TINYCHAR_WIDTH),
				y + QW_OBJ_SPELL_ICON_HEIGHT + 2,
				va("^0%s", sp->name),
				1.0f
			);
		}
	}


	// Blinking frame around the slot if it has been right-click selected for swapping
	if (qws->swap_spell == pos) {
		xq_DrawRect(
			x-3,
			y-3,
			obj->w+4,
			obj->h+4,
			4,
			xq_AlphaPulse(0x005500, 35)
		);
	}
}

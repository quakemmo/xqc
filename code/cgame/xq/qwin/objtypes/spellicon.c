#include "../../../cg_local.h"

void qw_Obj_SpellIcon_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt) {
	qw_window_t *w = &qws->win[win];
    qw_obj_t *o = &qws->obj[w->obj[obj].num];
	playerState_t *ps = &cg.snap->ps;


	// Don't do anything if mouse cursor isn't empty
	if (XQ64(ps->xq_inv_mouse1_1, ps->xq_inv_mouse1_2) != 0) return;
	if (ps->xq_mouse_money_amount != 0 || ps->xq_mouse_money_type != 0) return;
	if (o->slot < 1 || o->slot > XQ_SPFX_SLOTS) return;


	// No point clicking empty SPFX slots. The click should not even get here as clickthrough is set.
	if (!xqst->spfx[o->slot - 1]) return;


	// If the spell can be clicked off - go ahead, otherwise ignore the click
	xq_spell_t *sp = xq_SpellInfo(xqst->spfx[o->slot - 1]);
	if (sp) {
		if (button == 0) {
			if (sp->beneficial) {
				xq_scmd(va("/spfxclick %i", o->slot));
			}
		}
	}
}
void qw_Obj_SpellIcon_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	qw_obj_t *obj = &qws->obj[att->num];

	obj->w = QW_OBJ_SPELL_ICON_WIDTH;
	obj->h = QW_OBJ_SPELL_ICON_HEIGHT;

	int x = att->x;
	int y = att->y;
	qw_AttachWithBorder(win, att, &x, &y);

	int slot = obj->slot - 1;
	int spid = xqst->spfx[slot];
	if (!spid) return;
	int remain = xqst->spfx_remain[slot];


	xq_spell_t *sp = xq_SpellInfo(spid);
	if (sp) {
		float *col = xq_AlphaPulse(0, QW_SPFX_BLINK_RATE);
		if (remain > QW_SPFX_BLINK_REMAIN || col[3] > 0) {
			xq_DrawSpellIcon(x + obj->borderwidth, y + obj->borderwidth, sp->effect_icon);
		}
	}
}

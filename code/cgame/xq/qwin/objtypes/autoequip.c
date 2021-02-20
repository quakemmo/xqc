#include "../../../cg_local.h"

//#define DBG

void qw_Obj_AutoEquip_Draw(qw_window_t *win, qw_obj_attach_t *att) {

	int x = att->x;
	int y = att->y;
	qw_obj_t *obj = &qws->obj[att->num];
	qw_AttachWithBorder(win, att, &x, &y);

	xq_DrawRect(
		x,
		y,
		obj->w,
		obj->h,
		1,
		xq_Color2Vec(obj->bordercolor)
	);

	xq_FillRect(
		x + 1,
		y + 1,
		obj->w - 2,
		obj->h - 2,
		xq_Color2Vec(obj->filledcolor)
	);

	CG_DrawTinyString(win,
		x + (obj->w / 2) - (CG_DrawStrlen(qws->strings.autoequip) * TINYCHAR_WIDTH / 2),
		y + 100, qws->strings.autoequip, 1.0F);
}
void qw_Obj_AutoEquip_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt) {
	// Ask to autoequip whatever item or money we're holding on the mouse cursor
	if (
		xq_InvSlotItem(XQ_ITEM_SLOT_MOUSE1, 0, 0) != 0 ||
		cg.snap->ps.xq_mouse_money_amount != 0
	) {
		trap_S_StartLocalSound(SOUNDH("sound/inv/itemswap.wav"), CHAN_ANNOUNCER);
		xq_scmd("/autoequip");
	}
}

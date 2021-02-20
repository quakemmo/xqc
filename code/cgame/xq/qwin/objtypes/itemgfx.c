#include "../../../cg_local.h"

void qw_Obj_ItemGFX_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	qw_obj_t *obj = &qws->obj[att->num];
	if (obj->item == 0) return;

	xq_item_t *iinf = xq_ItemInfo(obj->item);

	if (iinf == NULL) {
		return;
	}

	int x = att->x;
	int y = att->y;
	qw_AttachWithBorder(win, att, &x, &y);

	xq_DrawItemIcon(win, x, y, iinf->icon, iinf->amount);
}

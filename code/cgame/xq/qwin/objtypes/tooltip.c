#include "../../../cg_local.h"

void qw_Obj_ToolTip_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	char *t;

	qw_obj_t *obj = &qws->obj[att->num];
	if (obj->text) {
		t = obj->text;
	} else if (obj->text_static) {
		t = obj->text_static;
	} else {
		return;
	}
	if (!t[0]) return;


	obj->h = TINYCHAR_HEIGHT + 4;
	obj->w = CG_DrawStrlen(t) * TINYCHAR_WIDTH + 4;

	int x = att->x;
	int y = att->y;
	qw_AttachWithBorder(win, att, &x, &y);

	// Make sure a tooltip is always fully shown on the screen
	if ((x + obj->w) > xqst->screenw) {
		att->actual_x = x = xqst->screenw - obj->w;
	}

	if ((y + obj->h) > xqst->screenh) {
		att->actual_y = y = xqst->screenh - obj->h;
	}


	// Draw the box
	xq_DrawRect(
		x,
		y,
		obj->w,
		obj->h,
		1,
		xq_Color2Vec(obj->bordercolor)
	);


	// Draw the background
	xq_FillRect(
		x + 1,
		y + 1,
		obj->w - 2,
		obj->h - 2,
		xq_Color2Vec(obj->filledcolor)
	);


	// Draw the tooltip text
	// NULL because we don't clip tooltips to any window
	CG_DrawTinyString(NULL, x+2, y+2, t, 1.0F);
}

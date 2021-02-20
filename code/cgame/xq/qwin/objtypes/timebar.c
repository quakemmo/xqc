#include "../../../cg_local.h"

void qw_Obj_TimeBar_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	qw_obj_t *obj = &qws->obj[att->num];
	if (obj->tsecs == 0) return;

	int remain = (obj->tsecs * 100) - (cg.time - obj->creation_ts);
	if (remain < 1) return;

	int x = att->x;
	int y = att->y;
	qw_AttachWithBorder(win, att, &x, &y);

	float perc = (float)remain / obj->tsecs;

	xq_FillRect(
		x + 1,
		y + 1,
		obj->barwidth * perc / 100,
		obj->barheight,
		xq_Color2Vec(obj->filledcolor)
	);
}

#include "../../cg_local.h"

void	qw_TooltipCloseAll(void) {
	for (int i = 1;  i < QW_MAX_OBJ;  i++) {
		if (qws->obj[i].type == QW_OBJ_TOOLTIP) {
			qw_ObjDetachFromAll(i);
		}
	}
}
int		qw_Hover(qw_obj_t *obj) {
	// Returns the (potentially changed) background color of a hovered object
	int retbg = obj->filledcolor;

	if (obj->hovered == 1) {
		int hbg = obj->hoverbgcolor;
		if (hbg != 1) {
			if (hbg == 0) {
				// Default Hovered bgcolor (increases r/g/b by QW_HOVER_ACCENT).
				int r = (obj->filledcolor >> 16) + QW_HOVER_ACCENT;
				if (r > 255) r = 255;
				int g = (obj->filledcolor >> 8 & 0x00FF) + QW_HOVER_ACCENT;
				if (g > 255) g = 255;
				int b = (obj->filledcolor & 0x0000FF)  + QW_HOVER_ACCENT;
				if (b > 255) b = 255;
				retbg = (r << 16) + (g << 8) + b;
			} else {
				// Hovered bgcolor is specified directly in obj->hoverbgcolor
				retbg = hbg;
			}
		}
	}
	return retbg;
}

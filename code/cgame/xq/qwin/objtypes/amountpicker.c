#include "../../../cg_local.h"
#include "../../../../client/keycodes.h"

//#define DBG

static void AmountPickerClose_Helper(void *win) {
	qw_Obj_AmountPicker_Reset();
}
void qw_Obj_AmountPicker_Reset(void) {
	qw_Obj_AmountPicker(0, 0, 0, 0, 0, 0, NULL, 0, 0, 0);
}
void qw_Obj_AmountPicker_Keypress(int key) {
	char str[QW_OBJ_AMOUNTPICKER_TEXT_LEN+1] = {0};
	qw_obj_t *o;
	int *val = &qws->amount_picker_value;

	snprintf((char *)str, QW_OBJ_AMOUNTPICKER_TEXT_LEN, "%i", *val);

	#ifdef DBG
		xq_clog(COLOR_YELLOW, "Amount picker: key pressed: %i", key);
	#endif

	if (qws->amount_picker_objnum < 1) {
		xq_clog(COLOR_RED, "Amount picker: objnum < 1 - weird");
		return;
	}
	o = &qws->obj[qws->amount_picker_objnum];

	if (key == K_BACKSPACE) {
		if (strlen(str) > 0) {
			str[strlen(str) - 1] = 0;
		}
		*val = atoi(str);
		return;
	}

	if (key == K_ENTER) {
		if (xq_seq(o->cmd, "swapwithmouseslot")) {
			if (*val > 0) {
				#ifdef DBG
					xq_clog(COLOR_YELLOW, "Submitting amountpicker value: %li - %li - %li",
						o->arg1, o->arg2, o->arg3);
				#endif
				xq_SwapWithMouseSlot(o->arg1, o->arg2, o->arg3, *val, 0);
				qw_Obj_AmountPicker_Reset();
			}
		} else if (xq_seq(o->cmd, "swapmoney")) {
			if (*val > 0) {
				xq_scmd(va("/swapmoney %li %li %i", o->arg1, o->arg2, *val));
				qw_Obj_AmountPicker_Reset();
			}
		}
		return;
	}

	if (strlen(str) == QW_OBJ_AMOUNTPICKER_TEXT_LEN) {
		return;
	} else {
		*val *= 10;
		*val += (key - 48);
		#ifdef DBG
			xq_clog(COLOR_WHITE, "minval: %i, maxval: %i, val: %i",
				o->num_minval, o->num_maxval, *val);
		#endif
		if (*val < o->num_minval) {
			*val = o->num_minval;
		}
		if (*val> o->num_maxval) {
			*val = o->num_maxval;
		}
	}
}
void qw_Obj_AmountPicker(int open, int preset, int minval, int maxval, int x, int y, char *cmd, int64_t arg1, int64_t arg2, int64_t arg3) {
	#ifdef DBG
		xq_clog(COLOR_WHITE,
			"Entering qw_Obj_AmountPicker() with preset %i, minval %i, maxval %i, cmd %s, arg1 %li, arg2 %li, arg3 %li",
			preset, minval, maxval, cmd, arg1, arg2, arg3);
	#endif


	if (open == 1) {
		// Make a new window and replace the existing one if any
		qw_WindowDelete("amount_picker");
		qw_window_t w = {
			.wid		= "amount_picker",
			.title		= "Amount Picker",
			.x			= x,
			.y			= y,
			.w			= 120,
			.h			= 45,
			.closex		= 1,
			.no_drag	= 1,
			.no_save	= 1,
			.closefnc	= AmountPickerClose_Helper
		};
		qw_WinNew(w);

		qw_obj_t tmpobj;
		qw_ObjInit(&tmpobj, QW_OBJ_AMOUNTPICKER);
		tmpobj.num_minval = minval;
		tmpobj.num_maxval = maxval;
		Q_strncpyz(tmpobj.cmd, cmd, sizeof(tmpobj.cmd));
		tmpobj.arg1 = arg1;
		tmpobj.arg2 = arg2;
		tmpobj.arg3 = arg3;
		qws->amount_picker_value = preset;

		int objnum = qw_ObjCreate(&tmpobj);
		qw_ObjAttach("amount_picker", objnum, 0, 0, 0, 0);
		qws->amount_picker_running = 1;
		qws->amount_picker_objnum = objnum;
		trap_Cvar_Set("cl_xq_amount_picker_running", "1");
	} else {
		// Remove running amount picker window
		qw_WindowDelete("amount_picker");
		qws->amount_picker_value = 0;
		qws->amount_picker_running = 0;
		qws->amount_picker_objnum = 0;
		trap_Cvar_Set("cl_xq_amount_picker_running", "0");
	}
}
void qw_Obj_AmountPicker_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	static int blink = 0;
	int val = qws->amount_picker_value;
	int x = att->x;
	int y = att->y;
	qw_AttachWithBorder(win, att, &x, &y);
	static char str[20];

	// We don't want to display anything if value is 0
	if (!val) {
		str[0] = 0;
	} else {
		snprintf((char *)str, 19, "%i", val);
	}

	// We display the cursor regardless
	if (blink++ > 20) {
		blink = 0;
	} else if (blink > 10) {
		strncat((char *)str, "|", 1);
	}
	CG_DrawTinyString(win, x+2, QW_WIN_STATUSBAR_HEIGHT + y+2, str, 1.0f);
}


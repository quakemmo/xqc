#include "../../cg_local.h"

void qw_Prompt(char *text, int actionyes, int actionno, int64_t arg1, int64_t arg2, int64_t arg3) {
	qw_obj_t tmpobj;
	char *txt = tmpobj.text_static;
	int len = sizeof(tmpobj.text_static);


	// This should probably not be here.
	// We should probably be able to have two prompt windows of different natures coexisting.
	if (qw_WindowExists("prompt") != -1) {
		return;
	}


	// Create a window
	qw_window_t win = {
		.wid		= "prompt",
		.title		= "Prompt",
		.x			= -1,
		.y			= -1,
		.w			= 300,
		.h			= 100,
		.no_save	= 1
	};
	qw_WinNew(win);


	// Add the YES button
	qw_ObjInit(&tmpobj, QW_OBJ_BUTTON);
	tmpobj.action = QW_OBJ_BUTTON_ACTION_PROMPT_YES;
	tmpobj.arg1 = actionyes;
	tmpobj.arg2 = arg1;
	tmpobj.arg3 = arg2;
	Q_strncpyz(txt, "Yes", len);
	qw_ObjAttach("prompt", qw_ObjCreate(&tmpobj), 50, 60, 0, 0);


	// Add the NO button
	qw_ObjInit(&tmpobj, QW_OBJ_BUTTON);
	tmpobj.action = QW_OBJ_BUTTON_ACTION_PROMPT_NO;
	tmpobj.arg1 = actionno;
	tmpobj.arg2 = arg1;
	tmpobj.arg3 = arg2;
	Q_strncpyz(txt, "No", len);
	qw_ObjAttach("prompt", qw_ObjCreate(&tmpobj), -50, 60, 0, 0);


	// Add the text object
	qw_ObjInit(&tmpobj, QW_OBJ_TEXT);
	Q_strncpyz(txt, text, len);
	qw_ObjAttach("prompt", qw_ObjCreate(&tmpobj), -1, 25, 1, 0);
}

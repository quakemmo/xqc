#include "../../cg_local.h"

static void MouseUp(int *mouseup_not_grabbed, int *dont_clear_mouse_spell) {

	// Mouse-up, any button
	if (qws->obj_clicked_win != -1 && qws->obj_clicked_id != -1) {
		qw_window_t *w = &qws->win[qws->obj_clicked_win];
		qw_obj_t *o = &qws->obj[w->obj[qws->obj_clicked_id].num];
		if (o->onmouseup != NULL) {
			o->onmouseup();
		}


		// Make sure mouse spell doesn't poof upon mouseup from the spell obj
		if (o->type == QW_OBJ_SPELLBOOK_SLOT) {
			*dont_clear_mouse_spell = 1;
		}
	} else {
		// RMB mouseup from temp 3D scene mouse view
		*mouseup_not_grabbed = 1;
	}

	qws->obj_clicked_win = -1;
	qws->obj_clicked_id = -1;

	if (qws->window_moving > -1) {
		qws->window_moving = -1;
		qw_SaveWindows();
	}
}
static void MouseDown(int *mouseup_not_grabbed, int *dont_clear_mouse_spell, int button, int shift, int ctrl, int alt, int clickwin) {
	int mx = xqst->mousex, my = xqst->mousey;
	qw_window_t *w = &qws->win[clickwin];
	qw_obj_click_t *findobj = qw_ObjectFindXY(clickwin, mx, my, 1, 0);
	int clickobj = findobj->objid;


	// Clicking in a window with no key modifiers will bring it to the top
	// and close the amount picker unless it is what's being clicked.
	if (!shift && !ctrl && !alt) {
		int global_topzi = qw_TopZI();
		if (w->zindex < global_topzi) {
			w->zindex = global_topzi + 1;
			qw_Obj_AmountPicker_Reset();
		}
	}


	// See if we clicked on an object or somewhere else in the window
	if (clickobj == -1) {
		// No object got the click - we're either going to drag the window around
		// or click the close X.

		if (button == 0) { // Only LMB will do anything when clicked on a window outside of any object
			if (w->no_title == 0 && w->closex == 1) {
				if (
					((w->x + w->w - QW_WIN_CLOSEX_WIDTH) < mx) &&
					((w->x + w->w) > mx) &&
					(w->y < my) &&
					((w->y + QW_WIN_STATUSBAR_HEIGHT) > my)
				) {

					if (w->closefnc) {
						w->closefnc((void *)w);
					} else {
						qw_WindowDelete(w->wid);
					}

					qws->mouse_spell = 0;
				}
			}


			// Start window dragging
			if (w->no_drag == 0) {
				qws->window_moving = clickwin;
				qws->window_change_init_x = mx - w->x;
				qws->window_change_init_y = my - w->y;
			}
		}
	} else {
		// Mousedown (any button) on an object
		qw_obj_t *o = &qws->obj[w->obj[clickobj].num];
		if (o->onmousedown != NULL) {
			qws->obj_clicked_win = clickwin;
			qws->obj_clicked_id = clickobj;
			o->onmousedown(
				clickwin,
				button,
				clickobj,
				mx - w->x - findobj->actual_x,
				my - w->y - findobj->actual_y,
				shift,
				ctrl,
				alt
			);


			// If we pick a scribed spell on the mouse, it has to stay there
			if (o->type == QW_OBJ_SPELLBOOK_SLOT) {
				*dont_clear_mouse_spell = 1;
			}
		}
	}
}

int qw_MouseClick(int button, int down, int shift, int ctrl, int alt) {
	// This will return 1 if QW caught the mouse click and 1 if it didn't.
	// Not catching the click usually means we clicked somewhere outside a QW window,
	// for example the 3D scene.

	int dont_clear_mouse_spell = 0;
	int mouseup_not_grabbed = 0;

	if (down == 1) {
		int clickwin = qw_WindowFindXY(xqst->mousex, xqst->mousey, NULL);
		if (clickwin == -1) {
			qws->mouse_spell = 0;
			return 0;
		} else {
			MouseDown(&mouseup_not_grabbed, &dont_clear_mouse_spell, button, shift, ctrl, alt, clickwin);
		}
	} else {
		MouseUp(&mouseup_not_grabbed, &dont_clear_mouse_spell);
	}


	// Clicking anything should clear the mouse spell unless specified.
	if (!dont_clear_mouse_spell) {
		qws->mouse_spell = 0;
	}


	if (mouseup_not_grabbed == 1) {
		return 0;
	} else {
		return 1;
	}
}
void qw_MouseMove(void) {
	int i;

	// Make sure the resize button is removed - it will be
	// added back if needed later on.
	if (qws->objhandles[QW_OBJID_GEN_RESIZE] > 0) {
		qw_ObjDetachFromAll(qws->objhandles[QW_OBJID_GEN_RESIZE]);
	}


	// Clear the hover tag from all objs
	for (i = 1; i < QW_MAX_OBJ - 1;  i++) {
		qws->obj[i].hovered = 0;
	}


	// See if we're currently hovering over a window.
	// If we aren't - hoverwin will be -1
	int hoverwin = qw_WindowFindXY(xqst->mousex, xqst->mousey, NULL);


	// Fade in alpha windows we're hovering over, fade out all others
	// Buying / banking / trading fades in all windows, regardless.
	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		qw_window_t *w = &qws->win[i];
		if (w->wid[0]) {
			if (
				hoverwin == i ||
				qws->buying == 1 ||
				qws->banking == 1 ||
				qws->trading > 0
			) {
				w->alpha_dir = 1;
			} else {
				w->alpha_dir = 2;
			}
		}
	}


	// If we are hovering over a window, do some stuff
	if (hoverwin != -1) {
		qw_window_t *hwh = &qws->win[hoverwin];

		// Attach the resize button if the window is resizable
		if (hwh->resizable == 1) {
			qw_ObjAttach(hwh->wid, qws->objhandles[QW_OBJID_GEN_RESIZE], -1, -1, 0, 0);
		}


		// Let's see if we're hovering over an object that's sensible to hovering
		// (ie: button for bg color change)
		qw_obj_click_t *findobj = qw_ObjectFindXY(hoverwin, xqst->mousex, xqst->mousey, 0, 0);
		if (findobj->objid > -1) {
			int objnum = hwh->obj[findobj->objid].num;
			qws->obj[objnum].hovered = 1;
		}
	}
}
void qw_AttachWithBorder(qw_window_t *win, qw_obj_attach_t *att, int *x, int *y) {
	// This is called by most qw_Obj_*_Draw() functions in order to
	// set the object's coords according to the window border.
	// There may be exceptions, eg: PercBar, where a custom logic is applied instead.

	att->actual_x = *x;
	att->actual_y = *y;

	*x += (win->x + QW_WIN_BORDER_WIDTH);
	*y += (win->y + QW_WIN_BORDER_WIDTH);
}
void qw_Render(void) {
	uint64_t st_time = xq_msec();

	int i, cnt = 0;
	static int sorted[QW_MAX_WINDOWS];


	// Get indexes of all active windows in an array
	memset(sorted, 0, QW_MAX_WINDOWS*sizeof(int));
	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (qws->win[i].wid[0]) {
			sorted[cnt++] = i;
		}
	}

	// Sorted them according to their zindex
	qsort(sorted, cnt, sizeof(int), qw_WindowZICompareFnc);

	// Draw them all
	for (i = 0;  i < cnt;  i++) {
		if (qws->win[sorted[i]].wid[0]) {
			qw_WindowDraw(sorted[i]);
		}
	}


	// Draw the arena-style ephemeral messages
	xq_debdisp_draw();


	uint64_t en_time = xq_msec();
	if (cg_qwBench.integer > 0) {
		if (cg_qwBench.integer < en_time - st_time) {
			xq_clog(COLOR_WHITE, "QWIN took %lu msec to finish", en_time - st_time);
		}
	}
}
void qw_Init(void) {
	// Create and save window resize object
	static char resizetext[] = "\213"; // box in bigchars.tga


	// Create and save the generic window resize button
	qw_obj_t tmpobj;
	qw_ObjInit(&tmpobj, QW_OBJ_TEXT);
	tmpobj.text = resizetext;
	tmpobj.onmousedown = qw_ResizeStart;
	tmpobj.onmouseup = qw_ResizeStop;
	tmpobj.clickthrough = 0;
	qws->objhandles[QW_OBJID_GEN_RESIZE] = qw_ObjCreate(&tmpobj);


	// Set some state things up
	qws->window_moving = -1;
	qws->window_resizing = -1;
	qws->obj_clicked_id = -1;
	qws->obj_clicked_win = -1;
}

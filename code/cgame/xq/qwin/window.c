#ifndef _WIN32
	#define _GNU_SOURCE
#endif
#include "../../cg_local.h"

static qw_window_save_t *qw_LoadAllWindows(void) {
	char	buf[QW_SAVE_WINDOWS * (QW_MAX_WID + 30)];
	char	wid[QW_MAX_WID+1] = {0};
	static qw_window_save_t	wins[QW_SAVE_WINDOWS];
	int		cnt = 0;

	memset(wins, 0, sizeof(wins));
	trap_Cvar_VariableStringBuffer("xq_winpos", buf, sizeof(buf));

	char *token = strtok(buf, ":");
	while (token != NULL) {
		wid[0] = 0;
		int x = -1, y = -1, w = -1, h = -1;
		//char *fmt = va("%%%is %%d %%d %%d %%d", QW_MAX_WID);
		char fmt[20] = {0};
		snprintf(fmt, 19, "%%%is %%d %%d %%d %%d", QW_MAX_WID);
		if (sscanf(token, fmt, wid, &x, &y, &w, &h) == 5) {
			Q_strncpyz(wins[cnt].wid, wid, sizeof(wins[cnt].wid));
			wins[cnt].x = x;
			wins[cnt].y = y;
			wins[cnt].w = w;
			wins[cnt].h = h;
			cnt++;
		}
        token = strtok(NULL, ":");
	}
	return wins;
}
static qw_window_save_t *qw_LoadWindow(const char *name) {
	int cnt = 0;
	qw_window_save_t *wins = qw_LoadAllWindows();

	while (cnt < QW_MAX_WINDOWS) {
		if (!wins[cnt].wid[0]) break;
		if (xq_seq((char *)wins[cnt].wid, (char *)name)) {
			return &wins[cnt];
		}
		cnt++;
	}
	return NULL;
}

void qw_SaveWindows(void) {
	int 	i, j;
	char 	pos[1024];
	char 	buf[100];
	char 	*ptr = pos;
	struct save {
		char 	wid[QW_MAX_WID+1];
	};
	struct save	done[QW_SAVE_WINDOWS];
	int save_done = 0;
	int not_saved = 0;

	int cnt = 0;
	qw_window_save_t *wins;
	int active_saved = 0;

	memset(done, 0, sizeof(done));


	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (qws->win[i].wid[0]) {
			not_saved = 1;
			for (j = 0;  j < QW_SAVE_WINDOWS;  j++) {
				if (xq_seq(qws->win[i].wid, done[j].wid)) {
					not_saved = 0;
					break;
				}
			}
		
			if (qws->win[i].no_save == 0 && not_saved == 1) {
				snprintf((char *)buf, 99,
					"%s %i %i %i %i:",
					qws->win[i].wid,
					(int)qws->win[i].x,
					(int)qws->win[i].y,
					(int)qws->win[i].w,
					(int)qws->win[i].h);
				strncpy(ptr, (char *)buf, strlen(buf));
				ptr += strlen(buf);
				*ptr = 0;
				Q_strncpyz(done[save_done++].wid, qws->win[i].wid, QW_MAX_WID);
			}
		}
	}
	active_saved = save_done;



	// Read cvar and copy all windows that haven't been saved yet to string
	wins = qw_LoadAllWindows();
	while (cnt < (QW_SAVE_WINDOWS - active_saved)) {
		not_saved = 1;
		for (j = 0;  j < QW_SAVE_WINDOWS;  j++) {
			if (xq_seq(wins[cnt].wid, done[j].wid)) {
				not_saved = 0;
				break;
			}
		}
		if (not_saved == 1) {
			snprintf((char *)buf, 99,
				"%s %i %i %i %i:",
				wins[cnt].wid,
				wins[cnt].x,
				wins[cnt].y,
				wins[cnt].w,
				wins[cnt].h
			);
			strncpy(ptr, (char *)buf, strlen(buf));
			ptr += strlen(buf);
			*ptr = 0;
			Q_strncpyz(done[save_done++].wid, wins[cnt].wid, QW_MAX_WID);
		}
		cnt++;
	}


	// Write the coords to cvar
	trap_Cvar_Set("xq_winpos", pos);
}
void	qw_WinAlpha(char *wid) {
	qw_window_t *win;
	int wnum = qw_WindowExists(wid);
	if (wnum == -1) return;

	win = &qws->win[wnum];
	
	// If we're in combat mode, we probably want windows on low alpha
	if (xqst->mousefree == 0) {
		win->alpha_dir = 2;
	}


	// Move the current window alpha value in the set direction
	if (win->alpha_dir == 1) {
		if (win->alpha < win->alpha_max) {
			win->alpha += win->alpha_max / 20; // fade in quickly
		}
	} else if (win->alpha_dir == 2) {
		if (win->alpha > win->alpha_min) {
			win->alpha -= win->alpha_max / 100; // fade out 5x as slowly
		}
	}


	// Don't let the alpha values overflow set min / max limits
	if (win->alpha > win->alpha_max) {
		win->alpha = win->alpha_max;
		win->alpha_dir = 0;
	}

	if (win->alpha < win->alpha_min) {
		win->alpha = win->alpha_min;
		win->alpha_dir = 0;
	}
}
int		qw_TopZI() {
	// returns the current highest zindex used by a window
	int topzi = -1;
	for (int i = 0;  i < QW_MAX_WINDOWS;  i++) {
		qw_window_t *w = &qws->win[i];
		if (!w->wid[0]) break;
		if (topzi < w->zindex) {
			topzi = w->zindex;
		}
	}
	return topzi;
}
int		qw_WindowFindXY(int x, int y, int *topzi) {
	// Find all windows at X/Y.  If there are none, just return -1.
	// Return value is the window number with the highest zindex - the one that
	// will receive the click at X/Y.  -1 if none
	// *topzi will contain the highest zindex found at the spot.

	int cnt = 0;
	int i;
	int win[QW_MAX_WINDOWS];

	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (qws->win[i].wid[0]) {
			if (
				qws->win[i].x <= x &&
				(qws->win[i].x + qws->win[i].w) >= x &&
				qws->win[i].y <= y &&
				(qws->win[i].y + qws->win[i].h) >= y
			) {
				win[cnt++] = i;
			}
		}
	}
	if (cnt == 0) return -1;


	// Find the window with the highest zindex *at the XY spot*
	int loctopzi = 0;
	int topwin = -1;
	for (i = 0;  i < cnt;  i++) {
		if (qws->win[win[i]].zindex > loctopzi) {
			loctopzi = qws->win[win[i]].zindex;
			topwin = i;
			if (topzi != NULL) {
				if (*topzi < loctopzi) {
					*topzi = loctopzi;
				}
			}
		}
	}
	return win[topwin];
}
void	qw_WinInfo(char *wname) {
	// Display some info about an existing window
	if (!wname || strlen(wname) < 1) {
		xq_clog(COLOR_YELLOW, "/ui_wininfo inv|chat|...");
		return;
	}

	int i;
	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (xq_seq(wname, qws->win[i].wid)) break;
	}

	if (i == QW_MAX_WINDOWS) {
		xq_clog(COLOR_YELLOW, "Window id not found");
		return;
	}

	qw_window_t *win = &qws->win[i];
	qw_obj_t *obj;

	for (i = 0;  i < QW_OBJ_PER_WIN;  i++) {
		qw_obj_attach_t *attach = &win->obj[i];
		if (attach->num > 0) {
			xq_clog(COLOR_WHITE,
				"Object attached at index %i: OID %i, x %i, y %i, actual_x %i, actual_y %i, x_centered %i, y_centered %i, zindex %i",
				i,
				attach->num,
				attach->x,
				attach->y,
				attach->actual_x,
				attach->actual_y,
				attach->x_centered,
				attach->y_centered,
				attach->zindex
			);

			obj = &qws->obj[attach->num];

			char *type = "??";
			switch (obj->type) {
				case QW_OBJ_NONE:			type = "NONE";				break;
				case QW_OBJ_PERCBAR:		type = "PERCBAR";			break;
				case QW_OBJ_TEXT:			type = "TEXT";				break;
				case QW_OBJ_INVSLOT:		type = "INVSLOT";			break;
				case QW_OBJ_TOOLTIP:		type = "TOOLTIP";			break;
				case QW_OBJ_ITEMINSPECTOR:	type = "ITEMINSPECTOR";		break;
				case QW_OBJ_AMOUNTPICKER:	type = "AMOUNTPICKER";		break;
				case QW_OBJ_AUTOEQUIP:		type = "AUTOEQUIP";			break;
				case QW_OBJ_MONEY:			type = "MONEY";				break;
				case QW_OBJ_BUTTON:			type = "BUTTON";			break;
				case QW_OBJ_ITEMGFX:		type = "ITEMGFX";			break;
				case QW_OBJ_TIMEBAR:		type = "TIMEBAR";			break;
				case QW_OBJ_SPELL_GEM:		type = "SPELL_GEM";			break;
				case QW_OBJ_SPELL_ICON:		type = "SPELL_ICON";		break;
				case QW_OBJ_SPELLBOOK_SLOT:	type = "SPELLBOOK_SLOT";	break;
				default:					type = "UNKNOWN";			break;
			}


			xq_clog(COLOR_YELLOW,
				"OID %i details: type %s, w %i, h %i, hidden %i, usecount %i",
				attach->num,
				type,
				obj->w,
				obj->h,
				obj->hidden,
				obj->usecount
			);
		}
	}
}
int		qw_WindowExists(const char *wid) {
	int i;

	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (xq_seq((char *)wid, qws->win[i].wid)) return i;
	}
	return -1;
}
void	qw_WindowSnap(qw_window_t *win) {
	// we do not allow windows to cross the screen boundaries

	int w = cgs.screenXScale * 640;
	int h = cgs.screenYScale * 480;
	if (w == 0 || h == 0) {
		// screen not initialized yet
		return;
	}


	// Window origin must not be outside the screen
	if (win->x < 0) win->x = 0;
	if (win->y < 0) win->y = 0;
	if (win->x >= w) win->x = 0;
	if (win->y >= h) win->y = 0;


	// Window must not even partly be outside the screen
	if ((win->x + win->w) > w) win->x = w - win->w;
	if ((win->y + win->h) > h) win->y = h - win->h;
}
void	qw_WindowDraw(int wid) {
	qw_window_t *win = &qws->win[wid];
	int i;
	float frame_color[4], color[4];
	int cnt = 0;
	uint32_t sortobj[QW_OBJ_PER_WIN];

	qw_WindowSnap(win);

	VectorCopy(xq_Color2Vec(win->brd_col), frame_color);
	frame_color[3] = win->alpha;


	// Draw the window's border
	xq_DrawRect(
		win->x,
		win->y,
		win->w,
		win->h,
		QW_WIN_BORDER_WIDTH,
		frame_color
	);


	// Draw the window background
	if (win->bg_shdr == 0) {
		// plain color background
		VectorCopy(xq_Color2Vec(win->bg_col), color);
		color[3] = win->alpha;
		xq_FillRect(
			win->x + QW_WIN_BORDER_WIDTH,
			win->y + QW_WIN_BORDER_WIDTH,
			win->w - QW_WIN_BORDER_WIDTH*2,
			win->h - QW_WIN_BORDER_WIDTH*2,
			color
		);
	} else {
		// bitmap background
		xq_DrawPic(
			win->x + QW_WIN_BORDER_WIDTH,
			win->y + QW_WIN_BORDER_WIDTH,
			win->w - QW_WIN_BORDER_WIDTH*2,
			win->h - QW_WIN_BORDER_WIDTH*2,
			win->bg_shdr
		);
	}


	// Sort the qw_obj_attach_t's attached to the window by the qw_obj_t zindex
	for (i = 0;  i < QW_OBJ_PER_WIN;  i++) {
		if (win->obj[i].num > 0) {
			sortobj[cnt++] = i;
		}
	}
	#ifdef _WIN32
	qsort_s(sortobj, cnt, sizeof(uint32_t), qw_ObjZICompareFnc, win->obj);
	#else
	qsort_r(sortobj, cnt, sizeof(uint32_t), qw_ObjZICompareFnc, win->obj);
	#endif


	// Draw the objects attached to the window
	for (i = 0;  i < cnt;  i++) {
		qw_ObjDraw(win, sortobj[i]);
	}


	// Special case for the chat window as
	// we're using some legacy code in here to handle the text entry line.
	// We are also displaying the chat window contents independently from the QW system.
	// (this should be redone at some point)
	if (xq_seq(win->wid, "chat")) {
		trap_ConNotify();
		xq_chat_display();
	}


	// Draw the title bar
	if (win->no_title) return;
	VectorCopy(xq_Color2Vec(QW_WIN_BORDER_COLOR), frame_color);
	color[3] = win->alpha;
	int w = win->w;
	int h = QW_WIN_STATUSBAR_HEIGHT + 1;


	// Draw the bar
	xq_DrawRect(
		win->x,
		win->y,
		w,
		h,
		1,
		frame_color
	);


	// Set the title bar background
	VectorCopy(xq_Color2Vec(QW_WIN_STATUSBAR_BGCOLOR), color);
	color[3] = win->alpha;
	xq_FillRect(
		win->x + 1,
		win->y + 1,
		win->w - 2,
		QW_WIN_STATUSBAR_HEIGHT - 1,
		color
	);


	// Draw the closing X separation line
	if (win->closex == 1) {
		xq_DrawRect(
			win->x + win->w - QW_WIN_CLOSEX_WIDTH - 1,
			win->y,
			1,
			QW_WIN_STATUSBAR_HEIGHT + 1,
			1,
			frame_color
		);

		CG_DrawTinyString(win, win->x + win->w - (CG_DrawStrlen("X") * TINYCHAR_WIDTH) - 2, win->y + 2, "X", 1.0f);
	}


	// Draw the title text
	if (win->title) {
		CG_DrawTinyString(
			win,
			win->x + (win->w / 2) - ((CG_DrawStrlen(win->title) * TINYCHAR_WIDTH) / 2),
			win->y + 2,
			win->title,
			1.0f
		);
	}
}
void	qw_ResizeStart(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt) {
	if (shift || ctrl || alt || button != 0) return;

	qws->window_resizing = win;
	qws->window_change_old_mouse_x = xqst->mousex;
	qws->window_change_old_mouse_y = xqst->mousey;
}
void	qw_ResizeStop(void) {
	qws->window_resizing = -1;
	qw_SaveWindows();
}
int		qw_WindowZICompareFnc(const void *a, const void *b) {
   return (
		qws->win[*(int*)a].zindex -
		qws->win[*(int*)b].zindex
	);
}
int		qw_WindowDeleteHighest(void) {
	// Delete the window with the highest zindex of those that can be deleted
	// Used for closing windows with ESC

	int topziwin = -1;
	qw_window_t *w;
	int topzi = -1;

	for (int i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (qws->win[i].wid[0]) {
			if (qws->win[i].zindex > topzi) {
				w = &qws->win[i];

				// We're not considering windows that are not deletable
				if (
					!xq_seq(w->wid, "smallTarget") &&
					!xq_seq(w->wid, "bigTarget") &&
					!xq_seq(w->wid, "smallBars") &&
					!xq_seq(w->wid, "bigBars") &&
					!xq_seq(w->wid, "spfx") &&
					!xq_seq(w->wid, "chat") &&
					!xq_seq(w->wid, "group") &&
					!xq_seq(w->wid, "spells") &&
					!xq_seq(w->wid, "xpBar")
				) {
					topzi = qws->win[i].zindex;
					topziwin = i;
				}
			}
		}
	}


	if (topziwin < 1) return 0; // nothing deletable has been found

	w = &qws->win[topziwin];


	// Prefer using the closefnc function pointer, if available
	if (w->closefnc) {
		w->closefnc((void *)w);
	} else {
		// Otherwise, just delete from the QW memory
		if (startsWith("cont_", w->wid)) {
			trap_S_StartLocalSound(SOUNDH("sound/inv/container_close.wav"), CHAN_ANNOUNCER);
		}
		qw_WindowDelete(w->wid);
	}
	return 1;
}
void	qw_WindowTitle(int win, char *title) {
	Q_strncpyz(qws->win[win].title, title, sizeof(qws->win[win].title));
}
void	qw_WindowResize(void) {
	int deltax = xqst->mousex - qws->window_change_old_mouse_x;
	int deltay = xqst->mousey - qws->window_change_old_mouse_y;
	int screenw = cgs.screenXScale * 640;
	int screenh = cgs.screenYScale * 480;

	if (xqst->mousex > screenw || xqst->mousey > screenh) return;
	
	qw_window_t *win = &qws->win[qws->window_resizing];

	if (deltax != 0 || deltay != 0) { // Only resize if the mouse actually moved

		int neww = win->w + deltax;
		int newh = win->h + deltay;

		if ((win->x + neww) >= screenw) {
			neww = screenw - win->x;
		}
		if ((win->y + newh) >= screenh) {
			newh = screenh - win->y;
		}


		// Don't let a window get smaller or bigger than these values
		int minw = QW_WIN_MIN_WIDTH;
		int minh = QW_WIN_MIN_HEIGHT;
		int maxw = QW_WIN_MAX_WIDTH;
		int maxh = QW_WIN_MAX_HEIGHT;
		if (xq_seq(win->wid, "chat")) {
			minw = QW_CHATWIN_MIN_WIDTH;
			minh = QW_CHATWIN_MIN_HEIGHT;
		}
		if (neww > minw && neww < maxw) {
			win->w = neww;
		}
		if (newh > minh && newh < maxh) {
			win->h = newh;
		}


		// Clip to screen limits
		qw_WindowSnap(win);


		// The chat window gets a special treatment as the code that displays
		// its contents will need these values.
		if (xq_seq(win->wid, "chat")) {
			xqst->ui_chat_window_w = win->w;
			xqst->ui_chat_window_h = win->h;
		}


		// Save current values for the next frame
		qws->window_change_old_mouse_x = xqst->mousex;
		qws->window_change_old_mouse_y = xqst->mousey;
	}
}
void	qw_WindowMove(void) {
	int deltax = xqst->mousex - qws->window_change_old_mouse_x;
	int deltay = xqst->mousey - qws->window_change_old_mouse_y;

	qw_window_t *win = &qws->win[qws->window_moving];

	if (deltax != 0 || deltay != 0) {

		int savex = win->x;
		int savey = win->y;

		win->x = (xqst->mousex - qws->window_change_init_x);
		win->y = (xqst->mousey - qws->window_change_init_y);

		qw_WindowSnap(win);

		if (savex == win->x) {
			qws->window_change_init_x = xqst->mousex - win->x;
		}
		if (savey == win->y) {
			qws->window_change_init_y = xqst->mousey - win->y;
		}


		// Chat window content displaying will need that
		if (xq_seq(win->wid, "chat")) {
			xqst->ui_chat_window_x = win->x;
			xqst->ui_chat_window_y = win->y;
		}

		// Save for next frame
		qws->window_change_old_mouse_x = xqst->mousex;
		qws->window_change_old_mouse_y = xqst->mousey;
	}
}
void	qw_WindowDelete(const char *wid) {
	for (int i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (xq_seq((char *)wid, qws->win[i].wid)) {

			// Destroy all objects attached to this window
			// except those that are attached to another window
			// and those whose handle is saved in qws->objhandles[]
			qw_window_t *win = &qws->win[i];
			for (int j = 0;  j < QW_OBJ_PER_WIN;  j++) {
				if (win->obj[j].num > 0) {
					qw_ObjDetach(i, win->obj[j].num);
				}
			}
			qw_SaveWindows();


			// Do not leave holes in the window QW structure
			if (i != (QW_MAX_WINDOWS-1)) {
				memcpy(&qws->win[i], &qws->win[i+1], sizeof(qw_window_t) * (QW_MAX_WINDOWS - (i + 1)));
			}


			// Clear the no longer used memory
			memset(&qws->win[QW_MAX_WINDOWS-1], 0, sizeof(qw_window_t));
			return;
		}
	}
}
int		qw_WinNew(qw_window_t p) {
	// Create a new window from the template parameters in p

	int i, j;
	qw_window_save_t *winsave;

	// If x is -1 then center window horizontally
	// If y is -1 then center window vertically
	if (p.x == -1) {
		p.x = (xqst->screenw / 2) - (p.w / 2);
	}
	if (p.y == -1) {
		p.y = (xqst->screenh / 2) - (p.h / 2);
	}


	// We do not allow duplicate wid
	if (qw_WindowExists(p.wid) != -1) {
		xq_clog(COLOR_RED, "Attempted to replace the [%s] window.", p.wid);
		return -1;
	}

	// Find a vacant window slot
	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (!qws->win[i].wid[0]) break;
	}


	// Out of window slots
	if (i == QW_MAX_WINDOWS) {
		xq_clog(COLOR_RED, "qw_WinNew: out of window slots for new window %s", p.wid);
		return -1;
	}


	// This will be our new window handle
	qw_window_t *w = &qws->win[i];


	// Figure out the zindex - we always open a new window on top of all existing ones
	int zi = 0;
	for (j = 0;  j < QW_MAX_WINDOWS;  j++) {
		if (qws->win[j].zindex > zi) {
			zi = qws->win[j].zindex;
		}
	}
	zi++;


	// Go ahead and create the window
	Q_strncpyz(w->wid, p.wid, sizeof(w->wid));
	if (p.title) {
		Q_strncpyz(w->title, p.title, sizeof(w->title));
	}
	w->zindex = zi;
	w->x = p.x;
	w->y = p.y;
	w->w = p.w;
	w->h = p.h;
	w->brd_col = (p.brd_col == 0 ? QW_WIN_BORDER_COLOR : p.brd_col);
	w->bg_col = (p.bg_col == 0 ? QW_WIN_BG_COLOR : p.bg_col);
	w->bg_shdr = p.bg_shdr;
	w->no_drag = p.no_drag;
	w->resizable = p.resizable;
	w->no_save = p.no_save;
	w->no_title = p.no_title;
	w->closex = p.closex;
	w->closefnc = p.closefnc;
	w->closefarg = p.closefarg;


	// By default, windows are always full alpha. Specifying p.alpha_min and p.alpha_max
	// allows for fade-in/out depending on mouse focus.
	if (p.alpha_max == 0) {
		w->alpha = w->alpha_max = w->alpha_min = 1.0F;
	} else {
		w->alpha = w->alpha_max = p.alpha_max;
		w->alpha_min = p.alpha_min;
		w->alpha_dir = 0;
	}


	// See if we have saved the coords of that window before, and load if so.
	if (p.no_drag == 0) {
		winsave = qw_LoadWindow(p.wid);
		if (winsave != NULL) {
			w->x = winsave->x;
			w->y = winsave->y;
			if (xq_seq((char *)p.wid, "chat")) {
				xqst->ui_chat_window_x = winsave->x;
				xqst->ui_chat_window_y = winsave->y;
				if (winsave->w < 320) winsave->w = 320;
				if (winsave->h < 100) winsave->w = 100;
				xqst->ui_chat_window_w = winsave->w;
				xqst->ui_chat_window_h = winsave->h;
			}
			if (p.resizable == 1) {
				w->w = winsave->w;
				w->h = winsave->h;
			}
		}
	}


	// Can't hurt to set some magics.
	w->magics = w->magice = qw_window_t_MAGIC;


	// Make sure the window doesn't appear outside of the screen or something.
	qw_WindowSnap(w);

	return i;
}

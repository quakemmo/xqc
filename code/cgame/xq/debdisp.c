
/* DESCRIPTION
	This displays permanent or ephemeral lines at the top of the screen for important announcements.
	Example: "Player X got the Quad Damage" in Arena mode.

	This needs to be eventually renamed to something more descriptive.
*/

#include "../cg_local.h"


#define LINES 		20 // Max lines that can be displayed at any given time
#define LINE_LEN	100 // Max length of each line

typedef struct xq_debdisp_s {
	int		col;
	char	str[LINE_LEN];
	int64_t	remove_ts;	// if 0: display forever, if -1: display just once (1 frame),
						// otherwise remove when remove_ts <= xq_msec()
	int64_t	start_ts;	// We use this to slightly flash the text upon initial appearing
} xq_debdisp_t;
xq_debdisp_t data[LINES];

static void xq_debdisp_printline(int *cur_y, xq_debdisp_t *line) {
	vec4_t col;
	Vector4Copy(g_color_table[ColorIndex(line->col)], col);
	col[3] = 0.75f;

	int64_t remain = line->remove_ts - xq_msec();
	int64_t runningfor = xq_msec() - line->start_ts;
	int flags = UI_SMALLFONT | UI_CENTER | UI_DROPSHADOW;
	if (line->remove_ts != -1 && remain < 600) {
		// If the line is about to expire, fade it out a bit
		col[3] = (remain / 600.0) * 0.75;
		if (col[3] < 0) col[3] = 0;
	} else if (line->remove_ts != -1 && runningfor < 600) {
		// If the line just arrived, flash it briefly
		// (this needs to be made independent of UI_PULSE (which revolves around cg.time) to be prettier
		flags |= UI_PULSE;
	}

	if (xq_debugDebdisp.integer) {
		xq_clog(COLOR_WHITE, "cur_y: %i, line->col: %i, line->str: %s, remove_ts: %li, col[3]: %f",
			*cur_y, line->col, line->str, line->remove_ts, col[3]);
	}
	int fadey = 16;
	if (remain < 200 && remain > 0) {
		// Start collapsing the line right before it disappears
		fadey = (fadey * remain) / 200;
		if (fadey < 0) fadey = 0;
	}
	UI_DrawProportionalString(320, 1 + *cur_y, line->str, flags, col);
	*cur_y += fadey;
}

// Called once upon cgame startup
void xq_debdisp_init(void) {
	memset(data, 0, sizeof(data));
}

// Called at the end of each client frame, does the actual displaying
void xq_debdisp_draw() {
	int i;

	int cur_y = 0;

	// First draw the lines that are scheduled to be displayed this frame only
	// (these should be there all the time / a long time, so keep them at the top)
	for (i = 0;  i < LINES && data[i].col;  i++) {
		if (data[i].remove_ts == -1) {
			xq_debdisp_printline(&cur_y, &data[i]);
		}
	}

	// Now draw the other lines (these disappear after a while)
	for (i = 0;  i < LINES && data[i].col;  i++) {
		if (data[i].remove_ts != -1) {
			xq_debdisp_printline(&cur_y, &data[i]);
		}
	}

	// remove expired lines or those that are only meant to be displayed once
	i = 0;
	while (i < LINES && data[i].col != 0) {
		if (data[i].col) {
			if (data[i].remove_ts <= xq_msec() && data[i].remove_ts != 0) {
				if (i < (LINES - 1)) {
					memcpy(&data[i], &data[i+1], sizeof(xq_debdisp_t) * ((LINES - 1) - i));
				}
				memset(&data[LINES-1], 0, sizeof(xq_debdisp_t));
				i = 0;
				continue;
			}
			i++;
		} else {
			break;
		}
	}
}

// Called whenever we need to display something on the top of the screen - adds stuff
// to the array that will be displayed at the end of the frame with xq_debdisp_draw()
// example: xq_debdisp(COLOR_WHITE, xq_msec() + 3000, "%s: %i", "Example string", 25);
// Will display "Example string: 25" at the top of the screen for 3 seconds
// remove_ts should be the xq_msec() of when we want the line to vanish.
// if remove_ts is -1, display only once, in the client frame the call has been made in
// if remove_ts is 0, display indefinitely

void xq_debdisp(int col, int64_t remove_ts, char *fmt, ...) {
	int i;
	for (i = 0;  i < LINES;  i++) {
		if (data[i].col == 0) {
			break;
		}
	}
	if (i == LINES) return; // out of disp slots

	if (remove_ts == 0) remove_ts = LONG_MAX; // 0 means display indefinitely


    va_list argptr;
    va_start(argptr, fmt);
    Q_vsnprintf(data[i].str, LINE_LEN-1, fmt, argptr);
    va_end(argptr);

	data[i].col = col;
	data[i].remove_ts = remove_ts;
	data[i].start_ts = xq_msec();
}

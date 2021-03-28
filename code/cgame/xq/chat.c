/* DESCRIPTION

	This handles displaying of text in the "chat" QWIN window, where all the XQ-related text messages are displayed.
	The regular way to add text to that window is to use the xq_clog() function.
	The text entry line is NOT handled here.

	This needs variable-width text support at some point.
*/

#include "../cg_local.h"

#define MAX_LINES			100
#define MAX_LINE_LEN		200
#define LINE_SEP			4

#define MAX_ROWS_PER_LINE	100 // a chat box line can be split into a maximum of that many rows to fit the window
#define MAX_ROW_LEN			200 // even if the window width allows us to, we'll never display chat box text rows
								// longer than that, we'll split them.

typedef struct line_s {
	uint32_t	ts;
	int			color;
	char		text[MAX_LINE_LEN+1];
} line_t;
typedef struct chat_s {
	line_t		line[MAX_LINES];
} chat_t;
static chat_t *chat = NULL;



static void chat_putchar(char c, int x, int y) {
	xq_DrawChar(x, y, XQ_CHAT_WIDTH, XQ_CHAT_HEIGHT, c);
	if (xq_debugChat.integer) {
		Com_Printf("chat_putchar: char: [%c], x: %i, y: %i\n", c, x, y);
	}
}
static void chat_printrow(char *txt, int xpos, int ypos) {
	if (xq_debugChat.integer) {
		Com_Printf("chat_printrow: txt: [%s], xpos: %i, ypos: %i\n", txt, xpos, ypos);
	}
	for (int i = 0;  i < strlen(txt);  i++) {
		chat_putchar(txt[i], xpos + i*XQ_CHAT_WIDTH, ypos);
	}
}
static int chat_printline(char *txt, int *rows_left, int chatx, int chaty, int rows_displayed, int chatw, int chath) {
	// rows_left has the number of remaining rows left in the window to be filled with text
	// (a log line can take several screen rows to fully display)
	// We must decrease rows_left by however many rows we used to display txt we are passed.

	// figure out how many rows this is going to take
	int chars_per_row = chatw / XQ_CHAT_WIDTH;
	if (chars_per_row < 1) chars_per_row = 1; // just in case

	typedef struct {
		char	row[MAX_ROW_LEN+1];
	} rows_t;
	rows_t rows[MAX_ROWS_PER_LINE];
	memset(&rows, 0, sizeof(rows));


	// split txt into rows
	char *curpos = txt;
	int sl = strlen(txt);
	int rows_needed = 0;
	while (1) {
		int len_to_copy = chars_per_row;

		// make sure we don't try to copy more chars than we have left in the string
		int remaining_len = sl - (curpos - txt);
		if (len_to_copy > remaining_len) {
			len_to_copy = remaining_len;
		}
		if (len_to_copy == 0) break;
		if (len_to_copy > chars_per_row) len_to_copy = chars_per_row;
		if (len_to_copy > MAX_ROW_LEN) len_to_copy = MAX_ROW_LEN;


		Q_strncpyz(rows[rows_needed].row, curpos, len_to_copy+1);
		if (xq_debugChat.integer) {
			Com_Printf("chat_printline: sl: %i, adding rows[%i] > [%s], len_to_copy: %i\n",
				sl, rows_needed, curpos, len_to_copy);
		}
		rows_needed++;
		curpos += len_to_copy;
	}


	// figure out how many rows we will actually be able to display
	int rows_we_will_display = rows_needed;
	if (*rows_left < rows_we_will_display) {
		rows_we_will_display = *rows_left;
	}

	// go ahead and display the rows, starting at the last one
	int cnt = 0;
	for (int i = rows_needed - 1;  i >= (rows_needed - rows_we_will_display);  i--) {
		int ypos = chaty - ((rows_displayed + cnt) + 2) * (XQ_CHAT_HEIGHT + LINE_SEP);
		ypos += chath;
		chat_printrow(rows[i].row, chatx, ypos);
		cnt++;
	}


	// finally report to parent about our row usage
	*rows_left -= rows_we_will_display;
	if (xq_debugChat.integer) {
		Com_Printf("chat_printline: sl: %i, chars_per_row: %i, rows_needed: %i, rows_we_will_display: %i for line [%s]\n",
			sl, chars_per_row, rows_needed, rows_we_will_display, txt);
	}
	return rows_we_will_display;
}




// called by QWIN on each client frame, right after the "chat" window is done drawing
void xq_chat_display(void) {
	if (!chat) {
		Com_Printf("xq_chat_display() called before chat init.  Not cool.\n");
		return;
	}

	int chatx = xqst->ui_chat_window_x;
	int chaty = xqst->ui_chat_window_y;
	int chatw = xqst->ui_chat_window_w;
	int chath = xqst->ui_chat_window_h;

	if (xq_debugChat.integer) {
		Com_Printf("chat_display: %i / %i / %i / %i\n", chatx, chaty, chatw, chath);
	}


	int rows_left = chath / (XQ_CHAT_HEIGHT + LINE_SEP) - 1; // leave the bottom row for typing
	int rows_displayed = 0;

	for (int curline = MAX_LINES - 1;  curline >= 0;  curline--) {
		line_t *line = &chat->line[curline];
		if (line->ts) {
			trap_R_SetColor(g_color_table[line->color]);
			rows_displayed += chat_printline(line->text, &rows_left, chatx, chaty, rows_displayed, chatw, chath);
			if (xq_debugChat.integer) {
				Com_Printf("chat_display: requesting to printline %i, with chatw %i\n", curline, chatw);
			}
		}
		if (rows_left < 1) break;
	}
	if (xq_debugChat.integer) {
		Com_Printf("chat_display: rows_displayed: %i\n", rows_displayed);
	}

	trap_R_SetColor(NULL);
}

// called once on first cgame init
void xq_chat_init() {
	// We want to keep the chat window contents over cgame reloads
	// We malloc it once here and keep the pointer to the area in client's memory
	int ret = trap_XQ_Pers_Get_I64("chat_data", (int64_t *)&chat);
	if (!ret) {
		chat = malloc(sizeof(chat_t));
		memset(chat, 0, sizeof(chat_t));
		trap_XQ_Pers_Set_I64("chat_data", (int64_t)chat);
	}
}

// called each time xq_clog() is used
void xq_chat_add(char *txt, int color) {
	if (xq_debugChat.integer) {
		Com_Printf("chat_add: [%s]\n", txt);
	}
	if (!chat) {
		Com_Printf("xq_chat_add() called before chat init.  Doing init now.  txt was: [%s]\n", txt);
		xq_chat_init();
	}


	// We are sent XQ_CHAR_CLEAR upon world entry (not on subsequent zoning)
	// We must just clear the chat buffer when this happens.
	if (txt[0] == XQ_CHAR_CLEAR) {
		memset(chat, 0, sizeof(chat_t));
		return;
	}

	int i;
	for (i = 0;  i < MAX_LINES;  i++) {
		if (!chat->line[i].ts) {
			break;
		}
	}
	if (i == MAX_LINES) {
		memcpy(&chat->line[0], &chat->line[1], sizeof(line_t) * (MAX_LINES-1));
		i--;
	}
	line_t *line = &chat->line[i];
	line->ts = time(NULL);
	line->color = color;
	Q_strncpyz(line->text, txt, sizeof(line->text));
}

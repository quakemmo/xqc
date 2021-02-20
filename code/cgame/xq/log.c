/*	DESCRIPTION

	Displaying of server messages.
*/

#include "../cg_local.h"


void xq_serverlog(const char *buf) {
	// Break a server log message into distinct log message. Each server log message
	// can contain one or more actual messages that are bunched up for ease of transmission.

	const char *ptr = buf;
	int count = 0;
	char lenlit[CLOG_LENLIT+1] = {0};
	char txt[CLOG_MAX_LEN+1] = {0};

	int done = 0;
	while (!done) {

		strncpy(lenlit, ptr, CLOG_LENLIT);
		lenlit[CLOG_LENLIT] = 0;
		int len = atoi(lenlit);
		ptr += CLOG_LENLIT;

		int flags = ptr[0];
		flags -= '0';
		flags <<= 8;
		ptr++;

		int color = ptr[0];
		ptr++;
		if (len >= CLOG_MAX_LEN) {
			// we have been sent too much data in one go
			break;
		}

		strncpy(txt, ptr, len);
		txt[len] = 0;
		ptr += len;
		if (!ptr[0]) {
			done = 1;
		}
		if (!(flags & XQ_CLOG_EMPHASIZE_ONLY)) {
			xq_clog(color, "%s" , txt);
		}
		count++;
		if (flags & XQ_CLOG_EMPHASIZE || flags & XQ_CLOG_EMPHASIZE_ONLY) {
			xq_debdisp(color - 48, xq_msec() + 3000, "%s", txt);
		}
	}
}
void xq_clog(int color, char *fmt, ...) {
	static char buf[1024] = {0};

	// Build the final string
	va_list argptr;
	va_start(argptr, fmt);
	Q_vsnprintf((char *)&buf, sizeof(buf), fmt, argptr);
	va_end(argptr);


	// put the line in the console
	CG_Printf("[XQ] ^%i%s\n", color - 48, (char *)&buf);


	// put the line in the chat window
	xq_chat_add(buf, color - 48);
}

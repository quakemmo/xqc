#include "../cg_local.h"

//#pragma GCC diagnostic error "-Wframe-larger-than=1"

char *xq_getname(centity_t *cent, int targetname) {
	// targetname = 1 means we only want the first name for PCs and no leading * for NPCs
	static char dst[XQ_MAX_FLOAT_TEXT_LEN+1] = {0};
	static char tmpbuf[XQ_MAX_CHAR_NAME+XQ_MAX_CHAR_SURNAME+1] = {0};	// XXX removing static on this MAY cause WEIRD issues
																		// (the nameplates alpha is wrong)
																		// need to find the problem, then redo this function
																		// properly.
																		// initially happened in commit 4a5d1c17a2c9032e5806e540ab8aa84e9cf902fd
	char dst_name[XQ_MAX_CHAR_NAME+1] = {0};
	char dst_surname[XQ_MAX_CHAR_SURNAME+1] = {0};

    playerState_t *ps = &cg.snap->ps;
	entityState_t *es = &cent->currentState;

	switch (es->xq_enttype) {
		case XQ_ENTTYPE_PC:;
			if (cent->currentState.number == ps->clientNum) {
				es->xq_name1 = ps->xq_name1;
				es->xq_name2 = ps->xq_name2;
				es->xq_name3 = ps->xq_name3;
				es->xq_name4 = ps->xq_name4;
				es->xq_name5 = ps->xq_name5;
				es->xq_name6 = ps->xq_name6;
				es->xq_name7 = ps->xq_name7;
				es->xq_name8 = ps->xq_name8;
			}
			strncpy(tmpbuf, xq_EntName(es), XQ_MAX_CHAR_NAME+XQ_MAX_CHAR_SURNAME-1);
			char *sep = strchr(tmpbuf, '|');
			strncpy(dst_name, tmpbuf, sep - tmpbuf);
			strncpy(dst_surname, sep+1, strlen(sep+1));
			dst_name[0] = toupper(dst_name[0]);


			// Optionally add the surname
			if (!targetname && strlen(dst_surname)) {
				dst_surname[0] = toupper(dst_surname[0]);
				snprintf(dst, XQ_MAX_FLOAT_TEXT_LEN-1, "%s %s", dst_name, dst_surname);
			} else {
				snprintf(dst, XQ_MAX_FLOAT_TEXT_LEN, "%s", dst_name);
			}
			break;
		case XQ_ENTTYPE_NPC:;
			char *linkdead;
			if (!targetname && (es->xq_flags & XQ_LINKDEAD)) {
				linkdead = " <Linkdead>";
			} else {
				linkdead = "";
			}

			char *fmt;
			if (targetname) {
				fmt = "%s%s";
			} else {
				fmt = "*%s%s";
			}
			snprintf(dst, XQ_MAX_FLOAT_TEXT_LEN, fmt, xq_EntName(es), linkdead);
			if (es->xq_flags & XQ_LINKDEAD) {
				if (targetname) {
					dst[0] = toupper(dst[0]);
				} else {
					dst[1] = toupper(dst[1]);
				}
			}
			break;
		case XQ_ENTTYPE_PCCORPSE:
		case XQ_ENTTYPE_NPCCORPSE:
			snprintf(dst, XQ_MAX_FLOAT_TEXT_LEN, "%s's corpse", xq_EntName(es));
			if (es->xq_enttype == XQ_ENTTYPE_PCCORPSE) {
				dst[0] = toupper(dst[0]);
			}
			break;
	}
	return dst;
}


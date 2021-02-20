/*	DESCRIPTION

	Shortcuts for some server CLI commands.
*/

#include "../cg_local.h"

void xq_scmd(char *cmd) {
	trap_SendConsoleCommand(va("cmd say %s\n", cmd));
}
void xq_CastGem(int slot) {
	xq_scmd(va("/cast %i", slot));
}
void xq_RunSlot(int slot) {
	xq_scmd(va("/runslot %i", slot));
}
void xq_SwapWithMouseSlot(int64_t slot, int64_t container_id, int64_t container_slot, int64_t amount, int64_t invoiceonly) {
	xq_scmd(va("/swapwithmouseslot %"PRId64" %"PRId64" %"PRId64" %"PRId64" %"PRId64, slot, container_id, container_slot, amount, invoiceonly));
}
void xq_Consider() {
	if (cg.snap->ps.xq_target < 1) {
		xq_clog(COLOR_YELLOW, "You need a target to /consider");
		return;
	}
	xq_scmd("/consider");
}
void xq_Sit() {
	if (cg.snap->ps.xq_sitting == 1) {
		xq_scmd("/stand");
	} else {
		xq_scmd("/sit");
	}
}
void xq_Hail() {
	// Hail (echoed locally for convenience)
	char *tname = xq_TargetName();
	if (strlen(tname) > 0) {
		xq_scmd(va("/saynoecho Hail, %s", tname));
		xq_clog(COLOR_WHITE, "You say, Hail, %s", tname);
	} else {
		xq_scmd("/saynoecho You say, Hail");
		xq_clog(COLOR_WHITE, "You say, Hail");
	}
}


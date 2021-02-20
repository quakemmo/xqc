#include "../cg_local.h"


qboolean xq_Target_CanChange(void) {
	playerState_t *ps = &cg.snap->ps;
	if (
		!(ps->xq_flags & XQ_LOOTING_PC) &&
		!(ps->xq_flags & XQ_LOOTING_NPC) &&
		!(ps->xq_flags & XQ_BUYING) &&
		!(ps->xq_flags & XQ_BANKING) &&
		!(ps->xq_flags & XQ_TRADING_PC) &&
		!(ps->xq_flags & XQ_TRADING_NPC)
	){
		return qtrue;
	}
	return qfalse;
}
static void groupMember(int num) {
	if (xq_Target_CanChange()) {
		int t = -1;
		switch (num) {
			case 2:	t = xq_cnum2cname(xqst->ui_group_member_1_name); break;
			case 3:	t = xq_cnum2cname(xqst->ui_group_member_2_name); break;
			case 4:	t = xq_cnum2cname(xqst->ui_group_member_3_name); break;
			case 5:	t = xq_cnum2cname(xqst->ui_group_member_4_name); break;
			case 6:	t = xq_cnum2cname(xqst->ui_group_member_5_name); break;
			default: break;
		}
		if (t != -1) {
			xq_SetTarget(t, 0);
		}
	}
}

void xq_Target_Group2() {
	groupMember(2);
}
void xq_Target_Group3() {
	groupMember(3);
}
void xq_Target_Group4() {
	groupMember(4);
}
void xq_Target_Group5() {
	groupMember(5);
}
void xq_Target_Group6() {
	groupMember(6);
}
void xq_Target_Self() {
	if (xq_Target_CanChange()) {
		xq_SetTarget(cg.snap->ps.clientNum, 0);
	}
}

char *xq_TargetName() {
	// This returns the name of whatever we have targeted, suitable for immediate displaying
	// in the target window
	int targ = cg.snap->ps.xq_target;
	if (targ == 0) {
		return "";
	}
	return xq_getname(&cg_entities[targ], 1);
}
void xq_FindTarget(int key, int down, int shift, int ctrl, int alt) {
	playerState_t *ps = &cg.snap->ps;
    xq_item_t *iinf;

	int64_t mouseitem = xq_InvSlotItem(XQ_ITEM_SLOT_MOUSE1, 0, 0);
	int tcnum = CG_ScanForMouseEntity();

	if (tcnum > -1 && tcnum != ENTITYNUM_WORLD && tcnum != ENTITYNUM_NONE) {
		// Clicked on something of interest
		centity_t *cent = &cg_entities[tcnum];
		entityState_t *es = &cent->currentState;
		int et = es->xq_enttype;
		if (xq_debugTarget.integer) {
			xq_clog(COLOR_WHITE, "Target found: entnum: %i, xq_enttype: %i, xq_corpse_id = %i", tcnum, et, es->xq_corpse_id);
			xq_clog(COLOR_WHITE, "es->eType: %i, es->solid = %i, es->eFlags = %i", es->eType, es->solid, es->eFlags);
		}

		if (et == XQ_ENTTYPE_GROUND && mouseitem == 0 &&
			(es->modelindex == XQ_GROUND_GENERIC || es->modelindex == XQ_GROUND_KICKABLE)
		) {
			// Pick something up
			xq_scmd(va("/pickup %i", tcnum));
			return;

		} else if (et == XQ_ENTTYPE_CLICKABLE) {
			// Activate something clickable
			xq_scmd(va("/activateclickable %i", tcnum));
			return;

		} else if (mouseitem > 0 || ps->xq_mouse_money_amount > 0) {
			// Initiate trade
			if (et == XQ_ENTTYPE_PC || et == XQ_ENTTYPE_NPC) {
				if (!(ps->xq_flags & XQ_TRADING_PC) && !(ps->xq_flags & XQ_TRADING_NPC)) {
					xq_scmd(va("/trade %i", tcnum));
				} else {
					xq_clog(COLOR_RED, "You are already trading with someone!");
				}
				return;
			}
		}


		if (et != XQ_ENTTYPE_GROUND) {
			xq_SetTarget(tcnum, 0);
		}
	} else {
		// Clicked in the 3D scene but not on anything on particular
        if (mouseitem > 0 || ps->xq_mouse_money_type != 0) {
			// Drop mouse item or money on the ground

			if (ps->xq_mouse_money_type == 0) {
				iinf = xq_ItemInfo(mouseitem);
				if (!iinf) {
					xq_clog(COLOR_RED, "Cannot get mouse item info.");
					return;
				} else {
					if (iinf->nodrop) {
						xq_clog(COLOR_RED, "You cannot drop NODROP items on the ground.");
						return;
					}
				}
			}
			qw_Prompt(
				va("Drop the %s on the ground?", mouseitem ? "item" : "money"),
				QW_PROMPT_ACTION_DROP, 0, 0, 0, 0
			);
			return;
		}
	}
}
void xq_SetTarget(int entnum, int consider) {
	if (!cg.snap) return;

	// 200 ms min delay between target change requests
	static int last_ask = 0;
	if ((cg.time - last_ask) < 200) {
		return;
	}


    playerState_t *ps = &cg.snap->ps;
	if (ps->xq_target != entnum || consider == 1) {
		if (xq_debugTarget.integer) {
			xq_clog(COLOR_WHITE, "Current ps->xq_target: %i, entnum: %i", ps->xq_target, entnum);
			xq_clog(COLOR_YELLOW, "Requesting to change target to %i, enttype: %i",
				entnum, cg_entities[entnum].currentState.xq_enttype);
		}
		xq_scmd(va("/targetnum %i", consider ? -entnum : entnum));
	}
}

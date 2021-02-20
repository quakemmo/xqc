#include "../cg_local.h"
#include "../../client/keycodes.h"	// K_ALT, K_CTRL etc
#include "../../qcommon/qcommon.h"	// field_t
#include "../../client/keys.h"		// MAX_KEYS, qkey_t

/*	DESCRIPTION
	Hard-coded key bindings for mouse mode. Most of these should be redone with native Q3 /binds

*/
int xq_KeyStatus(int key) {
	static qkey_t keys[MAX_KEYS];
	qkey_t *clkeys = (qkey_t *)trap_XQ_Get_Keys();
	memcpy(keys, clkeys, sizeof(keys));
	return keys[key].down;
}
int xq_ShiftPressed() {
	return xq_KeyStatus(K_SHIFT);
}
int xq_AltPressed() {
	return xq_KeyStatus(K_ALT);
}
int xq_CtrlPressed() {
	return xq_KeyStatus(K_CTRL);
}
void xq_KeyEvent(int key, unsigned time, qboolean down, int shift, int ctrl, int alt) {
	if (!cg.snap) return;
    playerState_t *ps = &cg.snap->ps;

	if (trap_Key_GetCatcher() & KEYCATCH_CONSOLE) return;
	if (trap_Key_GetCatcher() & KEYCATCH_MESSAGE) {
		if (key != K_MOUSE1 && key != K_MOUSE2) {
			return;
		}
	} 


	// If the amount picker is running, all keyboard number input and ENTER/BACKSPACES go to it.
	if (key != K_MOUSE1 && key != K_MOUSE2) {
		if ((key >= 48 && key <= 57) || key == K_BACKSPACE || key == K_ENTER) {
			if (qws->amount_picker_running == 1) {
				if (down == 0) {
					qw_Obj_AmountPicker_Keypress(key);
				}
				return;
			}
		}
	}

	
	if (key == K_MOUSE1) {
		// Left click
		if (qw_MouseClick(0, down, shift, ctrl, alt) == 0) {
			// We haven't been caught by the QWIN subsystem - looks like we're clicking
			// the 3D scene
			if (down == 1) {
				if (xq_Target_CanChange()) {
					xq_FindTarget(key, down, shift, ctrl, alt);
				}
			}
		}

	} else if (key == K_MOUSE2 && !shift && !ctrl && !alt) {
		if (qw_MouseClick(1, down, shift, ctrl, alt) == 0) {
			if (!(ps->xq_flags & XQ_LOOTING_PC) && !(ps->xq_flags & XQ_LOOTING_NPC)) {
				xq_RightClickScene(down);
			}
		} else {
			// QWIN grabbed the right click
		}

	} else if (key == K_ESCAPE) {
		// Close the highest ZI closeable window
		if (down == 1) {
			if (qw_WindowDeleteHighest() == 0) {
				xq_SetTarget(0, 0);
			}
		}

	} else if (key >= '1' && key <= '6' && !shift && (ctrl || alt) && down) {
		// ctrl-1/6 or alt-1/6: cast a spell gem
		if (ps->xq_sitting) {
			xq_clog(COLOR_YELLOW, "You cannot cast while sitting.");
		} else if (qws->buying) {
			xq_clog(COLOR_YELLOW, "You cannot cast while interacting with a merchant.");
		} else if (qws->looting) {
			xq_clog(COLOR_YELLOW, "You cannot cast while looting.");
		} else if (qws->trading) {
			xq_clog(COLOR_YELLOW, "You cannot cast while trading.");
		} else {
			if (ps->xq_class == XQ_BARD) {
				// Bards continuously play a song until they explicitely ask to stop it
				xqst->autocast_gem_change_ts = cg.time;
				if (xqst->autocast_gem == 0) {
					// Start song
					xqst->autocast_gem = key - '1' + 1;
				} else {
					// Stop song
					xqst->autocast_gem = 0;
					xq_scmd("/cast_interrupt");
				}
			} else {
				// Other classes just cast the spell directly
				if (xq_CanCastGem(key - '1' + 1)) {
					xq_CastGem(key - '1' + 1);
				}
			}
		}


	} else if (key >= '1' && key <= '6' && !shift && !ctrl && !alt && down) {
		xq_clog(COLOR_WHITE, "Hot bar %i", key - '1' + 1); // Hot bar not implemented

	} else {
		// All other mouse + modifier keys combinations
		// xq_clog(COLOR_WHITE, "Unknown key combo:  %i %i %i %i %i %i", K_ALT, key, down, shift, ctrl, alt);
	}
}

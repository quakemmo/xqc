/* DESCRIPTION
	xq_Frame() is called once per client frame, in cg_view.c right before rendering the scene.
	It will perform a bunch of things and also in turn call other functions such as xq_Spell_Frame()
	or xqui_Frame() that do more specific per-frame things.

*/

#include "../cg_local.h"

static void CheckMagics(void) {
	assert(xqst->magics == xq_cgame_state_t_MAGIC);
	assert(xqst->magice == xq_cgame_state_t_MAGIC);

	xq_Info_CheckMagics();
}
static void xq_disconnected(void) {
    Com_Error(ERR_SERVERDISCONNECT, "You have been disconnected.");
}
static void TargetPoof(playerState_t *ps) {
	if (ps->xq_target > 0) {
		int target_poofed = 0;
		centity_t *tcent = &cg_entities[ps->xq_target];
		if (
			tcent->currentState.xq_enttype != XQ_ENTTYPE_PC
			&& tcent->currentState.xq_enttype != XQ_ENTTYPE_NPC
			&& tcent->currentState.xq_enttype != XQ_ENTTYPE_PCCORPSE
			&& tcent->currentState.xq_enttype != XQ_ENTTYPE_NPCCORPSE
		) {
			xq_clog(COLOR_WHITE, "Current target is %i - not a valid enttype %i, resetting to 0",
				ps->xq_target, tcent->currentState.xq_enttype);
			target_poofed = 1;
		} else if (ps->xq_target != ps->clientNum && ps->xq_target != 0) {

			// See if the targeted entity is still in the packet entities
			target_poofed = 1;
			for (int i = 0;  i < cg.snap->numEntities; i++) {
				if (cg.snap->entities[i].number == ps->xq_target) {
					target_poofed = 0;
					break;
				}
   		 	}
		}

		if (target_poofed == 1) {
			xq_SetTarget(0, 0);
		}
	}
}
static void CoinSound(playerState_t *ps) {
	static int save_copper = -1, save_silver = -1, save_gold = -1, save_platinum = -1;
	if (
		ps->xq_copper != save_copper ||
		ps->xq_silver != save_silver ||
		ps->xq_gold != save_gold ||
		ps->xq_platinum != save_platinum
	) {
		if (save_copper != -1) { // skip 1st check
			trap_S_StartLocalSound(SOUNDH("sound/inv/coins.wav"), CHAN_ANNOUNCER);
		}
	}
	save_copper = ps->xq_copper;
	save_silver = ps->xq_silver;
	save_gold = ps->xq_gold;
	save_platinum = ps->xq_platinum;
}
static void PvPStatus(playerState_t *ps) {

	if (ps->xq_flags & XQ_ZONED_PROTECTED) {
		if (qws->zoned_protected == 0) {
			xq_clog(COLOR_YELLOW, "You are PROTECTED from PvP.");
			qws->zoned_protected = 1;
		}
	} else {
		if (qws->zoned_protected == 1) {
			xq_clog(COLOR_RED, "You are NO LONGER PROTECTED from PvP.");
			qws->zoned_protected = 0;
		}
	}
}
static void ChatWinCvars(void) {
	trap_Cvar_Set("cl_xq_chat_x", va("%i", xqst->ui_chat_window_x));
	trap_Cvar_Set("cl_xq_chat_y", va("%i", xqst->ui_chat_window_y));
	trap_Cvar_Set("cl_xq_chat_w", va("%i", xqst->ui_chat_window_w));
	trap_Cvar_Set("cl_xq_chat_h", va("%i", xqst->ui_chat_window_h));

	trap_Cvar_Set("cl_xq_chat_type_x",
		va("%i", xqst->ui_chat_window_x + 2));
	trap_Cvar_Set("cl_xq_chat_type_y",
		va("%i", xqst->ui_chat_window_y + xqst->ui_chat_window_h - XQ_CHAT_HEIGHT - 2));
}

static void FrameTime(void) {
	if (xq_printFrameTime.integer) {
		static uint64_t last = 0, now = 0;
		now = xq_usec();
		if (last != 0) {

			int64_t change = now - last;
			if (change > xq_printFrameTime.integer) {
				Com_Printf("%05li   now: %05li   change: %li", last, now, change);
				Com_Printf("\n");
			}

		}
		last = now;
	}
}
void xq_Frame(void) {
	// Stuff that will be run every client frame

	if (!cg.snap) return;

    centity_t *cent = &cg_entities[cg.snap->ps.clientNum];
    playerState_t *ps = &cg.snap->ps;
	xqst->frames++;
	xq_ping();

	// Process the xq_items[] queue - item info that we received from the server,
	// kept on the client (cl_parse.c), but haven't yet put in the cache in cgame (xq/info.c);
	xq_process_items();


	// Check that our target, if any, is still there
	TargetPoof(ps);


	// Play a sound each time our money changes in any way, except on initial load
	CoinSound(ps);


	// Set dynamic values for the UI.
	xqui_SetVals(cent, ps->xq_target ? &cg_entities[ps->xq_target] : NULL, ps);


	// Chat window's coords need to be copied to cl_ cvars as it will be used by
	// the Notify code in client/
	ChatWinCvars();


	// Run the UI frame - windows and such
	xqui_Frame();


	// Do the magic-related checks (spell gems or spell effects changing since last frame)
	xq_Spell_Frame(ps);


	// Inform the user about any change in the PvP protection status
	PvPStatus(ps);


	// Ask the server for a list of needed models if we haven't been sent one already
	if (xq_cmodels[0].default_scale == 0) {
		xq_InfoInfo(XQ_INFO_MODELS, 0, 0, 0);
	}


	// If we haven't received a snapshot for over 10 seconds,
	// we have probably been brutally disconnected in some way.
	// Quit bugging the server.
	if (cg.time > 2000 && (cg.time - cg.latestSnapshotTime) > 10000) {
		xq_disconnected();
	}


	// Move camera smoothly around the player if needed.
	xq_camera_frame();


	// Make sure we're not running on corrupted memory.
	CheckMagics();


	// Print time elapsed since last call
	FrameTime();
}

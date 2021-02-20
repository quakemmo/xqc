/*	DESCRIPTION

	Mouse-related stuff.
*/

#include "../cg_local.h"

int CG_ScanForMouseEntity( void ) {
    float w = 640.0f * (float)cgs.screenXScale;
    float h = 480.0f * (float)cgs.screenYScale;
	float mx = (float)xqst->mousex;
	float my = (float)xqst->mousey;
	float fov2 = (float)cg_fov.integer / 2.0;

	float mousex_diff;
	if (mx < w/2) {
		mousex_diff = (w/2 - mx)  / (w/2) * fov2;
	} else if (mx > w/2) {
		mousex_diff = -1 * ((mx - w/2)  / (w/2) * fov2);
	} else {
		mousex_diff = 0;
	}


	float mousey_diff;
	if (my < h/2) {
		mousey_diff = -1 * ((h/2 - my)  / (h/2) * fov2);
	} else if (my > h/2) {
		mousey_diff = (my - h/2)  / (h/2) * fov2;
	} else {
		mousey_diff = 0;
	}


	vec3_t angles, axis[3];
	VectorCopy(cg.refdefViewAngles, angles);

	angles[YAW] = mousex_diff + angles[YAW];
	angles[YAW] += 180.0f;
	angles[YAW] = fmod(angles[YAW], 360);
	angles[YAW] -= 180.0f;
	
	angles[PITCH] = mousey_diff + angles[PITCH];
	angles[PITCH] += 180.0f;
	angles[PITCH] = fmod(angles[PITCH], 360);
	angles[PITCH] -= 180.0f;

	AnglesToAxis( angles, axis);

	vec3_t start, end;
	VectorCopy(cg.refdef.vieworg, start);

	VectorMA(start, 131072, axis[0], end);

	trace_t trace;
	CG_Trace(&trace, start, vec3_origin, vec3_origin, end, 
		cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE);
	int entity = trace.entityNum;


	// if the target is in fog, don't show it
	int content = CG_PointContents(trace.endpos, 0);
	if (content & CONTENTS_FOG) {
		entity = -1;
	}

	return entity;
}
void xq_DrawMouse(void) {
    playerState_t *ps = &cg.snap->ps;


	// Don't draw the mouse cursor if we're moving a window
	if (qws->window_moving > -1) {
		return;
	}

	float w = 24;
	float h = 24;
	float x = cg_crosshairX.integer;
	float y = cg_crosshairY.integer;
	CG_AdjustFrom640( &x, &y, &w, &h );


	// Coords for stuff carried on mouse cursor
	int draggedx = xqst->mousex+30;
	int draggedy = xqst->mousey+30;

	// Draw mouse cursor
	int64_t mouseitem = XQ64(ps->xq_inv_mouse1_1, ps->xq_inv_mouse1_2);
    xq_item_t *iinf = NULL;
	if (mouseitem) {
		iinf = xq_ItemInfo(mouseitem);
	}
	trap_R_DrawStretchPic(xqst->mousex, xqst->mousey, w, h, 0, 0, 1, 1, cgs.media.mouseCursor);
	if (iinf) {
		// Draw item on mouse cursor
		xq_DrawItemIcon(NULL, draggedx, draggedy, iinf->icon, iinf->amount);

	} else if (ps->xq_mouse_money_type) {
		// Draw money on the mouse cursor
		xq_DrawItemIcon(NULL, draggedx, draggedy, -1 * ps->xq_mouse_money_type, ps->xq_mouse_money_amount);

	} else if (qws->mouse_spell) {
		// Draw spell on mouse cursor (this is purely local to the client)
		xq_spell_t *sp = xq_SpellInfo(qws->mouse_spell);
		if (sp) {
			xq_DrawSpellIcon(draggedx, draggedy, sp->effect_icon);
		}
	}
}
void xq_RightClickScene(int down) {
	static int64_t press_start = 0;
    int flags = cg.snap->ps.xq_flags;

	if (down) {
		press_start = xq_msec();
		//trap_Cvar_Set("cl_xq_mouselook", "1");
		trap_XQ_Mouselook(1);
	} else {
		//trap_Cvar_Set("cl_xq_mouselook", "0");
		trap_XQ_Mouselook(0);
	}

	if (down) return;

	if ((xq_msec() - press_start) > 300) return;	// Right click on a mob etc only works
													// if it's a relatively short right click

	// Right clicking a mob or a corpse
	int tcnum = CG_ScanForMouseEntity();
	if (tcnum > -1 && tcnum != ENTITYNUM_WORLD && tcnum != ENTITYNUM_NONE) {
		centity_t *cent = &cg_entities[tcnum];
		entityState_t *es = &cent->currentState;
		if (es->xq_enttype == XQ_ENTTYPE_NPC) {
			if (!(es->xq_flags & XQ_BANKER) && !(es->xq_flags & XQ_MERCHANT)) {
				xq_SetTarget(tcnum, 1); // 1 means set target and immediately /consider
			} else {
				if (es->xq_flags & XQ_BANKER) {
					if (!(flags & XQ_BANKING)) {
						xq_scmd(va("/bank %i", tcnum));
					} else {
						xq_clog(COLOR_RED, "You are already banking!");
					}
				} else if (es->xq_flags & XQ_MERCHANT) {
					if (!(flags & XQ_BUYING)) {
						xq_scmd(va("/merchant %i", tcnum));
					} else {
						xq_clog(COLOR_RED, "You are already buying!");
					}
				}
			}
		} else if (es->xq_enttype == XQ_ENTTYPE_PCCORPSE || es->xq_enttype == XQ_ENTTYPE_NPCCORPSE) {
			xq_scmd(va("/loot %i", tcnum));
		}
	}
}
void xq_MouseMove(int x, int y) {
	static int mx = 0, my = 0;

	x /= 1000;
	y /= 1000;

	if (x == 0 && y == 0) return;

	int w = (int) (640 * cgs.screenXScale);
	int h = (int) (480 * cgs.screenYScale);

	mx += x;
	my += y;
	if (mx < 0) {
		mx = 0;
	} else if (mx > w) {
		mx = w;
	}

	if (my < 0) {
		my = 0;
	} else if (my > h) {
		my = h;
	}

	xqst->mousex = mx;
	xqst->mousey = my;

	if (xqst->mousefree == 1) {
		if (qws->window_moving > -1) {
			qw_WindowMove();
		} else if (qws->window_resizing > -1) {
			qw_WindowResize();
		} else {
			qw_MouseMove();
		}
	}
}

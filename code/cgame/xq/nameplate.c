/*	DESCRIPTION

	Displaying of PC, NPC and corpse name plates.
*/

#include "../cg_local.h"

#define FAR_FADE_START		450		// name plate starts fading out if the distance to it is over this much
#define CLOSE_FADE_START	75		// or closer that this
#define FAR_CUTOFF			600		// name plate isn't displayed at all for distances over this much
#define CLOSE_CUTOFF		20		// or under this much
#define TARGET_PULSE_DEPTH	10		// targeted nameplate will pulsate betweet 100 and that many percent
#define TARGET_PULSE_STEP	3		// the higher, the faster the target pulse will be

static void showPlate(centity_t *cent, qhandle_t shader, refEntity_t *refent, int alpha, vec3_t col) {
    int rf = 0;
	float scale = (float)cent->currentState.xq_app_model_scale;
    if (cent->currentState.number == cg.snap->ps.clientNum) {
		scale = cg.snap->ps.xq_app_model_scale;
		if (!cg.renderingThirdPerson) {
			rf = RF_THIRD_PERSON;       // only show in mirrors
		}
    }

    refEntity_t ent = {0};
    VectorCopy(cent->lerpOrigin, ent.origin);
    ent.origin[2] += 42;
    ent.reType = RT_XQNAME;
    ent.customShader = shader;
    ent.radius = 10;
    ent.renderfx = rf;
    ent.shaderRGBA[0] = col[0];
    ent.shaderRGBA[1] = col[1];
    ent.shaderRGBA[2] = col[2];
    ent.shaderRGBA[3] = alpha;


	if (refent) {
		char *tag = "tag_head";
		int add_height = 14;


		// if we have specified a tag for the nameplate, use that
		if (trap_XQ_TagExists("tag_nameplate", refent->hModel)) {
			tag = "tag_nameplate";
			add_height = 0;
		}
		CG_PositionRotatedEntityOnTag(&ent, refent, refent->hModel, tag);
		if (scale > 100) {
			float n = (add_height / 100.0) * scale;
			add_height = n;
		}
		ent.origin[2] += add_height;
	}

    trap_R_AddRefEntityToScene(&ent);
}
void xq_name_plate(centity_t *cent, refEntity_t *refent) {
	entityState_t *es = &cent->currentState;
    playerState_t *ps = &cg.snap->ps;
	int level = es->xq_level;

	int self = 0;
	if (es->number == ps->clientNum) {
		level = ps->xq_level;
		self = 1;
	} else {
		// If we're looking at our own corpse - always display the plate
		char name1[100] = {0};
		strncpy(name1, xq_EntName(es), sizeof(name1));
		name1[0] = toupper(name1[0]);
		if (xq_seq(xqst->ui_charname, name1)) {
			self = 1;
		}
	}


	// Do not display the level in non-arena zones
	if (!(ps->xq_flags & XQ_ZONE_ARENA)) {
		level = 0;
	}


	// Float entity's name above it
	float dist = Distance(ps->origin, cent->lerpOrigin);
	if (dist > FAR_CUTOFF || dist < CLOSE_CUTOFF) {
		// we always display our own plate
		if (!self) {
			return;
		}
	}
	char *xqname = xq_getname(cent, 0);
	if (xqname == NULL) {
		return;
	}


	vec3_t col = {255, 255, 255};
	// If the name plate we're drawing belongs to our target
	// we want to pulsate it
	if (es->number == cg.snap->ps.xq_target) {
		static uint64_t last = 0;
		static int pulse = 100;
		static int dir = 1;


		// we adjust the step so the pulse rate is more or less the same
		// in different FPS situations
		int step = TARGET_PULSE_STEP;
		uint64_t elapsed = xq_msec() - last;
		if (last != 0) {
			if (elapsed != 6) {
				step *= elapsed;
				step /= 6;
			}
		}
		last = xq_msec();


		// move the pulse value around
		if (dir == 0) {
			pulse += step;
		} else {
			pulse -= step;
		}


		// change pulse direction as needed
		if (pulse > 100) {
			dir = 1;
			pulse = 100;
		} else if (pulse < TARGET_PULSE_DEPTH) {
			dir = 0;
			pulse = TARGET_PULSE_DEPTH;
		}


		// use the pulse value to set the color
		int clr = (pulse / 100.0) * 255.0;
		VectorSet(col, clr, clr, clr);
	}

	char *tmp;
	if (es->xq_invis || (self && ps->xq_invis)) {
		tmp = va("(%s)", xqname);
	} else {
		tmp = va("%s", xqname);
	}

	qhandle_t qh = trap_R_XQ_TShader(tmp, level);
	if (qh == -1) {
		return;
	}


	// We want to alpha fade the plate if we're too close or too far
	// xq_namePlateAlpha cvar: 0 = never fade, 1 = fade if too far,
	// 2 = fade if too close, 3 (default) = fade both far and close.
	int alpha = 255;
	if (!self) { // we never reduce alpha for our own plate
		int parm = xq_namePlateAlpha.integer;
		if (dist > FAR_FADE_START && (parm == 1 || parm == 3)) {
			float perc1 = (FAR_CUTOFF - FAR_FADE_START) / 100.0;
			float percs = (dist - FAR_FADE_START) / perc1;
			alpha *= (1.0 - (percs / 100.0));
		} else if (dist < CLOSE_FADE_START && (parm == 2 || parm == 3)) {
			float perc1 = (CLOSE_FADE_START - CLOSE_CUTOFF) / 100.0;
			float percs = (CLOSE_FADE_START - dist) / perc1;
			alpha *= (1.0 - (percs / 100.0));
		}
	}
	showPlate(cent, qh, refent, alpha, col);
}

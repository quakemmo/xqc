#include "../cg_local.h"

//#define DBG 1

void xq_draw_held(centity_t *cent, refEntity_t *torso, playerState_t *ps, int modelnum, int hand, int flags) {
	/*

	Displays an item held in the main hand or in the off hand by a PC, NPC or a corpse of either.
	This can be a melee weapon, nothing or any other item that has a model and is equippable in
	the PRIMARY1 or SECONDARY1 slot.

	This doesn't apply to items equipped in the PRIMARY slot other than PRIMARY1 (melee).

	This doesn't happen for PC's in arena-flagged zones - a generic melee weapon is displayed for all PCs.
	Linkdead PC's will still display a "proper" held model in arena zones before they time out.

	*/
	#ifdef DBG
		xq_clog(COLOR_YELLOW, "xq_draw_held: modelnum: %i, hand: %i, flags: %i",
			modelnum, hand, flags);
	#endif


	// Draw a held item
	if (modelnum > 0 && modelnum <= xqst->item_models_registered) {
		// We have an item with a particular model held
		refEntity_t model = {0};
		qhandle_t mod_h = xqst->item_models[modelnum - 1]; // modelnum #1 starts at index 0 in xqst->item_models[]
		if (!mod_h) {
			xq_clog(COLOR_RED, "No handle for held item model %i on hand %i", modelnum, hand);
			return;
		}

		VectorCopy(torso->lightingOrigin, model.lightingOrigin );
		model.shadowPlane = torso->shadowPlane;
		model.renderfx = torso->renderfx;
		model.hModel = mod_h;


		// Attach the held model to the torso's tag
		orientation_t lerped;
		trap_R_LerpTag(
			&lerped,
			torso->hModel,
			torso->oldframe,
			torso->frame,
			1.0 - torso->backlerp,
			hand == 1 ? "tag_weapon" : "tag_offhand"
		);
		VectorCopy(torso->origin, model.origin);
		VectorMA(model.origin, lerped.origin[0], torso->axis[0], model.origin);

		// left/right handed or centered
		if (ps) {
			VectorMA(model.origin, -lerped.origin[1], torso->axis[1], model.origin);
		} else {
			VectorMA(model.origin, lerped.origin[1], torso->axis[1], model.origin);
		}

		VectorMA(model.origin, lerped.origin[2], torso->axis[2], model.origin);

		MatrixMultiply(lerped.axis, (torso)->axis, model.axis);
		model.backlerp = torso->backlerp;

		// Adds the refentity to the scene, adding a custom shader
		// to the model if needed (quad, invuln, etc)
		CG_AddWeaponWithPowerups(&model, flags, &cent->currentState);
	}
}
int xq_emphasize_model(entityState_t *es) {
	// Returns 1 if the es's model needs to be arena-style emphasized, 0 otherwise.

	if (!cg.snap || cg_emphasizeArena.integer == 0) {
		return 0;
	}

	if (cg.snap->ps.xq_flags & XQ_ZONE_ARENA) { // Only for arena zones
		if (es->xq_enttype == XQ_ENTTYPE_PC) { // Only for PCs
			if (es->number != cg.snap->ps.clientNum) { // Don't emphasize if looking at oneself
				return 1;
			}
		}
	}
	return 0;
}
void xq_dump_anim(centity_t *cent, xq_animodel_t *am, int after) {
	int color = COLOR_WHITE;
	if (after) color = COLOR_GREEN;
    
#if 0
	animation_t *anim = cent->pe.torso.animation;
	entityState_t *es = &cent->currentState;
    xq_clog(color,

		"%i "

		"anim->firstFrame: %i, "
		"anim->numFrames: %i, "
		"anim->frameLerp: %i, "
		"anim->initialLerp: %i, "
		"c->pe.torso.animationNumber: %i, "
		"c->pe.torso.animationTime: %i, "
		"elapsed: %i, "
		"c->pe.torso.backlerp: %f, ",

		cg.time,

		anim->firstFrame,
		anim->numFrames,
		anim->frameLerp,
		anim->initialLerp,
		cent->pe.torso.animationNumber,
		cent->pe.torso.animationTime,
		cg.time - cent->pe.torso.animationTime, 
		cent->pe.torso.backlerp

	);

#endif

    xq_clog(color,

		"%i "

		"c->pe.torso.oldFrame: %i, "
		"c->pe.torso.frame: %i, "
		"c->pe.torso.oldFrameTime: %i, "
		"c->pe.torso.FrameTime: %i, ",

		cg.time,

		cent->pe.torso.oldFrame,
		cent->pe.torso.frame,
		cent->pe.torso.oldFrameTime,
		cent->pe.torso.frameTime

	);
}                                                                                                                                                                                         
void xq_ScaleModel(float scale, refEntity_t *legs, refEntity_t *torso, refEntity_t *head) {
	if (scale != 0) {
		scale /= 100.0;
	} else {
		return;
	}
	for (int i = 0;  i <= 2;  i++) {
		if (legs) {
			VectorScale(legs->axis[i], scale, legs->axis[i]);
   			legs->nonNormalizedAxes = qtrue;
		}

        if (head) {
			VectorScale(head->axis[i], scale, head->axis[i]);
    		head->nonNormalizedAxes = qtrue;
		}

        if (torso) {
			VectorScale(torso->axis[i], scale, torso->axis[i]);
    		torso->nonNormalizedAxes = qtrue;
		}
	}
}

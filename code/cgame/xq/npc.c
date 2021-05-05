#include "../cg_local.h"

/* DESCRIPTION

	NPC, NPC corpse and PC corpse rendering functions.
	Only xq_npc_draw() is public and is called from cg_ents.c for drawing NPC or NPC/PC corpse ents.

	A "simple" NPC has one model (eg: elemental pets).
	A "q3player" NPC is a Q3-style compound model of head/torso/legs.

	A PC with an illusion to a simple model is not rendered here.


	Possible animations:

	LEG ANIMS (deduced by the client based on movement)
	- idle stand
	- idle stand 2 (see desc below)
	- walk
	- run
	- turn
	- swim
	- crouch


	TORSO ANIMS (transmitted by the server)
	- idle (default)
	- idle2 (client will randomly use this instead of idle)
	- melee
	- cast
	- be in pain
*/


typedef struct torso_ret_s {
	// Used by torso_anim_arbiter() to return some info about
	// the selected torso anim
	int		torso_anim;
	int		candidate_anim;
	int		idle_variety;
	int		replace;
} torso_ret;

// Animations
static int find_best_candidate_anim(centity_t *cent) {
	int candidate = 0;
	// Is one of the two anims CAST?
	if (cent->xq_anim1 == XQ_ANIM_CAST) {
		if (cent->xq_anim2 == XQ_ANIM_CAST) {
			candidate = cent->xq_anim1_ts < cent->xq_anim2_ts ? 1 : 2;
		} else {
			candidate = 1;
		}
		if (xq_debugNpcDraw.integer) {
			xq_clog(COLOR_YELLOW, "%i candidate is CAST in slot %i", cg.time, candidate);
		}
	} else if (cent->xq_anim2 == XQ_ANIM_CAST) {
		candidate = 2;
		if (xq_debugNpcDraw.integer) {
			xq_clog(COLOR_YELLOW, "%i candidate is CAST in slot %i", cg.time, candidate);
		}
	} else {
		// None are CAST
		if (cent->xq_anim1 == XQ_ANIM_MELEE) {
			if (cent->xq_anim2 == XQ_ANIM_MELEE) {
				candidate = cent->xq_anim1_ts < cent->xq_anim2_ts ? 1 : 2;
			} else {
				candidate = 1;
			}
			if (xq_debugNpcDraw.integer) {
				xq_clog(COLOR_YELLOW, "%i candidate is MELEE in slot %i", cg.time, candidate);
			}
		} else if (cent->xq_anim2 == XQ_ANIM_MELEE) {
			candidate = 2;
			if (xq_debugNpcDraw.integer) {
				xq_clog(COLOR_YELLOW, "%i candidate is MELEE in slot %i", cg.time, candidate);
			}
		} else {
			// None are MELEE either
			if (cent->xq_anim1 == XQ_ANIM_PAIN) {
				if (cent->xq_anim2 == XQ_ANIM_PAIN) {
					candidate = cent->xq_anim1_ts < cent->xq_anim2_ts ? 1 : 2;
				} else {
					candidate = 1;
				}
				if (xq_debugNpcDraw.integer) {
					xq_clog(COLOR_YELLOW, "%i candidate is PAIN in slot %i", cg.time, candidate);
				}
			} else if (cent->xq_anim2 == XQ_ANIM_PAIN) {
				candidate = 2;
				if (xq_debugNpcDraw.integer) {
					xq_clog(COLOR_YELLOW, "%i candidate is PAIN in slot %i", cg.time, candidate);
				}
			} else {
				// Both spots are empty
				if (xq_debugNpcDraw.integer) {
					xq_clog(COLOR_RED, "%i candidate is IDLE", cg.time);
				}
			}
		}
	}
	return candidate;
}
static qboolean expire_anim_queue(centity_t *cent) {
	qboolean expired = qfalse;
	if (cent->xq_anim2) {
		if ((cg.time - cent->xq_anim2_ts) > XQ_ANIM_AGE) {
			cent->xq_anim2 = cent->xq_anim2_ts = 0;
			if (xq_debugNpcDraw.integer) {
				xq_clog(COLOR_WHITE, "%i Expiring slot 2", cg.time);
			}
			expired = qtrue;
		}
		if ((cg.time - cent->xq_anim1_ts) > XQ_ANIM_AGE) {
			if (cent->xq_anim2) {
				cent->xq_anim1 = cent->xq_anim2;
				cent->xq_anim1_ts = cent->xq_anim2_ts;
				cent->xq_anim2 = cent->xq_anim2_ts = 0;
				if (xq_debugNpcDraw.integer) {
					xq_clog(COLOR_WHITE, "%i Expiring slot 1 - moving 2 up", cg.time);
				}
				expired = qtrue;
			} else {
				if (xq_debugNpcDraw.integer) {
					xq_clog(COLOR_WHITE, "%i Expiring slot 1 - 2 was empty", cg.time);
				}
				cent->xq_anim1 = cent->xq_anim1_ts = 0;
				expired = qtrue;
			}
		}
	}
	return expired;
}
static void anim_q3player_newanim(xq_animodel_t *am, lerpFrame_t *lf, int newAnimation) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS ) {
		CG_Error( "Bad animation number: %i", newAnimation );
	}

	anim = &am->animations[ newAnimation ];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if (xq_debugNpcDraw.integer) {
		if (newAnimation == TORSO_GESTURE) {
			xq_clog(COLOR_YELLOW, "anim_q3player_newanim(): newAnimation: %i, lf->animationTime: %i, lf->frameTime: %i, anim->initialLerp: %i",
				newAnimation,  lf->animationTime, lf->frameTime, anim->initialLerp);
		}
	}
}
static void anim_q3player_runframe(xq_animodel_t *am, lerpFrame_t *lf, int newAnimation, float speedScale) {
	int deb = xq_debugNpcDraw.integer;

	// debugging tool to get no animations
	if ( cg_animSpeed.integer == 0 ) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
		if (deb) xq_clog(COLOR_GREEN, "Animation is switching");
		anim_q3player_newanim(am, lf, newAnimation); 
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if (cg.time >= lf->frameTime) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;
		if (deb)
			 xq_clog(COLOR_RED, "frame: %i, cg.time: %i, lf->oldFrameTime: %i, lf->frameTime: %i, diff1: %i, diff2: %i",
					 lf->frame, cg.time, lf->oldFrameTime, lf->frameTime, cg.time - lf->oldFrameTime, lf->frameTime - lf->oldFrameTime);

		// get the next frame based on the animation
		animation_t *anim = lf->animation;
		if ( !anim->frameLerp ) {
			return;		// shouldn't happen
		}
		if ( cg.time < lf->animationTime ) {
		if (deb) xq_clog(COLOR_RED, "1 Setting lf->frameTime: from %i to %i, lf->animationTime is %i", lf->frameTime, lf->animationTime, lf->animationTime);
			lf->frameTime = lf->animationTime;		// initial lerp
		if (deb) xq_clog(COLOR_RED, "anim->frameLerp: %i, cg.time: %i, lf->frameTime: %i, lf->animationTime: %i",
			anim->frameLerp, cg.time, lf->frameTime, lf->animationTime);
		} else {
			if (deb) xq_clog(COLOR_RED,
				"2 Setting lf->frameTime: from %i to %i, lf->animationTime is %i (diff is: %i)",
				lf->frameTime, lf->oldFrameTime + anim->frameLerp, lf->animationTime,
				lf->oldFrameTime + anim->frameLerp - lf->frameTime);
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		int f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		f *= speedScale;		// adjust for haste, etc


		int numFrames = anim->numFrames;
		if (anim->flipflop) {
			numFrames *= 2;
		}
		if ( f >= numFrames ) {
			f -= numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				if (deb) xq_clog(COLOR_RED, "3 Setting lf->frameTime: from %i to %i, lf->animationTime is %i", lf->frameTime, cg.time, lf->animationTime);
				lf->frameTime = cg.time;
			}
		}
		if ( anim->reversed ) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if (anim->flipflop && f>=anim->numFrames) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
		}
		else {
			lf->frame = anim->firstFrame + f;
		}
		if ( cg.time > lf->frameTime ) {
			if (deb) xq_clog(COLOR_RED, "4 Setting lf->frameTime: from %i to %i, lf->animationTime is %i", lf->frameTime, cg.time, lf->animationTime);
			lf->frameTime = cg.time;
			if ( cg_debugAnim.integer ) {
				CG_Printf( "Clamp lf->frameTime\n");
			}
		}
	}

	if ( lf->frameTime > cg.time + 200 ) {
		if (deb) xq_clog(COLOR_RED, "5 Setting lf->frameTime: from %i to %i, lf->animationTime is %i", lf->frameTime, cg.time, lf->animationTime);
		lf->frameTime = cg.time;
	}

	if ( lf->oldFrameTime > cg.time ) {
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}
static void add_anim_request_to_queue(centity_t *cent, int torso_anim, int times) {
	int i;
	int deb = xq_debugNpcDraw.integer;
	// CAST will overwrite anything but CAST
	// MELEE will overwrite PAIN but not cast
	// PAIN will not overwrite anything and only go in if anim1 or anim2 is empty
	for (i = 0;  i < times;  i++) {
		if (torso_anim == XQ_ANIM_CAST) {
			if (cent->xq_anim1 != XQ_ANIM_CAST) {
				cent->xq_anim1 = XQ_ANIM_CAST;
				cent->xq_anim1_ts = cg.time;
				if (deb) xq_clog(COLOR_YELLOW, "%i Setting CAST to slot 1", cg.time);
			} else if (cent->xq_anim2 != XQ_ANIM_CAST) {
				cent->xq_anim2 = XQ_ANIM_CAST;
				cent->xq_anim2_ts = cg.time;
				if (deb) xq_clog(COLOR_YELLOW, "%i Setting CAST to slot 2", cg.time);
			} else {
				// Both slots are filled with CAST anims and we have more CASTs inc - silently discarding CAST anim reqeust
				if (deb) xq_clog(COLOR_YELLOW, "%i Discarding CAST request", cg.time);
			}
		} else if (torso_anim == XQ_ANIM_MELEE) {
			if (cent->xq_anim1 != XQ_ANIM_CAST && cent->xq_anim1 != XQ_ANIM_MELEE) {
				cent->xq_anim1 = XQ_ANIM_MELEE;
				cent->xq_anim1_ts = cg.time;
				if (deb) xq_clog(COLOR_YELLOW, "%i Setting MELEE to slot 1", cg.time);
			} else if (cent->xq_anim2 != XQ_ANIM_CAST && cent->xq_anim2 != XQ_ANIM_MELEE) {
				cent->xq_anim2 = XQ_ANIM_MELEE;
				cent->xq_anim2_ts = cg.time;
				if (deb) xq_clog(COLOR_YELLOW, "%i Setting MELEE to slot 2", cg.time);
			} else {
				// Both slots are filled with CAST or older MELEE - silently discarding MELEE anim request
				if (deb) xq_clog(COLOR_YELLOW, "%i Discarding MELEE request", cg.time);
			}
		} else if (torso_anim == XQ_ANIM_PAIN) {
			if (cent->xq_anim1 == 0) {
				cent->xq_anim1 = XQ_ANIM_PAIN;
				cent->xq_anim1_ts = cg.time;
				if (deb) xq_clog(COLOR_YELLOW, "%i Setting PAIN to slot 1", cg.time);
			} else if (cent->xq_anim2 == 0) {
				cent->xq_anim2 = XQ_ANIM_PAIN;
				cent->xq_anim2_ts = cg.time;
				if (deb) xq_clog(COLOR_YELLOW, "%i Setting PAIN to slot 2", cg.time);
			} else {
				// Both slots are taken by something already - silently discarding PAIN anim request
				if (deb) xq_clog(COLOR_YELLOW, "%i Discarding PAIN request", cg.time);
			}
		}
	}
}
static void sound_arbiter(centity_t *cent, int torso_anim, int leg_anim, vec3_t moved, int idle_variety, xq_animodel_t *am) {
	entityState_t *es = &cent->currentState;
	int centnum = es->number;

	if (cent->xq_just_popped) {
		// We don't want the initial footstep upon pop
		return;
	}

	// Add torso sound effect if needed
	switch (torso_anim) { 
		case XQ_ANIM_IDLE:
			if (idle_variety == 2) {
				xq_OptSound(NULL, centnum, CHAN_BODY, "*altidle.wav", 0);
			}
			break;
		case XQ_ANIM_CAST:
			xq_OptSound(NULL, centnum, CHAN_BODY, "*casted.wav", 0);
			break;
		case XQ_ANIM_MELEE:
			xq_OptSound(NULL, centnum, CHAN_BODY, "*melee.wav", 0);
			break;
		case XQ_ANIM_PAIN:
			if ((cg.time - cent->xq_last_painsound_ts) > XQ_SOUND_PAIN_MIN_DELAY) {
				cent->xq_last_painsound_ts = cg.time;
				xq_OptSound(NULL, centnum, CHAN_BODY, "*pain100.wav", 0);
			}
			break;
		default:
			// torso_anim will be -1 and branch here if we don't need to worry about it
			break;
	}

	// Play footstep sound if we moved
	if (leg_anim == XQ_ANIM_WALK || leg_anim == XQ_ANIM_RUN) {
		// Legs played an animation involving footsteps
		if (moved[0] && moved[1]) {
			// We actually moved X and/or Y wise
			if (cg.time - cent->xq_last_footstep_ts > XQ_SOUND_FOOTSTEP_MIN_DELAY) {
				// We didn't play a footstep on this npc too recently
				if (cent->pe.legs.animation &&
					((cent->pe.legs.frame == cent->pe.legs.animation->firstFrame) ||
					(cent->pe.legs.frame == cent->pe.legs.animation->firstFrame + cent->pe.legs.animation->numFrames / 2))
				) { // If the walk/run animation is on the 1st or middle frame
					xq_OptSound(NULL, centnum, CHAN_BODY, NULL, cgs.media.footsteps[am->footsteps][rand()&3]);
					cent->xq_last_footstep_ts = cg.time;
				}
			}
		}
	}
}
static int leg_anim_arbiter(centity_t *cent, vec3_t m, int *legsOld, int *legs, float *legsBackLerp, xq_animodel_t *am, float speedScale) {
	entityState_t *es = &cent->currentState;
	int centnum = es->number;
	int leg_anim = XQ_ANIM_IDLE;

	float dist = VectorLength(es->pos.trDelta);
	if (m[0] || m[1] || m[2]) {
		//xq_clog(COLOR_WHITE, "Moved: %f / %f / %f", m[0], m[1], m[2]);
		//xq_clog(COLOR_GREEN, "Exact X/Y/Z distance moved: %f / %f / %f", es->pos.trDelta[0], es->pos.trDelta[1], es->pos.trDelta[2]);
		//xq_clog(COLOR_GREEN, "Dist: %f", dist);
	}
	if (!m[0] && !m[1] && !m[2]) {
		// We haven't moved
		leg_anim = XQ_ANIM_IDLE;
		// See if our legs are yawing
		if (cent->xq_last_snapshottime != cent->snapShotTime) {
			cent->xq_last_snapshottime = cent->snapShotTime;
			if (cent->xq_last_heading != es->xq_heading) {
				cent->xq_last_heading = es->xq_heading;
				cent->pe.legs.yawing = qtrue;
			} else {
				cent->pe.legs.yawing = qfalse;
			}
		}

	} else if (!m[0] && !m[1] && m[2] < 0) {
		// We have moved straight down - we don't care about instafalling gravity npcs - we care about flying ones
		leg_anim = XQ_ANIM_WALK;
	} else if (!m[0] && !m[1] && m[2] > 0) {
		// We moved straight up - should only happen with flying mobs
		leg_anim = XQ_ANIM_WALK;
	} else {
		// We walked/ran
		leg_anim = XQ_ANIM_WALK;
	}


	// If we walk, see if we should run
	if (dist > XQ_ANIM_RUN_THRESHOLD_NPC) {
		leg_anim = XQ_ANIM_RUN;
	}

	// If we are in the water, base anim can be either swim or idle only
	// A mob can stand upright in the water (on a surface), only swims if it's moving or not standing on a solid surface
	if (CG_PointContents(cent->lerpOrigin, centnum) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA)) {
		if (leg_anim != XQ_ANIM_IDLE) {
			leg_anim = XQ_ANIM_SWIM;
		} else {
			trace_t trace;
			vec3_t grd;
			VectorCopy(cent->lerpOrigin, grd);
			grd[2] -= 0.26;
			CG_Trace(&trace, cent->lerpOrigin, NULL, NULL, grd, 0, MASK_SOLID);
			if (trace.fraction == 1.0) {
				leg_anim = XQ_ANIM_SWIM;
			}
		}

	}

	// Translate xq_anim_t into animNumber_t for legsAnim
	es->legsAnim = xq_anim_to_q3(leg_anim, 0, 0);

    // do the shuffle turn frames locally
	if (cent->pe.legs.yawing && (es->legsAnim & ~ANIM_TOGGLEBIT) == LEGS_IDLE) {
		anim_q3player_runframe(am, &cent->pe.legs, LEGS_TURN, speedScale);
	} else {
		anim_q3player_runframe(am, &cent->pe.legs, es->legsAnim, speedScale);
	}

    *legsOld = cent->pe.legs.oldFrame;
    *legs = cent->pe.legs.frame;
    *legsBackLerp = cent->pe.legs.backlerp;
	return leg_anim;
}
static void torso_anim_arbiter(centity_t *cent, int *torsoOld, int *torso, float *torsoBackLerp, xq_animodel_t *am, float speedScale, int leg_anim, torso_ret *ret) {
	int deb = xq_debugNpcDraw.integer;
	entityState_t *es = &cent->currentState;

	// Server-supplied anim for this frame is in es->xq_anim
	// last nibble is the xq_anim_t number (XQ_ANIM_CAST, XQ_ANIM_MELEE, etc)
	// previous nibble is however many times the server wants us to play it (can be 1 or 2)
	// Things like npc double attack, double proc etc can happen in one server frame
	// We want the npc to swing twice if it double attacks and we have no more important
	// torso anims to play at the moment.

	int torso_anim = es->xq_anim & 0x0f;
	int times = es->xq_anim >> 4;
	if (times > 0 && torso_anim) {
		// New anim request received
		if (deb) xq_clog(COLOR_GREEN, "%i npc: %s#%i, cent->xq_torsoAnim: %i, torso_anim: %i, times: %i",
			cg.time, xq_getname(cent, 1), cg.time, cent->xq_torsoAnim & ~ANIM_TOGGLEBIT, torso_anim, times);

		// Clear request from entityState so we don't evaluate it again before next server snapshot is received
		es->xq_anim = 0;
		if (deb) xq_clog(COLOR_WHITE, "%i Queue before expiring: xq_anim1: %i / %i, xq_anim2: %i / %i",
			cg.time, cent->xq_anim1, cent->xq_anim1_ts, cent->xq_anim2, cent->xq_anim2_ts);
	}


	// We have cent->xq_anim1 and cent->xq_anim2, with corresponding cg.time timestamps in cent->xq_anim1_ts and cent->xq_anim2_ts
	// First, expire them if they are too old
	// Shift 2nd anim up to 1st spot if needed
	if (expire_anim_queue(cent)) {
		if (deb) xq_clog(COLOR_WHITE, "%i Queue after expiring: xq_anim1: %i / %i, xq_anim2: %i / %i",
			cg.time, cent->xq_anim1, cent->xq_anim1_ts, cent->xq_anim2, cent->xq_anim2_ts);
	}


	// We don't actually honor double PAIN requests.  It doesn't look too natural.
	if (torso_anim == XQ_ANIM_PAIN && times == 2) times = 1;


	// If we have a non idle anim request, add it to the queue
	// The following function handles animation priority and will overwrite some anims with
	// more important ones
	add_anim_request_to_queue(cent, torso_anim, times);
	if (deb) {
		if (cent->xq_anim1 || cent->xq_anim1_ts || cent->xq_anim2 || cent->xq_anim2_ts) {
			xq_clog(COLOR_GREEN, "%i Queue status : xq_anim1: %i / %i, xq_anim2: %i / %i",
				cg.time, cent->xq_anim1, cent->xq_anim1_ts, cent->xq_anim2, cent->xq_anim2_ts);
		}
	}


	// By now we have a properly formed queue of up to two anim requests,
	// that takes into account anim requests received in this snapshot.
	// The queue is in no particular order other than 1st spot shouldn't be empty if 2nd one isn't
	// Find out the next anim from this queue we'd play, if we had to start one now - call it candidate
	int candidate = find_best_candidate_anim(cent);

	// By now we have a candidate anim which will maybe be replacing the currently playing (or finished) one


	// Let's see if the current animation can be switched to a new one
	// If it's an important anim, it has to be finished.
	// If it's an idle anim, it can be interrupted at any time (we always consider it finished)
	animation_t *anim = cent->pe.torso.animation;
	int finished = 0;
	int curanim = cent->xq_torsoAnim & ~ANIM_TOGGLEBIT;
	int frame = -1;
	if (curanim == TORSO_STAND2 || curanim == TORSO_STAND || curanim == 0) {
		// It's ok to always interrupt IDLE anim or BOTH_DEATH1
		finished = 1;
		if (deb) xq_clog(COLOR_GREEN, "%i low priority anim (%i) - considering it finished", cg.time, curanim);
	} else {
		if (anim) {
			frame = cent->pe.torso.frame;
			if (frame >= anim->firstFrame + anim->numFrames - 1) {
				if (cent->pe.torso.animationTime < cg.time) {
					// Make sure the high priority anim actually had a chance to play and properly set ->frame etc
					finished = 1;
					if (deb) xq_clog(COLOR_GREEN, "%i high priority anim (%i) - it is finished (it started on: %i)",
						cg.time, curanim, cent->pe.torso.animationTime);
				}
			}
		}
	}
	if (deb) {
		if (!finished) {
			xq_clog(COLOR_GREEN, "%i anim (%i) - it is NOT finished: frame: %i, anim->firstFrame: %i, anim->numFrames: %i",
				cg.time, curanim, frame, anim->firstFrame, anim->numFrames);
		}
	}


	xq_anim_t candidate_anim = XQ_ANIM_IDLE;
	if (candidate == 1) {
		candidate_anim = cent->xq_anim1;
	} else if (candidate == 2) {
		candidate_anim = cent->xq_anim2;
	}


	// By now we have the following data:
	// - finished: 0 or 1, currently played animation is considered finished playing
	// - candidate: 0, 1 or 2.  candidate anim slot.  0 means none
	// - candidate_anim: the xq_anim_t anim, if any
	// - curanim: the animNumber_t currently playing
	// We need to figure out whether or not we replace the current anim with the candidate if any
	// If we do, we'll have to empty the relevant request queue spot
	int replace = 0;

	// We only consider a new anim if the current one is considered finished and we have an actual candidate
	if (candidate && finished) {
		// We don't replace an IDLE anim by an IDLE anim
		if (!(candidate_anim == XQ_ANIM_IDLE && (curanim == TORSO_STAND2 || curanim == TORSO_STAND))) {
			replace = 1;
			if (deb) xq_clog(COLOR_WHITE, "%i we are going to replace current anim %i with candidate_anim %i",
				cg.time, curanim, candidate_anim);
		} else {
			if (deb) xq_clog(COLOR_WHITE, "%i not going to replace idle with idle", cg.time);
		}
	}


	// Upon pop a mob has default anim of value 0 which is BOTH_DEATH1.
	// This needs to be changed to IDLE
	if (curanim == BOTH_DEATH1) {
		if (deb) xq_clog(COLOR_GREEN, "%i seems dead", cg.time);
		replace = 1;
	}


	// See if we need to resume the IDLE anim because another anim has finished and nothing better has come up
	if (!candidate && finished && candidate_anim == XQ_ANIM_IDLE && curanim != TORSO_STAND2 && curanim != TORSO_STAND) {
		if (deb) xq_clog(COLOR_YELLOW, "%i need to switch back to IDLE", cg.time);
		replace = 1;
	}


	// Sometimes we want to play idle TORSO_STAND instead of the default idle TORSO_STAND2 for variety
	// We only play it occasionally and when nothing else is going on and the mob is standing still
	int idle_variety = 1;
	if (finished && !candidate && candidate_anim == XQ_ANIM_IDLE && leg_anim == XQ_ANIM_IDLE) {
		if (anim) {
			frame = cent->pe.torso.frame;
			if (frame >= anim->firstFrame + anim->numFrames - 1) {
				if ((curanim == TORSO_STAND2 && (rand() % XQ_IDLE_VARIETY_DELAY) == 1) || (curanim == TORSO_STAND)) {
					replace = 1;
					idle_variety = curanim == TORSO_STAND2 ? 2 : 1;
					if (deb) xq_clog(COLOR_GREEN, "%i Switching to idle variety %i:  %i / %i",
						cg.time, idle_variety, frame, anim->firstFrame);
				}
			}
		}
	}


	// If we replace the current anim, remove candidate from request queue, then change cent->xq_torsoAnim to the candidate
	if (replace) {
		// Rearrange the queue if needed - don't leave 2nd spot filled if 1st one is emptied, push 2nd up to 1st
		if (candidate == 1) {
			cent->xq_anim1 = cent->xq_anim2;
			cent->xq_anim1_ts = cent->xq_anim2_ts;
			cent->xq_anim2 = cent->xq_anim2_ts = 0;
		} else {
			cent->xq_anim2 = cent->xq_anim2_ts = 0;
		}

		// Translate xq_anim_t into animNumber_t for xq_torsoAnim
		torso_anim = xq_anim_to_q3(candidate_anim, 1, idle_variety);

		// Go ahead and set new anim
		if ((cent->xq_torsoAnim & ~ANIM_TOGGLEBIT) == torso_anim) {
			if (deb) xq_clog(COLOR_GREEN, "%i Setting same torso anim again: %i (candidate slot: %i)\n",
				cg.time, torso_anim, candidate);
			cent->xq_torsoAnim = ((cent->xq_torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | torso_anim;
		} else {
			if (deb) xq_clog(COLOR_GREEN, "%i Setting new torso anim: %i (candidate slot: %i)\n",
				cg.time, torso_anim, candidate);
			cent->xq_torsoAnim = torso_anim;
		}
	}


	// Run the animation frame
    anim_q3player_runframe(am, &cent->pe.torso, cent->xq_torsoAnim, speedScale);
    *torsoOld = cent->pe.torso.oldFrame;
    *torso = cent->pe.torso.frame;
    *torsoBackLerp = cent->pe.torso.backlerp;

	ret->candidate_anim = candidate_anim;
	ret->replace = replace;
	ret->idle_variety = idle_variety;
	ret->torso_anim = torso_anim;
}
static void anim_q3player(centity_t *cent, int *legsOld, int *legs, float *legsBackLerp, int *torsoOld, int *torso, float *torsoBackLerp, int model_index) {
	xq_animodel_t *am = &xq_animodels[model_index];
	float speedScale = 1; // Can use to speedup anim if needed
	vec3_t m;
	torso_ret tor;
	if (xq_debugNpcDraw.integer) {
		xq_clog(COLOR_WHITE, "%i Entering anim_q3player with xq_torsoAnim at %i",
			cg.time, cent->xq_torsoAnim & ~ANIM_TOGGLEBIT);
	}
	// To disable anims: *legsOld = *legs = *torsoOld = *torso = 0; return;

	// m holds the distance the entity moved between last frame and now
	VectorSubtract(cent->lerpOrigin, cent->xq_previous_origin, m);

	// Decide on leg anim, considering movement since last frame
	// leg_anim_arbiter() and torso_anim_arbiter() set everything in cent and returns the results just for us to use later if needed
	int leg_anim = leg_anim_arbiter(cent, m, legsOld, legs, legsBackLerp, am, speedScale);

	// We have leg anim figured, let's see if we have a server-specified anim for the torso to use instead of idle
	torso_anim_arbiter(cent, torsoOld, torso, torsoBackLerp, am, speedScale, leg_anim, &tor);


	// Maybe play a sound that goes with the torso anim (casting noise, pain argh, etc)
	sound_arbiter(cent, tor.replace ? tor.candidate_anim : -1, leg_anim, m, tor.idle_variety, am);
}

static void draw_npc(centity_t *cent, int corpse, int legs_only) {
	// Draw a composite Q3-style model (head, torso, legs)
	// Simple models are drawn here as well, if legs_only is set

	int deb = xq_debugNpcDraw.integer;
	entityState_t		*es = 			&cent->currentState;
	int 				model_idx = 	es->xq_app_model;
	xq_model_t 			*model = 		&xq_cmodels[model_idx];
	refEntity_t			legs, torso, head;

	if (es->xq_pop_id != cent->xq_last_pop_id) {
		cent->xq_just_popped = 1;
	} else {
		cent->xq_just_popped = 0;
	}


    if (
		model_idx != cent->xq_last_model_index ||
		corpse != cent->xq_last_corpse_status ||
		es->xq_pop_id != cent->xq_last_pop_id
	) {
        cent->xq_last_model_index = model_idx;
        cent->xq_last_corpse_status = corpse;
		cent->xq_last_pop_id = es->xq_pop_id;
        CG_ResetPlayerEntity(cent);
		if (deb) xq_clog(COLOR_RED, "Resetting cent %i", es->number);
		cent->pe.legs.animationTime = cg.time;
		cent->pe.legs.frameTime = cg.time;
		cent->pe.torso.animationTime = cg.time;
		cent->pe.torso.frameTime = cg.time;
    }

	memset(&legs, 0, sizeof(legs));
	memset(&torso, 0, sizeof(torso));
	memset(&head, 0, sizeof(head));

	VectorCopy(cent->lerpOrigin, legs.origin);
	VectorCopy(cent->lerpOrigin, legs.oldorigin);

	if (corpse) {
		if (deb) xq_clog(COLOR_WHITE, "BEFORE legs.oldframe: %i, legs.frame %i, legs.backlerp: %f",
			legs.oldframe, legs.frame, legs.backlerp);
		if (deb) xq_clog(COLOR_WHITE, "BEFORE cent->pe.legs.oldFrame: %i, cent->pe.legs.frame %i, cent->pe.legs.backlerp: %f",
			cent->pe.legs.oldFrame, cent->pe.legs.frame, cent->pe.legs.backlerp);

		// Corpses are handled by vanilla Q3 anim code
		CG_PlayerAnimation(cent, &legs.oldframe, &legs.frame, &legs.backlerp, &torso.oldframe, &torso.frame, &torso.backlerp, model_idx);
	} else {
		// Living mobs are animated by this function
		anim_q3player(cent, &legs.oldframe, &legs.frame, &legs.backlerp, &torso.oldframe, &torso.frame, &torso.backlerp, model_idx);
	}

	if (corpse && deb) {
		xq_clog(COLOR_WHITE, "AFTER legs.oldframe: %i, legs.frame %i, legs.backlerp: %f",
			legs.oldframe, legs.frame, legs.backlerp);
		xq_clog(COLOR_WHITE, "AFTER cent->pe.legs.oldFrame: %i, cent->pe.legs.frame %i, cent->pe.legs.backlerp: %f",
			cent->pe.legs.oldFrame, cent->pe.legs.frame, cent->pe.legs.backlerp);
	}

	legs.hModel = xq_animodel(model_idx)->handle_legs;
	if (!legs_only) {
		torso.hModel = xq_animodel(model_idx)->handle_torso;
		head.hModel = xq_animodel(model_idx)->handle_head;
	}


	if (deb) xq_clog(COLOR_WHITE, "%i %i %i", legs.hModel, torso.hModel, head.hModel);

	int tint_legs = es->xq_app_texture_num_legs >> 8;
	int tint_torso = 0, tint_head = 0;
	if (!legs_only) {
		tint_torso = es->xq_app_texture_num_torso >> 8;
		tint_head = es->xq_app_texture_num_head >> 8;
	}

	if (deb) xq_clog(COLOR_WHITE, "%i %i %i", es->xq_app_texture_num_head, es->xq_app_texture_num_torso, es->xq_app_texture_num_legs);
	if (deb) xq_clog(COLOR_WHITE, "%i %i %i", tint_head, tint_torso, tint_legs);

	legs.customShader = xq_animodel_ModelShader(model->id, XQ_BODY_PART_LEGS, es->xq_app_texture_num_legs, tint_legs)->shader;
	legs.customTint = tint_legs;
	if (!legs_only) {
		torso.customShader = xq_animodel_ModelShader(model->id, XQ_BODY_PART_TORSO, es->xq_app_texture_num_torso, tint_torso)->shader;
		torso.customTint = tint_torso;
		head.customShader = xq_animodel_ModelShader(model->id, XQ_BODY_PART_HEAD, es->xq_app_texture_num_head, tint_head)->shader;
		head.customTint = tint_head;
	}

	// Some models allow for setting different shaders for different body parts (all playable races at least)
	// Some others do not - mostly NPC-only models. These have just full body "skins".
	if (model->full_armor) {
		legs.feetShader = xq_animodel_ModelShader(model->id, XQ_BODY_PART_FEET, es->xq_app_texture_num_feet, tint_legs)->shader;
		if (!legs_only) {
			torso.armsShader = xq_animodel_ModelShader(model->id, XQ_BODY_PART_ARMS, es->xq_app_texture_num_arms, tint_torso)->shader;
			torso.leftwristShader = xq_animodel_ModelShader(model->id, XQ_BODY_PART_LEFTWRIST, es->xq_app_texture_num_leftwrist, tint_torso)->shader;
			torso.rightwristShader = xq_animodel_ModelShader(model->id, XQ_BODY_PART_RIGHTWRIST, es->xq_app_texture_num_rightwrist, tint_torso)->shader;
			torso.handsShader = xq_animodel_ModelShader(model->id, XQ_BODY_PART_HANDS, es->xq_app_texture_num_hands, tint_torso)->shader;
		}
	} else {
		legs.feetShader = legs.customShader;
		if (!legs_only) {
			torso.armsShader =
			torso.leftwristShader =
			torso.rightwristShader =
			torso.handsShader = torso.customShader;
		}
	}


	vec3_t ang = {0, 0, 0};
    ang[0] = es->xq_pitch;
    ang[1] = es->xq_heading;
    ang[2] = es->xq_roll;
    VectorCopy(ang, cent->lerpAngles);
    AnglesToAxis(ang, legs.axis);
    ang[0] = 0;
    ang[1] = 0;
    ang[2] = 0;
	if (!legs_only) {
		AnglesToAxis(ang, torso.axis);
		AnglesToAxis(ang, head.axis);
	}


	float scale = 1.0f;
	if (es->xq_app_model_scale != 100) {
		scale = (1.0 / 100.0) * es->xq_app_model_scale;
	}
	for (int i = 0;  i <= 2;  i++) {
		VectorScale(legs.axis[i], scale, legs.axis[i]);
	}
	legs.nonNormalizedAxes = qtrue;
	if (!legs_only) {
		torso.nonNormalizedAxes = qtrue;
		head.nonNormalizedAxes = qtrue;
	}

	CG_AddRefEntityWithPowerups(&legs, &cent->currentState, TEAM_FREE);

	CG_PositionRotatedEntityOnTag(&torso, &legs, legs.hModel, "tag_torso");
	CG_AddRefEntityWithPowerups(&torso, &cent->currentState, TEAM_FREE);
	xq_particle_arbiter(cent, legs_only ? &legs : &torso);

	if (!legs_only) {
		CG_PositionRotatedEntityOnTag(&head, &torso, torso.hModel, "tag_head");
	    CG_AddRefEntityWithPowerups(&head, &cent->currentState, TEAM_FREE);
	}



	if (es->xq_app_held_primary_model > 0) {
		// draw melee weapon or held item
		xq_draw_held(cent, legs_only ? &legs : &torso, NULL, es->xq_app_held_primary_model, 1, 0);
	} else {
		// add the gun / barrel / flash
		CG_AddPlayerWeapon(legs_only ? &legs : &torso, NULL, cent, -(es->xq_app_held_primary_model), 0);
	}

	xq_draw_held(cent, legs_only ? &legs : &torso, NULL, es->xq_app_held_secondary_model, 2, 0);


	xq_drawbbox(cent);
	xq_name_plate(cent, legs_only ? &legs : &head);
}

void xq_npc_draw(centity_t *cent) {
	entityState_t 	*es = &cent->currentState;
	xq_model_t 		*model = &xq_cmodels[es->xq_app_model];
	int				centnum = es->number;

	if (xq_debugNpcDraw.integer) {
		xq_clog(COLOR_WHITE, "npc: %s, xq_app_model: %i", xq_getname(cent, 1), es->xq_app_model);
	}
	switch (model->type) {
		case XQ_MODEL_TYPE_SIMPLE:
			draw_npc(cent, 0, 1);
			VectorCopy(cent->lerpOrigin, cent->xq_previous_origin);
			break;
		case XQ_MODEL_TYPE_Q3PLAYER:
			if (es->eType == ET_XQ_MOB) {
				// Live NPC
				draw_npc(cent, 0, 0);
				VectorCopy(cent->lerpOrigin, cent->xq_previous_origin);
			} else {
				// PC or NPC corpse
				draw_npc(cent, 1, 0);
			}
			break;
		default:
			xq_clog(COLOR_RED, "xq_npc_draw(): Unknown model type %i for cent %s#%i", model->type, xq_getname(cent, 1), centnum);
			break;
	}
}

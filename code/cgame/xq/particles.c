#include "../cg_local.h"
/*	DESCRIPTION

	Display spell particle effects.
	The draw() function is a kludge wrapper the Q3 particle effects,
	and should be redone properly.

	The whole thing probably needs a rewrite.

*/

static void draw(centity_t *cent, refEntity_t *torso, xq_particle_status_t *p) {
	char *tag = (p->type == 1 ? "tag_weapon" : "tag_torso");

	orientation_t lerped = {0};
	trap_R_LerpTag(&lerped, torso->hModel, torso->oldframe, torso->frame, 1.0 - torso->backlerp, tag);

	vec3_t particle_origin;
    VectorCopy(torso->origin, particle_origin);
    VectorMA(particle_origin, lerped.origin[0], torso->axis[0], particle_origin);

	if (p->type == 2) particle_origin[2] += 30;
	vec3_t vel = {0};
	switch (p->pfx) {
		case 0: break; // just a sound requested, no pfx
		case 1:
		default:
			vel[0] = vel[1] = 0;
			vel[2] = -20;
			for (int i = 1;  i <= 10;  i++) {
				CG_ParticleSparks(particle_origin, vel, 1000, 5, 5, 0); //duration, x, y, speed
			}
			break;
		case 2:
			CG_ParticleBloodCloud(cent, particle_origin, vel);
			break;
		case 3:
			CG_ParticleDust(cent, particle_origin, vel);
			break;
	}
}
static void addFx(centity_t *cent, int anim, int type, int sound) {
	int i;
	entityState_t *es = &cent->currentState;
	int centnum = es->number;
	xq_particle_status_t *p;
	int present = 0;

	// See if we already have that pfx being played
	// Refresh end_ts if so and it's an author request
	// (ie: we don't know when it will finish casting)
	for (i = 0;  i < XQ_PFX_PER_BEING;  i++) {
		p = &xqst->pfx[centnum][i];
		if (p->type == type && p->pfx == anim && p->sound == sound && p->casting_hash == es->xq_casting_hash) {
			if (p->type == 1) {
				// refresh existing author request
				p->end_ts = xq_msec();
			}
			present = 1; // sounds get overwritten, eg: touch a lot of pods in a row
			break;
		}
	}


	if (!present) {
		//xq_clog(COLOR_YELLOW, "%i addFx: pfx %i type %i for cent %s#%i", cg.time, anim, type, xq_getname(cent, 0), centnum);
		// We need to add the requested pfx to the list.
		// Overwrite the one with oldest end_ts.
		// Note that author pfx adding are always requested
		// last so they will never get really overwritten.
		int oldest_end_ts_pos = -1;
		int oldest_end_ts = xq_msec();

		for (i = 0;  i < XQ_PFX_PER_BEING;  i++) {
			p = &xqst->pfx[centnum][i];
			if (p->end_ts < oldest_end_ts) {
				oldest_end_ts_pos = i;
				oldest_end_ts = p->end_ts;
			}
			if (oldest_end_ts == 0) {
				break;
			}
		}
		if (oldest_end_ts_pos == -1) {
			// For some reason we couldn't find a suitable position to add our anim to
			xq_clog(COLOR_YELLOW, "addFx: Failed to find a slot for pfx %i type %i for cent %s#%i",
				anim, type, xq_getname(cent, 0), centnum);
		} else {
			p = &xqst->pfx[centnum][oldest_end_ts_pos];
			p->type = type;
			p->casting_hash = es->xq_casting_hash;
			p->end_ts = xq_msec();	// we set end_ts to now as we don't know when it's going to actually stop casting.
									// The effect will still play for a little after end_ts.
			p->pfx = anim;
			p->sound = sound;
			if (p->sound) {
				char path[MAX_QPATH];
				snprintf(path, MAX_QPATH, "sound/spells/cast/%i.wav", p->sound);
				sfxHandle_t h = trap_S_RegisterSound(path, qfalse);
				//trap_S_AddLoopingSound(centnum, cent->lerpOrigin, cent->lerpOrigin, h);
				trap_S_StartSound(NULL, centnum, CHAN_AUTO, h);
			}
		}
	}
}
void xq_particle_arbiter(centity_t *cent, refEntity_t *torso) {
	entityState_t *es = &cent->currentState;
	int centnum = es->number;

#if 0
	if (es->xq_pfx_received || es->xq_pfx_received_sound) {
		xq_clog(COLOR_WHITE, "cent %s#%i xq_pfx_received %i, xq_pfx_received_sound %i",
			xq_getname(cent, 0), es->number, es->xq_pfx_received, es->xq_pfx_received_sound);
	}
#endif



	// See if we are casting something right now that we need to add
	if (es->xq_casting || es->xq_casting_sound) {
		addFx(cent, es->xq_casting, 1, es->xq_casting_sound);
	}



	for (int i = 0;  i < XQ_PFX_PER_BEING;  i++) {
		xq_particle_status_t *p = &xqst->pfx[centnum][i];

		if (p->type) {
			if ((xq_msec() - p->end_ts) < 2000) { // We keep playing it for 2 seconds after it's no longer actually being requested
				draw(cent, torso, p);
				//xq_clog(COLOR_WHITE, "%i cent %s#%i drawing type %i, pfx %i", cg.time, xq_getname(cent, 0), es->number, p->type, p->pfx);
			} else {
				if (p->type == 1) {
					// XXX need to find a way to stop the casting sound. None of the below works.
					//trap_S_StopLoopingSound(centnum);
					//trap_S_StartSound(NULL, centnum, CHAN_ANNOUNCER, cgs.media.silenceSound);
					//xq_clog(COLOR_RED, "%i STOP sound entnum %i", cg.time, centnum);
				}
				memset(p, 0, sizeof(xq_particle_status_t));
			}
		}
	}
}
void xq_pfx_spell_hit(centity_t *cent) {
	entityState_t *es = &cent->currentState;
	int sound = (es->eventParm >> 8) & 0xff;
	int pfx = es->eventParm & 0xff;

	// add the pfx
	if (pfx) {
		addFx(cent, pfx, 2, 0);
	}

	// start the sound here and forget it
	if (sound) {
		sfxHandle_t h = trap_S_RegisterSound(va("sound/spells/hit/%i.wav", sound), qfalse); 
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, h);
	}
}

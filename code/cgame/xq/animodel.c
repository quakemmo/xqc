#include "../cg_local.h"

static qboolean parse_animation_file(int midx) {
	#define MAX_TEXT 20000
	if (xq_debugAnimodel.integer) {
		xq_clog(COLOR_WHITE, "parse_animation_file(%i): entering", midx);
	}
	xq_model_t *model = &xq_cmodels[midx];
	char path[MAX_QPATH*2+1] = {0};
	snprintf(path, MAX_QPATH*2, "%s/%s/animation.cfg", XQ_APP_PATH, model->path);

	if (xq_debugAnimodel.integer) {
		xq_clog(COLOR_WHITE, "parse_animation_file(%i): going to read %s file", midx, path);
	}

	// load the file
	fileHandle_t f;
	int len = trap_FS_FOpenFile(path, &f, FS_READ);
	if ( len <= 0 ) {
		return qfalse;
	}

	char text[MAX_TEXT+1] = {0};
	if (len >= sizeof(text) - 1) {
		xq_clog(COLOR_RED, "parse_animation_file(%i): animation.cfg file %s too long", midx, path);
		trap_FS_FCloseFile(f);
		return qfalse;
	}
	trap_FS_Read(text, len, f);
	text[len] = 0;
	trap_FS_FCloseFile(f);
	if (xq_debugAnimodel.integer) {
		xq_clog(COLOR_WHITE, "parse_animation_file(%i): read %i bytes of text", midx, len);
	}

	// parse the text
	char *text_p = text;
	xq_animodel_t *am = &xq_animodels[midx];

	am->footsteps = FOOTSTEP_NORMAL;
	am->gender = GENDER_MALE;
	am->fixedlegs = qfalse;
	am->fixedtorso = qfalse;

	// read optional parameters
	char *token;
	while (1) {
		char *prev = text_p;	// so we can unget
		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp( token, "footsteps" ) ) {
			token = COM_Parse( &text_p );
			if ( !token[0] ) {
				break;
			}
			if ( !Q_stricmp( token, "default" ) || !Q_stricmp( token, "normal" ) ) {
				am->footsteps = FOOTSTEP_NORMAL;
			} else if ( !Q_stricmp( token, "boot" ) ) {
				am->footsteps = FOOTSTEP_BOOT;
			} else if ( !Q_stricmp( token, "flesh" ) ) {
				am->footsteps = FOOTSTEP_FLESH;
			} else if ( !Q_stricmp( token, "mech" ) ) {
				am->footsteps = FOOTSTEP_MECH;
			} else if ( !Q_stricmp( token, "energy" ) ) {
				am->footsteps = FOOTSTEP_ENERGY;
			} else {
				CG_Printf("parse_animation_file(%i): Bad footsteps parm in %s: %s\n", midx, path, token);
			}
			if (xq_debugAnimodel.integer) {
				xq_clog(COLOR_WHITE,
					"parse_animation_file(%i): set footsteps to %i", midx, am->footsteps);
			}
			continue;
		} else if ( !Q_stricmp( token, "sex" ) ) {
			token = COM_Parse( &text_p );
			if ( !token[0] ) {
				break;
			}
			if ( token[0] == 'f' || token[0] == 'F' ) {
				am->gender = GENDER_FEMALE;
			} else if ( token[0] == 'n' || token[0] == 'N' ) {
				am->gender = GENDER_NEUTER;
			} else {
				am->gender = GENDER_MALE;
			}
			if (xq_debugAnimodel.integer) {
				xq_clog(COLOR_WHITE,
					"parse_animation_file(%i): set gender to %i", midx, am->gender);
			}
			continue;
		} else if ( !Q_stricmp( token, "fixedlegs" ) ) {
			am->fixedlegs = qtrue;
			if (xq_debugAnimodel.integer) {
				xq_clog(COLOR_WHITE,
					"parse_animation_file(%i): set fixedlegs to %i", midx, am->fixedlegs);
			}
			continue;
		} else if ( !Q_stricmp( token, "fixedtorso" ) ) {
			am->fixedtorso = qtrue;
			if (xq_debugAnimodel.integer) {
				xq_clog(COLOR_WHITE,
					"parse_animation_file(%i): set fixedtorso to %i", midx, am->fixedtorso);
			}
			continue;
		}

		// if it is a number, start parsing animations
		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}
		Com_Printf("parse_animation_file(%i): unknown token '%s' in %s\n", midx, token, path);
	}

	// read information for each frame
	animation_t *an = am->animations;
	int skip, i;
	for (i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			if( i >= TORSO_GETFLAG && i <= TORSO_NEGATIVE ) {
				an[i].firstFrame = an[TORSO_GESTURE].firstFrame;
				an[i].frameLerp = an[TORSO_GESTURE].frameLerp;
				an[i].initialLerp = an[TORSO_GESTURE].initialLerp;
				an[i].loopFrames = an[TORSO_GESTURE].loopFrames;
				an[i].numFrames = an[TORSO_GESTURE].numFrames;
				an[i].reversed = qfalse;
				an[i].flipflop = qfalse;
				continue;
			}
			break;
		}

		an[i].firstFrame = atoi( token );
		if (xq_debugAnimodel.integer) {
			xq_clog(COLOR_WHITE,
				"parse_animation_file(%i): anim %i: firstFrame %i", midx, i, an[i].firstFrame);
		}


		// leg only frames are adjusted to not count the upper body only frames
		if ( i == LEGS_WALKCR ) {
			skip = an[LEGS_WALKCR].firstFrame - an[TORSO_GESTURE].firstFrame;
		}
		if ( i >= LEGS_WALKCR && i<TORSO_GETFLAG) {
			an[i].firstFrame -= skip;
		}



		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		an[i].numFrames = atoi( token );

		an[i].reversed = qfalse;
		an[i].flipflop = qfalse;

		// if numFrames is negative the animation is reversed
		if (an[i].numFrames < 0) {
			an[i].numFrames = -an[i].numFrames;
			an[i].reversed = qtrue;
		}




		if (xq_debugAnimodel.integer) {
			xq_clog(COLOR_WHITE,
				"parse_animation_file(%i): anim %i: numFrames %i, reversed: %i, flipflop: %i",
					midx, i, an[i].numFrames, an[i].reversed, an[i].flipflop);
		}

		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		an[i].loopFrames = atoi( token );
		if (xq_debugAnimodel.integer) {
			xq_clog(COLOR_WHITE,
				"parse_animation_file(%i): anim %i: loopFrames %i",
					midx, i, an[i].loopFrames);
		}




		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		float fps = atof( token );
		if ( fps == 0 ) {
			fps = 1;
		}
		an[i].frameLerp = 1000 / fps;
		an[i].initialLerp = 1000 / fps;

		if (xq_debugAnimodel.integer) {
			xq_clog(COLOR_WHITE,
				"parse_animation_file(%i): anim %i: fps %f, frameLerp %i, initialLerp %i",
					midx, i, fps, an[i].frameLerp, an[i].initialLerp);
		}
	}

	if ( i != MAX_ANIMATIONS ) {
		CG_Printf("parse_animation_file(%i): Error parsing animation file: %s\n", midx, path);
		return qfalse;
	}

	// crouch backward animation
	memcpy(&an[LEGS_BACKCR], &an[LEGS_WALKCR], sizeof(animation_t));
	an[LEGS_BACKCR].reversed = qtrue;
	// walk backward animation
	memcpy(&an[LEGS_BACKWALK], &an[LEGS_WALK], sizeof(animation_t));
	an[LEGS_BACKWALK].reversed = qtrue;
	// flag moving fast
	an[FLAG_RUN].firstFrame = 0;
	an[FLAG_RUN].numFrames = 16;
	an[FLAG_RUN].loopFrames = 16;
	an[FLAG_RUN].frameLerp = 1000 / 15;
	an[FLAG_RUN].initialLerp = 1000 / 15;
	an[FLAG_RUN].reversed = qfalse;
	// flag not moving or moving slowly
	an[FLAG_STAND].firstFrame = 16;
	an[FLAG_STAND].numFrames = 5;
	an[FLAG_STAND].loopFrames = 0;
	an[FLAG_STAND].frameLerp = 1000 / 20;
	an[FLAG_STAND].initialLerp = 1000 / 20;
	an[FLAG_STAND].reversed = qfalse;
	// flag speeding up
	an[FLAG_STAND2RUN].firstFrame = 16;
	an[FLAG_STAND2RUN].numFrames = 5;
	an[FLAG_STAND2RUN].loopFrames = 1;
	an[FLAG_STAND2RUN].frameLerp = 1000 / 15;
	an[FLAG_STAND2RUN].initialLerp = 1000 / 15;
	an[FLAG_STAND2RUN].reversed = qtrue;
	//
	// new anims changes
	//
	//	an[TORSO_GETFLAG].flipflop = qtrue;
	//	an[TORSO_GUARDBASE].flipflop = qtrue;
	//	an[TORSO_PATROL].flipflop = qtrue;
	//	an[TORSO_AFFIRMATIVE].flipflop = qtrue;
	//	an[TORSO_NEGATIVE].flipflop = qtrue;
	//
	return qtrue;
}

char *cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
    "*death.wav",
    "*jump.wav",
    "*pain25.wav",
    "*pain50.wav",
    "*pain75.wav",
    "*pain100.wav",
    "*melee.wav",
    "*altidle.wav",
    "*gasp.wav",
    "*drown.wav",
    "*fall.wav",
    "*casted.wav"
};
xq_shader_t *xq_animodel_ModelShader(int midx, xq_body_part_t body_part, int texture_num, int tint) {

	// This keeps a cache of shaders for animodel body parts and registers new ones 
	// on the fly as needed.  Different tints = different shaders.


	texture_num = texture_num & 0xff;
	xq_shader_t *cache = xq_modelshader_cache;

	if (midx <= 0 || midx >= XQ_MAX_MODELS) {
		// model index out of bounds, return something valid though
		return &cache[0];
	}

	// If the requested model doesn't have enough textures (ie: model has only 4 texture sets but we're asked for texture #5),
	// return the 0's texture (should be the naked-ish texture).
	xq_model_t *model = &xq_cmodels[midx];
	if (model->numskins <= texture_num) {
		texture_num = 0;
	}

	if (body_part == XQ_BODY_PART_NONE) return &cache[0]; // Return something valid even for invalid calls

	// See if we have a shader already known for the given params
	int i, need_create = 0;
	for (i = 0;  i < (MAX_REFENTITIES - 1);  i++) {
		xq_shader_t *sh = &cache[i];
		if (sh->body_part == XQ_BODY_PART_NONE) {
			// We reached the end of cache elements without hitting anything
			need_create = 1;
			break;
		} else if (sh->model_id == midx && sh->body_part == body_part && sh->texture_num == texture_num && sh->tint == tint) {
			// We found a cache entry
			sh->last_used_ts = cg.time;
			return sh;
		}
	}


	// We don't - decide on a cache spot to store it.
	int new_idx;
	if (need_create == 0) {
		// Cache is full, need to replace the least recently used entry
		uint32_t lru = cg.time + 1;
		int lru_idx = -1;
		for (i = 0;  i < (MAX_REFENTITIES - 1);  i++) {
			if (cache[i].last_used_ts < lru) {
				lru = cache[i].last_used_ts;
				lru_idx = i;
			}
		}
		new_idx = lru_idx;
		if (xq_debugModelShader.integer) {
			xq_clog(COLOR_WHITE, "Replacing shader cache LRU entry %i", new_idx);
		}
	} else {
		// Cache has room for the new element
		new_idx = i;
		if (xq_debugModelShader.integer) {
			xq_clog(COLOR_WHITE, "Creating new shader cache entry %i", new_idx);
		}
	}


	// By now we have a spot for the new shader in cache[new_idx].
	// Figure out the shader path to register.
	char *body_part_lit;
	switch (body_part) {
		case XQ_BODY_PART_LEGS:			body_part_lit = "legs";			break;
		case XQ_BODY_PART_TORSO:		body_part_lit = "torso";		break;
		case XQ_BODY_PART_HEAD:			body_part_lit = "head";			break;
		case XQ_BODY_PART_FEET:			body_part_lit = "feet";			break;
		case XQ_BODY_PART_ARMS:			body_part_lit = "arms";			break;
		case XQ_BODY_PART_LEFTWRIST:	body_part_lit = "leftwrist";	break;
		case XQ_BODY_PART_RIGHTWRIST:	body_part_lit = "rightwrist";	break;
		case XQ_BODY_PART_HANDS:		body_part_lit = "hands";		break;
		default:						body_part_lit = "legs";			break;
	}


	// Create new cache entry and return it
	char shaderpath[MAX_QPATH+1] = {0};
	snprintf(shaderpath, MAX_QPATH, "app/%s/%s/%i#%i", model->path, body_part_lit, texture_num, tint);
    cache[new_idx].model_id = midx;
    cache[new_idx].body_part = body_part;
    cache[new_idx].texture_num = texture_num;
    cache[new_idx].tint = tint;
    cache[new_idx].shader = trap_R_RegisterShader(shaderpath);
    cache[new_idx].last_used_ts = cg.time;
	if (xq_debugModelShader.integer) {
		xq_clog(COLOR_WHITE, "New shader cache entry shaderpath %s, handle: %i", shaderpath, cache[new_idx].shader);
	}

	return &cache[new_idx];
}
xq_animodel_t *xq_animodel(int midx) {
	static xq_animodel_t default_animodel = {0};
	static int init_done = 0;
	if (!init_done) {
		init_done = 1;
		default_animodel.magics = default_animodel.magice = xq_cgame_animodel_t_MAGIC;
		default_animodel.registered = 1;
	}

	xq_model_t *model = &xq_cmodels[midx];
	if (!model->path[0]) {
		// we might be asked for an empty path if the server hasn't sent the model list yet
		// this should draw the default nullmodel
		return &default_animodel;
	}


	// Register the simple or compound model
	xq_animodel_t *am = &xq_animodels[midx];
	if (am->registered == 0) {
		char *p = model->path;
		if (xq_debugAnimodel.integer) {
			xq_clog(COLOR_WHITE, "Registering animodel %i (%s)", midx, p);
		}
		
		memset(am, 0, sizeof(xq_animodel_t));
		am->magics = am->magice = xq_cgame_animodel_t_MAGIC;

		am->handle_legs = trap_R_RegisterModel(va(XQ_APP_PATH"/%s/lower.md3", p));
		am->model_index = midx;
		if (xq_debugAnimodel.integer) {
			xq_clog(COLOR_WHITE, "xq_animodel: Registered legs model #%i (%s), handle %i", midx, p, am->handle_legs);
		}

		if (model->type == XQ_MODEL_TYPE_Q3PLAYER) {
			am->handle_torso = trap_R_RegisterModel(va(XQ_APP_PATH"/%s/upper.md3", p));
			if (xq_debugAnimodel.integer) {
				xq_clog(COLOR_WHITE, "xq_animodel: Registered torso model #%i (%s), handle %i", midx, p, am->handle_torso);
			}
			am->handle_head = trap_R_RegisterModel(va(XQ_APP_PATH"/%s/head.md3", p));
			if (xq_debugAnimodel.integer) {
				xq_clog(COLOR_WHITE, "xq_animodel: Registered head model #%i (%s), handle %i", midx, p, am->handle_head);
			}

		}
		parse_animation_file(midx);


		// Add the default or custom sounds
		char path[MAX_QPATH+1] = {0};
		for (int i = 0;  i < MAX_CUSTOM_SOUNDS;  i++) {
			char *s = cg_customSoundNames[i];
			if (!s) {
				break;
			}
			// if the model doesn't have a specific sound, use default sound
			snprintf(path, MAX_QPATH, XQ_APP_PATH"/%s/sound/%s", p, s + 1);
			if (CG_FileExists(path)) {
				am->sounds[i] = trap_S_RegisterSound(path, qfalse);
				if (xq_debugAnimodel.integer) {
					xq_clog(COLOR_WHITE, "xq_Animodel(%i): custom sound %s registered for model %s", midx, s + 1, p);
				}
			} else {
				if (xq_debugAnimodel.integer) {
					xq_clog(COLOR_WHITE, "xq_Animodel(%i): custom sound %s file not found for model %s", midx, path, p);
				}
				snprintf(path, MAX_QPATH, "sound/app/default/%s",s + 1);
				if (CG_FileExists(path)) {
					// There can be no default sound - ie: no default altidle sound
					snprintf(path, MAX_QPATH, "sound/app/default/%s", s + 1);
					am->sounds[i] = trap_S_RegisterSound(path, qfalse);
					if (xq_debugAnimodel.integer) {
						xq_clog(COLOR_WHITE, "xq_Animodel(%i): fallback sound %s registered for model %s", midx, s + 1, p);
					}
				}
			}
        }


		// Flag the model as successfully registered
		am->registered = 1;
    }
	return am;
}

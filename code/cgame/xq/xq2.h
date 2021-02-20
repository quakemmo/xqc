
typedef struct xq_animodel_s {
    #define         xq_cgame_animodel_t_MAGIC          0xb4b9efd3

	int			magics;
	int			model_index;
    int         registered;
    qhandle_t   handle_head;
    qhandle_t   handle_torso;
    qhandle_t   handle_legs;
    animation_t animations[MAX_TOTALANIMATIONS];
    sfxHandle_t	sounds[MAX_CUSTOM_SOUNDS];
    footstep_t  footsteps;
    qboolean    fixedlegs;
    qboolean    fixedtorso;
    gender_t    gender;
	int			magice;
} xq_animodel_t;


extern xq_animodel_t *xq_animodels;



xq_shader_t *xq_animodel_ModelShader(int model_index, xq_body_part_t body_part, int texture_num, int tint);
xq_animodel_t *xq_animodel(int model_index);

// drawbbox.c
void xq_drawbbox(centity_t *ent);

// nameplate.c
void xq_name_plate(centity_t *cent, refEntity_t *refent);

// cg_players.c
void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
                        int *torsoOld, int *torso, float *torsoBackLerp, int model_index);


// 3d.c
void xq_dump_anim(centity_t *cent, xq_animodel_t *am, int after);
void xq_ScaleModel(float scale, refEntity_t *legs, refEntity_t *torso, refEntity_t *head);

// npc.c
void xq_draw_held(centity_t *cent, refEntity_t *parent, playerState_t *ps, int item_model, int hand, int flags);
void xq_npc_draw(centity_t *cent);

// particles.c
void xq_particle_arbiter(centity_t *cent, refEntity_t *torso);
void xq_pfx_spell_hit(centity_t *cent);

// ui/setvals.c
void xqui_SetVals(centity_t *cent, centity_t *tcent, playerState_t *ps);

// getname.c
char *xq_getname(centity_t *cent, int targetname);

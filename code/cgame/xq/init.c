#ifndef _WIN32
#   include <sys/resource.h>
#   include <errno.h>
#endif

#include "../cg_local.h"

xq_model_t           	*xq_cmodels;
xq_animodel_t           *xq_animodels;
xq_shader_t				*xq_modelshader_cache;
xq_cgame_state_t		*xqst;
qw_state_t				*qws;
xq_cmdCookie_t			*xq_CmdCookies;


void xq_Deinit(void) {
	free(xqst);
	free(qws);
	free(xq_modelshader_cache);

	xq_Info_Deinit();

	free(xq_animodels);
	free(xq_cmodels);
	free(xq_CmdCookies);
}
void xq_Init(void) {

	int i;
	xq_chat_init();
	xq_debdisp_init();

	#ifndef _WIN32
		// We want to dump cores on Unix.
		struct rlimit rlim;
		rlim.rlim_cur = RLIM_INFINITY;
		rlim.rlim_max = RLIM_INFINITY;
		if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
			Com_Printf("Failed setting RLIMIT_CORE to infinity: %s\n", strerror(errno));
			exit(0);
		}
		getrlimit(RLIMIT_CORE, &rlim);
		Com_Printf("Max core file size. Current: %"PRId64", max: %"PRId64"\n",
			rlim.rlim_cur, rlim.rlim_max);
		if (rlim.rlim_cur != RLIM_INFINITY || rlim.rlim_max != RLIM_INFINITY) {
			Com_Printf("RLIMIT_CORE is not infinite!: %s\n", strerror(errno));
			exit(0);
		}
		Com_Printf("RLIMIT_CORE set to infinity.\n");
	#endif


	// Init and clear some basic structs
	//	xqst
	xqst = calloc(1, sizeof(xq_cgame_state_t));
	xqst->magics = xqst->magice = xq_cgame_state_t_MAGIC;

	//  xq_modelshader_cache
	xq_modelshader_cache = calloc(MAX_REFENTITIES, sizeof(xq_shader_t));

	//  cache of CmdCookies - some item info will not be processed unless
	//  their ->cmdCookie is in that cache.
	xq_CmdCookies = calloc(XQ_CMDCOOKIES_MAX, sizeof(xq_cmdCookie_t));

	//  qws - global qwin state structure
	qws = calloc(1, sizeof(qw_state_t));
	qws->magics = qws->magice = qw_t_MAGIC;

	//  xq_animodels
	xq_animodels = calloc(XQ_MAX_MODELS, sizeof(xq_animodel_t));
	for (i = 0;  i < XQ_MAX_MODELS;  i++) {
		xq_animodels[i].magics = xq_animodels[i].magice = xq_cgame_animodel_t_MAGIC;
	}

	//  xq_cmodels
	xq_cmodels = calloc(XQ_MAX_MODELS, sizeof(xq_model_t));

	xq_Info_Init();



	xq_SetTarget(0, 0);
	xqui_Init();


	// Reset the camera view to first person mode
	xq_camera_CycleView(qtrue);
}
void xq_MediaInit(void) {

	char **utilmodels = (char *[]){
					"ground/box", 						"models/ground/box.md3",
					"ground/health", 					"models/ground/health.md3",
					"ground/arena/health_5", 			"models/ground/arena/health_5_cross.md3",
					"ground/arena/health_5_second", 	"models/ground/arena/health_5_sphere.md3",
					"ground/arena/health_25", 			"models/ground/arena/health_25_cross.md3",
					"ground/arena/health_25_second",	"models/ground/arena/health_25_sphere.md3",
					"ground/arena/health_50", 			"models/ground/arena/health_50_cross.md3",
					"ground/arena/health_50_second",	"models/ground/arena/health_50_sphere.md3",
					"ground/arena/health_100", 			"models/ground/arena/health_100_cross.md3",
					"ground/arena/health_100_second",	"models/ground/arena/health_100_sphere.md3",
					"ground/arena/energy_100", 			"models/ground/arena/energy_100.md3",
					"ground/arena/endurance_100", 		"models/ground/arena/endurance_100.md3",
					"ground/mana", 						"models/ground/mana.md3",
					"ground/spell", 					"models/ground/spell.md3",
					"ground/damage", 					"models/ground/damage.md3",
					"ground/energize", 					"models/ground/energize.md3",
					"ground/coinspell", 				"models/ground/coinspell.md3"
				};
	int utilmodels_count = 17;
	int i;
	qhandle_t qh;
	for (i = 0;  i < utilmodels_count * 2;  i += 2) {
		if (xqst->utilmodels_registered >= XQ_MAX_UTILMODELS) break;
		qh = trap_R_RegisterModel(utilmodels[i+1]);
		if (qh > 0) {
			xqst->utilmodels[xqst->utilmodels_registered].qhandle = qh;
			Q_strncpyz((char *)&xqst->utilmodels[xqst->utilmodels_registered++].name, utilmodels[i], MAX_QPATH);
		} else {
			xq_clog(COLOR_RED, "xq_MediaInit(): Couldn't reg utilmodel name [%s], path [%s]",
				utilmodels[i], utilmodels[i+1]);
		}
	}


	int itemmodels_count = 6;
	for (i = 0;  i < itemmodels_count;  i++) {
		if (xqst->item_models_registered >= XQ_MAX_ITEM_MODELS) break;
		qh = trap_R_RegisterModel(va("models/items/%i.md3", i+1));
		if (qh > 0) {
			xqst->item_models[xqst->item_models_registered++] = qh;
		} else {
			xq_clog(COLOR_RED, "xq_MediaInit(): Couldn't reg item_model number [%i]", i);
		}
	}
}

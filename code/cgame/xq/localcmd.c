#include "../cg_local.h"
/* 	DESCRIPTION
	CLI commands that are ran locally (as opposed to transmitted to the server)
*/

int xq_localcmd(int64_t arg) {
	char *cmd = (char *)arg;
	int argc = 0;
	char argv[10][100];

	const char *token;
   
	token = strtok(cmd, " ");
	while (token != NULL && argc < 10) {
		if (token != NULL) {
			Q_strncpyz(argv[argc++], token, 100);
		}
		token = strtok(NULL, " ");
	}

	argc--;

	cmd = argv[0];


	if (xq_seq(cmd, "/test")) {
		xq_clog(COLOR_WHITE, "TEST!");
		return qtrue;
	} else if (xq_seq(cmd, "/ui_wininfo")) {
		qw_WinInfo(argv[1]);
	} else if (xq_seq(cmd, "/testinfo")) {
		xq_info_t *info = xq_InfoInfo(XQ_INFO_MODELS, 0, 0, 0);
		if (info) {
			xq_model_t *mods = (xq_model_t *)(info->data);
			for (int i = 0;  i < XQ_MAX_MODELS;  i++) {
				if (mods[i].default_scale == 0) break;
				xq_clog(COLOR_WHITE, "Got model: %i / %i / %i / %s / %i / %i / %i",
					mods[i].id, mods[i].type, mods[i].default_scale, mods[i].path, mods[i].numskins, mods[i].numfaces, mods[i].crawler);
			}
		}
		return qtrue;
	} else if (xq_seq(cmd, "/cacheinfo") || xq_seq(cmd, "/ci")) {
		if (argc == 0) {
				xq_localcmd_cacheinfo(0);
		} else if (xq_seq(argv[1], "item") || xq_seq(argv[1], "i")) {
				xq_localcmd_cacheinfo(1);
		} else if (xq_seq(argv[1], "spell") || xq_seq(argv[1], "s")) {
				xq_localcmd_cacheinfo(2);
		} else if (xq_seq(argv[1], "info") || xq_seq(argv[1], "in")) {
				xq_localcmd_cacheinfo(3);
		} else {
			xq_localcmd_cacheinfo(100);
		}
	} else if (xq_seq(cmd, "/cacheclear")) {

		xq_clog(COLOR_WHITE, "argc: %i", argc);
		xq_localcmd_cacheclear(argc, argv[1], argv[2], argv[3], argv[4], argv[5]);
	}

	return qfalse;
}

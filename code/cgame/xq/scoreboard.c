/*	DESCRIPTION

	Display the arena scoreboard.
*/

#include "../cg_local.h"

typedef struct xq_score_info_s {
	int client;
	int ping;
	int kills;
	int deaths;
	int time;
} xq_score_info_t;

static void drawScore(int y, xq_score_info_t *score, float *color, int x_kd, int x_ping, int x_time, int x_name) {
	if (score->client < 0 || score->client >= cgs.maxclients) {
		Com_Printf("Bad score->client: %i\n", score->client);
		return;
	}

	centity_t *cent = &cg_entities[score->client];

	// highlight your position
	if ( score->client == cg.snap->ps.clientNum ) {
		vec4_t col_highlight = { 0, 0, 0.3, 1 };
		CG_FillRect( x_kd, y, x_name, BIGCHAR_HEIGHT+1, col_highlight);
	}

	char string[20];
	Com_sprintf(string, sizeof(string), "^2%i/^1%i", score->kills, score->deaths);
	CG_DrawBigString(x_kd, y, string, 0.5);

	Com_sprintf(string, sizeof(string), "%i", score->ping);
	CG_DrawBigString(x_ping, y, string, 0.5);

	Com_sprintf(string, sizeof(string), "%i", score->time);
	CG_DrawBigString(x_time, y, string, 0.5);

	CG_DrawBigString(x_name, y, xq_getname(cent, 1), 0.5);
}
static int scoreboardSort(const void *a, const void *b) {
	xq_score_info_t *ap = (xq_score_info_t *)a;
	xq_score_info_t *bp = (xq_score_info_t *)b;

	if (ap->client == 0 && bp->client > 0) return 1;
	if (ap->kills > bp->kills) {
		return -1;
	} else if (ap->kills < bp->kills) {
		return 1;
	}


	// reached here = need to untie kill score
	// we're putting ourselves first
	if (ap->client == cg.snap->ps.clientNum) {
		return -1;
	} else if (bp->client == cg.snap->ps.clientNum) {
		return 1;
	}

	return 0;
}

qboolean xq_scoreboard(void) {

	// we only want to display score board if we are in an arena zone, and we're either dead or requested it with +scores

	int dontshow = 0;
	if (!(cg.snap->ps.xq_flags & XQ_ZONE_ARENA)) dontshow = 1;
	if (!cg.showScores && !(cg.snap->ps.xq_flags & XQ_DEAD)) dontshow = 1;
	if (dontshow) return qfalse;


	playerState_t *ps = &cg.snap->ps;

	int maxdisplay = 8;
	int i;
	int myrank = 1;
	int myk = ps->xq_arena_kills;


	xq_score_info_t info[MAX_CLIENTS];
	memset(&info, 0, sizeof(info));


	for (i = 1;  i <= cgs.maxclients;  i++) {
		centity_t *cent = &cg_entities[i];
		if (cent->currentState.xq_enttype == XQ_ENTTYPE_PC) {
			entityState_t *es = &cent->currentState;

			// populate array before sorting by kills
			info[i].client = cent->currentState.number;
			if (cent->currentState.number == cg.snap->ps.clientNum) {
				info[i].ping = ps->xq_ping;
				info[i].time = ps->xq_time;
				info[i].kills = ps->xq_arena_kills;
				info[i].deaths = ps->xq_arena_deaths;
			} else {
				info[i].ping = es->xq_ping;
				info[i].time = es->xq_time;
				info[i].kills = es->xq_arena_kills;
				info[i].deaths = es->xq_arena_deaths;
			}

			// figure out my rank
			if (cent->currentState.number != cg.snap->ps.clientNum) {
				if (es->xq_arena_kills > myk) {
					myrank++;
				}
			}
		}
	}

	qsort(&info, MAX_CLIENTS, sizeof(xq_score_info_t), scoreboardSort);


	int		header_spread = 120;
	int		x_kd = 100;
	int		x_ping = x_kd + header_spread;
	int		x_time = x_kd + header_spread * 2;
	int		x_name = x_kd + header_spread * 3;


	int		x, y, w;
	float	fade;
	char	*s;

	// don't draw anuthing if the menu or console is up
	if (cg_paused.integer) return qfalse;

	fade = 1.0;

	y = 60;

	s = va("^8kills older than 5 minutes aren't counted");
	w = CG_DrawStrlen(s) * 8;
	x = ((SCREEN_WIDTH*cgs.screenXScale) / 2 ) - (w / 2);

	CG_DrawTinyString(NULL, x, y*cgs.screenYScale, s, 1.0);
	y += 10;

	s = va("%s place with %i", CG_PlaceString(myrank), myk);
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	x = (SCREEN_WIDTH / 2 ) - (w / 2);
	CG_DrawBigString( x, y, s, fade );
	y += 30;




	vec4_t col_header = {0.5, 0.5, 0.5, 1};
	CG_DrawBigStringColor(x_kd, y, "K/D", col_header);
	CG_DrawBigStringColor(x_ping, y, "PING", col_header);
	CG_DrawBigStringColor(x_time, y, "TIME", col_header);
	CG_DrawBigStringColor(x_name, y, "NAME", col_header);
	y += 10;

	CG_DrawBigStringColor(x_kd-3, y, "___________________________", col_header);
	y += 30;

	vec4_t col_row = {1, 1, 1, 1};
	for (i = 0;  i < maxdisplay;  i++) {
		if (info[i].client) {
			drawScore(y, &info[i], col_row, x_kd, x_ping, x_time, x_name);
			y += 20;
		}
	}

	return qtrue;
}

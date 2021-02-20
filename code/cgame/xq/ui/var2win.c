#include "../../cg_local.h"
#include "style.h"

/* DESCRIPTION

	Handle ui-related cvars such as cg_Bigbars and create/delete windows if their value changes.

*/

enum windowTypes_e {
	BIGBARS,
	BIGTARGET,
	XPBAR,
	WINTYPES_NUM
};

static void bigBars_ON(void) {
	qw_WindowDelete("smallBars");

	int y = 1;
	int winwidth = XQUI_BIGBARS_BAR_WIDTH + XQUI_BIGBARS_MARGIN * 2 + QW_WIN_BORDER_WIDTH * 2;
	int nb_bars = 4;
	if (!xq_is_magic_user(cg.snap->ps.xq_class)) {
		// Pure melee classes don't need the mana bar
		nb_bars--;
	};
	int winheight = nb_bars * XQUI_BIGBARS_BAR_HEIGHT + 3;


	// Create the window
	qw_window_t w = {
		.wid		= "bigBars",
		.w			= winwidth,
		.h			= winheight,
		.no_title	= 1
	};
	qw_WinNew(w);

	qw_obj_t o = {0};

	// Add the HP bar
	qw_ObjInit(&o, QW_OBJ_PERCBAR);
	o.perc = &xqst->ui_hp_perc;
	o.barwidth = XQUI_BIGBARS_BAR_WIDTH;
	o.barheight = XQUI_BIGBARS_BAR_HEIGHT;
	o.filledcolor = XQUI_BIGBARS_HP_BAR_BGCOL;
	o.gradientcolor = XQUI_BIGBARS_HP_BAR_GRADCOL;
	o.gradientmirror = XQUI_BIGBARS_HP_BAR_GRADMIRROR;
	o.coloralpha = XQUI_BIGBARS_BAR_ALPHA;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), XQUI_BIGBARS_MARGIN, y, 0, 0);

	qw_ObjInit(&o, QW_OBJ_TEXT);
	o.text = "HEALTH";
	o.colorvec = XQUI_BIGBARS_TEXT_COLOR;
	o.coloralpha = XQUI_BIGBARS_TEXT_ALPHA;
	o.charsize = XQUI_BIGBARS_TEXT_SIZE;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), XQUI_BIGBARS_MARGIN, y+XQUI_BIGBARS_TEXT_Y_MARGIN, 0, 0);

	o.text = (char *)&xqst->ui_hponlytxt;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), -1, y+XQUI_BIGBARS_TEXT_Y_MARGIN, 0, 0);

	y += XQUI_BIGBARS_BAR_HEIGHT;


	// Add the MANA bar if applicable
	if (nb_bars == 4) {
		qw_ObjInit(&o, QW_OBJ_PERCBAR);
		o.perc = &xqst->ui_mana_perc;
		o.barwidth = XQUI_BIGBARS_BAR_WIDTH;
		o.barheight = XQUI_BIGBARS_BAR_HEIGHT;
		o.filledcolor = XQUI_BIGBARS_MANA_BAR_BGCOL;
		o.gradientcolor = XQUI_BIGBARS_MANA_BAR_GRADCOL;
		o.gradientmirror = XQUI_BIGBARS_MANA_BAR_GRADMIRROR;
		o.coloralpha = XQUI_BIGBARS_BAR_ALPHA;
		qw_ObjAttach("bigBars", qw_ObjCreate(&o), XQUI_BIGBARS_MARGIN, y, 0, 0);

		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = "MANA";
		o.colorvec = XQUI_BIGBARS_TEXT_COLOR;
		o.coloralpha = XQUI_BIGBARS_TEXT_ALPHA;
		o.charsize = XQUI_BIGBARS_TEXT_SIZE;
		qw_ObjAttach("bigBars", qw_ObjCreate(&o), XQUI_BIGBARS_MARGIN, y+XQUI_BIGBARS_TEXT_Y_MARGIN, 0, 0);

		o.text = (char *)&xqst->ui_manaonlytxt;
		qw_ObjAttach("bigBars", qw_ObjCreate(&o), -1, y+XQUI_BIGBARS_TEXT_Y_MARGIN, 0, 0);

		y += XQUI_BIGBARS_BAR_HEIGHT;
	}


	// Add the ENDURANCE bar
	qw_ObjInit(&o, QW_OBJ_PERCBAR);
	o.perc = &xqst->ui_endurance_perc;
	o.barwidth = XQUI_BIGBARS_BAR_WIDTH;
	o.barheight = XQUI_BIGBARS_BAR_HEIGHT;
	o.filledcolor = XQUI_BIGBARS_END_BAR_BGCOL;
	o.gradientcolor = XQUI_BIGBARS_END_BAR_GRADCOL;
	o.gradientmirror = XQUI_BIGBARS_END_BAR_GRADMIRROR;
	o.coloralpha = XQUI_BIGBARS_BAR_ALPHA;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), XQUI_BIGBARS_MARGIN, y, 0, 0);

	qw_ObjInit(&o, QW_OBJ_TEXT);
	o.text = "ENDURANCE";
	o.colorvec = XQUI_BIGBARS_TEXT_COLOR;
	o.coloralpha = XQUI_BIGBARS_TEXT_ALPHA;
	o.charsize = XQUI_BIGBARS_TEXT_SIZE;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), XQUI_BIGBARS_MARGIN, y+XQUI_BIGBARS_TEXT_Y_MARGIN, 0, 0);

	o.text = (char *)&xqst->ui_enduranceonlytxt;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), -1, y+XQUI_BIGBARS_TEXT_Y_MARGIN, 0, 0);

	y += XQUI_BIGBARS_BAR_HEIGHT;


	// Add the ENERGY bar
	qw_ObjInit(&o, QW_OBJ_PERCBAR);
	o.perc = &xqst->ui_energy_perc;
	o.barwidth = XQUI_BIGBARS_BAR_WIDTH;
	o.barheight = XQUI_BIGBARS_BAR_HEIGHT;
	o.filledcolor = XQUI_BIGBARS_NRG_BAR_BGCOL;
	o.gradientcolor = XQUI_BIGBARS_NRG_BAR_GRADCOL;
	o.gradientmirror = XQUI_BIGBARS_NRG_BAR_GRADMIRROR;
	o.coloralpha = XQUI_BIGBARS_BAR_ALPHA;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), XQUI_BIGBARS_MARGIN, y, 0, 0);

	qw_ObjInit(&o, QW_OBJ_TEXT);
	o.text = "ENERGY";
	o.colorvec = XQUI_BIGBARS_TEXT_COLOR;
	o.coloralpha = XQUI_BIGBARS_TEXT_ALPHA;
	o.charsize = XQUI_BIGBARS_TEXT_SIZE;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), XQUI_BIGBARS_MARGIN, y+XQUI_BIGBARS_TEXT_Y_MARGIN, 0, 0);

	o.text = (char *)&xqst->ui_energyonlytxt;
	qw_ObjAttach("bigBars", qw_ObjCreate(&o), -1, y+XQUI_BIGBARS_TEXT_Y_MARGIN, 0, 0);
}
static void bigBars_OFF(void) {
	qw_WindowDelete("bigBars");


	// Create the window
	qw_window_t w = {
		.wid		= "smallBars",
		.w			= XQUI_SMALLBARS_BAR_WIDTH + QW_WIN_BORDER_WIDTH * 2 + XQUI_SMALLBARS_MARGIN * 2,
		.h			= XQUI_SMALLBARS_BAR_HEIGHT * 4 + 20,
		.no_title	= 1
	};
	qw_WinNew(w);


	// Add the character name and level
	qw_ObjAttach("smallBars", qws->objhandles[XQ_UI_OBJID_CHAR_NAME], XQUI_SMALLBARS_MARGIN, 0, 0, 0);
	qw_ObjAttach("smallBars", qws->objhandles[XQ_UI_OBJID_CHAR_LEVEL], -1, 0, 0, 0);
	int y = 20;


	// Add the HP bar
	qw_obj_t o = {0};
	qw_ObjInit(&o, QW_OBJ_PERCBAR);
	o.perc = &xqst->ui_hp_perc;
	o.barwidth = XQUI_SMALLBARS_BAR_WIDTH;
	o.barheight = XQUI_SMALLBARS_BAR_HEIGHT;
	o.borderwidth = XQUI_SMALLBARS_BAR_BRDWIDTH;
	o.filledcolor = XQUI_SMALLBARS_HP_COL;
	qw_ObjAttach("smallBars", qw_ObjCreate(&o), XQUI_SMALLBARS_MARGIN, y, 0, 0);
	y+= XQUI_SMALLBARS_BAR_HEIGHT;


	// Mana bar
	o.perc = &xqst->ui_mana_perc;
	o.filledcolor = XQUI_SMALLBARS_MANA_COL;
	qw_ObjAttach("smallBars", qw_ObjCreate(&o), XQUI_SMALLBARS_MARGIN, y, 0, 0);
	y+= XQUI_SMALLBARS_BAR_HEIGHT;


	// Endurance bar
	o.perc = &xqst->ui_endurance_perc;
	o.filledcolor = XQUI_SMALLBARS_END_COL;
	qw_ObjAttach("smallBars", qw_ObjCreate(&o), XQUI_SMALLBARS_MARGIN, y, 0, 0);
	y+= XQUI_SMALLBARS_BAR_HEIGHT;


	// Energy bar
	o.perc = &xqst->ui_energy_perc;
	o.filledcolor = XQUI_SMALLBARS_NRG_COL;
	qw_ObjAttach("smallBars", qw_ObjCreate(&o), XQUI_SMALLBARS_MARGIN, y, 0, 0);
}
static void bigTarget_ON(void) {
	qw_WindowDelete("smallTarget");


	// Create the window
	qw_window_t w = {
		.wid		= "bigTarget",
		.w			= XQUI_BIGTARGET_BAR_WIDTH + QW_WIN_BORDER_WIDTH * 2 + XQUI_BIGTARGET_MARGIN * 2,
		.h			= XQUI_BIGTARGET_BAR_HEIGHT + QW_WIN_BORDER_WIDTH * 2,
		.no_title	= 1
	};
	qw_WinNew(w);

	// target hp percentage
	qw_obj_t o = {0};
	qw_ObjInit(&o, QW_OBJ_PERCBAR);
	o.perc = &xqst->ui_target_hp_perc;
	o.barwidth = XQUI_BIGTARGET_BAR_WIDTH;
	o.barheight = XQUI_BIGTARGET_BAR_HEIGHT;
	o.filledcolor = XQUI_BIGTARGET_BAR_BGCOL;
	o.gradientmirror = XQUI_BIGTARGET_BAR_GRADMIRROR;
	o.gradientcolor = XQUI_BIGTARGET_BAR_GRADCOL;
	o.coloralpha = XQUI_BIGTARGET_BAR_ALPHA;
	qw_ObjAttach("bigTarget", qw_ObjCreate(&o), XQUI_BIGTARGET_MARGIN, 0, 0, 0);


	// actual target name
	qw_ObjInit(&o, QW_OBJ_TEXT);
	o.text = (char *)&xqst->ui_target_name;
	o.charsize = XQUI_BIGTARGET_TEXT_SIZE;
	o.clickthrough = 1;
	qw_ObjAttach("bigTarget", qw_ObjCreate(&o), XQUI_BIGTARGET_MARGIN, 8, 0, 0);


	// placeholder if nothing is targeted
	o.text = "TARGET";
	o.hide_if_not_null = (char *)&xqst->ui_target_name;
	o.coloralpha = 1;
	qw_ObjAttach("bigTarget", qw_ObjCreate(&o), XQUI_BIGTARGET_MARGIN, 8, -1, 0);

}
static void bigTarget_OFF(void) {
	qw_WindowDelete("bigTarget");


	// Create the window
	qw_window_t w = {
		.wid		= "smallTarget",
		.w			= XQUI_SMALLTARGET_BAR_WIDTH + QW_WIN_BORDER_WIDTH * 2 + XQUI_SMALLTARGET_MARGIN * 2,
		.h			= XQUI_SMALLTARGET_BAR_HEIGHT + QW_WIN_BORDER_WIDTH * 2 + 10,
		.no_title	= 1
	};
	qw_WinNew(w);


	// Target HP bar
	qw_obj_t o = {0};
	qw_ObjInit(&o, QW_OBJ_PERCBAR);
	o.perc = &xqst->ui_target_hp_perc;
	o.barwidth = XQUI_SMALLTARGET_BAR_WIDTH;
	o.barheight = XQUI_SMALLTARGET_BAR_HEIGHT;
	o.borderwidth = XQUI_SMALLTARGET_BAR_BRDWIDTH;
	o.filledcolor = XQUI_SMALLTARGET_BGCOL;
	qw_ObjAttach("smallTarget", qw_ObjCreate(&o), XQUI_SMALLTARGET_MARGIN, 9, 0, 0);


	// Target name
	qw_ObjInit(&o, QW_OBJ_TEXT);
	o.text = (char *)&xqst->ui_target_name;
	o.clickthrough = 1;
	qw_ObjAttach("smallTarget", qw_ObjCreate(&o), XQUI_SMALLTARGET_MARGIN, 0, 0, 0);
}
static void xpBar_ON(void) {
	// Create the window
	qw_window_t w = {
		.wid		= "xpBar",
		.w          = XQUI_XP_BAR_WIDTH + QW_WIN_BORDER_WIDTH * 2 + XQUI_XP_BAR_MARGIN * 2,
		.h			= XQUI_XP_BAR_HEIGHT + QW_WIN_BORDER_WIDTH * 2,
		.no_title	= 1
	};
	qw_WinNew(w);


	// XP in level
	qw_obj_t o = {0};
	qw_ObjInit(&o, QW_OBJ_PERCBAR);
	o.perc = &xqst->ui_xp_perc;
	o.barwidth = XQUI_XP_BAR_WIDTH;
	o.barheight = XQUI_XP_BAR_HEIGHT;
	o.filledcolor = XQUI_XP_BAR_BGCOL;
	o.gradientmirror = XQUI_XP_BAR_GRADMIRROR;
	o.gradientcolor = XQUI_XP_BAR_GRADCOL;
	o.coloralpha = XQUI_XP_BAR_ALPHA;
	o.multiplier = 1000;
	o.fastbar = 5;
	o.fastbarheight = XQUI_XP_FASTBAR_HEIGHT;
	qw_ObjAttach("xpBar", qw_ObjCreate(&o), XQUI_XP_BAR_MARGIN, 0, 0, 0);


	// Level x
	qw_ObjInit(&o, QW_OBJ_TEXT);
	o.text = (char *)&xqst->ui_level;
	o.charsize = XQUI_XP_TEXT_SIZE;
	o.clickthrough = 1;
	qw_ObjAttach("xpBar", qw_ObjCreate(&o), XQUI_XP_BAR_MARGIN, 8, 0, 0);

}
static void xpBar_OFF(void) {
	qw_WindowDelete("xpBar");
}
static void CheckVar(int win) {
	// controls switching on and off windows according to the state of cg_bigBars and such

	static int state[WINTYPES_NUM];
	static int init_done = 0;
	if (!init_done) {
		init_done = 1;
		memset(state, -1, sizeof(state));
	}

	char *cvar_name = NULL;
	switch (win) {
		case BIGBARS:		cvar_name = "cg_bigBars";		break;
		case BIGTARGET:		cvar_name = "cg_bigTarget";		break;
		case XPBAR:			cvar_name = "cg_xpBar";			break;
		default:	return;
	}
	if (!cvar_name) return;



	// See if the var changed.
	if (cg_bigBars.integer != state[win]) {
		state[win] = cg_bigBars.integer;
		if (state[win] == 0) {
			switch(win) {
				case BIGBARS:		bigBars_OFF();		break;
				case BIGTARGET:		bigTarget_OFF();	break;
				case XPBAR:			xpBar_OFF();		break;
			}
		} else {
			switch(win) {
				case BIGBARS:		bigBars_ON();		break;
				case BIGTARGET:		bigTarget_ON();		break;
				case XPBAR:			xpBar_ON();			break;
			}
		}
	}
}

void xqui_VarsToWindows() {
	CheckVar(BIGBARS);
	CheckVar(BIGTARGET);
	CheckVar(XPBAR);
}

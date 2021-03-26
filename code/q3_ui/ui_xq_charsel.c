#include "ui_local.h"

typedef struct character_s {
	char name[100];
	char zone[100];
	int class;
	int race;
	int face;
	int gender;
	int level;
} character_t;
character_t chars[8];

typedef struct s_xqcharsel_s {
	menuframework_s	menu;

	menutext_s		banner;

	menubitmap_s	arrow[8];
	menutext_s		c[8];

	menubitmap_s	enter;
	menubitmap_s	delete;

	menutext_s		pname;
	menutext_s		pzone;
	menubitmap_s	player;
} s_xqcharsel_t;
static s_xqcharsel_t s_xqcharsel;

static playerInfo_t pi = {0};
static int selected_slot = -1;
int go_straight_to_charcreation = 0;
xq_model_t xq_playable_models[XQ_RACES][2];


#define ART_ARROW		"menu/art/gs_arrows_r"
#define ART_DELETE0		"menu/art/delete_0"
#define ART_DELETE1		"menu/art/delete_1"
#define ART_ENTER0		"menu/art/enter_0"
#define ART_ENTER1		"menu/art/enter_1"

#define ID_CHAR			13
#define ID_DELETE		1000
#define ID_ENTER		1001


static void updateModel(void) {
	static char pname[200] = {0};
	static char pzone[200] = {0};
	if (selected_slot == -1) {
		pname[0] = 0;
		pzone[0] = 0;
		return;
	}

	// Set the player model
	vec3_t	viewangles;
	vec3_t	moveangles;

	viewangles[YAW]   = 180 - 0;
	viewangles[PITCH] = 0;
	viewangles[ROLL]  = 0;
	VectorClear(moveangles);

	pi.race = chars[selected_slot].race;
	int gender = chars[selected_slot].gender;

	char *path = xq_playable_models[pi.race][gender].path;
	UI_PlayerInfo_SetModel(&pi, path, chars[selected_slot].face);
	UI_PlayerInfo_SetInfo(&pi, LEGS_IDLE, TORSO_STAND, viewangles, moveangles, WP_NONE, qfalse);


	// Set the player name, level, class and zone text
	// appearing above the model

	snprintf(pname, 200, "%s (%i %s)",
		chars[selected_slot].name,
		chars[selected_slot].level,
		xq_class_lit(chars[selected_slot].class, 0, 0)
	);
	snprintf(pzone, 200, "%s", chars[selected_slot].zone);
	s_xqcharsel.pname.string = pname;
	s_xqcharsel.pzone.string = pzone;
}
static void DrawPlayer(void *self) {
	if (selected_slot == -1) {
		return;
	}
	menubitmap_s *b = (menubitmap_s *)self;

	UI_DrawPlayer(b->generic.x, b->generic.y, b->width, b->height, &pi, uis.realtime/2);
}
static void deleteChar(qboolean result) {
	if (!result) {
		// Said "No" to delete confirmation prompt
		return;
	}
	if (selected_slot < 0) {
		return;
	}
    char *charname = (char *)s_xqcharsel.c[selected_slot].generic.name;

	trap_Cvar_Set("cl_charname", charname);
	trap_Cvar_Set("xq_charDelete", "1");
	trap_Cvar_Set("xq_charSelOn", "0");
	trap_Cmd_ExecuteText(EXEC_APPEND, "reconnect\n");
}
static void showArrow(void) {
	// Show the arrow next to the selected char
	for (int i = 0;  i < 8;  i++) {
		if (selected_slot == i) {
			s_xqcharsel.arrow[i].width = 25;
		} else {
			s_xqcharsel.arrow[i].width = 0;
		}
	}

	// Only show the delete/enter world buttons if we have a char selected
	if (selected_slot == -1) {
		s_xqcharsel.enter.width = 0;
		s_xqcharsel.delete.width = 0;
	} else {
		s_xqcharsel.enter.width = 128;
		s_xqcharsel.delete.width = 128;
	}
}
static void menuEvent(void *ptr, int event) {
	if (event != QM_ACTIVATED) {
		return;
	}

	int id = ((menucommon_s*)ptr)->id;
	char *charname;
	switch (id) {
		case ID_CHAR:
		case ID_CHAR+1:
		case ID_CHAR+2:
		case ID_CHAR+3:
		case ID_CHAR+4:
		case ID_CHAR+5:
		case ID_CHAR+6:
		case ID_CHAR+7:
		case ID_CHAR+8:;
			charname = (char *)((menucommon_s*)ptr)->name;
			if (xq_seq(charname, "Empty slot")) {
				selected_slot = -1;
				UI_XQCharCreatorMenu("");
			} else {
				selected_slot = id - ID_CHAR;
			}
			updateModel();
			showArrow();
			break;

		case ID_ENTER:;
			charname = (char *)s_xqcharsel.c[selected_slot].generic.name;
			trap_Cvar_Set("cl_charname", charname);
			trap_Cvar_Set("xq_charDelete", "0");
			trap_Cvar_Set("xq_charSelOn", "0");
			trap_Cmd_ExecuteText(EXEC_APPEND, "reconnect\n");
			break;

		case ID_DELETE:
			UI_ConfirmMenu("Delete character?", 0, deleteChar);
			break;
	}
}
static void menuInit(char *ss) {
	trap_Cvar_Set("cl_charname", "");
	char *s = ss+2;
	char toons[10000] = {0};
	char models[10000] = {0};

	char *ptr = strchr(s, '!');
	if (ptr) {
		Q_strncpyz(models, ptr+1, sizeof(models));
		*ptr = 0;
	}
	Q_strncpyz(toons, s, sizeof(toons));


	UI_XQCharsel_Cache();

	int num = 0;

	memset(chars, 0, sizeof(chars));

	memset(&s_xqcharsel, 0, sizeof(s_xqcharsel));
	s_xqcharsel.menu.wrapAround = qtrue;
	s_xqcharsel.menu.fullscreen = qtrue;

	s_xqcharsel.banner.generic.type	= MTYPE_TEXT;
	s_xqcharsel.banner.generic.x	= 320;
	s_xqcharsel.banner.generic.y	= 6;
	s_xqcharsel.banner.string		= "CHARACTER SELECTION";
	s_xqcharsel.banner.color		= color_white;
	s_xqcharsel.banner.style		= UI_CENTER;

	int charx = 20;
	int chary = 50;

	int modelx = 380;
	int modely = -30;

	char *tok1 = strtok(toons, "|");
	while (tok1 != NULL) {
		int ret = sscanf(tok1, "%s %i %i %s %i %i %i",
			chars[num].name,
			&chars[num].class,
			&chars[num].level,
			chars[num].zone,
			&chars[num].race,
			&chars[num].face,
			&chars[num].gender
		);

		if (strlen(chars[num].name) > 0 && ret == 7) {

			s_xqcharsel.c[num].generic.type		= MTYPE_PTEXT;
			s_xqcharsel.c[num].generic.name		= chars[num].name;
			s_xqcharsel.c[num].generic.flags	= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
			s_xqcharsel.c[num].generic.x		= charx;
			s_xqcharsel.c[num].generic.y		= chary + num * 30;
			s_xqcharsel.c[num].generic.id		= ID_CHAR + num;
			s_xqcharsel.c[num].generic.callback	= menuEvent;
			s_xqcharsel.c[num].string			= chars[num].name;
			s_xqcharsel.c[num].color			= color_red;
			s_xqcharsel.c[num].style			= UI_LEFT;
			num++;

			if (num == 8) break;
		}
		tok1 = strtok(NULL, "|");
	}


	int i;
	for (i = num;  i < 8;  i++) {
		s_xqcharsel.c[i].generic.type			= MTYPE_PTEXT;
		s_xqcharsel.c[i].generic.name			= "Empty slot";
		s_xqcharsel.c[i].generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
		s_xqcharsel.c[i].generic.x				= charx;
		s_xqcharsel.c[i].generic.y				= chary + i * 30;
		s_xqcharsel.c[i].generic.id				= ID_CHAR + i;
		s_xqcharsel.c[i].generic.callback		= menuEvent;
		s_xqcharsel.c[i].string					= "Empty slot";
		s_xqcharsel.c[i].color					= color_orange;
		s_xqcharsel.c[i].style					= UI_LEFT;
	}
	for (i = 0;  i < 8;  i++) {
		s_xqcharsel.arrow[i].generic.type		= MTYPE_BITMAP;
		s_xqcharsel.arrow[i].generic.name		= ART_ARROW;
		s_xqcharsel.arrow[i].generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
		s_xqcharsel.arrow[i].generic.x			= charx - 30;
		s_xqcharsel.arrow[i].generic.y			= chary + i * 30;
		s_xqcharsel.arrow[i].width				= 25;
		s_xqcharsel.arrow[i].height				= 33;
	}

	s_xqcharsel.delete.generic.type				= MTYPE_BITMAP;
	s_xqcharsel.delete.generic.name				= ART_DELETE0;
	s_xqcharsel.delete.generic.flags			= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_xqcharsel.delete.generic.id				= ID_DELETE;
	s_xqcharsel.delete.generic.callback			= menuEvent;
	s_xqcharsel.delete.generic.x				= 640;
	s_xqcharsel.delete.generic.y				= 480-64;
	s_xqcharsel.delete.width					= 128;
	s_xqcharsel.delete.height					= 64;
	s_xqcharsel.delete.focuspic					= ART_DELETE1;

	s_xqcharsel.enter.generic.type				= MTYPE_BITMAP;
	s_xqcharsel.enter.generic.name				= ART_ENTER0;
	s_xqcharsel.enter.generic.flags				= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_xqcharsel.enter.generic.id				= ID_ENTER;
	s_xqcharsel.enter.generic.callback			= menuEvent;
	s_xqcharsel.enter.generic.x					= 440;
	s_xqcharsel.enter.generic.y					= 480-64;
	s_xqcharsel.enter.width						= 128;
	s_xqcharsel.enter.height					= 64;
	s_xqcharsel.enter.focuspic					= ART_ENTER1;



	s_xqcharsel.player.generic.type				= MTYPE_BITMAP;
	s_xqcharsel.player.generic.flags			= QMF_INACTIVE;
	s_xqcharsel.player.generic.ownerdraw		= DrawPlayer;
	s_xqcharsel.player.generic.x				= modelx;
	s_xqcharsel.player.generic.y				= modely;
	s_xqcharsel.player.width					= 32*10;
	s_xqcharsel.player.height					= 56*10;

	s_xqcharsel.pname.generic.type				= MTYPE_PTEXT;
	s_xqcharsel.pname.generic.x					= modelx + 90;
	s_xqcharsel.pname.generic.y					= modely + 80;
	s_xqcharsel.pname.string					= "";
	s_xqcharsel.pname.color						= color_yellow;
	s_xqcharsel.pname.style						= UI_SMALLFONT|UI_CENTER;

	s_xqcharsel.pzone.generic.type				= MTYPE_PTEXT;
	s_xqcharsel.pzone.generic.x					= modelx + 90;
	s_xqcharsel.pzone.generic.y					= modely + 100;
	s_xqcharsel.pzone.string					= "";
	s_xqcharsel.pzone.color						= color_yellow;
	s_xqcharsel.pzone.style						= UI_SMALLFONT|UI_CENTER;


	for (int i = 0;  i < 8;  i++) {
		Menu_AddItem(&s_xqcharsel.menu, &s_xqcharsel.c[i]);
		Menu_AddItem(&s_xqcharsel.menu, &s_xqcharsel.arrow[i]);
	}
	Menu_AddItem(&s_xqcharsel.menu, &s_xqcharsel.banner);
	Menu_AddItem(&s_xqcharsel.menu, &s_xqcharsel.delete);
	Menu_AddItem(&s_xqcharsel.menu, &s_xqcharsel.enter);
	Menu_AddItem(&s_xqcharsel.menu, &s_xqcharsel.player);
	Menu_AddItem(&s_xqcharsel.menu, &s_xqcharsel.pname);
	Menu_AddItem(&s_xqcharsel.menu, &s_xqcharsel.pzone);

	// set the global xq_playable_models array with the playable model path, gender and race info
	// that is used by the char creator menu
	memset(xq_playable_models, 0, sizeof(xq_playable_models));
	if (strlen(models)) {
		tok1 = strtok(models, "^");
		while (tok1 != NULL) {
			int race, gender, facenum;
			char path[XQ_MODEL_MAX_PATH] = {0};
			int ret = sscanf(tok1, "%i.%i.%i.%s", &race, &gender, &facenum, path);
			if (strlen(path) > 0 && strlen(path) <= XQ_MODEL_MAX_PATH && ret == 4) {
				Q_strncpyz(
					xq_playable_models[race][gender].path,
					path,
					sizeof(xq_playable_models[race][gender].path));
				xq_playable_models[race][gender].numfaces = facenum;
			}
			tok1 = strtok(NULL, "^");
		}
	}
}

void UI_XQCharsel_Cache(void) {
	trap_R_RegisterShaderNoMip(ART_ARROW);
	trap_R_RegisterShaderNoMip(ART_DELETE0);
	trap_R_RegisterShaderNoMip(ART_DELETE1);
	trap_R_RegisterShaderNoMip(ART_ENTER0);
	trap_R_RegisterShaderNoMip(ART_ENTER1);
}
void UI_XQCharselMenu(char *s) {
	trap_Cvar_Set("xq_charDelete", "0");
	selected_slot = -1;
	updateModel();

	// remove trailing \n
	if (strlen(s) > 0) {
		if (s[strlen(s)-1] == '\n') {
			s[strlen(s)-1] = 0;
		}
	}
	if (s[1] == '0') {
		go_straight_to_charcreation = 0;
	} else if (s[1] == '1') {
		go_straight_to_charcreation = 1;
		s[1] = '0';
	}
	menuInit(s);
	UI_PushMenu(&s_xqcharsel.menu);

	if (go_straight_to_charcreation) {
		go_straight_to_charcreation = 0;
		UI_XQCharCreatorMenu("Name unavailable");
	}
	showArrow();
}

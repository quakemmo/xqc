#include "ui_local.h"

#define MODEL_BACK0			"menu/art/back_0"
#define MODEL_BACK1			"menu/art/back_1"
#define MODEL_ACCEPT0		"menu/art/accept_0"
#define MODEL_ACCEPT1		"menu/art/accept_1"
#define MODEL_ARROWS		"menu/art/gs_arrows_0"
#define MODEL_ARROWSL		"menu/art/gs_arrows_l"
#define MODEL_ARROWSR		"menu/art/gs_arrows_r"

#define LOW_MEMORY			(5 * 1024 * 1024)

static int selected_class = 0;
static int selected_race = 0;
static int selected_gender = 0;
static int selected_face = 0;
static char selected_gender_lit[100] = {0};

static char class_lit[20][XQ_CLASSES] = {0};
static char race_lit[20][XQ_RACES] = {0};

static char status_txt[100] = {0};

static char *xqchar_artlist[] = {
	MODEL_BACK0,
	MODEL_BACK1,
	MODEL_ACCEPT0,
	MODEL_ACCEPT1,
	MODEL_ARROWS,
	MODEL_ARROWSL,
	MODEL_ARROWSR,
	NULL
};


#define ID_PREVGENDER		100
#define ID_NEXTGENDER		101
#define ID_BACK				102
#define ID_PREVFACE			103
#define ID_NEXTFACE			104
#define ID_ACCEPT			105
#define ID_CLASS			1000
#define ID_RACE				2000

typedef struct xqchar_s {
	menuframework_s	menu;
	menutext_s		banner;
	menubitmap_s	back;
	menubitmap_s	accept;
	menubitmap_s	player;
	menubitmap_s	left;
	menubitmap_s	right;
	menutext_s		modelname;
	menutext_s		skinname;
	menutext_s		playername_label;
	menufield_s		playername_field;
	menutext_s		status;

	menutext_s		gender_text;
	menubitmap_s	gender_arrows;
	menubitmap_s	gender_left;
	menubitmap_s	gender_right;

	menutext_s		face_text;
	menubitmap_s	face_arrows;
	menubitmap_s	face_left;
	menubitmap_s	face_right;

	menubitmap_s 	class_arrow[XQ_CLASSES];
	menutext_s		class[XQ_CLASSES];

	menubitmap_s 	race_arrow[XQ_CLASSES];
	menutext_s		race[XQ_RACES];

	playerInfo_t	playerinfo;
} xqchar_t;

static xqchar_t s_xqchar;

static void st(char *t) {
	Q_strncpyz(status_txt, t, sizeof(status_txt));
}

static void UI_XQCharCreator_Cache(void) {
	for (int i = 0;  xqchar_artlist[i];  i++) {
		trap_R_RegisterShaderNoMip(xqchar_artlist[i]);
	}
}
static void XQCharCreator_UpdateModel(void) {
	playerInfo_t *pi = &s_xqchar.playerinfo;
	vec3_t	viewangles;
	vec3_t	moveangles;

	memset(pi, 0, sizeof(playerInfo_t));

	viewangles[YAW]   = 180 - 0;
	viewangles[PITCH] = 0;
	viewangles[ROLL]  = 0;
	VectorClear(moveangles);

	pi->race = selected_race;

	char *path = xq_playable_models[selected_race][selected_gender].path;

	UI_PlayerInfo_SetModel(pi, path, selected_face);
	UI_PlayerInfo_SetInfo(pi, LEGS_IDLE, TORSO_STAND, viewangles, moveangles, WP_NONE, qfalse);
}
static void submitChar(void) {
	char *n = s_xqchar.playername_field.field.buffer;
	if (strlen(n) < XQ_MIN_CHAR_NAME) {
		st("Name too short");
		return;
	}

	// We pop charcreator and charselector where we inevitably came from
	UI_PopMenu();
	UI_PopMenu();

	trap_Cvar_Set("xq_charcreation_data", va("%s|%i|%i|%i|%i",
		n, selected_class, selected_race, selected_gender, selected_face));
	trap_Cvar_Set("cl_charname", "");
	trap_Cvar_Set("xq_charSelOn", "0");
	st("Submitting name...");
	trap_Cmd_ExecuteText(EXEC_APPEND, "reconnect\n");
}
static void DrawName(void *self) {
	menufield_s		*f;
	int				length;
	char*			p;

	f = (menufield_s*)self;

	// GRANK_FIXME - enforce valid characters
	for( p = f->field.buffer; *p != '\0'; p++ ) {
		//if( ispunct(*p) || isspace(*p) )
		if( !Q_isalpha(*p)) {
			*p = '\0';
		}
		if (*p) {
			if (p == f->field.buffer) {
				*p = toupper(*p);
			} else {
				*p = tolower(*p);
			}
		}
	}

	// strip color codes
	Q_CleanStr(f->field.buffer);
	length = strlen(f->field.buffer);
	if (f->field.cursor > length) {
		f->field.cursor = length;
	}
	if (length >= 1) {
		s_xqchar.accept.width = 64;
	} else {
		s_xqchar.accept.width = 0;
	}

	MenuField_Draw(self);
}
static void updateRaces(void) {
	int *c = &selected_class;
	int *r = &selected_race;
	menutext_s *but;
	for (int i = 0;  i < XQ_RACES;  i++) {
		but = &s_xqchar.race[i];
		if (xq_class_race_combo(*c, i)) {
			but->color = text_color_normal;
			but->generic.flags = QMF_PULSEIFFOCUS;
		} else {
			but->color = text_color_disabled;
			but->generic.flags |= QMF_INACTIVE;
		}
	}

	if (!xq_class_race_combo(*c, *r)) {
		// newly selected class doesn't allow for currently selected race - pick 1st allowed race
		for (int i = 0;  i < XQ_RACES;  i++) {
			if (xq_class_race_combo(*c, i)) {
				*r = i;
				selected_face = 0;
				but = &s_xqchar.race[i];
				but->color = text_color_normal;
				but->generic.flags = QMF_PULSEIFFOCUS;
				return;
			}
		}
	}
}
static void showArrows(void) {
	for (int i = 0;  i < XQ_CLASSES;  i++) {
		if (selected_class == i) {
			s_xqchar.class_arrow[i].width = 25;
		} else {
			s_xqchar.class_arrow[i].width = 0;
		}
	}

	for (int i = 0;  i < XQ_RACES;  i++) {
		if (selected_race == i) {
			s_xqchar.race_arrow[i].width = 25;
		} else {
			s_xqchar.race_arrow[i].width = 0;
		}
	}
}
static void XQCharCreator_MenuEvent(void *ptr, int event) {
	if (event != QM_ACTIVATED)
		return;

	int id = ((menucommon_s*)ptr)->id;

	switch (id) {
		case ID_PREVGENDER:
		case ID_NEXTGENDER:
			if (selected_gender == 0) {
				selected_gender = 1;
				Q_strncpyz(selected_gender_lit, "FEMALE", sizeof(selected_gender_lit));
			} else {
				selected_gender = 0;
				Q_strncpyz(selected_gender_lit, "MALE", sizeof(selected_gender_lit));
			}
			selected_face = 0;
			break;

		case ID_PREVFACE:
			if (selected_face == 0) return;
			selected_face--;
			break;

		case ID_NEXTFACE:
			if (selected_face == xq_playable_models[selected_race][selected_gender].numfaces - 1) return;
			selected_face++;
			break;

		case ID_BACK:
			UI_PopMenu();
			break;

		case ID_ACCEPT:
			// Pop char creation AND char selection screens
			submitChar();
			break;
	}
	if (id >= ID_CLASS && id <= (ID_CLASS + XQ_CLASSES)) {
		selected_class = id - ID_CLASS;
		updateRaces();
		showArrows();
	}
	if (id >= ID_RACE && id <= (ID_RACE + XQ_RACES)) {
		selected_race = id - ID_RACE;
		selected_face = 0;
		showArrows();
	}
	XQCharCreator_UpdateModel();
}
static sfxHandle_t XQCharCreator_MenuKey(int key) {
/*
	// Kept for future reference
	menucommon_s*	m;

	switch (key) {
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
			m = Menu_ItemAtCursor(&s_xqchar.menu);
			break;

		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			m = Menu_ItemAtCursor(&s_xqchar.menu);
			break;

		case K_MOUSE2:
		case K_ESCAPE:
			break;
	}

*/
	return Menu_DefaultKey(&s_xqchar.menu, key);
}
static void XQCharCreator_DrawPlayer(void *self) {
	menubitmap_s *b = (menubitmap_s *)self;

	if(trap_MemoryRemaining() <= LOW_MEMORY) {
		UI_DrawProportionalString(b->generic.x, b->generic.y + b->height / 2, "LOW MEM", UI_LEFT, color_red);
		return;
	}

	UI_DrawPlayer(b->generic.x, b->generic.y, b->width, b->height, &s_xqchar.playerinfo, uis.realtime/2);
}
static void XQCharCreator_MenuInit(void) {
	int i;
	selected_class = 0;
	selected_gender = 0;
	selected_face = 0;
	selected_race = 0;

	int model_x = 540;
	int model_y = -140;
	int arrow_width = 128;
	int arrow_height = 32;
	int genderx = model_x + 95;
	int gendery = model_y + 410;

	int facex = genderx;
	int facey = gendery + 50;
	int classx = 1;
	int racex = 320;
	int liney = 30;
	int playernamex = 345;
	int playernamey = 450;

	int statusx = playernamex - 47;
	int statusy = playernamey-20;

	Q_strncpyz(selected_gender_lit, "MALE", sizeof(selected_gender_lit));
	memset(&s_xqchar, 0, sizeof(xqchar_t));

	UI_XQCharCreator_Cache();

	s_xqchar.menu.key        = XQCharCreator_MenuKey;
	s_xqchar.menu.wrapAround = qtrue;
	s_xqchar.menu.fullscreen = qtrue;

	s_xqchar.banner.generic.type					= MTYPE_TEXT;
	s_xqchar.banner.generic.x						= 320;
	s_xqchar.banner.generic.y						= 6;
	s_xqchar.banner.string							= "CHARACTER CREATION";
	s_xqchar.banner.color							= color_white;
	s_xqchar.banner.style							= UI_CENTER;


	s_xqchar.status.generic.type					= MTYPE_TEXT;
	s_xqchar.status.generic.flags					= QMF_INACTIVE;
	s_xqchar.status.generic.x						= statusx;
	s_xqchar.status.generic.y						= statusy;
	s_xqchar.status.string							= status_txt;
	s_xqchar.status.style							= UI_LEFT;
	s_xqchar.status.color							= color_yellow;

	s_xqchar.playername_label.generic.type			= MTYPE_PTEXT;
	s_xqchar.playername_label.generic.flags			= QMF_INACTIVE;
	s_xqchar.playername_label.generic.x				= playernamex;
	s_xqchar.playername_label.generic.y				= playernamey;
	s_xqchar.playername_label.string				= "NAME:";
	s_xqchar.playername_label.style					= UI_CENTER;
	s_xqchar.playername_label.color					= color_yellow;

	s_xqchar.playername_field.generic.type			= MTYPE_FIELD;
	s_xqchar.playername_field.generic.ownerdraw		= DrawName;
	s_xqchar.playername_field.generic.name			= "";
	s_xqchar.playername_field.generic.flags			= 0;
	s_xqchar.playername_field.generic.x				= playernamex + 70;
	s_xqchar.playername_field.generic.y				= playernamey + 7;
	s_xqchar.playername_field.field.widthInChars	= XQ_MAX_CHAR_NAME;
	s_xqchar.playername_field.field.maxchars		= XQ_MAX_CHAR_NAME;



	s_xqchar.player.generic.type					= MTYPE_BITMAP;
	s_xqchar.player.generic.flags					= QMF_INACTIVE;
	s_xqchar.player.generic.ownerdraw				= XQCharCreator_DrawPlayer;
	s_xqchar.player.generic.x						= model_x;
	s_xqchar.player.generic.y						= model_y;
	s_xqchar.player.width							= 32*10;
	s_xqchar.player.height							= 56*10;


	s_xqchar.gender_text.generic.type				= MTYPE_TEXT;
	s_xqchar.gender_text.generic.flags				= QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_xqchar.gender_text.generic.x					= genderx;
	s_xqchar.gender_text.generic.y					= gendery;
	s_xqchar.gender_text.string						= selected_gender_lit;
	s_xqchar.gender_text.style						= UI_CENTER;
	s_xqchar.gender_text.color						= text_color_normal;

	s_xqchar.gender_arrows.generic.type				= MTYPE_BITMAP;
	s_xqchar.gender_arrows.generic.name				= MODEL_ARROWS;
	s_xqchar.gender_arrows.generic.flags			= QMF_INACTIVE;
	s_xqchar.gender_arrows.generic.x				= genderx-60;
	s_xqchar.gender_arrows.generic.y				= gendery+10;
	s_xqchar.gender_arrows.width					= arrow_width;
	s_xqchar.gender_arrows.height					= arrow_height;

	s_xqchar.gender_left.generic.type				= MTYPE_BITMAP;
	s_xqchar.gender_left.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_xqchar.gender_left.generic.callback			= XQCharCreator_MenuEvent;
	s_xqchar.gender_left.generic.id					= ID_PREVGENDER;
	s_xqchar.gender_left.generic.x					= genderx-60;
	s_xqchar.gender_left.generic.y					= gendery+10;
	s_xqchar.gender_left.width						= arrow_width / 2;
	s_xqchar.gender_left.height						= arrow_height;
	s_xqchar.gender_left.focuspic					= MODEL_ARROWSL;

	s_xqchar.gender_right.generic.type				= MTYPE_BITMAP;
	s_xqchar.gender_right.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_xqchar.gender_right.generic.callback			= XQCharCreator_MenuEvent;
	s_xqchar.gender_right.generic.id				= ID_NEXTGENDER;
	s_xqchar.gender_right.generic.x					= genderx+arrow_width / 2 - 60;
	s_xqchar.gender_right.generic.y					= gendery+10;
	s_xqchar.gender_right.width						= arrow_width / 2;
	s_xqchar.gender_right.height					= arrow_height;
	s_xqchar.gender_right.focuspic					= MODEL_ARROWSR;


	s_xqchar.face_text.generic.type					= MTYPE_TEXT;
	s_xqchar.face_text.generic.flags				= QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_xqchar.face_text.generic.x					= facex;
	s_xqchar.face_text.generic.y					= facey;
	s_xqchar.face_text.string						= "FACE";
	s_xqchar.face_text.style						= UI_CENTER;
	s_xqchar.face_text.color						= text_color_normal;

	s_xqchar.face_arrows.generic.type				= MTYPE_BITMAP;
	s_xqchar.face_arrows.generic.name				= MODEL_ARROWS;
	s_xqchar.face_arrows.generic.flags				= QMF_INACTIVE;
	s_xqchar.face_arrows.generic.x					= facex-60;
	s_xqchar.face_arrows.generic.y					= facey+10;
	s_xqchar.face_arrows.width						= arrow_width;
	s_xqchar.face_arrows.height						= arrow_height;

	s_xqchar.face_left.generic.type					= MTYPE_BITMAP;
	s_xqchar.face_left.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_xqchar.face_left.generic.callback				= XQCharCreator_MenuEvent;
	s_xqchar.face_left.generic.id					= ID_PREVFACE;
	s_xqchar.face_left.generic.x					= facex-60;
	s_xqchar.face_left.generic.y					= facey+10;
	s_xqchar.face_left.width						= arrow_width / 2;
	s_xqchar.face_left.height						= arrow_height;
	s_xqchar.face_left.focuspic						= MODEL_ARROWSL;

	s_xqchar.face_right.generic.type				= MTYPE_BITMAP;
	s_xqchar.face_right.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_xqchar.face_right.generic.callback			= XQCharCreator_MenuEvent;
	s_xqchar.face_right.generic.id					= ID_NEXTFACE;
	s_xqchar.face_right.generic.x					= facex+arrow_width / 2 - 60;
	s_xqchar.face_right.generic.y					= facey+10;
	s_xqchar.face_right.width						= arrow_width / 2;
	s_xqchar.face_right.height						= arrow_height;
	s_xqchar.face_right.focuspic					= MODEL_ARROWSR;


	s_xqchar.back.generic.type						= MTYPE_BITMAP;
	s_xqchar.back.generic.name						= MODEL_BACK0;
	s_xqchar.back.generic.flags						= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_xqchar.back.generic.callback					= XQCharCreator_MenuEvent;
	s_xqchar.back.generic.id						= ID_BACK;
	s_xqchar.back.generic.x							= 630;
	s_xqchar.back.generic.y							= 480-32;
	s_xqchar.back.width								= 64;
	s_xqchar.back.height							= 32;
	s_xqchar.back.focuspic							= MODEL_BACK1;

	s_xqchar.accept.generic.type					= MTYPE_BITMAP;
	s_xqchar.accept.generic.name					= MODEL_ACCEPT0;
	s_xqchar.accept.generic.flags					= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_xqchar.accept.generic.callback				= XQCharCreator_MenuEvent;
	s_xqchar.accept.generic.id						= ID_ACCEPT;
	s_xqchar.accept.generic.x						= 630;
	s_xqchar.accept.generic.y						= 480-64;
	s_xqchar.accept.width							= 64;
	s_xqchar.accept.height							= 32;
	s_xqchar.accept.focuspic						= MODEL_ACCEPT1;


	// Class buttons
	for (i = 0;  i < XQ_CLASSES;  i++) {
		s_xqchar.class_arrow[i].generic.type		= MTYPE_BITMAP;
		s_xqchar.class_arrow[i].generic.name		= MODEL_ARROWSR;
		s_xqchar.class_arrow[i].generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
		s_xqchar.class_arrow[i].generic.x			= classx - 20;
		s_xqchar.class_arrow[i].generic.y			= liney*(i + 1);
		s_xqchar.class_arrow[i].width				= 25;
		s_xqchar.class_arrow[i].height				= 33;

		s_xqchar.class[i].generic.type				= MTYPE_PTEXT;
		s_xqchar.class[i].generic.flags				= QMF_PULSEIFFOCUS;
		s_xqchar.class[i].generic.x					= classx;
		s_xqchar.class[i].generic.y					= liney*(i + 1);
		Q_strncpyz(class_lit[i], xq_class_lit(i, 1, 0), sizeof(class_lit[i]));
		s_xqchar.class[i].string					= class_lit[i];
		s_xqchar.class[i].style						= UI_LEFT;
		s_xqchar.class[i].color						= text_color_normal;
		s_xqchar.class[i].generic.id				= ID_CLASS + i;
		s_xqchar.class[i].generic.callback			= XQCharCreator_MenuEvent;

		Menu_AddItem(&s_xqchar.menu,	&s_xqchar.class_arrow[i]);
		Menu_AddItem(&s_xqchar.menu,	&s_xqchar.class[i]);
	}

	// Race buttons
	for (i = 0;  i < XQ_RACES;  i++) {
		s_xqchar.race_arrow[i].generic.type			= MTYPE_BITMAP;
		s_xqchar.race_arrow[i].generic.name			= MODEL_ARROWSR;
		s_xqchar.race_arrow[i].generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
		s_xqchar.race_arrow[i].generic.x			= racex - 20;
		s_xqchar.race_arrow[i].generic.y			= liney*(i + 1);
		s_xqchar.race_arrow[i].width				= 25;
		s_xqchar.race_arrow[i].height				= 33;

		s_xqchar.race[i].generic.type				= MTYPE_PTEXT;
		s_xqchar.race[i].generic.flags				= QMF_PULSEIFFOCUS;
		s_xqchar.race[i].generic.x					= racex;
		s_xqchar.race[i].generic.y					= liney*(i + 1);
		Q_strncpyz(race_lit[i], xq_race_lit(i, 1, 0), sizeof(race_lit[i]));
		s_xqchar.race[i].string						= race_lit[i];
		s_xqchar.race[i].style						= UI_LEFT;
		s_xqchar.race[i].color						= text_color_normal;
		s_xqchar.race[i].generic.id					= ID_RACE + i;
		s_xqchar.race[i].generic.callback			= XQCharCreator_MenuEvent;

		Menu_AddItem(&s_xqchar.menu,	&s_xqchar.race_arrow[i]);
		Menu_AddItem(&s_xqchar.menu,	&s_xqchar.race[i]);
	}

	// Banner text
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.banner);

	// Player name label and input
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.playername_label);
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.playername_field);


	// Player model preview
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.player);

	// Gender select arrows
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.gender_text);
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.gender_arrows);
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.gender_left);
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.gender_right);

	// Face select arrows
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.face_text);
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.face_arrows);
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.face_left);
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.face_right);

	// Back / Accept buttons
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.back);
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.accept);

	// Status line
	Menu_AddItem(&s_xqchar.menu,	&s_xqchar.status);


	XQCharCreator_UpdateModel();
	updateRaces();
	showArrows();
	trap_Cvar_Set("xq_charcreation_data", "");
}

void UI_XQCharCreatorMenu(char *status) {
	if (!status[0]) {
		XQCharCreator_MenuInit();
	}
	UI_PushMenu(&s_xqchar.menu);
	st(status);
}

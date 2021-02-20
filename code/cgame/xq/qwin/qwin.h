
#include "style.h"

// General QWIN limits
#define QW_MAX_WINDOWS					20		// How many windows can there be open AT ONCE
#define QW_SAVE_WINDOWS					30		// How many windows at most will have their positions go in the save cvar
#define QW_MAX_WID						40		// Max length of an internal, textual window id, eg: "inv" or "chat"
#define QW_MAX_WIN_TITLE				20		// A window's title (shown to user, eg: "Inventory") max length
#define QW_OBJ_PER_WIN					150		// Inventory window with tooltips on can be quite a bit of objects
#define QW_MAX_OBJ						QW_OBJ_PER_WIN * QW_MAX_WINDOWS + 1 // Max objs per window (obj index starts at 1)
#define QW_MAX_OBJ_SAVE					1000	// How many objects handles can be saved outside of a window context
#define QW_SMALLTEXT					20
#define QW_MEDTEXT						50


// Object types and some of their specific properties
#define QW_OBJ_NONE									0
#define QW_OBJ_PERCBAR								1
#define QW_OBJ_TEXT									2
#define		QW_OBJ_TEXT_MAX_TEXT_LEN				10240
#define QW_OBJ_INVSLOT								3
#define QW_OBJ_TOOLTIP								4
#define		QW_OBJ_TOOLTIP_MAX_TEXT_LEN				1024
#define		QW_OBJ_TOOLTIP_FRAMES					40
#define QW_OBJ_ITEMINSPECTOR						5
#define QW_OBJ_AMOUNTPICKER							6
#define		QW_OBJ_AMOUNTPICKER_TEXT_LEN 			9 		// Max string length that can be entered in an amount picker window
#define		QW_OBJ_AMOUNTPICKER_CMD_LEN  			20		// Max string length of the command to be executed upon submitting of the amount picker, eg: "swapmoney"
#define QW_OBJ_AUTOEQUIP							7
#define QW_OBJ_MONEY								8
#define QW_OBJ_BUTTON								9
#define 	QW_OBJ_BUTTON_ACTION_BUY				1
#define 	QW_OBJ_BUTTON_ACTION_SELL				2
#define 	QW_OBJ_BUTTON_ACTION_TRADE				3
#define 	QW_OBJ_BUTTON_ACTION_DESTROY			4
#define 	QW_OBJ_BUTTON_ACTION_PROMPT_YES			5
#define 	QW_OBJ_BUTTON_ACTION_PROMPT_NO			6
#define 	QW_OBJ_BUTTON_ACTION_COMBINE			7
#define 	QW_OBJ_BUTTON_ACTION_SPELLBOOK_SWITCH	8
#define 	QW_OBJ_BUTTON_ACTION_SKILL_CAPUP		9
#define 	QW_OBJ_BUTTON_ACTION_BOOK_SWITCH		15	
#define QW_OBJ_ITEMGFX								10
#define QW_OBJ_TIMEBAR								11
#define QW_OBJ_SPELL_GEM							12
#define QW_OBJ_SPELL_ICON							13
#define QW_OBJ_SPELLBOOK_SLOT						14


// Popup windows such as "Destroy this item?" have an action attached to them
enum qw_prompt_action_e {
	QW_PROMPT_ACTION_NONE,
	QW_PROMPT_ACTION_DESTROY,
	QW_PROMPT_ACTION_LOOT,
	QW_PROMPT_ACTION_DROP,
	QW_PROMPT_ACTION_SCRIBE,
	QW_PROMPT_ACTION_REZACCEPT,
	QW_PROMPT_ACTION_REZDECLINE,
	QW_PROMPT_ACTION_SKILLUP,
	QW_PROMPT_ACTION_SKILL_CAPUP
};


// A window cannot be resized to something impractical.
#define QW_WIN_MIN_WIDTH							10
#define QW_WIN_MIN_HEIGHT							10
#define QW_WIN_MAX_WIDTH							1600
#define QW_WIN_MAX_HEIGHT							800

// The chat window (the one with CLI commands typed in) has more restrictive limits.
#define QW_CHATWIN_MIN_WIDTH						320
#define QW_CHATWIN_MIN_HEIGHT						100


enum qw_objids {
	XQ_UI_OBJID_CHAR_NAME,
	XQ_UI_OBJID_CHAR_LEVEL,
	XQ_UI_OBJID_CHAR_CLASSRACE,
	QW_OBJID_GEN_RESIZE,
	XQ_UI_OBJID_INVSLOT_LEFTEAR,
	XQ_UI_OBJID_INVSLOT_RIGHTEAR,
	XQ_UI_OBJID_INVSLOT_HEAD,
	XQ_UI_OBJID_INVSLOT_FACE,
	XQ_UI_OBJID_INVSLOT_CHEST,
	XQ_UI_OBJID_INVSLOT_ARMS,
	XQ_UI_OBJID_INVSLOT_WAIST,
	XQ_UI_OBJID_INVSLOT_LEFTWRIST,
	XQ_UI_OBJID_INVSLOT_RIGHTWRIST,
	XQ_UI_OBJID_INVSLOT_LEGS,
	XQ_UI_OBJID_INVSLOT_HANDS,
	XQ_UI_OBJID_INVSLOT_FEET,
	XQ_UI_OBJID_INVSLOT_SHOULDERS,
	XQ_UI_OBJID_INVSLOT_BACK,
	XQ_UI_OBJID_INVSLOT_NECK,
	XQ_UI_OBJID_INVSLOT_CARRY1,
	XQ_UI_OBJID_INVSLOT_CARRY2,
	XQ_UI_OBJID_INVSLOT_CARRY3,
	XQ_UI_OBJID_INVSLOT_CARRY4,
	XQ_UI_OBJID_INVSLOT_CARRY5,
	XQ_UI_OBJID_INVSLOT_CARRY6,
	XQ_UI_OBJID_INVSLOT_LEFTFINGER,
	XQ_UI_OBJID_INVSLOT_RIGHTFINGER,
	XQ_UI_OBJID_INVSLOT_PRIMARY1,
	XQ_UI_OBJID_INVSLOT_SECONDARY1,
	XQ_UI_OBJID_INVSLOT_PRIMARY2,
	XQ_UI_OBJID_INVSLOT_PRIMARY3,
	XQ_UI_OBJID_INVSLOT_PRIMARY4,
	XQ_UI_OBJID_INVSLOT_PRIMARY5,
	XQ_UI_OBJID_INVSLOT_PRIMARY6,
	XQ_UI_OBJID_INVSLOT_PRIMARY7,
	XQ_UI_OBJID_INVSLOT_PRIMARY8,
	XQ_UI_OBJID_INVSLOT_PRIMARY9,
	XQ_UI_OBJID_BUTTON_BUY,
	XQ_UI_OBJID_BUTTON_SELL,
	XQ_UI_OBJID_MERCHANT_ITEMGFX,
	XQ_UI_OBJID_MERCHANT_ITEMTXT,
	XQ_UI_OBJID_BUTTON_TRADE_ME,
	XQ_UI_OBJID_BUTTON_TRADE_THEM,
	XQ_UI_OBJID_GROUP_MEMBER_1_NAME,
	XQ_UI_OBJID_GROUP_MEMBER_2_NAME,
	XQ_UI_OBJID_GROUP_MEMBER_3_NAME,
	XQ_UI_OBJID_GROUP_MEMBER_4_NAME,
	XQ_UI_OBJID_GROUP_MEMBER_5_NAME,
	XQ_UI_OBJID_GROUP_MEMBER_1_HP_PERC,
	XQ_UI_OBJID_GROUP_MEMBER_2_HP_PERC,
	XQ_UI_OBJID_GROUP_MEMBER_3_HP_PERC,
	XQ_UI_OBJID_GROUP_MEMBER_4_HP_PERC,
	XQ_UI_OBJID_GROUP_MEMBER_5_HP_PERC,
	XQ_UI_OBJID_SPELLBOOK_SWITCH,
	XQ_UI_OBJID_BUTTON_SPELLBOOK_PREV,
	XQ_UI_OBJID_BUTTON_SPELLBOOK_NEXT,
	XQ_UI_OBJID_BOOK_SWITCH,
	XQ_UI_OBJID_BUTTON_BOOK_PREV,
	XQ_UI_OBJID_BUTTON_BOOK_NEXT,
	XQ_UI_OBJID_BOOK_TEXT,
	XQ_UI_OBJID_SPFX_SLOT_1,
	XQ_UI_OBJID_SPFX_SLOT_2,
	XQ_UI_OBJID_SPFX_SLOT_3,
	XQ_UI_OBJID_SPFX_SLOT_4,
	XQ_UI_OBJID_SPFX_SLOT_5,
	XQ_UI_OBJID_SPFX_SLOT_6,
	XQ_UI_OBJID_SPFX_SLOT_7,
	XQ_UI_OBJID_SPFX_SLOT_8,
	XQ_UI_OBJID_SPFX_SLOT_9,
	XQ_UI_OBJID_SPFX_SLOT_10
};


typedef struct qw_obj_click_s {
    int objid;
    int actual_x;
    int actual_y;
} qw_obj_click_t;
typedef struct qw_obj_s {
	#define qw_obj_t_MAGIC							0x9afb327f
	int		magics;

	int		type;
	int		usecount;		// When the object is attached to a window, its usecount is incremented.

	// GENERIC

	int		w;
	int		h;
	int		color;
	void	(*onmousedown)(int win, int button, int obj, int x_in_obj, int y_in_obj, int shift, int ctrl, int alt);
	void	(*onmouseup)(void);
	char	*tooltip;
	int		multiplier;			// To avoid messing with floats, some values can be multiplied by this value (eg: xp perc bar).  0 means use raw value.
	int		hidden;				// Generally don't draw the object, clicks on it won't matter, etc
	int		hovered;			// Only one obj should have this set to 1 at any given time (the one which is hovered by the mouse).
	int		creation_ts;		// cg.time at the time of obj creation (actually, at the time qw_Objinit() is ran on it)
	int		clickthrough;		// If 1, don't grab mouse clicks.  Useful for being able to drag windows containing objects that fill the window completely (ie: ItemInspector)
	char	*hide_if_not_null;	// if not 0, contains a pointer to a char. If that char's value is not 0, do not display this object.  (Example: bigTarget window to display placeholder text)

	// PERCBAR, TOOLTIP, BUTTON
	int		bordercolor;



	// PERCBAR, TOOLTIP, BUTTON, INVSLOT, TIMEBAR
	int		filledcolor;

	// PERCBAR, TIMEBAR
	int		barwidth;
	int		barheight;
	int		emptycolor;

	// PERCBAR, SPELL_GEM, BUTTON
	int		borderwidth;

	// PERCBAR
	int		*perc;
	int		notches;
	int		smallnotchcolor;
	int		bignotchcolor;
	int		fastbar;			// if not 0, draws 2ndary bar going that times faster than regular bar.
	int		fastbarheight;		// if fastbar is on, this sets the fastbar height. If this is not set, fastbar height will be 1.
	int		fastbarcolor;
	int		disabledcolor;
	int		gradientcolor;		// if not 0, the perc bar will be filled with a verticall gradient from filledcolor to gradientcolor
	int		gradientmirror;		// if 1, fade in and fade out the gradient, peak at the center. Otherwise just fade in 0 > 100


	// PERCBAR, TEXT
	float	coloralpha;			// if 0, fill alpha will be 1. If not 0, it will be that value.


	// TEXT
	int		group_leader;		// If it's 1, the group member is the group leader and will have the (L) mark
	float	colorvec;			// if (coloralpha isn't 0, this will be used to colorize the text)
	int		charsize;			// 0: tinychar, 1: smallchar, 2: bigchar, 3: giantchar

	// TIMEBAR
	int		tsecs;				// How many tenths of second should the timebar last for

	// TEXT, BUTTON, TOOLTIP
	char		*text;
	char		text_static[QW_OBJ_TEXT_MAX_TEXT_LEN+1];
	uint64_t	text_itemname_invslot;	// if not 0, we're displaying the name of the item in that inv slot (if not empty)

	// TEXT, BUTTON
	char	capitalize;			// If 1, will capitalize 1st letter of the string

	// INVSLOT, SPELL_GEM, SPELL_ICON
	int		slot;



	// INVSLOT
	int64_t	container_id;
	int		container_slot;
	int		selected;
	int		parent_slot;

	// ITEMINSPECTOR, ITEMGFX
	int64_t	item;


	// ITEMINSPECTOR
	int		title_window;
	int		spell;


	// AMOUNTPICKER, BUTTON
	int		num_minval;
	int		num_maxval;
	char 	cmd[QW_OBJ_AMOUNTPICKER_CMD_LEN+1];
	int64_t	arg1;
	int64_t	arg2;
	int64_t	arg3;
	int64_t	arg4;

	// MONEY
	int		money_type;
	int		money_source;

	// BUTTON
	void	(*onclick)(int win, int button, int obj, int x_in_obj, int y_in_obj, int shift, int ctrl, int alt);
	int		action;
	int		padding_x;
	int		padding_y;
	char	bitmap[MAX_QPATH+1];
	int		bitmap_w;
	int		bitmap_h;
	int		nobgfill; // For bitmap buttons, set it to 1

	// INVSLOT, BUTTON, GEM_SPELL
	int		hoverbgcolor;	// 0 means change current bgcolor slightly.  1 means don't change bgcolor when hovering at all.


	int		magice;
} qw_obj_t;
typedef struct qw_obj_attach_s {
	int num;
	int x;
	int y;
	int	actual_x; // For centered_x or negative x, compute actual x.
	int	actual_y; // same for y.
	int	x_centered;
	int	y_centered;
	int zindex;
} qw_obj_attach_t;
typedef struct qw_window_save_s {
	char	wid[QW_MAX_WID+1];
	int		x;
	int		y;
	int		w;
	int		h;
} qw_window_save_t;
typedef struct qw_window_s {
#define qw_window_t_MAGIC			0xf9a583f3
	int					magics;

	char 				wid[QW_MAX_WID+1];
	char 				title[QW_MAX_WIN_TITLE+1];
	float				x;
	float 				y;
	float 				h;
	float 				w;
	int					no_drag;
	int					resizable;
	int					no_title;
	int					no_delete;
	int					closex;
	void				(*closefnc)(void *);
	int64_t				closefarg;
	int					zindex;
	int					brd_col;
	int					bg_col;
	qhandle_t			bg_shdr;
	float				alpha;
	float				alpha_min;
	float				alpha_max;
	float				alpha_dir;
	int					no_save;
	qw_obj_attach_t 	obj[QW_OBJ_PER_WIN];

	int					magice;
} qw_window_t;

// Strings such as "Empty slot" or "Previous page" in the qws->strings struct
#define QW_MAX_STRING_LEN 100
typedef char qw_string_t[QW_MAX_STRING_LEN];
typedef struct qw_strings_s {
	qw_string_t emptyslot;
	qw_string_t spellbook;
	qw_string_t nextpage;
	qw_string_t previouspage;
	qw_string_t autoequip;
	qw_string_t scribeTheSpell;
	qw_string_t combine;
	qw_string_t lootTheNodropItem;
} qw_strings_t;

typedef struct qw_state_t {
	#define qw_t_MAGIC					0x49f917df
	int				magics;
	qw_window_t 	win[QW_MAX_WINDOWS];
	qw_obj_t		obj[QW_MAX_OBJ];
	qw_strings_t	strings;
	int				objhandles[QW_MAX_OBJ_SAVE];		// When we define an object, say, character name, we MAY save its handle to this array.
														// We might need this handle again, eg: we want to attach this object to another window
														// for example, char name will appear in the "smallBars" window and will be attached to the "inv"
														// window whenever it's created. (An object can be attached to 0, 1 or more windows).
														// Objects whose handle is saved to this array won't be destroyed when the last window to which they are
														// attached is deleted.

	int				window_moving;						// Used for window moving.  Contains the numeric id of window currently being moved or -1
	int				window_resizing;					// Same for resizing.
	int				window_change_old_mouse_x;			// 
	int				window_change_old_mouse_y;			// 
	int				window_change_init_x;				// X coord of the mouse INSIDE the dragged/resized window.
	int				window_change_init_y;				// Y coord of the mouse INSIDE the dragged/resized window.
	int				obj_clicked_id;						// Is -1 or the (window internal) number of the object that has been given onmousedown to,
														// but onemouseup hasn't been triggered yet.
	int				obj_clicked_win;					// Window num for the above object or -1

	int				amount_picker_value;				// Holds the current value of the amount picker.
	int				amount_picker_running;				// 1 if we have an amount picker open (so the keystrokes go to it)
	int				amount_picker_objnum;				// Holds the object number of the currently running amount picker.

	int				looting;							// 1 if we must have loot window up.  0 otherwise.
	int				banking;							// 1 if we must have bank window up.  0 otherwise.
	int				buying;								// 1 if we must have merchant window up.  0 otherwise.
	int				trading;							// 1 if we must have trade window up.  0 otherwise.
	int				spellbook_page;						// page number if we have spell book up. 0 otherwise.
	int				current_book_page;					// display page number of the book we're reading - this is handled 100% on the client
	int				zoned_protected;					// 1 if we are protected for a certain amount of time because we just zoned in
														// a zone that has that protection.  Prevents casting, attacking and protects from PvP attacks.

	char			spellbook_page_txt[20];				// Textual representation of spell book page, such as "Page 5".

	int				spell_gem_tooltips[XQ_SPELL_SLOTS];	// array of objects for spell gem tooltips popping up when ALT is held

	int				mouse_spell;						// ID of the spell we clicked on in the spell book (mouse transforms into that spell icon) so we can mem it.
	int				swap_spell;							// Position of the spell in the spellbook we have selected for switching with right click, 0 if none.
	struct			{
		int				curval;
		int				goalval;
	} gem_fade[XQ_SPELL_SLOTS];							// Keeps current fade-in/fade-out status for spell gems


	int 			magice;
} qw_state_t;


extern qw_state_t *qws;


// engine.c
void				qw_Init(void);
void				qw_AttachWithBorder(qw_window_t *win, qw_obj_attach_t *attach, int *x, int *y);
void				qw_Render(void);
int					qw_MouseClick(int button, int down, int shift, int ctrl, int alt);
void				qw_MouseMove(void);

// frame.c
void				qw_Frame(void);

// window.c
void				qw_WinAlpha(char *wid);
int					qw_WindowFindXY(int x, int y, int *topzi);
void				qw_WindowSnap(qw_window_t *win);
int					qw_WindowExists(const char *wid);
void				qw_WinInfo(char *wname);
int					qw_WindowZICompareFnc(const void *a, const void *b);
void				qw_WindowDraw(int wid);
void				qw_ResizeStart(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt);
void				qw_ResizeStop(void);
void				qw_SaveWindows(void);
int					qw_WindowDeleteHighest(void);
void				qw_WindowTitle(int win, char *title);
void				qw_WindowResize(void);
void				qw_WindowMove(void);
void				qw_WindowDelete(const char *wid);
int					qw_WinNew(qw_window_t p);
int					qw_TopZI(void);


// object.c
qw_obj_click_t *	qw_ObjectFindXY(int win, int mx, int my, int skip_clickthrough);
int					qw_ObjZICompareFnc(const void *a, const void *b);
void				qw_ObjDraw(qw_window_t *win, int attachnum);
void				qw_ObjDetach(int win, int oid);
void				qw_ObjDetachFromAll(int oid);
void				qw_ObjInit(qw_obj_t *obj, int class);
int					qw_ObjAttach(char *wid, int oid, int x, int y, int x_centered, int y_centered);
int					qw_ObjCreate(qw_obj_t *tmpobj);

// util.c
void				qw_TooltipCloseAll(void);
int					qw_Hover(qw_obj_t *obj);

// prompt.c
void				qw_Prompt(char *text, int actionyes, int actionno, int64_t arg1, int64_t arg2, int64_t arg3);                     




// objtypes/money.c
int					qw_Obj_Money_GetAmount(qw_obj_t *obj);
void				qw_Obj_Money_Draw(qw_window_t *win, qw_obj_attach_t *att);
void				qw_Obj_Money_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt);

// objtypes/amountpicker.c
void				qw_Obj_AmountPicker(int open, int preset, int minval, int maxval, int x, int y, char *callback, int64_t arg1,
						int64_t arg2, int64_t arg3);
void				qw_Obj_AmountPicker_Reset(void);
void				qw_Obj_AmountPicker_Keypress(int key);
void				qw_Obj_AmountPicker_Draw(qw_window_t *win, qw_obj_attach_t *att);

// objtypes/iteminspector.c
void				qw_Obj_ItemInspector_Draw(qw_window_t *win, qw_obj_attach_t *att);

// objtypes/invslot.c
void				qw_Obj_InvSlot_Draw(qw_window_t *win, qw_obj_attach_t *att);
void				qw_Obj_InvSlot_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt);

// objtypes/itemgfx.c
void				qw_Obj_ItemGFX_Draw(qw_window_t *win, qw_obj_attach_t *att);

// objtypes/timebar.c
void				qw_Obj_TimeBar_Draw(qw_window_t *win, qw_obj_attach_t *att);

// objtypes/percbar.c
void				qw_Obj_PercBar_Draw(qw_window_t *win, qw_obj_attach_t *att);


// objtypes/button.c
void				qw_Obj_Button_Draw(qw_window_t *win, qw_obj_attach_t *att);
void				qw_Obj_Button_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt);

// objtypes/text.c
void				qw_Obj_Text_Draw(qw_window_t *win, qw_obj_attach_t *att);

// objtypes/tooltip.c
void				qw_Obj_ToolTip_Draw(qw_window_t *win, qw_obj_attach_t *att);

// objtypes/spellbookslot.c
void				qw_Obj_SpellBookSlot_Draw(qw_window_t *win, qw_obj_attach_t *att);
void				qw_Obj_SpellBookSlot_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt);


// objtypes/spellicon.c
void				qw_Obj_SpellIcon_Draw(qw_window_t *win, qw_obj_attach_t *att);
void				qw_Obj_SpellIcon_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt);


// objtypes/spellgem.c
void				qw_Obj_SpellGem_Draw(qw_window_t *win, qw_obj_attach_t *att);
void				qw_Obj_SpellGem_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt);


// objtypes/autoequip.c
void				qw_Obj_AutoEquip_Draw(qw_window_t *win, qw_obj_attach_t *att);
void				qw_Obj_AutoEquip_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt);



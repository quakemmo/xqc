#include "qwin/qwin.h"

// Mob animation / sounds
#define XQ_IDLE_VARIETY_DELAY				30000	// The lower this is, the more often NPC's will play their alternative idle animation/sound
#define XQ_SOUND_FOOTSTEP_MIN_DELAY			200		// Minimum time before footstep sounds for a particular mob
#define XQ_SOUND_PAIN_MIN_DELAY				2000	// Same for pain sound
#define XQ_ANIM_AGE							1000	// If a mob anim request is older than that (cg.time), it won't be played at all
													// If mobs seem to be skipping important anims like casting, this may be increased
#define XQ_ITEM								1
#define XQ_SPELL							2
#define XQ_INFO								3

#define XQ_BOOK_PAGES						20
#define XQ_WEAPON_SELECTOR_H2H				401
#define XQ_MAX_UTILMODELS					100     // How many utilmodels can be registered 
#define XQ_LOW_ENERGY_WARNING				2		// If the player has less than that many shots of current weapon worth of energy, they will get a low energy warning

#define XQ_APP_PATH							"models/app"
#define SOUNDH(x)							(trap_S_RegisterSound(x, qtrue))

typedef struct xq_shader_s {
	int					model_id;
	xq_body_part_t		body_part;
	int					texture_num;
	qhandle_t			shader;
	int					last_used_ts;
	int					tint;
} xq_shader_t;
typedef struct xq_utilmodel_s {
	char 		name[MAX_QPATH+1];
	qhandle_t	qhandle;
} xq_utilmodel_t;


#define XQ_PFX_PER_BEING					5		// How many spell particle effects (of each kind, author or receiver) can a being (npc or pc) have active on them at any given time
typedef struct xq_particle_status_s {
	int			type;			//	0 = none; 1 = author, 2 = receiver.  Author means hands will be the origin, receiver means torso will be.
	uint64_t	start_ts;		//	xq_msec() of start request received from the server, either through a "start_particle" command (for the receiver effects)
								//	or through xq_pfx_author_current (for the author effects)
	int			pfx;			//	The actual pfx (0 = none)
	uint64_t	end_ts;			//	xq_msec() time this anim ended - we will still play it for a bit afterwards.
	int			sound;			//	whatever sound we're supposed to play.  For author (type 1) pfx it will be looping.  For receiver (type 2), not looping.
	int			casting_hash;	//  This is needed to make sure that if a spell is interrupted then immediately recasted, we know we have to restart the sound / pfx stuff
} xq_particle_status_t;

#define XQ_MAX_ITEM_MODELS					1000
typedef struct xq_cgame_state_s {
	#define 		xq_cgame_state_t_MAGIC			0x9381f9ad
	int				magics;	

	int 			mousex;
	int 			mousey;
	int 			mousefree;

	int				screenw;
	int				screenh;

	int				frames;								// How many xq_Frame() calls have been done so far.  Useful for timing stuff to appear with a delay after our spawn.

	int 			ui_hp_perc;							// My HP percentage.  Keeping this up to date for UI HP bar.
	int 			ui_mana_perc;						// Same for my mana.
	int 			ui_endurance_perc;					// Same for my endurance.
	int 			ui_energy_perc;						// Same for my energy.
	char 			ui_charname[XQ_MAX_NAME];
	char 			ui_level[QW_SMALLTEXT+1];
	char 			ui_classrace[QW_MEDTEXT+1];
	char 			ui_hptxt[QW_SMALLTEXT+1];						// hp / hp_max textual HP representation
	char 			ui_hponlytxt[QW_SMALLTEXT+1];					// hp textual HP representation
	char 			ui_manaonlytxt[QW_SMALLTEXT+1];					// same for mana
	char 			ui_enduranceonlytxt[QW_SMALLTEXT+1];			// same for endurance
	char 			ui_energyonlytxt[QW_SMALLTEXT+1];				// same for energy
	char			ui_target_name[XQ_MAX_NAME];
	int				ui_target_hp_perc;
	int				ui_xp_perc;
	int				ui_copper;
	int				ui_silver;
	int				ui_gold;
	int				ui_platinum;
	char			ui_strtxt[QW_SMALLTEXT+1];
	char			ui_weighttxt[QW_SMALLTEXT+1];
	char			ui_mrtxt[QW_SMALLTEXT+1];
	char			ui_frtxt[QW_SMALLTEXT+1];
	char			ui_crtxt[QW_SMALLTEXT+1];
	char			ui_prtxt[QW_SMALLTEXT+1];
	char			ui_drtxt[QW_SMALLTEXT+1];
	char			ui_psrtxt[QW_SMALLTEXT+1];
	char			ui_actxt[QW_SMALLTEXT+1];
	char			ui_atktxt[QW_SMALLTEXT+1];

	int 			ui_state_inv_open;

	int				ui_chat_window_x;
	int				ui_chat_window_y;
	int				ui_chat_window_w;
	int				ui_chat_window_h;




	xq_utilmodel_t	utilmodels[XQ_MAX_UTILMODELS];
	int				utilmodels_registered;

	char			ui_group_member_1_name[XQ_MAX_CHAR_NAME+1];
	char			ui_group_member_2_name[XQ_MAX_CHAR_NAME+1];
	char			ui_group_member_3_name[XQ_MAX_CHAR_NAME+1];
	char			ui_group_member_4_name[XQ_MAX_CHAR_NAME+1];
	char			ui_group_member_5_name[XQ_MAX_CHAR_NAME+1];
	int				ui_group_member_1_hp_perc;
	int				ui_group_member_2_hp_perc;
	int				ui_group_member_3_hp_perc;
	int				ui_group_member_4_hp_perc;
	int				ui_group_member_5_hp_perc;

	int				memmed_spells[XQ_SPELL_SLOTS];				// spell gems
	int				memmed_spells_down[XQ_SPELL_SLOTS];			// spell gems state
	int				memmed_spells_prev[XQ_SPELL_SLOTS];			// spell gems in previous client frame
	int				memmed_spells_down_prev[XQ_SPELL_SLOTS];	// spell gems state in previous client frame
	int				casting_down;
	int				casting_down_prev;

	int				show_gem_tooltips;

	int				spfx[XQ_SPFX_SLOTS];
	int				spfx_remain[XQ_SPFX_SLOTS];
	int				spfx_hash[XQ_SPFX_SLOTS];
	char			spfx_tooltips[XQ_SPFX_SLOTS][XQ_SPELL_MAX_NAME + 10];

	char			ui_skill_tailoring[QW_MEDTEXT+1];
	char			ui_skill_cooking[QW_MEDTEXT+1];
	char			ui_skill_blacksmithing[QW_MEDTEXT+1];
	char			ui_skill_swimming[QW_MEDTEXT+1];
	char			ui_skill_channelling[QW_MEDTEXT+1];
	char			ui_skill_melee[QW_MEDTEXT+1];
	char			ui_skill_fire[QW_MEDTEXT+1];
	char			ui_skill_cold[QW_MEDTEXT+1];
	char			ui_skill_poison[QW_MEDTEXT+1];
	char			ui_skill_disease[QW_MEDTEXT+1];
	char			ui_skill_psy[QW_MEDTEXT+1];
	char			ui_skill_h2h[QW_MEDTEXT+1];
	char			ui_skill_w2[QW_MEDTEXT+1];
	char			ui_skill_w3[QW_MEDTEXT+1];
	char			ui_skill_w4[QW_MEDTEXT+1];
	char			ui_skill_w5[QW_MEDTEXT+1];
	char			ui_skill_w6[QW_MEDTEXT+1];
	char			ui_skill_w7[QW_MEDTEXT+1];
	char			ui_skill_w8[QW_MEDTEXT+1];
	char			ui_skill_w9[QW_MEDTEXT+1];

	char			ui_skill_capups[QW_MEDTEXT+1];

	int				autocast_gem;					// if this is not 0, that spell gem will be attempted to be casted as soon as it's up (used for bard songs)
	int				autocast_gem_change_ts;			// cg.time of the last time autocast_gem has been changed (used so bards can manually recast songs quicker than through autocast)

	int				rez_box;						// 1 if it's up, 0 otherwise.
	xq_particle_status_t	pfx[MAX_GENTITIES][XQ_PFX_PER_BEING]; // Spell particle effects (PFX) currently being active for beings

	qhandle_t		item_models[XQ_MAX_ITEM_MODELS]; // Model displayed for an item which is held in hand or dropped on the ground
	int				item_models_registered;

	int				magice;
} xq_cgame_state_t;

extern xq_cmdCookie_t *xq_CmdCookies;


extern xq_model_t *xq_cmodels;
extern xq_shader_t *xq_modelshader_cache;
extern xq_cgame_state_t *xqst;


// frame.c
void	xq_Frame(void);

// mouse.c
void	xq_RightClickScene(int down);
void	xq_DrawMouse(void);
void	xq_MouseMove(int x, int y);
int		CG_ScanForMouseEntity(void);


// key.c
void	xq_KeyEvent(int key, unsigned time, qboolean down, int shift, int ctrl, int alt);
int		xq_KeyStatus(int key);
int		xq_ShiftPressed(void);
int		xq_AltPressed(void);
int		xq_CtrlPressed(void);


// info.c
void	xq_CacheDelete(int type);
void	xq_ClearItemCache(int64_t item);
void	xq_CacheClear(int type, int64_t what1, int64_t what2, int64_t what3, int64_t what4);
void	xq_SetCache(uint64_t ptr, int type, int action);
void	*xq_GetCache(int type, int64_t what1, int64_t what2, int64_t what3, int64_t what4);
void	xq_localcmd_cacheclear(int argc, char *param1, char *param2, char *param3, char *param4, char *param5);
void	xq_localcmd_cacheinfo(int type);
void	xq_Info_CheckMagics(void);
void	xq_Info_Init(void);
void	xq_Info_Deinit(void);
xq_item_t	*xq_ItemInfo(int64_t itemid);
xq_spell_t	*xq_SpellInfo(int spellid);
xq_info_t	*xq_InfoInfo(int id1, int id2, int id3, int id4);

uint64_t xq_ItemInfoAsk(void);
uint64_t xq_SpellInfoAsk(void);
uint64_t xq_InfoInfoAsk(void);

// ping.c
void	xq_ping(void);

// target.c
void	xq_SetTarget(int entnum, int consider);
char	*xq_TargetName(void);
void	xq_FindTarget(int key, int down, int shift, int ctrl, int alt);
void	xq_Target_Self(void);
void	xq_Target_Group2(void);
void	xq_Target_Group3(void);
void	xq_Target_Group4(void);
void	xq_Target_Group5(void);
void	xq_Target_Group6(void);
qboolean	xq_Target_CanChange(void);

// localcmd.c
int		xq_localcmd(int64_t arg);

// 3d.c
int		xq_emphasize_model(entityState_t *es);

// log.c
void	xq_serverlog(const char *buf);
void	xq_clog(int color, char *fmt, ...)  __attribute__ ((format (gnu_printf, 2, 3)));

// camera.c
void	xq_camera_MouseZoom(qboolean dir);
void	xq_camera_CycleView(qboolean reset);
void	xq_camera_frame(void);
void	xq_Camera_ZoomIn(void);
void	xq_Camera_ZoomOut(void);

// chat.c
void	xq_chat_init(void);
void	xq_chat_add(char *txt, int color);
void	xq_chat_display(void);

// debdisp.c
void	xq_debdisp(int col, int64_t remove_ts, char *fmt, ...)  __attribute__ ((format (gnu_printf, 3, 4)));
void	xq_debdisp_draw(void);
void	xq_debdisp_init(void);

// ui/ui.c
void	xqui_Init(void);
void	xqui_Frame(void);
void	xqui_RezBox(void);
void	xqui_StartCastingBar(int time);
void	xqui_StopCastingBar(void);
void	xqui_ReadBook(int64_t item);
void	xqui_InvSlotUnselectAll(void);
void	xqui_BookPageRefresh(void);
void	xqui_BookButtons(void);
void	xqui_Skills(void);
void	xqui_OpenInspector(int64_t item, int spell);
void	xqui_SpellBook(void);
void	xqui_SpellBook_Toggle(void);
void	xqui_ContainerOpen(int slot, int silent);
void	xqui_ContainerClose(int slot, int silent);
void	xqui_Container_GetWID(int slot, char *buf);
void	xqui_CloseTradeContainers(void);


void	xqui_Inventory(int open);
void	xqui_Inventory_Toggle(void);
void	xqui_LootOpen(void);
void	xqui_LootClose(void *win);
void	xqui_TradeOpen(void);
void	xqui_TradeClose(void *win);
void	xqui_MerchantOpen(void);
void	xqui_MerchantClose(void *win);
void	xqui_BankOpen(void);
void	xqui_BankClose(void *win);
void	xqui_Bags(void);
void	xqui_Tooltips_On(void);
void	xqui_Tooltips_Off(void);

// ui/var2win.c
void	xqui_VarsToWindows(void);

// init.c
void	xq_Init(void);
void	xq_Deinit(void);
void	xq_MediaInit(void);

// item.c
void	xq_DrawItemIcon(qw_window_t *win, int x, int y, int icon, int amount);
int64_t	xq_InvSlotItem(int slot, int64_t container_id, int container_slot);
int64_t	xq_InvSlotItemArena(int slot);

// spell.c
void	xq_DrawSpellIcon(int x, int y, int icon);
void	xq_Spell_Frame(playerState_t *ps);
int		xq_CanCastGem(int slot);
void	xq_CastGem(int slot);

// srvcmd.c
void	xq_Hail(void);
void	xq_Sit(void);
void	xq_Consider(void);
void	xq_RunSlot(int slot);
void	xq_SwapWithMouseSlot(int64_t slot, int64_t container_id, int64_t container_slot, int64_t amount, int64_t invoiceonly);
void	xq_scmd(char *cmd);

// util.c
void	xq_RefreshAllItems(void);
void	xq_OptSound(vec3_t origin, int entnum, int chan, char *name, sfxHandle_t handle);
float	*xq_Color2Vec(int color);
float	*xq_AlphaPulse(int col, int speed);
void	xq_DrawZoningText(void);
int		xq_WeaponActiveSlot(void);
int		xq_cnum2cname(char *cname);
qhandle_t	xq_UtilModel(char *name);
qhandle_t	xq_GfxShader(char *name, char *dfl);
qboolean	xq_1second(void);
void	xq_CmdCookie_Add(int cookie);

// scoreboard.c
qboolean	xq_scoreboard(void);


void xq_process_items(void);


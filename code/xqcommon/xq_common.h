
#define XQ_JSON_OK							0
#define XQ_JSON_UNKNOWN_SERVER_ERROR		1


#define XQ_MAX_PASSWORD_LEN					40
#define XQ_MAX_CHARS_PER_ACCOUNT			8
#define XQ_MIN_CHAR_NAME					5
#define XQ_MAX_CHAR_NAME					12 // Increasing this and the surname requires expanding xq_EntName()
#define XQ_MAX_CHAR_SURNAME					18
#define XQ_MAX_GUILD_LEN					20
#define XQ_MAX_FLOAT_TEXT_LEN				XQ_MAX_CHAR_NAME + XQ_MAX_CHAR_SURNAME + XQ_MAX_GUILD_LEN + 4 // spaces and <>
#define XQ_MAX_NPC_NAME						32 // Increasing this requires to change code in xq_EntName()

#define XQ_MAX_NAME							(XQ_MAX_CHAR_NAME > XQ_MAX_NPC_NAME ? XQ_MAX_CHAR_NAME : XQ_MAX_NPC_NAME)
#define XQ_MAX_USERINFO_LEN					100
#define XQ_MAX_USERINFO_KEY_LEN				10
#define XQ_MAX_USERINFO_VAL_LEN				20

#define	XQ_MAX_INPUT_LINE_LEN				120	// Maximum length of character typed text.

#define	XQ_ENDURANCE_PER_JUMP				10

#define XQ_ANIM_RUN_THRESHOLD_PLAYER		100 // slower speeds will make players look like they are walking
#define XQ_ANIM_RUN_THRESHOLD_NPC			100 // slower speeds will make mobs look like they are walking

#define XQ_ITEM_ERROR						-1000000000


#define XQ_ITEM_SLOT_PRIMARY				(1)
#define XQ_ITEM_SLOT_SECONDARY				(1 << 1)
#define XQ_ITEM_SLOT_LEFTEAR				(1 << 2)
#define XQ_ITEM_SLOT_RIGHTEAR				(1 << 3)
#define XQ_ITEM_SLOT_HEAD					(1 << 4)
#define XQ_ITEM_SLOT_FACE					(1 << 5)
#define XQ_ITEM_SLOT_CHEST					(1 << 6)
#define XQ_ITEM_SLOT_ARMS					(1 << 7)
#define XQ_ITEM_SLOT_WAIST					(1 << 8)
#define XQ_ITEM_SLOT_LEFTWRIST				(1 << 9)
#define XQ_ITEM_SLOT_RIGHTWRIST				(1 << 10)
#define XQ_ITEM_SLOT_LEGS					(1 << 11)
#define XQ_ITEM_SLOT_HANDS					(1 << 12)
#define XQ_ITEM_SLOT_LEFTFINGER				(1 << 13)
#define XQ_ITEM_SLOT_RIGHTFINGER			(1 << 14)
#define XQ_ITEM_SLOT_FEET					(1 << 15)
#define XQ_ITEM_SLOT_SHOULDERS				(1 << 16)
#define XQ_ITEM_SLOT_BACK					(1 << 17)
#define XQ_ITEM_SLOT_NECK					(1 << 18)
#define XQ_ITEM_SLOT_SECONDARY1				((1 << 21) + 1)
#define XQ_ITEM_SLOT_PRIMARY1				((1 << 22) + 1)
#define XQ_ITEM_SLOT_PRIMARY2				((1 << 22) + 2)
#define XQ_ITEM_SLOT_PRIMARY3				((1 << 22) + 3)
#define XQ_ITEM_SLOT_PRIMARY4				((1 << 22) + 4)
#define XQ_ITEM_SLOT_PRIMARY5				((1 << 22) + 5)
#define XQ_ITEM_SLOT_PRIMARY6				((1 << 22) + 6)
#define XQ_ITEM_SLOT_PRIMARY7				((1 << 22) + 7)
#define XQ_ITEM_SLOT_PRIMARY8				((1 << 22) + 8)
#define XQ_ITEM_SLOT_PRIMARY9				((1 << 22) + 9)

#define XQ_ITEM_SLOT_MOUSE1					((1 << 29) + 1)	// 536870913
#define XQ_ITEM_SLOT_MOUSE2					((1 << 29) + 2)
#define XQ_ITEM_SLOT_MOUSE3					((1 << 29) + 3)
#define XQ_ITEM_SLOT_MOUSE4					((1 << 29) + 4)
#define XQ_ITEM_SLOT_MOUSE5					((1 << 29) + 5)
#define XQ_ITEM_SLOT_MOUSE6					((1 << 29) + 6)
#define XQ_ITEM_SLOT_MOUSE7					((1 << 29) + 7)
#define XQ_ITEM_SLOT_MOUSE8					((1 << 29) + 8)

#define XQ_ITEM_SLOT_CARRY1					((1 << 28) + 1)
#define XQ_ITEM_SLOT_CARRY2					((1 << 28) + 2)
#define XQ_ITEM_SLOT_CARRY3					((1 << 28) + 3)
#define XQ_ITEM_SLOT_CARRY4					((1 << 28) + 4)
#define XQ_ITEM_SLOT_CARRY5					((1 << 28) + 5)
#define XQ_ITEM_SLOT_CARRY6					((1 << 28) + 6)
#define XQ_ITEM_SLOT_INCONTAINER			((1 << 27)) 	// 134217728

#define XQ_ITEM_SLOT_LOOT1					((1 << 26) + 1)
#define XQ_ITEM_SLOT_LOOT2					((1 << 26) + 2)
#define XQ_ITEM_SLOT_LOOT3					((1 << 26) + 3)
#define XQ_ITEM_SLOT_LOOT4					((1 << 26) + 4)
#define XQ_ITEM_SLOT_LOOT5					((1 << 26) + 5)
#define XQ_ITEM_SLOT_LOOT6					((1 << 26) + 6)
#define XQ_ITEM_SLOT_LOOT7					((1 << 26) + 7)
#define XQ_ITEM_SLOT_LOOT8					((1 << 26) + 8)
#define XQ_ITEM_SLOT_LOOT9					((1 << 26) + 9)
#define XQ_ITEM_SLOT_LOOT10					((1 << 26) + 10)
#define XQ_ITEM_SLOT_LOOT11					((1 << 26) + 11)
#define XQ_ITEM_SLOT_LOOT12					((1 << 26) + 12)
#define XQ_ITEM_SLOT_LOOT13					((1 << 26) + 13)
#define XQ_ITEM_SLOT_LOOT14					((1 << 26) + 14)
#define XQ_ITEM_SLOT_LOOT15					((1 << 26) + 15)
#define XQ_ITEM_SLOT_LOOT16					((1 << 26) + 16)
#define XQ_ITEM_SLOT_LOOT17					((1 << 26) + 17)
#define XQ_ITEM_SLOT_LOOT18					((1 << 26) + 18)
#define XQ_ITEM_SLOT_LOOT19					((1 << 26) + 19)
#define XQ_ITEM_SLOT_LOOT20					((1 << 26) + 20)
#define XQ_ITEM_SLOT_LOOT21					((1 << 26) + 21)
#define XQ_ITEM_SLOT_LOOT22					((1 << 26) + 22)
#define XQ_ITEM_SLOT_LOOT23					((1 << 26) + 23)
#define XQ_ITEM_SLOT_LOOT24					((1 << 26) + 24)
#define XQ_ITEM_SLOT_LOOT25					((1 << 26) + 25)
#define XQ_ITEM_SLOT_LOOT26					((1 << 26) + 26)
#define XQ_ITEM_SLOT_LOOT27					((1 << 26) + 27)
#define XQ_ITEM_SLOT_LOOT28					((1 << 26) + 28)
#define XQ_ITEM_SLOT_LOOT29					((1 << 26) + 29)
#define XQ_ITEM_SLOT_LOOT30					((1 << 26) + 30)
#define XQ_ITEM_SLOT_LOOT31					((1 << 26) + 31)
#define XQ_ITEM_SLOT_LOOT32					((1 << 26) + 32)
#define XQ_ITEM_SLOT_LOOT33					((1 << 26) + 33)
#define XQ_ITEM_SLOT_LOOT34					((1 << 26) + 34)
#define XQ_ITEM_SLOT_LOOT35					((1 << 26) + 35)
#define XQ_ITEM_SLOT_LOOT36					((1 << 26) + 36)
#define XQ_ITEM_SLOT_LOOT37					((1 << 26) + 37)
#define XQ_ITEM_SLOT_LOOT38					((1 << 26) + 38)
#define XQ_ITEM_SLOT_LOOT39					((1 << 26) + 39)
#define XQ_ITEM_SLOT_LOOT40					((1 << 26) + 40)
#define XQ_ITEM_SLOT_LOOT41					((1 << 26) + 41)
#define XQ_ITEM_SLOT_LOOT42					((1 << 26) + 42)
#define XQ_ITEM_SLOT_LOOT43					((1 << 26) + 43)
#define XQ_ITEM_SLOT_LOOT44					((1 << 26) + 44)
#define XQ_ITEM_SLOT_LOOT45					((1 << 26) + 45)

#define XQ_ITEM_SLOT_BANK1					((1 << 25) + 1)
#define XQ_ITEM_SLOT_BANK2					((1 << 25) + 2)
#define XQ_ITEM_SLOT_BANK3					((1 << 25) + 3)
#define XQ_ITEM_SLOT_BANK4					((1 << 25) + 4)

#define XQ_ITEM_SLOT_MERCHANT1				((1 << 24) + 1)
#define XQ_ITEM_SLOT_MERCHANT2				((1 << 24) + 2)
#define XQ_ITEM_SLOT_MERCHANT3				((1 << 24) + 3)
#define XQ_ITEM_SLOT_MERCHANT4				((1 << 24) + 4)
#define XQ_ITEM_SLOT_MERCHANT5				((1 << 24) + 5)
#define XQ_ITEM_SLOT_MERCHANT6				((1 << 24) + 6)
#define XQ_ITEM_SLOT_MERCHANT7				((1 << 24) + 7)
#define XQ_ITEM_SLOT_MERCHANT8				((1 << 24) + 8)
#define XQ_ITEM_SLOT_MERCHANT9				((1 << 24) + 9)
#define XQ_ITEM_SLOT_MERCHANT10				((1 << 24) + 10)
#define XQ_ITEM_SLOT_MERCHANT11				((1 << 24) + 11)
#define XQ_ITEM_SLOT_MERCHANT12				((1 << 24) + 12)
#define XQ_ITEM_SLOT_MERCHANT13				((1 << 24) + 13)
#define XQ_ITEM_SLOT_MERCHANT14				((1 << 24) + 14)
#define XQ_ITEM_SLOT_MERCHANT15				((1 << 24) + 15)
#define XQ_ITEM_SLOT_MERCHANT16				((1 << 24) + 16)
#define XQ_ITEM_SLOT_MERCHANT17				((1 << 24) + 17)
#define XQ_ITEM_SLOT_MERCHANT18				((1 << 24) + 18)
#define XQ_ITEM_SLOT_MERCHANT19				((1 << 24) + 19)
#define XQ_ITEM_SLOT_MERCHANT20				((1 << 24) + 20)

#define XQ_ITEM_SLOT_TRADE_GIVE1			((1 << 23) + 1)
#define XQ_ITEM_SLOT_TRADE_GIVE2			((1 << 23) + 2)
#define XQ_ITEM_SLOT_TRADE_GIVE3			((1 << 23) + 3)
#define XQ_ITEM_SLOT_TRADE_GIVE4			((1 << 23) + 4)
#define XQ_ITEM_SLOT_TRADE_RECEIVE1			((1 << 23) + 101)
#define XQ_ITEM_SLOT_TRADE_RECEIVE2			((1 << 23) + 102)
#define XQ_ITEM_SLOT_TRADE_RECEIVE3			((1 << 23) + 103)
#define XQ_ITEM_SLOT_TRADE_RECEIVE4			((1 << 23) + 104)




#define XQ_ITEM_MAX_NAME                    64
#define XQ_ITEM_MAX_BOOK                    2000
#define XQ_ITEM_MAX_CACHE					500 

#define XQ_ENTTYPE_UNKNOWN					0
#define XQ_ENTTYPE_PC						1
#define XQ_ENTTYPE_NPC						2
#define XQ_ENTTYPE_PCCORPSE					3
#define XQ_ENTTYPE_NPCCORPSE				4
#define XQ_ENTTYPE_GROUND					5
#define XQ_ENTTYPE_CLICKABLE				6

#define XQ_STAT_ERROR						0
#define XQ_STAT_HP							1
#define XQ_STAT_HP_MAX						2
#define XQ_STAT_MANA						3
#define XQ_STAT_MANA_MAX					4
#define XQ_STAT_ENDURANCE					5
#define XQ_STAT_ENDURANCE_MAX				6
#define XQ_STAT_STR							7
#define XQ_STAT_WEIGHT						8
#define XQ_STAT_WEIGHT_MAX					9
#define XQ_STAT_MR							10	
#define XQ_STAT_FR							11
#define XQ_STAT_CR							12
#define XQ_STAT_PR							13
#define XQ_STAT_DR							14
#define XQ_STAT_PSR							15
#define XQ_STAT_AC							16
#define XQ_STAT_ATK							17
#define XQ_STAT_SPEED						18
#define XQ_STAT_JUMP_VELOCITY				19
#define XQ_STAT_HP_PERC						20
#define XQ_STAT_FLAGS						21
#define XQ_STAT_DELAY						22
#define XQ_STAT_WEAPON_DAMAGE				23
#define XQ_STAT_DAMAGE_MITIGATION			24
#define XQ_STAT_ENERGY						25
#define XQ_STAT_ENERGY_MAX					26
#define XQ_STAT_MODEL_INDEX					100
#define XQ_STAT_MODEL_SCALE					101
#define XQ_STAT_TEXTNUM_LEGS				111
#define XQ_STAT_TEXTNUM_TORSO				112
#define XQ_STAT_TEXTNUM_HEAD				113
#define XQ_STAT_TEXTNUM_FEET				114
#define XQ_STAT_TEXTNUM_ARMS				115
#define XQ_STAT_TEXTNUM_LEFTWRIST			116
#define XQ_STAT_TEXTNUM_RIGHTWRIST			117
#define XQ_STAT_TEXTNUM_HANDS				118
#define XQ_STAT_HELD_PRIMARY_MODEL			130
#define XQ_STAT_HELD_SECONDARY_MODEL		131

#define XQ_TRIGGER_KICKABLE_GOAL1			201
#define XQ_TRIGGER_KICKABLE_GOAL2			202

#define XQ_CLICKABLE_TELEPORT				1


#define XQ_MONEY_COPPER						1
#define XQ_MONEY_SILVER						2
#define XQ_MONEY_GOLD						3
#define XQ_MONEY_PLATINUM				 	4

#define XQ_MONEY_SOURCE_INV					1
#define XQ_MONEY_SOURCE_BANK				2
#define XQ_MONEY_SOURCE_GIVE				3
#define XQ_MONEY_SOURCE_RECEIVE				4

#define XQ_MERCHANT_MAX_SLOTS				20

#define XQ_LOOT_MAX_SLOT					45

#define XQ_SPELL_MAX_NUM                    1000    // Maximum number of spells that can exist
#define XQ_SPELL_MAX_DESCRIPTION			250    	// Maximum length of spell description
#define XQ_SPELL_SLOTS                      6       // How many spell mem slots (gems) are there?
#define XQ_SPELL_MAX_NAME					64		// Max length of a spell name
#define XQ_SPELL_BOOK_PAGES					10		// Spell book pages
#define XQ_SPELL_BOOK_PAGESPELLS            6       // How many spells on every spellbook page
#define XQ_SPELL_MAX_MSG					100		// Max text size for spell messages (cast, fade, etc).

#define XQ_SPELL_TARGET_NONE				0		// target none means spell is disabled
													// (for clickies: means inherit target of spell)
#define XQ_SPELL_TARGET_SELF				1
#define XQ_SPELL_TARGET_SINGLE				2
#define XQ_SPELL_TARGET_GROUP				3
#define XQ_SPELL_TARGET_PBAE				4
#define XQ_SPELL_TARGET_TGAE				5


#define XQ_SPFX_SLOTS						10		// How many duration spells can affect a character at once

#define XQ_MAGIC_PURE						0
#define XQ_MAGIC_FIRE						1
#define XQ_MAGIC_COLD						2
#define XQ_MAGIC_POISON						3
#define XQ_MAGIC_DISEASE					4
#define XQ_MAGIC_PSY						5



// Trainable skills
#define XQ_SKILL_TAILORING                  1
#define XQ_SKILL_COOKING                    2
#define XQ_SKILL_BLACKSMITHING              3
#define XQ_SKILL_SWIMMING                   4		// Improves swimming speed
#define XQ_SKILL_CHANNELLING                5		// Helps casting spells through melee/movement
#define XQ_SKILL_MELEE						6		// Makes hit harder with melee
#define XQ_SKILL_FIRE						7		// Helps spells getting through resists
#define XQ_SKILL_COLD						8		// ...
#define XQ_SKILL_POISON						9		// ...
#define XQ_SKILL_DISEASE					10		// ...
#define XQ_SKILL_PSY						11		// ...

#define XQ_SKILL_H2H						12		// Increases damage of hand-to-hand (weaponless) melee
#define XQ_SKILL_W2							13		// Reduces mana consumption for this particular weapon type
#define XQ_SKILL_W3							14		// ...
#define XQ_SKILL_W4							15		// ...
#define XQ_SKILL_W5							16		// ...
#define XQ_SKILL_W6							17		// ...
#define XQ_SKILL_W7							18		// ...
#define XQ_SKILL_W8							19		// ...
#define XQ_SKILL_W9							20		// ...

#define XQ_SKILL_QUESTCOMBINE				1000	// Never failing combines "tradeskill" for quest containers


// Weapon types
#define XQ_WEAPON_SKILL_H2H					0
#define XQ_WEAPON_SKILL_1HS					1
#define XQ_WEAPON_SKILL_1HB					2
#define XQ_WEAPON_SKILL_1HP					3
#define XQ_WEAPON_SKILL_2HS					4
#define XQ_WEAPON_SKILL_2HB					5
#define XQ_WEAPON_SKILL_2HP					6

#define XQ_WEAPON_SKILL_MACHINEGUN			102
#define XQ_WEAPON_SKILL_SHOTGUN				103
#define XQ_WEAPON_SKILL_GRENADE				104
#define XQ_WEAPON_SKILL_ROCKET				105
#define XQ_WEAPON_SKILL_SHAFT				106
#define XQ_WEAPON_SKILL_RAIL				107
#define XQ_WEAPON_SKILL_PLASMA				108
#define XQ_WEAPON_SKILL_BFG					109


#define XQ_BUYING						(1)
#define XQ_TRADING_PC					(1 << 1)
#define XQ_TRADING_NPC					(1 << 2)
#define XQ_BANKING						(1 << 3)
#define XQ_LOOTING_PC					(1 << 4)
#define XQ_LOOTING_NPC					(1 << 5)
#define XQ_BANKER						(1 << 6)
#define XQ_MERCHANT						(1 << 7)
#define XQ_STUNNED						(1 << 8)
#define XQ_ZONED_PROTECTED				(1 << 9)
#define XQ_LINKDEAD						(1 << 10)
#define XQ_DEAD							(1 << 11)
#define XQ_ZONE_ARENA					(1 << 12)
#define XQ_SPELL_VISUAL1				(1 << 13)
#define XQ_SPELL_VISUAL2				(1 << 14)
#define XQ_SPELL_VISUAL3				(1 << 15)
#define XQ_SPELL_VISUAL4				(1 << 16)
#define XQ_SPELL_VISUAL5				(1 << 17)


#define XQ_MALE							0
#define XQ_FEMALE						1
#define XQ_NEUTRAL						2


// >>>>>>>>>>>>>>>>>> ADD SOMETHING HERE?  ADD IT TO CHAR CREATION CODE <<<<<<<<<<<<<<<<<<<<<
#define XQ_HUMAN						0
#define XQ_OGRE							1
#define XQ_GNOME						2
#define XQ_BARBARIAN					3
#define XQ_DARKELF						4
#define XQ_HIGHELF						5
#define XQ_WOODELF						6
#define XQ_HALFELF						7
#define XQ_DWARF						8
#define XQ_TROLL						9
#define XQ_ERUDITE						10
#define XQ_HALFLING						11
#define XQ_ALLRACES						(1 << XQ_HUMAN		|\
										1 << XQ_OGRE		|\
										1 << XQ_GNOME		|\
										1 << XQ_BARBARIAN	|\
										1 << XQ_DARKELF		|\
										1 << XQ_HIGHELF		|\
										1 << XQ_WOODELF		|\
										1 << XQ_HALFELF		|\
										1 << XQ_DWARF		|\
										1 << XQ_TROLL		|\
										1 << XQ_ERUDITE		|\
										1 << XQ_HALFLING)
#define XQ_RACES						12 // Number of above races not counting ALLRACES

#define XQ_WARRIOR						0
#define XQ_MONK							1
#define XQ_ROGUE						2
#define XQ_CLERIC						3
#define XQ_SHAMAN						4
#define XQ_DRUID						5
#define XQ_PALADIN						6
#define XQ_RANGER						7
#define XQ_SHADOWKNIGHT					8
#define XQ_ENCHANTER					9
#define XQ_WIZARD						10
#define XQ_MAGICIAN						11
#define XQ_NECROMANCER					12
#define XQ_BARD							13
#define XQ_SCOUT						14
#define XQ_ALLCLASSES					(1 << XQ_WARRIOR		|\
										1 << XQ_MONK			|\
										1 << XQ_ROGUE			|\
										1 << XQ_CLERIC			|\
										1 << XQ_SHAMAN			|\
										1 << XQ_DRUID			|\
										1 << XQ_PALADIN			|\
										1 << XQ_RANGER			|\
										1 << XQ_SHADOWKNIGHT	|\
										1 << XQ_ENCHANTER		|\
										1 << XQ_WIZARD			|\
										1 << XQ_MAGICIAN		|\
										1 << XQ_NECROMANCER		|\
										1 << XQ_BARD			|\
										1 << XQ_SCOUT)


#define XQ_CLASSES						15 // Number of above classes not counting ALLCLASSES
// >>>>>>>>>>>>>>>>>> ADD SOMETHING HERE?  ADD IT TO CHAR CREATION CODE <<<<<<<<<<<<<<<<<<<<<


typedef struct xq_spell_s {
	int					id;
	char				name[XQ_SPELL_MAX_NAME+1];
	int					casting_time;
	int					gem_icon;
	int					effect_icon;
	int					beneficial;
	int					target;
	char				description[XQ_SPELL_MAX_DESCRIPTION+1];
	int					maxrange;
	int					mana;
	int					minlevel[XQ_CLASSES];
} xq_spell_t;

#define XQ_MAX_INFO_LEN	50240
typedef struct xq_info_s {
	int		len;
	int		id1;
	int		id2;
	int		id3;
	int		id4;
	char	data[XQ_MAX_INFO_LEN+1];
} xq_info_t;



typedef struct xq_item_s {
	int64_t		id;
	int			cmdCookie; // Do not process this item until a server command with that cookie has been processed
	char		name[XQ_ITEM_MAX_NAME+1];
	int			icon;
	int			slot;
	int			hp;
	int			mana;
	int			endurance;
	int			energy;
	int			charges;
	int			max_charges;
	int			container_slots;
	int64_t		container_slot_1;
	int64_t		container_slot_2;
	int64_t		container_slot_3;
	int64_t		container_slot_4;
	int64_t		container_slot_5;
	int64_t		container_slot_6;
	int64_t		container_slot_7;
	int64_t		container_slot_8;
	int64_t		container_slot_9;
	int64_t		container_slot_10;
	int			amount;
	int			stackable;
	int			nodrop;
	int			lore;
	int			expendable;
	int			norent;
	int			str;
	int			weight;
	int			weight_reduction;
	int			spell_scroll;
	int			clicky_spell;
	int			clicky_spell_minlvl;
	int			clicky_spell_target;
	int			clicky_mustequip;
	int			casting_time;
	int			mr;
	int			fr;
	int			cr;
	int			pr;
	int			dr;
	int			psr;
	int			skillup_skill;
	int			skillup_minskill;
	int			skillup_maxskill;
	int			food;
	int			water;
	char		book[XQ_ITEM_MAX_BOOK+1];
	int			worn_spell;
	int			worn_spell_minlvl;
	int			weapon_skill;
	int			weapon_damage;
	int			weapon_delay;
	int			weapon_energycost;
	int			ac;
	int			atk;
	int			equip_minlvl;
	int			classes;
	int			races;
	int			weapon_proc_fire;
	int			weapon_proc_fire_level;
	int			weapon_proc_impact;
	int			weapon_proc_impact_level;
	int			weapon_proc_explode;
	int			weapon_proc_explode_level;
	int			tradeskill;
} xq_item_t;

typedef struct xq_container_slot_s {
	int64_t		container_id;
	int			container_slot;
} xq_container_slot_t;


char *xq_EntName(entityState_t *ps);
char *xq_RezName(playerState_t *ps);
char *xq_GroupName(playerState_t *ps, int member);
char *xq_GenericName(playerState_t *ps);

char *xq_skills_literal(int skill, int type, int leftpad);

int xq_weapon_skill_to_weapon(int skill);

void xq_break(void);


#define XQ_INFO_MAX_CACHE					100
#define XQ_INFO_MAX_JSON_PAYLOAD_SIZE		50000
#define XQ_INFO_ASSET						1
#define XQ_INFO_MODELS						2
#define XQ_INFO_SUCCESS						1
#define XQ_INFO_ERROR						2



typedef enum xq_groundspawn_type_e {
	XQ_GROUND_NONE,
	XQ_GROUND_GENERIC,
	XQ_GROUND_HEALTH,
	XQ_GROUND_HEALTH_ARENA_5,
	XQ_GROUND_HEALTH_ARENA_25,
	XQ_GROUND_HEALTH_ARENA_50,
	XQ_GROUND_HEALTH_ARENA_100,
	XQ_GROUND_ENERGY_ARENA_100,
	XQ_GROUND_ENDURANCE_ARENA_100,
	XQ_GROUND_MANA,
	XQ_GROUND_DAMAGE,
	XQ_GROUND_ENERGIZE,
	XQ_GROUND_COINSPELL,
	XQ_GROUND_SPELL,
	XQ_GROUND_KICKABLE
} xq_groundspawn_type_t;


#define XQ_ARENA_WEAPON1		174
#define XQ_ARENA_WEAPON2		175
#define XQ_ARENA_WEAPON3		176
#define XQ_ARENA_WEAPON4		177
#define XQ_ARENA_WEAPON5		178
#define XQ_ARENA_WEAPON6		179
#define XQ_ARENA_WEAPON7		180
#define XQ_ARENA_WEAPON8		181
#define XQ_ARENA_WEAPON9		182


#define XQ_MODEL_MAX_PATH	50
#define XQ_MAX_MODELS 		100	// Max possible models

typedef enum xq_modeltype_e {
	XQ_MODEL_TYPE_SIMPLE,
	XQ_MODEL_TYPE_Q3PLAYER
} xq_modeltype_t;
typedef struct xq_model_s {
	int				id;
	xq_modeltype_t	type;
	char			path[XQ_MODEL_MAX_PATH];
	int				default_scale;
	int				numfaces;
	int				numskins;
	int				crawler;		// 1 means it will pitch and roll according to the slope it's standing on, if any
	int				full_armor;		// 1 means it is a detailed q3 model with surface groups for arms, feet, hands in addition to legs, head and torso (basically a playable race).
	vec3_t			npc_mins;
	vec3_t			npc_maxs;
	int				playable;
	int				gender;
} xq_model_t;
typedef enum xq_body_part_e {
	XQ_BODY_PART_NONE,
	XQ_BODY_PART_LEGS,
	XQ_BODY_PART_TORSO,
	XQ_BODY_PART_HEAD,
	XQ_BODY_PART_FEET,
	XQ_BODY_PART_ARMS,
	XQ_BODY_PART_LEFTWRIST,
	XQ_BODY_PART_RIGHTWRIST,
	XQ_BODY_PART_HANDS
} xq_body_part_t;


#define CLOG_LENLIT 5
#define CLOG_MAX_LEN 1024

typedef enum xq_anim_s {
// base anims
	XQ_ANIM_IDLE,
	XQ_ANIM_IDLE_ALT,
	XQ_ANIM_IDLE_DEAD, // living mob pretending to be dead when unaggroed
	XQ_ANIM_WALK,
	XQ_ANIM_RUN,
	XQ_ANIM_SWIM,
	XQ_ANIM_FALL,

// special anims
	XQ_ANIM_PAIN,
	XQ_ANIM_CAST,
	XQ_ANIM_MELEE,
	XQ_ANIM_LOOT	// player-only
} xq_anim_t;

int xq_anim_to_q3(xq_anim_t xqanim, int torso, int idle_variety);
uint64_t xq_msec_new(void);
uint64_t xq_msec(void);
uint64_t xq_usec(void);
char *xq_since_start(uint64_t start_msec);
char *xq_class_lit(int c, int uppercase, int abbr);
char *xq_race_lit(int c, int uppercase, int abbr);
void xq_strtolower(char *c);
void xq_strtoupper(char *c);
qboolean xq_is_magic_user(int class);
char *xq_slot_lit(int slot);
int startsWith(const char *pre, const char *str);
qboolean xq_seq(char *str1, char *str2);
int hex2dec(char hex);
char *dec2bin(int64_t dec);
char *dec2bin32(int dec);
int xq_class_race_combo(int class, int race);
char xq_netchars_escape(char in);
char xq_netchars_unescape(char in);

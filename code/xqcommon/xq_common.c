#include <sys/timeb.h>
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../game/bg_public.h"

static void readName(int32_t *ptr, int ints, char *buf) {
	// buf must be of size (ints*4 + 1) for terminating zero
	for (int i = 0;  i < ints;  i++) {
		buf[i*4+0] = *(ptr+i) >> 24;
		buf[i*4+1] = *(ptr+i) << 8 >> 24;
		buf[i*4+2] = *(ptr+i) << 16 >> 24;
		buf[i*4+3] = *(ptr+i) << 24 >> 24;
	}
}
char *xq_EntName(entityState_t *ps) {
	static char buf[8*4+1];
	memset(buf, 0, sizeof(buf));

	readName(&ps->xq_name1, 8, buf);
	return (char *)buf;
}
char *xq_GenericName(playerState_t *ps) {
	static char buf[5*4+1];
	memset(buf, 0, sizeof(buf));

	readName(&ps->xq_generic_name1, 5, buf);
	return (char *)buf;
}
char *xq_GroupName(playerState_t *ps, int member) {
	static char buf[5*4+1];
	memset(buf, 0, sizeof(buf));

	readName(&ps->xq_group_member1_name1 + ((member - 1) * 5), 8, buf);
	return (char *)buf;
}
char *xq_RezName(playerState_t *ps) {
	static char buf[5*4+1];
	memset(buf, 0, sizeof(buf));

	readName(&ps->xq_rez_author_1, 5, buf);
	return (char *)buf;
}
uint64_t xq_msec() {
	struct timeb t;
	ftime(&t);
	return t.time * 1000 + t.millitm;
}
char * xq_since_start(uint64_t start_time) {
	static char ret[30];
	memset(ret, 0, sizeof(ret));

	uint64_t diff = xq_msec() - start_time;
	uint64_t sec = diff / 1000;
	uint64_t msec = (diff - sec * 1000);

	snprintf(ret, sizeof(ret), "%" PRIu64 ".%03" PRIu64 "", sec, msec);

	return (char *)ret;
}
char *xq_skills_literal(int skill, int type, int leftpad) {
	static char ret[100];
	char tmp[100] = {0};
	memset(ret, 0, sizeof(ret));
	strcpy(tmp, "UNKNOWN SKILL");

	switch (skill) {
		case XQ_SKILL_TAILORING:		strcpy(tmp, type ? "skill_tailoring"		:	"Tailoring");		break;
		case XQ_SKILL_COOKING:			strcpy(tmp, type ? "skill_cooking"			:	"Cooking");			break;
		case XQ_SKILL_BLACKSMITHING:	strcpy(tmp, type ? "skill_blacksmithing"	:	"Blacksmithing");	break;
		case XQ_SKILL_SWIMMING:			strcpy(tmp, type ? "skill_swimming"			: 	"Swimming");		break;
		case XQ_SKILL_CHANNELLING:		strcpy(tmp, type ? "skill_channelling"		:	"Channelling");		break;
		case XQ_SKILL_MELEE:			strcpy(tmp, type ? "skill_melee"			:	"Melee");			break;
		case XQ_SKILL_FIRE:				strcpy(tmp, type ? "skill_fire"				:	"Fire");			break;
		case XQ_SKILL_COLD:				strcpy(tmp, type ? "skill_cold"				:	"Cold");			break;
		case XQ_SKILL_POISON:			strcpy(tmp, type ? "skill_poison"			:	"Poison");			break;
		case XQ_SKILL_DISEASE:			strcpy(tmp, type ? "skill_disease"			:	"Disease");			break;
		case XQ_SKILL_PSY:				strcpy(tmp, type ? "skill_psy"				:	"Psy");				break;

		case XQ_SKILL_H2H:				strcpy(tmp, type ? "skill_h2h"				:	"Hand-to-hand");	break;
		case XQ_SKILL_W2:				strcpy(tmp, type ? "skill_w2" 				:	"Weapon type 2");	break;
		case XQ_SKILL_W3:				strcpy(tmp, type ? "skill_w3" 				:	"Weapon type 3");	break;
		case XQ_SKILL_W4:				strcpy(tmp, type ? "skill_w4" 				:	"Weapon type 4");	break;
		case XQ_SKILL_W5:				strcpy(tmp, type ? "skill_w5" 				:	"Weapon type 5");	break;
		case XQ_SKILL_W6:				strcpy(tmp, type ? "skill_w6" 				:	"Weapon type 6");	break;
		case XQ_SKILL_W7:				strcpy(tmp, type ? "skill_w7" 				:	"Weapon type 7");	break;
		case XQ_SKILL_W8:				strcpy(tmp, type ? "skill_w8" 				:	"Weapon type 8");	break;
		case XQ_SKILL_W9:				strcpy(tmp, type ? "skill_w9" 				:	"Weapon type 9");	break;
	}

	int i = 0;
	if (leftpad > strlen(tmp)) {
		for (i = 0;  i < (leftpad - strlen(tmp));  i++) {
			ret[i] = 32;
		}
	}
	strncpy(ret + i, tmp, strlen(tmp));

	return (char *)ret;
}
int xq_weapon_skill_to_weapon(int skill) {
	switch (skill) {
		case XQ_WEAPON_SKILL_H2H:
		case XQ_WEAPON_SKILL_1HS:
		case XQ_WEAPON_SKILL_1HB:
		case XQ_WEAPON_SKILL_1HP:
		case XQ_WEAPON_SKILL_2HS:
		case XQ_WEAPON_SKILL_2HB:
		case XQ_WEAPON_SKILL_2HP:
			return WP_GAUNTLET;
			break;
		case XQ_WEAPON_SKILL_MACHINEGUN:
			return WP_MACHINEGUN;
			break;
		case XQ_WEAPON_SKILL_SHOTGUN:
			return WP_SHOTGUN;
			break;
		case XQ_WEAPON_SKILL_GRENADE:
			return WP_GRENADE_LAUNCHER;
			break;
		case XQ_WEAPON_SKILL_ROCKET:
			return WP_ROCKET_LAUNCHER;
			break;
		case XQ_WEAPON_SKILL_SHAFT:
			return WP_LIGHTNING;
			break;
		case XQ_WEAPON_SKILL_RAIL:
			return WP_RAILGUN;
			break;
		case XQ_WEAPON_SKILL_PLASMA:
			return WP_PLASMAGUN;
			break;
		case XQ_WEAPON_SKILL_BFG:
			return WP_BFG;
			break;
		default:
			Com_Printf("xq_weapon_skill_to_weapon: Unknown skill %i", skill);
			return WP_GAUNTLET;
	}
}
void xq_break() {
}
int xq_anim_to_q3(xq_anim_t xqanim, int torso, int idle_variety) {
	switch (xqanim) {
		case XQ_ANIM_IDLE:
			if (torso) return (idle_variety == 1 ? TORSO_STAND2 : TORSO_STAND); else return LEGS_IDLE;
			break;
		case XQ_ANIM_WALK:
			if (torso) return TORSO_RAISE /* error */ ; else return LEGS_WALK;
			break;
		case XQ_ANIM_RUN:
			if (torso) return TORSO_RAISE /* error */ ; else return LEGS_RUN;
			break;
		case XQ_ANIM_SWIM:
			if (torso) return TORSO_RAISE /* error */ ; else return LEGS_SWIM;
			break;
		case XQ_ANIM_MELEE:
			if (torso) return TORSO_ATTACK2; 	else return LEGS_IDLECR /* error */;
			break;
		case XQ_ANIM_CAST:
			if (torso) return TORSO_GESTURE; 	else return LEGS_IDLECR /* error */;
			break;
		case XQ_ANIM_PAIN:
			if (torso) return TORSO_DROP; 		else return LEGS_IDLECR /* error */;
			break;
		default:
			if (torso) return TORSO_RAISE /* error */ ;	else return LEGS_IDLECR /* error */;
	}
}
qboolean xq_is_magic_user(int class) {
    if (class == XQ_WARRIOR) return qfalse;
    if (class == XQ_MONK) return qfalse;
    if (class == XQ_ROGUE) return qfalse;
    if (class == XQ_SCOUT) return qfalse;
	return qtrue;
}
char *xq_class_lit(int c, int uppercase, int abbr) {
    static char ret[20];
	memset(ret, 0, sizeof(ret));

    switch (c) {
        case XQ_WARRIOR:		strcpy(ret, abbr ? "War" : "Warrior");		break;
        case XQ_CLERIC:			strcpy(ret, abbr ? "Cle" : "Cleric");		break;
        case XQ_ENCHANTER:		strcpy(ret, abbr ? "Enc" : "Enchanter");	break;
        case XQ_SHAMAN:			strcpy(ret, abbr ? "Shm" : "Shaman");		break;
        case XQ_PALADIN:		strcpy(ret, abbr ? "Pal" : "Paladin");		break;
        case XQ_SHADOWKNIGHT:	strcpy(ret, abbr ? "Shd" : "Shadowknight");	break;
        case XQ_MONK:			strcpy(ret, abbr ? "Mnk" : "Monk");			break;
        case XQ_RANGER:			strcpy(ret, abbr ? "Rng" : "Ranger");		break;
        case XQ_ROGUE:			strcpy(ret, abbr ? "Rog" : "Rogue");		break;
        case XQ_MAGICIAN:		strcpy(ret, abbr ? "Mag" : "Magician");		break;
        case XQ_NECROMANCER:	strcpy(ret, abbr ? "Nec" : "Necromancer");	break;
        case XQ_WIZARD:			strcpy(ret, abbr ? "Wiz" : "Wizard");		break;
        case XQ_BARD:			strcpy(ret, abbr ? "Brd" : "Bard");			break;
        case XQ_DRUID:			strcpy(ret, abbr ? "Dru" : "Druid");		break;
        case XQ_SCOUT:			strcpy(ret, abbr ? "Sct" : "Scout");		break;

        default:				strcpy(ret, abbr ? "Bng" : "Being");		break;
    }
	if (uppercase) {
		xq_strtoupper(ret);
	}
    return ret;
}
char *xq_race_lit(int c, int uppercase, int abbr) {
    static char ret[20];
	memset(ret, 0, sizeof(ret));

    switch (c) {
        case XQ_HUMAN: strcpy(ret, abbr ? "Hum" : "Human"); break;
        case XQ_OGRE: strcpy(ret, abbr ? "Ogr" : "Ogre"); break;
        case XQ_GNOME: strcpy(ret, abbr ? "Gnm" : "Gnome"); break;
        case XQ_BARBARIAN: strcpy(ret, abbr ? "Bar" : "Barbarian"); break;
        case XQ_DARKELF: strcpy(ret, abbr ? "Def" : "Dark Elf"); break;
        case XQ_HIGHELF: strcpy(ret, abbr ? "Hie" : "High Elf"); break;
        case XQ_WOODELF: strcpy(ret, abbr ? "Elf" : "Wood Elf"); break;
        case XQ_HALFELF: strcpy(ret, abbr ? "Hef" : "Half Elf"); break;
        case XQ_DWARF: strcpy(ret, abbr ? "Dwf" : "Dwarf"); break;
        case XQ_TROLL: strcpy(ret, abbr ? "Trl" : "Troll"); break;
        case XQ_ERUDITE: strcpy(ret, abbr ? "Eru" : "Erudite"); break;
        case XQ_HALFLING: strcpy(ret, abbr ? "Hfl" : "Halfling"); break;
        default: strcpy(ret, abbr ? "Unk" : "Unknown Race"); break;
    }
	if (uppercase) {
		xq_strtoupper(ret);
	}
    return ret;
}
void xq_strtolower(char *p) {
	for ( ; *p; ++p) *p = tolower(*p);
}
void xq_strtoupper(char *p) {
	for ( ; *p; ++p) *p = toupper(*p);
}
char *xq_slot_lit(int slot) {
	switch (slot) {
		case XQ_ITEM_SLOT_PRIMARY:					return "PRIMARY";
		case XQ_ITEM_SLOT_SECONDARY:				return "SECONDARY";
		case XQ_ITEM_SLOT_LEFTEAR:					return "LEFTEAR";
		case XQ_ITEM_SLOT_RIGHTEAR:					return "RIGHTEAR";
		case XQ_ITEM_SLOT_HEAD:						return "HEAD";
		case XQ_ITEM_SLOT_FACE:						return "FACE";
		case XQ_ITEM_SLOT_CHEST:					return "CHEST";
		case XQ_ITEM_SLOT_ARMS:						return "ARMS";
		case XQ_ITEM_SLOT_WAIST:					return "WAIST";
		case XQ_ITEM_SLOT_LEFTWRIST:				return "LEFTWRIST";
		case XQ_ITEM_SLOT_RIGHTWRIST:				return "RIGHTWRIST";
		case XQ_ITEM_SLOT_LEGS:						return "LEGS";
		case XQ_ITEM_SLOT_HANDS:					return "HANDS";
		case XQ_ITEM_SLOT_LEFTFINGER:				return "LEFTFINGER";
		case XQ_ITEM_SLOT_RIGHTFINGER:				return "RIGHTFINGER";
		case XQ_ITEM_SLOT_FEET:						return "FEET";
		case XQ_ITEM_SLOT_SHOULDERS:				return "SHOULDERS";
		case XQ_ITEM_SLOT_BACK:						return "BACK";
		case XQ_ITEM_SLOT_NECK:						return "NECK";
		case XQ_ITEM_SLOT_SECONDARY1:				return "SECONDARY1";
		case XQ_ITEM_SLOT_PRIMARY1:					return "PRIMARY1";
		case XQ_ITEM_SLOT_PRIMARY2:					return "PRIMARY2";
		case XQ_ITEM_SLOT_PRIMARY3:					return "PRIMARY3";
		case XQ_ITEM_SLOT_PRIMARY4:					return "PRIMARY4";
		case XQ_ITEM_SLOT_PRIMARY5:					return "PRIMARY5";
		case XQ_ITEM_SLOT_PRIMARY6:					return "PRIMARY6";
		case XQ_ITEM_SLOT_PRIMARY7:					return "PRIMARY7";
		case XQ_ITEM_SLOT_PRIMARY8:					return "PRIMARY8";
		case XQ_ITEM_SLOT_PRIMARY9:					return "PRIMARY9";
		case XQ_ITEM_SLOT_MOUSE1:					return "MOUSE1";
		case XQ_ITEM_SLOT_MOUSE2:					return "MOUSE2";
		case XQ_ITEM_SLOT_MOUSE3:					return "MOUSE3";
		case XQ_ITEM_SLOT_MOUSE4:					return "MOUSE4";
		case XQ_ITEM_SLOT_MOUSE5:					return "MOUSE5";
		case XQ_ITEM_SLOT_MOUSE6:					return "MOUSE6";
		case XQ_ITEM_SLOT_MOUSE7:					return "MOUSE7";
		case XQ_ITEM_SLOT_MOUSE8:					return "MOUSE8";
		case XQ_ITEM_SLOT_CARRY1:					return "CARRY1";
		case XQ_ITEM_SLOT_CARRY2:					return "CARRY2";
		case XQ_ITEM_SLOT_CARRY3:					return "CARRY3";
		case XQ_ITEM_SLOT_CARRY4:					return "CARRY4";
		case XQ_ITEM_SLOT_CARRY5:					return "CARRY5";
		case XQ_ITEM_SLOT_CARRY6:					return "CARRY6";
		case XQ_ITEM_SLOT_INCONTAINER:				return "INCONTAINER";
		case XQ_ITEM_SLOT_LOOT1:					return "LOOT1";
		case XQ_ITEM_SLOT_LOOT2:					return "LOOT2";
		case XQ_ITEM_SLOT_LOOT3:					return "LOOT3";
		case XQ_ITEM_SLOT_LOOT4:					return "LOOT4";
		case XQ_ITEM_SLOT_LOOT5:					return "LOOT5";
		case XQ_ITEM_SLOT_LOOT6:					return "LOOT6";
		case XQ_ITEM_SLOT_LOOT7:					return "LOOT7";
		case XQ_ITEM_SLOT_LOOT8:					return "LOOT8";
		case XQ_ITEM_SLOT_LOOT9:					return "LOOT9";
		case XQ_ITEM_SLOT_LOOT10:					return "LOOT10";
		case XQ_ITEM_SLOT_LOOT11:					return "LOOT11";
		case XQ_ITEM_SLOT_LOOT12:					return "LOOT12";
		case XQ_ITEM_SLOT_LOOT13:					return "LOOT13";
		case XQ_ITEM_SLOT_LOOT14:					return "LOOT14";
		case XQ_ITEM_SLOT_LOOT15:					return "LOOT15";
		case XQ_ITEM_SLOT_LOOT16:					return "LOOT16";
		case XQ_ITEM_SLOT_LOOT17:					return "LOOT17";
		case XQ_ITEM_SLOT_LOOT18:					return "LOOT18";
		case XQ_ITEM_SLOT_LOOT19:					return "LOOT19";
		case XQ_ITEM_SLOT_LOOT20:					return "LOOT20";
		case XQ_ITEM_SLOT_LOOT21:					return "LOOT21";
		case XQ_ITEM_SLOT_LOOT22:					return "LOOT22";
		case XQ_ITEM_SLOT_LOOT23:					return "LOOT23";
		case XQ_ITEM_SLOT_LOOT24:					return "LOOT24";
		case XQ_ITEM_SLOT_LOOT25:					return "LOOT25";
		case XQ_ITEM_SLOT_LOOT26:					return "LOOT26";
		case XQ_ITEM_SLOT_LOOT27:					return "LOOT27";
		case XQ_ITEM_SLOT_LOOT28:					return "LOOT28";
		case XQ_ITEM_SLOT_LOOT29:					return "LOOT29";
		case XQ_ITEM_SLOT_LOOT30:					return "LOOT30";
		case XQ_ITEM_SLOT_LOOT31:					return "LOOT31";
		case XQ_ITEM_SLOT_LOOT32:					return "LOOT32";
		case XQ_ITEM_SLOT_LOOT33:					return "LOOT33";
		case XQ_ITEM_SLOT_LOOT34:					return "LOOT34";
		case XQ_ITEM_SLOT_LOOT35:					return "LOOT35";
		case XQ_ITEM_SLOT_LOOT36:					return "LOOT36";
		case XQ_ITEM_SLOT_LOOT37:					return "LOOT37";
		case XQ_ITEM_SLOT_LOOT38:					return "LOOT38";
		case XQ_ITEM_SLOT_LOOT39:					return "LOOT39";
		case XQ_ITEM_SLOT_LOOT40:					return "LOOT40";
		case XQ_ITEM_SLOT_LOOT41:					return "LOOT41";
		case XQ_ITEM_SLOT_LOOT42:					return "LOOT42";
		case XQ_ITEM_SLOT_LOOT43:					return "LOOT43";
		case XQ_ITEM_SLOT_LOOT44:					return "LOOT44";
		case XQ_ITEM_SLOT_LOOT45:					return "LOOT45";
		case XQ_ITEM_SLOT_BANK1:					return "BANK1";
		case XQ_ITEM_SLOT_BANK2:					return "BANK2";
		case XQ_ITEM_SLOT_BANK3:					return "BANK3";
		case XQ_ITEM_SLOT_BANK4:					return "BANK4";
		case XQ_ITEM_SLOT_MERCHANT1:				return "MERCHANT1";
		case XQ_ITEM_SLOT_MERCHANT2:				return "MERCHANT2";
		case XQ_ITEM_SLOT_MERCHANT3:				return "MERCHANT3";
		case XQ_ITEM_SLOT_MERCHANT4:				return "MERCHANT4";
		case XQ_ITEM_SLOT_TRADE_GIVE1:				return "TRADE_GIVE1";
		case XQ_ITEM_SLOT_TRADE_GIVE2:				return "TRADE_GIVE2";
		case XQ_ITEM_SLOT_TRADE_GIVE3:				return "TRADE_GIVE3";
		case XQ_ITEM_SLOT_TRADE_GIVE4:				return "TRADE_GIVE4";
		case XQ_ITEM_SLOT_TRADE_RECEIVE1:			return "TRADE_RECEIVE1";
		case XQ_ITEM_SLOT_TRADE_RECEIVE2:			return "TRADE_RECEIVE2";
		case XQ_ITEM_SLOT_TRADE_RECEIVE3:			return "TRADE_RECEIVE3";
		case XQ_ITEM_SLOT_TRADE_RECEIVE4:			return "TRADE_RECEIVE4";
		default:	return va("UNKNOWN: %i", slot);
	}
}
int startsWith(const char *pre, const char *str) {
	size_t lenpre = strlen(pre);
	int lenstr = strlen(str);
	return lenstr < lenpre ? 0 : memcmp(pre, str, lenpre) == 0;
}
qboolean xq_seq(char *str1, char *str2) {
	if (strlen(str1) != strlen(str2)) return qfalse;

	if (!strncmp(str1, str2, strlen(str1))) return qtrue;

	return qfalse;
}
int hex2dec(char hex) {
    // Converts a single hex digit to decimal
    if (hex >= '0' && hex <= '9') return hex - 48;
    if (hex >= 'a' && hex <= 'f') return hex - 87;
    if (hex >= 'A' && hex <= 'F') return hex - 55;
    return 0;
}
char *dec2bin(int64_t dec) {
	static char ret[65+7];
	memset(ret, 32, sizeof(ret));
	ret[71] = 0;
	int off = 0;

	for (int i = 63;  i >= 0;  i--) {
		if ((i+1) % 8 == 0) {
			off++;
		}
		if (dec & (((int64_t)1) << i)) {
			ret[(63-i)+off] = '1';
		} else {
			ret[(63-i)+off] = '0';
		}
	}

	return ret;
}
char *dec2bin32(int dec) {
	static char ret[33+3];
	memset(ret, 32, sizeof(ret));
	ret[35] = 0;
	int off = 0;

	for (int i = 31;  i >= 0;  i--) {
		if ((i+1) % 8 == 0) {
			off++;
		}
		if (dec & (((int32_t)1) << i)) {
			ret[(31-i)+off] = '1';
		} else {
			ret[(31-i)+off] = '0';
		}
	}

	return ret;
}
int xq_class_race_combo(int class, int race) {
	switch (class) {
		case XQ_BARD:
			if (race == XQ_HALFELF)		return 1;
			if (race == XQ_HUMAN)		return 1;
			if (race == XQ_WOODELF)		return 1;
			break;
		case XQ_CLERIC:
			if (race == XQ_DARKELF)		return 1;
			if (race == XQ_DWARF)		return 1;
			if (race == XQ_ERUDITE)		return 1;
			if (race == XQ_GNOME)		return 1;
			if (race == XQ_HALFLING)	return 1;
			if (race == XQ_HIGHELF)		return 1;
			if (race == XQ_HUMAN)		return 1;
			break;
		case XQ_DRUID:
			if (race == XQ_HALFELF)		return 1;
			if (race == XQ_HALFLING)	return 1;
			if (race == XQ_HUMAN)		return 1;
			if (race == XQ_WOODELF)		return 1;
			break;
		case XQ_ENCHANTER:
			if (race == XQ_DARKELF)		return 1;
			if (race == XQ_ERUDITE)		return 1;
			if (race == XQ_GNOME)		return 1;
			if (race == XQ_HIGHELF)		return 1;
			if (race == XQ_HUMAN)		return 1;
			break;
		case XQ_MAGICIAN:
			if (race == XQ_DARKELF)		return 1;
			if (race == XQ_ERUDITE)		return 1;
			if (race == XQ_GNOME)		return 1;
			if (race == XQ_HIGHELF)		return 1;
			if (race == XQ_HUMAN)		return 1;
			break;
		case XQ_MONK:
			if (race == XQ_HUMAN)		return 1;
			break;
		case XQ_NECROMANCER:
			if (race == XQ_DARKELF)		return 1;
			if (race == XQ_ERUDITE)		return 1;
			if (race == XQ_GNOME)		return 1;
			if (race == XQ_HUMAN)		return 1;
			break;
		case XQ_PALADIN:
			if (race == XQ_DWARF)		return 1;
			if (race == XQ_ERUDITE)		return 1;
			if (race == XQ_HALFELF)		return 1;
			if (race == XQ_HIGHELF)		return 1;
			if (race == XQ_HUMAN)		return 1;
			break;
		case XQ_RANGER:
			if (race == XQ_HALFELF)		return 1;
			if (race == XQ_HUMAN)		return 1;
			if (race == XQ_WOODELF)		return 1;
			break;
		case XQ_ROGUE:
			if (race == XQ_BARBARIAN)	return 1;
			if (race == XQ_DARKELF)		return 1;
			if (race == XQ_DWARF)		return 1;
			if (race == XQ_GNOME)		return 1;
			if (race == XQ_HALFELF)		return 1;
			if (race == XQ_HALFLING)	return 1;
			if (race == XQ_HUMAN)		return 1;
			if (race == XQ_WOODELF)		return 1;
			break;
		case XQ_SHADOWKNIGHT:
			if (race == XQ_DARKELF)		return 1;
			if (race == XQ_ERUDITE)		return 1;
			if (race == XQ_HUMAN)		return 1;
			if (race == XQ_OGRE)		return 1;
			if (race == XQ_TROLL)		return 1;
			break;
		case XQ_SHAMAN:
			if (race == XQ_BARBARIAN)	return 1;
			if (race == XQ_OGRE)		return 1;
			if (race == XQ_TROLL)		return 1;
			break;
		case XQ_WARRIOR:
			if (race == XQ_BARBARIAN)	return 1;
			if (race == XQ_DARKELF)		return 1;
			if (race == XQ_DWARF)		return 1;
			if (race == XQ_GNOME)		return 1;
			if (race == XQ_HALFELF)		return 1;
			if (race == XQ_HALFLING)	return 1;
			if (race == XQ_HUMAN)		return 1;
			if (race == XQ_OGRE)		return 1;
			if (race == XQ_TROLL)		return 1;
			if (race == XQ_WOODELF)		return 1;
			break;
		case XQ_WIZARD:
			if (race == XQ_DARKELF)		return 1;
			if (race == XQ_ERUDITE)		return 1;
			if (race == XQ_GNOME)		return 1;
			if (race == XQ_HIGHELF)		return 1;
			if (race == XQ_HUMAN)		return 1;
			break;
		case XQ_SCOUT:
			if (race == XQ_HUMAN)		return 1;
			break;
	}

	return 0;
}


// We do not transmit % and " in strings over the network,
// these chars are replaced by ascii 30 and 29 values.
// This function is called right before drawing the actual character on the screen,
// so we draw the proper character.
char xq_netchars_unescape(char in) {
	switch (in) {
		case 30:	return '%';
		case 29:	return '"';
		default:	return in;
	}
}
char xq_netchars_escape(char in) {
	switch (in) {
		case '%':	return 30;
		case '"':	return 29;
		default:	return in;
	}
}

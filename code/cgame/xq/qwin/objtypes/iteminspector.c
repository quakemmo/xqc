#include "../../../cg_local.h"

#define LINE_LEN 100

static void DrawLine(qw_window_t *win, qw_obj_t *obj, int x, int *y, char *line) {
	// Display the line of text in the inspector window and clear the line
	if (strlen(line) < 1) return;

	int took_lines = CG_DrawTinyString(win, x, *y, line, 1.0f);
	*y += TINYCHAR_HEIGHT * (took_lines + 1);
	obj->h += TINYCHAR_HEIGHT * (took_lines + 1);
	obj->w = (obj->w < CG_DrawStrlen(line) ? CG_DrawStrlen(line) : obj->w);
	memset(line, 0, LINE_LEN+1);
}
static void Spell_ClassLevel(char *line, xq_spell_t *sp) {
	// Create a line of a spell's class/level requirements
	for (int i = 0;  i < XQ_CLASSES;  i++) {
		int minlvl = sp->minlevel[i];
		if (minlvl > 0 && minlvl < 255) {
			strncat(line, va("%s(%i)  ", xq_class_lit(i, 1, 1), minlvl), LINE_LEN - strlen(line));
		}
	}
}
static void DrawSpell(qw_window_t *win, qw_obj_t *obj, int x, int y) {
	int cury;
	xq_spell_t *sp;
	char line[LINE_LEN+1] = {0};

	sp = xq_SpellInfo(obj->spell);
	if (!sp) return;

	cury = y;


	snprintf(line, LINE_LEN, "Spell: %s", sp->name);
	qw_WindowTitle(obj->title_window, line);

	xq_DrawSpellIcon(x + 340, y + 5, sp->effect_icon);

	snprintf(line, LINE_LEN, "Mana cost: %i ", sp->mana);
	DrawLine(win, obj, x, &cury, line);

	snprintf(line, LINE_LEN, "Classes: ");
	Spell_ClassLevel(line, sp);
	DrawLine(win, obj, x, &cury, line);

	snprintf(line, LINE_LEN, "Range: %i ", sp->maxrange);
	DrawLine(win, obj, x, &cury, line);

	snprintf(line, LINE_LEN, "Casting time: %.1f sec", ((float)sp->casting_time / 10));
	DrawLine(win, obj, x, &cury, line);

	cury += 20;
	if (sp->description[0]) {
		DrawLine(win, obj, x, &cury, va("Spell effect: %s", sp->description));
	}
}
static void AddSlot(char *line, int itemslots, int slot) {
	if (itemslots & slot) {
		strncat(line, va("%s ", xq_slot_lit(slot)), LINE_LEN - strlen(line));
	}
}

static void SimpleSpell(char *line, int spell, int minlvl, char *label) {
	if (!spell) return;

	xq_spell_t *sp = xq_SpellInfo(spell);
	if (sp) {
		snprintf(line, LINE_LEN, "%s: %s%s",
			label,
			sp->name,
			minlvl < 2 ? "" : va(" (min. level: %i)", minlvl)
		);
	}
}
void qw_Obj_ItemInspector_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	#define DRAWLINE if (line[0]) DrawLine(win, obj, x, &cury, line)

	char line[LINE_LEN+1] = {0};
	xq_spell_t *sp;

	qw_obj_t *obj = &qws->obj[att->num];
	int x = att->x;
	int y = att->y;
	qw_AttachWithBorder(win, att, &x, &y);

	int cury = y;

	obj->w = QW_OBJ_ITEMINSPECTOR_WIDTH - QW_WIN_BORDER_WIDTH;
	obj->h = QW_OBJ_ITEMINSPECTOR_HEIGHT - QW_WIN_BORDER_WIDTH - QW_WIN_STATUSBAR_HEIGHT;

	if (obj->spell > 0) {
		DrawSpell(win, obj, x, y);
		return;
	}

	xq_item_t *iinf = xq_ItemInfo(obj->item);
	if (iinf == NULL) {
		return;
	}

	qw_WindowTitle(obj->title_window, iinf->name);
	xq_DrawItemIcon(win, x + 360, y + 5, iinf->icon, iinf->amount);


	int s = iinf->slot;
	// If the item can be equipped in a particular slot, report that.
	if (s) {
		strncat(line, "Slots: ", LINE_LEN);
		AddSlot(line, s, XQ_ITEM_SLOT_PRIMARY);
		AddSlot(line, s, XQ_ITEM_SLOT_SECONDARY);
		AddSlot(line, s, XQ_ITEM_SLOT_HEAD);
		AddSlot(line, s, XQ_ITEM_SLOT_FACE);
		AddSlot(line, s, XQ_ITEM_SLOT_CHEST);
		AddSlot(line, s, XQ_ITEM_SLOT_ARMS);
		AddSlot(line, s, XQ_ITEM_SLOT_WAIST);
		AddSlot(line, s, XQ_ITEM_SLOT_WAIST);
		AddSlot(line, s, XQ_ITEM_SLOT_LEGS);
		AddSlot(line, s, XQ_ITEM_SLOT_HANDS);
		AddSlot(line, s, XQ_ITEM_SLOT_FEET);
		AddSlot(line, s, XQ_ITEM_SLOT_SHOULDERS);
		AddSlot(line, s, XQ_ITEM_SLOT_BACK);
		AddSlot(line, s, XQ_ITEM_SLOT_NECK);


		// instead of reporting both left and right slots such as "LEFT EAR, RIGHT EAR"
		// just report "EAR", when both slots are enabled
		if (s & XQ_ITEM_SLOT_LEFTWRIST && s & XQ_ITEM_SLOT_RIGHTWRIST) {
			strncat(line, "WRIST ", LINE_LEN - strlen(line));
		} else {
			AddSlot(line, s, XQ_ITEM_SLOT_LEFTWRIST);
			AddSlot(line, s, XQ_ITEM_SLOT_RIGHTWRIST);
		}

		if (s & XQ_ITEM_SLOT_LEFTEAR && s & XQ_ITEM_SLOT_RIGHTEAR) {
			strncat(line, "EAR ", LINE_LEN - strlen(line));
		} else {
			AddSlot(line, s, XQ_ITEM_SLOT_LEFTEAR);
			AddSlot(line, s, XQ_ITEM_SLOT_RIGHTEAR);
		}

		if (s & XQ_ITEM_SLOT_LEFTFINGER && s & XQ_ITEM_SLOT_RIGHTFINGER) {
			strncat(line, "FINGER ", LINE_LEN - strlen(line));
		} else {
			AddSlot(line, s, XQ_ITEM_SLOT_LEFTFINGER);
			AddSlot(line, s, XQ_ITEM_SLOT_RIGHTFINGER);
		}

		DRAWLINE;
	}


	// Report item flags
	if (iinf->nodrop || iinf->lore || iinf->expendable || iinf->norent) {
		snprintf(line, LINE_LEN, "%s%s%s%s",
			(iinf->nodrop ?		"NO DROP  "		: ""),
			(iinf->lore ?		"LORE ITEM  "	: ""),
			(iinf->expendable ?	"EXPENDABLE  "	: ""),
			(iinf->norent ?		"NO RENT  "		: "")
		);
		DRAWLINE;
	}


	// Report the classes that can use the item, if a restriction exists
	if (iinf->classes != 0) {
		strncat(line, "Classes: ", LINE_LEN);
		if (iinf->classes != XQ_ALLCLASSES) {
			for (int i = 0;  i < XQ_CLASSES;  i++) {
				if (iinf->classes & (1 << i)) {
					strncat(line, va("%s ", xq_class_lit(i, 1, 1)), LINE_LEN - strlen(line));
				}
			}
		} else {
			strncat(line, "ALL", LINE_LEN - strlen(line));
		}
		DRAWLINE;
	}

	// Races
	if (iinf->races != 0) {
		strncat(line, "Races: ", LINE_LEN);
		if (iinf->races != XQ_ALLRACES) {
			for (int i = 0;  i < XQ_RACES;  i++) {
				if (iinf->races & (1 << i)) {
					strncat(line, va("%s ", xq_race_lit(i, 1, 1)), LINE_LEN - strlen(line));
				}
			}
		} else {
			sprintf(line, "ALL");
		}
		DRAWLINE;
	}


	// Spell scroll specific info
	if (iinf->spell_scroll > 0) {
		sp = xq_SpellInfo(iinf->spell_scroll);
		if (sp) {
			snprintf(line, LINE_LEN, "Spell: %s", sp->name);
			DRAWLINE;

			snprintf(line, LINE_LEN, "Mana cost: %i", sp->mana);
			DRAWLINE;

			snprintf(line, LINE_LEN, "Classes: ");
			Spell_ClassLevel(line, sp);
			DRAWLINE;

			snprintf(line, LINE_LEN, "Casting time: %.1f", ((float)sp->casting_time / 10));
			DRAWLINE;
		}
	}


	// AC and ATK
	if (iinf->ac != 0) {
		snprintf(line, LINE_LEN, "AC: %+i  ", iinf->ac);
	}
	if (iinf->atk != 0) {
		snprintf(line + strlen(line), LINE_LEN - strlen(line), "ATK: %+i", iinf->atk);
	}
	DRAWLINE;


	// HP, MANA, ENDURANCE, ENERGY
	if (iinf->hp != 0) {
		strncat(line, va("HP: %+i ", iinf->hp), LINE_LEN - strlen(line));
	}
	if (iinf->mana != 0) {
		strncat(line, va("MANA: %+i ", iinf->mana), LINE_LEN - strlen(line));
	}
	if (iinf->endurance != 0) {
		strncat(line, va("ENDURANCE: %+i ", iinf->endurance), LINE_LEN - strlen(line));
	}
	if (iinf->energy != 0) {
		strncat(line, va("ENERGY: %+i ", iinf->energy), LINE_LEN - strlen(line));
	}
	DRAWLINE;


	// Stats such as STR etc
	if (iinf->str != 0) {
		snprintf(line, LINE_LEN, "STR: %+i ", iinf->str);
	}
	DRAWLINE;


	// Skill-up clicky
	if (iinf->skillup_skill != 0) {
		snprintf(line, LINE_LEN, "Skill up: %s (skill levels %i - %i)",
			xq_skills_literal(iinf->skillup_skill, 0, 0), iinf->skillup_minskill + 1, iinf->skillup_maxskill + 1);
		DRAWLINE;
	}

	// Resists
	if (iinf->mr != 0) strncat(line, va("MAGIC: %+i ", iinf->mr), LINE_LEN - strlen(line));
	if (iinf->fr != 0) strncat(line, va("FIRE: %+i ", iinf->fr), LINE_LEN - strlen(line));
	if (iinf->cr != 0) strncat(line, va("COLD: %+i ", iinf->cr), LINE_LEN - strlen(line));
	if (iinf->pr != 0) strncat(line, va("POISON: %+i ", iinf->pr), LINE_LEN - strlen(line));
	if (iinf->dr != 0) strncat(line, va("DISEASE: %+i ", iinf->dr), LINE_LEN - strlen(line));
	if (iinf->psr != 0) strncat(line, va("PSY: %+i ", iinf->psr), LINE_LEN - strlen(line));
	DRAWLINE;


	// Charges
	if (iinf->max_charges > 0) {
		snprintf(line, LINE_LEN, "Charges: %i  ", iinf->charges);
		DRAWLINE;
	}


	// Food / drink
	if (iinf->food > 0) {
		snprintf(line, LINE_LEN, "Food: %i kcal", iinf->food);
		DRAWLINE;
	}
	if (iinf->water > 0) {
		snprintf(line, LINE_LEN, "Water: %i cl", iinf->water);
		DRAWLINE;
	}


	// Book
	if (iinf->book[0]) {
		snprintf(line, LINE_LEN, "%s", "This is a book. Right-click to read.");
		DRAWLINE;
	}


	// Clicky spell
	if (iinf->clicky_spell) {
		sp = xq_SpellInfo(iinf->clicky_spell);
		if (sp) {
			int casting_time;
			if (iinf->casting_time == -1) {
				casting_time = sp->casting_time;
			} else {
				casting_time = iinf->casting_time;
			}
			snprintf(line, LINE_LEN, "Effect%s: %s (Casting time: %.1f sec)",
				iinf->clicky_mustequip ? "*" : "", sp->name, ((float)casting_time) / 10);

			DRAWLINE;
		}
	}


	// Worn spell
	SimpleSpell(line, iinf->worn_spell, iinf->worn_spell_minlvl, "Worn effect");
	DRAWLINE;

	// Min. level needed to equip the item
	if (iinf->equip_minlvl) {
		snprintf(line, 20, "Level to equip: %i", iinf->equip_minlvl);
		DrawLine(win, obj, x, &cury, line);
	}


	// Weapon
	char *txtskill = NULL;
	if (iinf->weapon_skill) {
		switch (iinf->weapon_skill) {
			case XQ_WEAPON_SKILL_H2H:			txtskill = "hand to hand";		break;
			case XQ_WEAPON_SKILL_1HS:			txtskill = "one hand slashing";	break;
			case XQ_WEAPON_SKILL_1HB:			txtskill = "one hand blunt";	break;
			case XQ_WEAPON_SKILL_1HP:			txtskill = "one hand piercing";	break;
			case XQ_WEAPON_SKILL_2HS:			txtskill = "two hand slashing";	break;
			case XQ_WEAPON_SKILL_2HB:			txtskill = "two hand blunt";	break;
			case XQ_WEAPON_SKILL_2HP:			txtskill = "two hand piercing";	break;
			case XQ_WEAPON_SKILL_MACHINEGUN:	txtskill = "magic 2";			break;
			case XQ_WEAPON_SKILL_SHOTGUN:		txtskill = "magic 3";			break;
			case XQ_WEAPON_SKILL_GRENADE:		txtskill = "magic 4";			break;
			case XQ_WEAPON_SKILL_ROCKET:		txtskill = "magic 5";			break;
			case XQ_WEAPON_SKILL_SHAFT:			txtskill = "magic 6";			break;
			case XQ_WEAPON_SKILL_RAIL:			txtskill = "magic 7";			break;
			case XQ_WEAPON_SKILL_PLASMA:		txtskill = "magic 8";			break;
			case XQ_WEAPON_SKILL_BFG:			txtskill = "magic 9";			break;

			default:
				txtskill = "UNKNOWN";
				break;
		}
		snprintf(line, LINE_LEN, "Weapon: %s  Energy cost: %i", txtskill, iinf->weapon_energycost);
		DRAWLINE;
		snprintf(line, LINE_LEN, "Damage: %i  Delay: %i", iinf->weapon_damage, iinf->weapon_delay);
		DRAWLINE;
	}


	// Procs
	SimpleSpell(line, iinf->weapon_proc_fire, iinf->weapon_proc_fire_level, "Fire proc");
	DRAWLINE;
	SimpleSpell(line, iinf->weapon_proc_impact, iinf->weapon_proc_impact_level, "Impact proc");
	DRAWLINE;
	SimpleSpell(line, iinf->weapon_proc_explode, iinf->weapon_proc_explode_level, "Explode proc");
	DRAWLINE;


	// Weight and weight reduction
	if (iinf->weight != 0) {
		snprintf(line, LINE_LEN, "Weight: %.1f ", (float)iinf->weight / 10.0);
	}
	if (iinf->weight_reduction != 0) {
		strncat(line, va("Weight reduction: %i%%", iinf->weight_reduction), LINE_LEN - strlen(line));
	}
	DRAWLINE;
}

#include "../../cg_local.h"

static void	CheckMagics(void) {
	int i;

	assert(qws->magics == qw_t_MAGIC);
	assert(qws->magice == qw_t_MAGIC);

	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (qws->win[i].wid[0]) {
			assert(qws->win[i].magics == qw_window_t_MAGIC);
			assert(qws->win[i].magice == qw_window_t_MAGIC);
		}
	}

	for (i = 0;  i < QW_MAX_OBJ;  i++) {
		if (qws->obj[i].type != QW_OBJ_NONE) {
			assert(qws->obj[i].magics == qw_obj_t_MAGIC);
			assert(qws->obj[i].magice == qw_obj_t_MAGIC);
		}
	}
}

static void AdvanceAlpha(void) {
	// call qw_WinAlpha() for every existing alpha window.
	for (int i = 0;  i < QW_MAX_WINDOWS;  i++) {
		qw_window_t *win = &qws->win[i];
		if (win->wid[0]) {
			if (win->alpha_min != 1.0F) {
				qw_WinAlpha(win->wid);
			}
		}
	}
}
static void HoverTooltip(void) {
	// Find hovered object and see if we've been hovering
	// over it for more than QW_OBJ_TOOLTIP_FRAMES. If so - show the tooltip, if any.

	static int old_hoverwin = -1, old_hoverobj = -1;
	static int tooltip_shown = 0;
	int hoverobj = -1;
	static int frames = 0;
	char *tooltiptxt = NULL;


	// See how is our hovering state compared to the previous frame.
	int hoverwin = qw_WindowFindXY(xqst->mousex, xqst->mousey, NULL);
	if (hoverwin > -1) {
		qw_obj_click_t *findobj =
			qw_ObjectFindXY(hoverwin, xqst->mousex, xqst->mousey, 0, QW_OBJ_TOOLTIP);
		hoverobj = findobj->objid;
	}



	if (old_hoverwin != hoverwin || old_hoverobj != hoverobj) {
		// Something changed - remove the tooltips for now
		frames = 0;
		tooltip_shown = 0;
		qw_TooltipCloseAll();
	} else {
		if (hoverwin > -1 && hoverobj > -1) {
			frames++;
		}
	}
	old_hoverwin = hoverwin;
	old_hoverobj = hoverobj;

	if (frames < QW_OBJ_TOOLTIP_FRAMES) return;
	if (tooltip_shown) return;


	// If we're not hovering over an object - we're done.
	if (hoverobj == -1) {
		return;
	}


	// The tooltip isn't shown and we've been hovering over something long enough
	qw_window_t *win = &qws->win[hoverwin];
	int objnum = win->obj[hoverobj].num;

	if (objnum != -1) {
		qw_obj_t *ttobj = &qws->obj[objnum];

		if (ttobj->tooltip != NULL) {
			tooltiptxt = ttobj->tooltip;
		} else if (ttobj->type == QW_OBJ_SPELL_GEM && xqst->memmed_spells[ttobj->slot - 1] > 0) {
			xq_spell_t *spell = xq_SpellInfo(xqst->memmed_spells[ttobj->slot - 1]);
			if (spell) { // This can fail if we didn't get (yet) the spell info from the server.
				tooltiptxt = spell->name;
			}
		}

		// We have a valid tooltip text - go ahead and display it
		if (tooltiptxt && strlen(tooltiptxt)) {
			// Find attach x/y of the object in the window
			// so we can stick the tooltip nearby.
			int x = win->obj[hoverobj].actual_x;
			int y = win->obj[hoverobj].actual_y;

			if (ttobj->type == QW_OBJ_SPELL_GEM) {
				x += (QW_OBJ_SPELL_GEM_WIDTH + ttobj->borderwidth + 3);
				y += (QW_OBJ_SPELL_GEM_HEIGHT / 2);
			}

			tooltip_shown = 1;
			qw_obj_t tmpobj;
			qw_ObjInit(&tmpobj, QW_OBJ_TOOLTIP);
			tmpobj.text = tooltiptxt;
			qw_ObjAttach(win->wid, qw_ObjCreate(&tmpobj), x, y, 0, 0);
		}
	}
}
static void AltTooltips(void) {
	static int save = 0;

	// Don't do anything if ALT key state haven't changed since last frame
	if (xqst->show_gem_tooltips == save) return;
	save = xqst->show_gem_tooltips;


	// ALT key no longer pressed - delete all tooltips
	if (save == 0) {
		qw_TooltipCloseAll();
		return;
	}

	// ALT key just got pressed - show tooltips for everything that can have one
	// Go through all the windows, and through every attachment slot of each window
	// and if it has a tooltip - display it.
	for (int i = 0;  i < QW_MAX_WINDOWS;  i++) {
		qw_window_t *win = &qws->win[i];

		// If the window isn't even active, skip it
		if (!win->wid[0]) continue;

		for (int j = 0;  j < QW_OBJ_PER_WIN;  j++) {
			qw_obj_attach_t *att = &win->obj[j];
			// If the object attachment has no object - skip it
			if (!att->num) continue;

			qw_obj_t *obj = &qws->obj[att->num];
			qw_obj_t tmpobj;
			qw_ObjInit(&tmpobj, QW_OBJ_TOOLTIP);

			// Invslot
			if (obj->type == QW_OBJ_INVSLOT) {
				int64_t item = xq_InvSlotItem(obj->slot, obj->container_id, obj->container_slot);
				// Skip the invslot if it doesn't have an item in it
				if (!item) continue;

				xq_item_t *iinf = xq_ItemInfo(item);
				// Skip the invslot if we didn't get the item info from the server yet
				if (!iinf) continue;

				// Show the invslot tooltip
				Q_strncpyz(tmpobj.text_static, iinf->name, sizeof(tmpobj.text_static));
				qw_ObjAttach(win->wid, qw_ObjCreate(&tmpobj), att->x, att->y, 0, 0);

			// Effects window
			} else if (obj->type == QW_OBJ_SPELL_ICON) {
				// Skip it if there's no spell effect in that slot
				if (xqst->spfx[obj->slot - 1] == 0) continue;

				// Show the spell effect tooltip
				tmpobj.text = obj->tooltip; // includes remaining timer text
				qw_ObjAttach("spfx", qw_ObjCreate(&tmpobj), att->x, att->y, 0, 0);

			// Spell gems
			} else if (obj->type == QW_OBJ_SPELL_GEM && xq_seq(win->wid, "spells")) {
				int spid = xqst->memmed_spells[obj->slot-1];
				// Skip if no memmed spell for that gem
				if (!spid) continue;

				// Skip if we don't have server info on the spell
				xq_spell_t *spell = xq_SpellInfo(spid);
				if (!spell) continue;

				// Go ahead and show the tooltip
				qw_ObjInit(&tmpobj, QW_OBJ_TOOLTIP);
				tmpobj.text = spell->name;
				qw_ObjAttach(
					"spells",
					qw_ObjCreate(&tmpobj),
					att->x + QW_OBJ_SPELL_GEM_WIDTH + obj->borderwidth + 3,
					att->y + QW_OBJ_SPELL_GEM_HEIGHT / 2,
					0,
					0
				);
			}
		}
	}
}
static void InvSlotSelectionCheck(void) {
	// If an item poofs while vendoring (probably due to merchant transaction), this will make
	// sure the inv/merch slot will not continue appearing as selected.

	for (int i = 1;  i < QW_MAX_OBJ;  i++) {
		qw_obj_t *o = &qws->obj[i];
		if (o->type == QW_OBJ_INVSLOT && o->selected) {
			if (xq_InvSlotItem(o->slot, o->container_id, o->container_slot) == 0) {
				o->selected = 0;
			}
		}
	}
}
static void GemSet(int slotnum, int goalval, int setcurval, int curval) {
	qws->gem_fade[slotnum].goalval = goalval;

	if (setcurval) {
		qws->gem_fade[slotnum].curval = curval;
	}
}
static void SpellGemFade(void) {
	// Sets spell gem alpha goals depending on their state.
	// Regardless of that, we'll also move current alpha values of spell gems
	// if they haven't reached their goals.

	for (int i = 0;  i < XQ_SPELL_SLOTS;  i++) {
		if (!xqst->memmed_spells[i]) {
			if (xqst->memmed_spells_prev[i]) {
				// Slot changed from full to empty - just snap reset it to default state (full visibility).
				GemSet(i, 0, 1, 0); 
			}
		} else {
			if (xqst->memmed_spells[i]) {
				if (!xqst->memmed_spells_prev[i]) {
					// Slot changed from empty to full.
					// Snap set it to faded (it will be set to fade in to full vis if and when it becomes castable)
					GemSet(i, 100, 1, 100); 
				} else {
					// By now we have a memmed spell, and it was already memmed last frame.
					// See if its castability status needs to be taken into account.
					int down = 0, down_prev = 0;
					if (xqst->casting_down || xqst->memmed_spells_down[i]) {
						down = 1;
					}
					if (xqst->casting_down_prev || xqst->memmed_spells_down_prev[i]) {
						down_prev = 1;
					}

					if (down && !down_prev) {
						// Spell gem changed from up to down this frame - set it to fade out gently.
						GemSet(i, 100, 0, 0); 
					} else if (!down && down_prev) {
						if (!xqst->autocast_gem) {
							// And the other way around (except if we're autocasting a bard song).
							// Set it to fade in gently.
							GemSet(i, 0, 0, 0); 
						}
					} else if (down && down_prev) {
						// We're down and were down last frame.
						// Make sure we're either faded, or our goal is to fade, otherwise make it so.
						if (qws->gem_fade[i].goalval != 100) {
							GemSet(i, 100, 0, 0);
						}
					} else if (!down && !down_prev) {
						// We're up and were up last frame.
						// Make sure we're either visible, or our goal is to be visible, otherwise make it so (unless we're autocasting)
						if (qws->gem_fade[i].goalval != 0) {
							if (!xqst->autocast_gem) {
								GemSet(i, 0, 0, 0);
							}
						}
					}
				}
			}
		}

		// Copy current info to _prev structure to be used in the next client frame.
		xqst->memmed_spells_prev[i] = xqst->memmed_spells[i];
		xqst->memmed_spells_down_prev[i] = xqst->memmed_spells_down[i];
		xqst->casting_down_prev = xqst->casting_down;


		// Advance the curval values according to their directions
		int *curval = &qws->gem_fade[i].curval;
		int *goalval = &qws->gem_fade[i].goalval;
		if (*curval < *goalval) {
			*curval += 4;
		} else if (*curval > *goalval) {
			*curval -= 4;
		}
	}
}

void qw_Frame() {
	// Low-level QW-related stuff that is executed once per client frame,
	// such as windows alpha changes, hover tooltips, etc.


	CheckMagics();


	// Increase/decrease window alpha values if they are already set to change in a way
	AdvanceAlpha();


	// See if we a tooltip should be displayed because of mouse hovering
	HoverTooltip();


	// Holding ALT key triggers tooltips on everything
	AltTooltips();


	// Has an item marked as vendor-selected poofed for some reason?
	InvSlotSelectionCheck();


	// Set spell gem fade in/out state depending on what's going on with the gem
	SpellGemFade();
}

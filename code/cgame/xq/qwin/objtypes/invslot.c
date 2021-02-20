#include "../../../cg_local.h"

static void ItemRightClick(xq_item_t *iinf, int slot) {
	playerState_t *ps = &cg.snap->ps;

	if (iinf->spell_scroll) {
		qw_Prompt(qws->strings.scribeTheSpell, QW_PROMPT_ACTION_SCRIBE, 0, slot, 0, 0);

	} else if (iinf->clicky_spell) {
		if (iinf->max_charges > 0) {
			if (iinf->charges < 1) {
				xq_clog(COLOR_RED, "The item is out of charges.");
				return;
			}
		}
		switch (iinf->clicky_spell_target) {
			case XQ_SPELL_TARGET_TGAE:
			case XQ_SPELL_TARGET_SINGLE:
				if (!ps->xq_target) {
					xq_clog(COLOR_RED, "You need a target for this spell.");
					return;
				}
		}
		xq_RunSlot(slot);

	} else if (iinf->skillup_skill) {
		qw_Prompt(
			va("This will increase your %s skill. Proceed?",xq_skills_literal(iinf->skillup_skill, 0, 0)),
			QW_PROMPT_ACTION_SKILLUP, 0, slot, 0, 0);

	} else if (iinf->food || iinf->water) {
		xq_RunSlot(slot);

	} else if (iinf->book[0]) {
		xqui_ReadBook(iinf->id);
	}
}
static void ContainerRightClick(int slot) {
	char wname[QW_MAX_WID+1] = {0};

	xqui_Container_GetWID(slot, wname);
	if (!wname[0]) return;


	// Close a container if it's open, open it otherwise
	if (qw_WindowExists(wname) > -1) {
		xqui_ContainerClose(slot, 0);
	} else {
		xqui_ContainerOpen(slot, 0);
	}
}
static void LootClick(qw_obj_t *obj, int button) {
	// Find out the slot offset
	int slot = obj->slot - XQ_ITEM_SLOT_LOOT1 + 1;


	// Make sure the requested slot is valid
	if (slot > XQ_LOOT_MAX_SLOT) return;


	// Make sure we have something to loot in the slot
    playerState_t *ps = &cg.snap->ps;
	int64_t item = XQ64(
		*(&ps->xq_loot_slot_1_1 + ((slot - 1) * 2)),
		*(&ps->xq_loot_slot_1_2 + ((slot - 1) * 2))
	);
	if (item == 0) return;


	// If we're looting a NODROP item from an NPC corpse - go through a confirmation popup.
	if (ps->xq_flags & XQ_LOOTING_NPC) {
		xq_item_t *iinf = xq_ItemInfo(item);
		if (iinf == NULL) {
			xq_clog(COLOR_RED, "Cannot fetch item info for looting.");
			return;
		}

		if (iinf->nodrop) {
			qw_Prompt(qws->strings.lootTheNodropItem, QW_PROMPT_ACTION_LOOT, 0, button + 1, slot, 0);
			return;
		}
	}

	// Go ahead and loot the item
	if (button == 0) {
		xq_scmd(va("/lootslot 1 %i", slot));
	} else if (button == 1) {
		xq_scmd(va("/lootslot 2 %i", slot));
	}
}

void qw_Obj_InvSlot_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt) {
	qw_obj_t *o;
	qw_obj_t *buy_button, *sell_button;
	qw_window_t *w;
    playerState_t *ps = &cg.snap->ps;
	int64_t item = 0;
	xq_item_t *iinf, *iinf_tmp;
	int slot;

	w = &qws->win[win];
	o = &qws->obj[w->obj[obj].num];
	buy_button = &qws->obj[qws->objhandles[XQ_UI_OBJID_BUTTON_BUY]];
	sell_button = &qws->obj[qws->objhandles[XQ_UI_OBJID_BUTTON_SELL]];



	// Handle loot slots separately and return
	slot = o->slot - XQ_ITEM_SLOT_LOOT1 + 1;
	if (slot >= 1 && slot <= XQ_LOOT_MAX_SLOT) {
		if (
			// If we loot (with left or right mouse button)
				(button == 1 && !shift && !ctrl && !alt) ||
				(button == 0 && !shift && !ctrl && !alt)
			) {
			LootClick(o, button);
			return;
		} else if (!(button == 0 && !shift && !ctrl && alt)) {
			// If we don't Alt-left click to inspect
			return;
		}
		item = XQ64(
			*(&ps->xq_loot_slot_1_1 + ((slot - 1) * 2)),
			*(&ps->xq_loot_slot_1_2 + ((slot - 1) * 2))
		);
	}


	// Figure out what item (if any) we have in that inventory slot.
	// We don't use pointers here so the order of the relevant ps-> fields can remain arbitrary
	switch (o->slot) {
		case XQ_ITEM_SLOT_PRIMARY1: 		item = XQ64(ps->xq_inv_primary1_1,		ps->xq_inv_primary1_2); 		break;
		case XQ_ITEM_SLOT_SECONDARY1: 		item = XQ64(ps->xq_inv_secondary1_1,	ps->xq_inv_secondary1_2);		break;
		case XQ_ITEM_SLOT_PRIMARY2: 		item = XQ64(ps->xq_inv_primary2_1,		ps->xq_inv_primary2_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY3: 		item = XQ64(ps->xq_inv_primary3_1,		ps->xq_inv_primary3_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY4: 		item = XQ64(ps->xq_inv_primary4_1,		ps->xq_inv_primary4_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY5: 		item = XQ64(ps->xq_inv_primary5_1,		ps->xq_inv_primary5_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY6: 		item = XQ64(ps->xq_inv_primary6_1,		ps->xq_inv_primary6_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY7: 		item = XQ64(ps->xq_inv_primary7_1,		ps->xq_inv_primary7_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY8: 		item = XQ64(ps->xq_inv_primary8_1,		ps->xq_inv_primary8_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY9: 		item = XQ64(ps->xq_inv_primary9_1,		ps->xq_inv_primary9_2); 		break;

		case XQ_ITEM_SLOT_LEFTEAR: 			item = XQ64(ps->xq_inv_leftear_1,		ps->xq_inv_leftear_2); 			break;
		case XQ_ITEM_SLOT_RIGHTEAR: 		item = XQ64(ps->xq_inv_rightear_1,		ps->xq_inv_rightear_2); 		break;
		case XQ_ITEM_SLOT_HEAD: 			item = XQ64(ps->xq_inv_head_1,			ps->xq_inv_head_2); 			break;
		case XQ_ITEM_SLOT_FACE: 			item = XQ64(ps->xq_inv_face_1,			ps->xq_inv_face_2); 			break;
		case XQ_ITEM_SLOT_CHEST: 			item = XQ64(ps->xq_inv_chest_1,			ps->xq_inv_chest_2); 			break;
		case XQ_ITEM_SLOT_ARMS: 			item = XQ64(ps->xq_inv_arms_1,			ps->xq_inv_arms_2); 			break;
		case XQ_ITEM_SLOT_WAIST: 			item = XQ64(ps->xq_inv_waist_1,			ps->xq_inv_waist_2); 			break;
		case XQ_ITEM_SLOT_LEFTWRIST: 		item = XQ64(ps->xq_inv_leftwrist_1,		ps->xq_inv_leftwrist_2); 		break;
		case XQ_ITEM_SLOT_RIGHTWRIST: 		item = XQ64(ps->xq_inv_rightwrist_1,	ps->xq_inv_rightwrist_2); 		break;
		case XQ_ITEM_SLOT_LEGS: 			item = XQ64(ps->xq_inv_legs_1,			ps->xq_inv_legs_2); 			break;
		case XQ_ITEM_SLOT_HANDS: 			item = XQ64(ps->xq_inv_hands_1,			ps->xq_inv_hands_2); 			break;
		case XQ_ITEM_SLOT_LEFTFINGER: 		item = XQ64(ps->xq_inv_leftfinger_1,	ps->xq_inv_leftfinger_2); 		break;
		case XQ_ITEM_SLOT_RIGHTFINGER: 		item = XQ64(ps->xq_inv_rightfinger_1,	ps->xq_inv_rightfinger_2);		break;
		case XQ_ITEM_SLOT_FEET: 			item = XQ64(ps->xq_inv_feet_1,			ps->xq_inv_feet_2); 			break;
		case XQ_ITEM_SLOT_SHOULDERS: 		item = XQ64(ps->xq_inv_shoulders_1,		ps->xq_inv_shoulders_2); 		break;
		case XQ_ITEM_SLOT_BACK: 			item = XQ64(ps->xq_inv_back_1,			ps->xq_inv_back_2); 			break;
		case XQ_ITEM_SLOT_NECK: 			item = XQ64(ps->xq_inv_neck_1,			ps->xq_inv_neck_2); 			break;
		case XQ_ITEM_SLOT_CARRY1: 			item = XQ64(ps->xq_inv_carry1_1,		ps->xq_inv_carry1_2); 			break;
		case XQ_ITEM_SLOT_CARRY2: 			item = XQ64(ps->xq_inv_carry2_1,		ps->xq_inv_carry2_2); 			break;
		case XQ_ITEM_SLOT_CARRY3: 			item = XQ64(ps->xq_inv_carry3_1,		ps->xq_inv_carry3_2); 			break;
		case XQ_ITEM_SLOT_CARRY4: 			item = XQ64(ps->xq_inv_carry4_1,		ps->xq_inv_carry4_2); 			break;
		case XQ_ITEM_SLOT_CARRY5: 			item = XQ64(ps->xq_inv_carry5_1,		ps->xq_inv_carry5_2); 			break;
		case XQ_ITEM_SLOT_CARRY6: 			item = XQ64(ps->xq_inv_carry6_1,		ps->xq_inv_carry6_2); 			break;
		case XQ_ITEM_SLOT_BANK1: 			item = XQ64(ps->xq_bank_slot_1_1,		ps->xq_bank_slot_1_2); 			break;
		case XQ_ITEM_SLOT_BANK2: 			item = XQ64(ps->xq_bank_slot_2_1,		ps->xq_bank_slot_2_2); 			break;
		case XQ_ITEM_SLOT_BANK3: 			item = XQ64(ps->xq_bank_slot_3_1,		ps->xq_bank_slot_3_2); 			break;
		case XQ_ITEM_SLOT_BANK4: 			item = XQ64(ps->xq_bank_slot_4_1,		ps->xq_bank_slot_4_2); 			break;

		case XQ_ITEM_SLOT_MERCHANT1: 		item = XQ64(ps->xq_merchant_slot_1_1,	ps->xq_merchant_slot_1_2);		break;
		case XQ_ITEM_SLOT_MERCHANT2: 		item = XQ64(ps->xq_merchant_slot_2_1,	ps->xq_merchant_slot_2_2);		break;
		case XQ_ITEM_SLOT_MERCHANT3: 		item = XQ64(ps->xq_merchant_slot_3_1,	ps->xq_merchant_slot_3_2);		break;
		case XQ_ITEM_SLOT_MERCHANT4: 		item = XQ64(ps->xq_merchant_slot_4_1,	ps->xq_merchant_slot_4_2);		break;
		case XQ_ITEM_SLOT_MERCHANT5: 		item = XQ64(ps->xq_merchant_slot_5_1,	ps->xq_merchant_slot_5_2);		break;
		case XQ_ITEM_SLOT_MERCHANT6: 		item = XQ64(ps->xq_merchant_slot_6_1,	ps->xq_merchant_slot_6_2);		break;
		case XQ_ITEM_SLOT_MERCHANT7: 		item = XQ64(ps->xq_merchant_slot_7_1,	ps->xq_merchant_slot_7_2);		break;
		case XQ_ITEM_SLOT_MERCHANT8: 		item = XQ64(ps->xq_merchant_slot_8_1,	ps->xq_merchant_slot_8_2);		break;
		case XQ_ITEM_SLOT_MERCHANT9: 		item = XQ64(ps->xq_merchant_slot_9_1,	ps->xq_merchant_slot_9_2);		break;
		case XQ_ITEM_SLOT_MERCHANT10: 		item = XQ64(ps->xq_merchant_slot_10_1,	ps->xq_merchant_slot_10_2);		break;
		case XQ_ITEM_SLOT_MERCHANT11: 		item = XQ64(ps->xq_merchant_slot_11_1,	ps->xq_merchant_slot_11_2);		break;
		case XQ_ITEM_SLOT_MERCHANT12: 		item = XQ64(ps->xq_merchant_slot_12_1,	ps->xq_merchant_slot_12_2);		break;
		case XQ_ITEM_SLOT_MERCHANT13: 		item = XQ64(ps->xq_merchant_slot_13_1,	ps->xq_merchant_slot_13_2);		break;
		case XQ_ITEM_SLOT_MERCHANT14: 		item = XQ64(ps->xq_merchant_slot_14_1,	ps->xq_merchant_slot_14_2);		break;
		case XQ_ITEM_SLOT_MERCHANT15: 		item = XQ64(ps->xq_merchant_slot_15_1,	ps->xq_merchant_slot_15_2);		break;
		case XQ_ITEM_SLOT_MERCHANT16: 		item = XQ64(ps->xq_merchant_slot_16_1,	ps->xq_merchant_slot_16_2);		break;
		case XQ_ITEM_SLOT_MERCHANT17: 		item = XQ64(ps->xq_merchant_slot_17_1,	ps->xq_merchant_slot_17_2);		break;
		case XQ_ITEM_SLOT_MERCHANT18: 		item = XQ64(ps->xq_merchant_slot_18_1,	ps->xq_merchant_slot_18_2);		break;
		case XQ_ITEM_SLOT_MERCHANT19: 		item = XQ64(ps->xq_merchant_slot_19_1,	ps->xq_merchant_slot_19_2);		break;
		case XQ_ITEM_SLOT_MERCHANT20: 		item = XQ64(ps->xq_merchant_slot_20_1,	ps->xq_merchant_slot_20_2);		break;

		case XQ_ITEM_SLOT_TRADE_GIVE1: 		item = XQ64(ps->xq_give_slot_1_1,		ps->xq_give_slot_1_2); 			break;
		case XQ_ITEM_SLOT_TRADE_GIVE2: 		item = XQ64(ps->xq_give_slot_2_1,		ps->xq_give_slot_2_2); 			break;
		case XQ_ITEM_SLOT_TRADE_GIVE3: 		item = XQ64(ps->xq_give_slot_3_1,		ps->xq_give_slot_3_2); 			break;
		case XQ_ITEM_SLOT_TRADE_GIVE4: 		item = XQ64(ps->xq_give_slot_4_1,		ps->xq_give_slot_4_2); 			break;
		case XQ_ITEM_SLOT_TRADE_RECEIVE1: 	item = XQ64(ps->xq_receive_slot_1_1,	ps->xq_receive_slot_1_2);		break;
		case XQ_ITEM_SLOT_TRADE_RECEIVE2: 	item = XQ64(ps->xq_receive_slot_2_1,	ps->xq_receive_slot_2_2);		break;
		case XQ_ITEM_SLOT_TRADE_RECEIVE3: 	item = XQ64(ps->xq_receive_slot_3_1,	ps->xq_receive_slot_3_2);		break;
		case XQ_ITEM_SLOT_TRADE_RECEIVE4: 	item = XQ64(ps->xq_receive_slot_4_1,	ps->xq_receive_slot_4_2);		break;

		// We do use pointers here because we consider that container slot fields will always be in order
		case XQ_ITEM_SLOT_INCONTAINER:
			iinf_tmp = xq_ItemInfo(o->container_id);
			if (iinf_tmp != NULL) {
				item = *(&iinf_tmp->container_slot_1 + o->container_slot - 1);
			}
			break;
					
	}


	// Go ahead and do the deed
	if (button == 0 && !alt) {
		// Left click - swap slot with mouse cursor
		// Close container if needed
		// If we click on a stackable item, unless it's just one, open amount picker.
		//
		// Shift left click - pick whole stack if stackable, or just pick the item if not.
		//
		// Ctrl left click - pick one item out of stack if stackable, or just pick the item if not.
		//

		int quoteonly = 0;
		if (o->slot == XQ_ITEM_SLOT_INCONTAINER) {
			// Don't let left clicks on in-container slots go through if container is in a give or receive slot.
			if (
				o->container_id == xq_InvSlotItem(XQ_ITEM_SLOT_TRADE_GIVE1, 0, 0)
				|| o->container_id == xq_InvSlotItem(XQ_ITEM_SLOT_TRADE_GIVE2, 0, 0)
				|| o->container_id == xq_InvSlotItem(XQ_ITEM_SLOT_TRADE_GIVE3, 0, 0)
				|| o->container_id == xq_InvSlotItem(XQ_ITEM_SLOT_TRADE_GIVE4, 0, 0)
				|| o->container_id == xq_InvSlotItem(XQ_ITEM_SLOT_TRADE_RECEIVE1, 0, 0)
				|| o->container_id == xq_InvSlotItem(XQ_ITEM_SLOT_TRADE_RECEIVE2, 0, 0)
				|| o->container_id == xq_InvSlotItem(XQ_ITEM_SLOT_TRADE_RECEIVE3, 0, 0)
				|| o->container_id == xq_InvSlotItem(XQ_ITEM_SLOT_TRADE_RECEIVE4, 0, 0)) {
					return;

			}
		}


		qw_TooltipCloseAll();

		// Don't do anything if we left click an empty inv slot with an empty mouse
		if (XQ64(ps->xq_inv_mouse1_1, ps->xq_inv_mouse1_2) == 0 && item == 0) {
			return;
		}


		// Whatever happens next, the eventual open container window for the target slot needs to go
		xqui_ContainerClose(o->slot, 0);


		// Deal with the shift-LMB / ctrl-LMB stack picking stuff
		int amount = 0;
		if (item != 0) {
			iinf = xq_ItemInfo(item);
			if (iinf == NULL) {
				return;
			}
			if (iinf->stackable > 0 && qws->buying == 0) {
				if (ctrl) {
					amount = 1;
				} else if (shift) {
					amount = iinf->amount;
				} else {
					if (iinf->amount == 1) {
						amount = 1;
					} else {
						if (qws->amount_picker_value > 0) {
							amount = qws->amount_picker_value;
							if (amount > iinf->stackable) {
								amount = iinf->stackable;
							}
						} else {
							if (XQ64(ps->xq_inv_mouse1_1, ps->xq_inv_mouse1_2) == 0) {
								qw_Obj_AmountPicker(1, 1, 1, iinf->amount, xqst->mousex, xqst->mousey, "swapwithmouseslot", o->slot, o->container_id, o->container_slot);
								return;
							}
						}
					}
				}
			}
		}

		// If we're interacting with a mechant, LBM clicks on an item don't actually do
		// anything other than ask the merchant for a price quote / cash offer
		if (qws->buying) {

			// Set the item's info (if there's an item) to display in the merchant's window
			qws->obj[qws->objhandles[XQ_UI_OBJID_MERCHANT_ITEMGFX]].item = item;
			if (item) {
				Q_strncpyz(qws->obj[qws->objhandles[XQ_UI_OBJID_MERCHANT_ITEMTXT]].text_static,
					iinf->name,
					sizeof(qws->obj[qws->objhandles[0]].text_static)
				);
			} else {
				qws->obj[qws->objhandles[XQ_UI_OBJID_MERCHANT_ITEMTXT]].text_static[0] = 0;
			}

			// Depending on whether we click on an item the merchant sells or an item
			// in our own inventory, set the sell/buy buttons accordingly
			if (o->slot >= XQ_ITEM_SLOT_MERCHANT1 && o->slot <= XQ_ITEM_SLOT_MERCHANT20) {
				buy_button->arg1 = o->slot;
				buy_button->arg2 = 0;
				buy_button->arg3 = 0;
				buy_button->arg4 = item;
				sell_button->arg1 = 0;
				sell_button->arg2 = 0;
				sell_button->arg3 = 0;
				sell_button->arg4 = 0;
				buy_button->hidden = 0;
				sell_button->hidden = 1;
			} else {
				sell_button->arg1 = o->slot;
				sell_button->arg2 = (o->slot == XQ_ITEM_SLOT_INCONTAINER ? o->container_id : 0);
				sell_button->arg3 = (o->slot == XQ_ITEM_SLOT_INCONTAINER ? o->container_slot : 0);
				sell_button->arg4 = item;
				buy_button->arg1 = 0;
				buy_button->arg2 = 0;
				buy_button->arg3 = 0;
				buy_button->arg4 = 0;
				buy_button->hidden = 1;
				sell_button->hidden = 0;
			}
			quoteonly = 1; // LMB while vendoring only asks for asked/offered price for the slot's item

			// This marks the quoted item as selected - will draw something around it to show it
			if (item) {
				xqui_InvSlotUnselectAll();
				o->selected = 1;
			}
		}
		trap_S_StartLocalSound(SOUNDH("sound/inv/itemswap.wav"), CHAN_ANNOUNCER);


		// Issue the server command
		xq_SwapWithMouseSlot(
			o->slot,
			(o->slot == XQ_ITEM_SLOT_INCONTAINER ? o->container_id : 0),
			o->slot == XQ_ITEM_SLOT_INCONTAINER ? o->container_slot : 0,
			amount,
			quoteonly
		);

	} else if (button == 0 && !shift && !ctrl && alt && item != 0) {
		// Alt left click - inspect item
		xqui_OpenInspector(item, 0);

	} else if (button == 1 && !shift && !ctrl && !alt && item != 0) {
		// Right click an item
		if (
			o->slot == XQ_ITEM_SLOT_CARRY1
			|| o->slot == XQ_ITEM_SLOT_CARRY2
			|| o->slot == XQ_ITEM_SLOT_CARRY3
			|| o->slot == XQ_ITEM_SLOT_CARRY4
			|| o->slot == XQ_ITEM_SLOT_CARRY5
			|| o->slot == XQ_ITEM_SLOT_CARRY6
			|| o->slot == XQ_ITEM_SLOT_BANK1
			|| o->slot == XQ_ITEM_SLOT_BANK2
			|| o->slot == XQ_ITEM_SLOT_BANK3
			|| o->slot == XQ_ITEM_SLOT_BANK4
			|| o->slot == XQ_ITEM_SLOT_TRADE_GIVE1
			|| o->slot == XQ_ITEM_SLOT_TRADE_GIVE2
			|| o->slot == XQ_ITEM_SLOT_TRADE_GIVE3
			|| o->slot == XQ_ITEM_SLOT_TRADE_GIVE4
			|| o->slot == XQ_ITEM_SLOT_TRADE_RECEIVE1
			|| o->slot == XQ_ITEM_SLOT_TRADE_RECEIVE2
			|| o->slot == XQ_ITEM_SLOT_TRADE_RECEIVE3
			|| o->slot == XQ_ITEM_SLOT_TRADE_RECEIVE4
			|| (o->slot >= XQ_ITEM_SLOT_PRIMARY && o->slot <= XQ_ITEM_SLOT_NECK)
		) {

			iinf = xq_ItemInfo(item);
			if (iinf != NULL) {
				if (iinf->container_slots > 0) {
					ContainerRightClick(o->slot);
				} else {
					ItemRightClick(iinf, o->slot);
				}
			}
		}
	}
}
void qw_Obj_InvSlot_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	int64_t val = -1;
	xq_item_t *iinf;
	int col, col_selected;
	qw_obj_t *obj = &qws->obj[att->num];

	obj->h = QW_OBJ_INVSLOT_HEIGHT;
	obj->w = QW_OBJ_INVSLOT_WIDTH;

	int x = att->x;
	int y = att->y;

	if (att->x_centered == 0) {
		if (x < 0) x = (win->w - obj->w) - QW_WIN_BORDER_WIDTH;
	} else {
		x = (win->w / 2) - obj->w / 2;
	}
	if (att->y_centered == 0) {
		if (y < 0) y = (win->h - QW_OBJ_INVSLOT_HEIGHT) - QW_WIN_BORDER_WIDTH;
	} else {
		y = (win->h / 2) - QW_OBJ_INVSLOT_HEIGHT / 2;
	}

	att->actual_x = x;
	att->actual_y = y;

	x += (win->x + QW_WIN_BORDER_WIDTH);
	y += (win->y + QW_WIN_BORDER_WIDTH);


	// Decide on the border color
	col = QW_OBJ_INVSLOT_UNSELECTED;
	col_selected = QW_OBJ_INVSLOT_SELECTED;
	if (xq_WeaponActiveSlot() == obj->slot) {
		col = QW_OBJ_INVSLOT_SELECTED_WEAP;
	}
	xq_DrawRect(
		x,
		y,
		obj->w,
		obj->h,
		1,
		xq_Color2Vec(col)
	);


	// Draw the background
	xq_FillRect(
		x + 1,
		y + 1,
		obj->w - 2,
		obj->h - 2,
		xq_Color2Vec(qw_Hover(obj))
	);



	// If this invslot is part of a container, make sure the id of the container hasn't changed,
	// which can happen for various reasons. If it changed, update the object.
	if (obj->parent_slot) {
		int64_t parent_item = xq_InvSlotItem(obj->parent_slot, 0, 0);
		if (parent_item) {
			if (parent_item != obj->container_id) {
				obj->container_id = parent_item;
			}
		}
	}


	// Draw the actual item, or an eventual placeholder background icon if the slot is empty
	val = xq_InvSlotItem(obj->slot, obj->container_id, obj->container_slot);
	if (val != 0 && val != XQ_ITEM_ERROR) {
		iinf = xq_ItemInfo(val);
		if (iinf != NULL) {
			obj->tooltip = iinf->name;
			xq_DrawItemIcon(win, x + 3, y + 3, iinf->icon, iinf->amount);
		}
	} else {
		// Figure out the background icon to display
		char buf[100] = {0};
		switch (obj->slot) {
			case XQ_ITEM_SLOT_PRIMARY1:
				strcpy((char *)buf, "gfx/2d/inventory/primary.png");								break;

			case XQ_ITEM_SLOT_PRIMARY2:
			case XQ_ITEM_SLOT_PRIMARY3:
			case XQ_ITEM_SLOT_PRIMARY4:
			case XQ_ITEM_SLOT_PRIMARY5:
			case XQ_ITEM_SLOT_PRIMARY6:
			case XQ_ITEM_SLOT_PRIMARY7:
			case XQ_ITEM_SLOT_PRIMARY8:
			case XQ_ITEM_SLOT_PRIMARY9:
				strcpy((char *)buf, "gfx/2d/inventory/magic_weapon.png");							break;

			case XQ_ITEM_SLOT_SECONDARY1:	strcpy((char *)buf, "gfx/2d/inventory/secondary.png");	break;
			case XQ_ITEM_SLOT_CHEST: 		strcpy((char *)buf, "gfx/2d/inventory/chest.png");		break;
			case XQ_ITEM_SLOT_FEET: 		strcpy((char *)buf, "gfx/2d/inventory/feet.png");		break;
			case XQ_ITEM_SLOT_NECK: 		strcpy((char *)buf, "gfx/2d/inventory/neck.png");		break;
			case XQ_ITEM_SLOT_LEFTFINGER: 	strcpy((char *)buf, "gfx/2d/inventory/finger.png");		break;
			case XQ_ITEM_SLOT_RIGHTFINGER: 	strcpy((char *)buf, "gfx/2d/inventory/finger.png");		break;
		}

		obj->tooltip = qws->strings.emptyslot;
		if (buf[0]) {
			xq_DrawPic(
				x + 3,
				y + 3,
				34,
				34,
				xq_GfxShader(buf, NULL)
			);
		}
	}

	// Outline the object if it's selected
	if (obj->selected) {
		xq_DrawRect(
			x+1,
			y+1,
			obj->w-2,
			obj->h-2,
			1,
			xq_Color2Vec(col_selected)
		);
	}
}

#include "../cg_local.h"

void xq_DrawItemIcon(qw_window_t *win, int x, int y, int icon, int amount) {
	char *buf;
	if (icon > 0) {
		buf = va("gfx/items/%i.png", icon);
	} else if (icon == -XQ_MONEY_COPPER) {
		buf = "gfx/2d/inventory/coin_copper.png";
	} else if (icon == -XQ_MONEY_SILVER) {
		buf = "gfx/2d/inventory/coin_silver.png";
	} else if (icon == -XQ_MONEY_GOLD) {
		buf = "gfx/2d/inventory/coin_gold.png";
	} else if (icon == -XQ_MONEY_PLATINUM) {
		buf = "gfx/2d/inventory/coin_platinum.png";
	} else {
		buf = "gfx/items/default.png";
	}


	// Draw the item icon
	xq_DrawPic(x, y, 34, 34, xq_GfxShader(buf, "gfx/items/default.png"));


	// Stackable number
	if (amount > 0) {
		xq_FillRect(
			x + 23,
			y + 28,
			14,
			9,
			xq_Color2Vec(0x0)
		);
		CG_DrawTinyString(win, x+22, y+29, va("%i", amount), 1.0f);
	}
}
int64_t xq_InvSlotItemArena(int slot) {
	// special function for arena zones - used for drawing arena weapon selector icon even though we may
	// have something else equipped in the slot or have nothing at all.

	switch (slot) {
		case XQ_ITEM_SLOT_PRIMARY1:		return -XQ_ARENA_WEAPON1;
		case XQ_ITEM_SLOT_PRIMARY2:		return -XQ_ARENA_WEAPON2;
		case XQ_ITEM_SLOT_PRIMARY3:		return -XQ_ARENA_WEAPON3;
		case XQ_ITEM_SLOT_PRIMARY4:		return -XQ_ARENA_WEAPON4;
		case XQ_ITEM_SLOT_PRIMARY5:		return -XQ_ARENA_WEAPON5;
		case XQ_ITEM_SLOT_PRIMARY6:		return -XQ_ARENA_WEAPON6;
		case XQ_ITEM_SLOT_PRIMARY7:		return -XQ_ARENA_WEAPON7;
		case XQ_ITEM_SLOT_PRIMARY8:		return -XQ_ARENA_WEAPON8;
		case XQ_ITEM_SLOT_PRIMARY9:		return -XQ_ARENA_WEAPON9;
		default:	return 0;
	}
}
int64_t xq_InvSlotItem(int slot, int64_t container_id, int container_slot) {
	int64_t val = XQ_ITEM_ERROR;
	playerState_t *ps = &cg.snap->ps;

	switch (slot) {
		case XQ_ITEM_SLOT_MOUSE1: 			val = XQ64(ps->xq_inv_mouse1_1, ps->xq_inv_mouse1_2); 			break;

		case XQ_ITEM_SLOT_PRIMARY1: 		val = XQ64(ps->xq_inv_primary1_1, ps->xq_inv_primary1_2); 		break;
		case XQ_ITEM_SLOT_SECONDARY1: 		val = XQ64(ps->xq_inv_secondary1_1, ps->xq_inv_secondary1_2); 	break;
		case XQ_ITEM_SLOT_PRIMARY2: 		val = XQ64(ps->xq_inv_primary2_1, ps->xq_inv_primary2_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY3: 		val = XQ64(ps->xq_inv_primary3_1, ps->xq_inv_primary3_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY4: 		val = XQ64(ps->xq_inv_primary4_1, ps->xq_inv_primary4_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY5: 		val = XQ64(ps->xq_inv_primary5_1, ps->xq_inv_primary5_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY6: 		val = XQ64(ps->xq_inv_primary6_1, ps->xq_inv_primary6_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY7: 		val = XQ64(ps->xq_inv_primary7_1, ps->xq_inv_primary7_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY8: 		val = XQ64(ps->xq_inv_primary8_1, ps->xq_inv_primary8_2); 		break;
		case XQ_ITEM_SLOT_PRIMARY9: 		val = XQ64(ps->xq_inv_primary9_1, ps->xq_inv_primary9_2); 		break;

		case XQ_ITEM_SLOT_LEFTEAR: 			val = XQ64(ps->xq_inv_leftear_1, ps->xq_inv_leftear_2); 		break;
		case XQ_ITEM_SLOT_RIGHTEAR: 		val = XQ64(ps->xq_inv_rightear_1, ps->xq_inv_rightear_2);		break;
		case XQ_ITEM_SLOT_HEAD: 			val = XQ64(ps->xq_inv_head_1, ps->xq_inv_head_2); 				break;
		case XQ_ITEM_SLOT_FACE: 			val = XQ64(ps->xq_inv_face_1, ps->xq_inv_face_2); 				break;
		case XQ_ITEM_SLOT_CHEST: 			val = XQ64(ps->xq_inv_chest_1, ps->xq_inv_chest_2); 			break;
		case XQ_ITEM_SLOT_ARMS: 			val = XQ64(ps->xq_inv_arms_1, ps->xq_inv_arms_2); 				break;
		case XQ_ITEM_SLOT_LEFTWRIST: 		val = XQ64(ps->xq_inv_leftwrist_1, ps->xq_inv_leftwrist_2); 	break;
		case XQ_ITEM_SLOT_WAIST: 			val = XQ64(ps->xq_inv_waist_1, ps->xq_inv_waist_2); 			break;
		case XQ_ITEM_SLOT_RIGHTWRIST: 		val = XQ64(ps->xq_inv_rightwrist_1, ps->xq_inv_rightwrist_2);	break;
		case XQ_ITEM_SLOT_LEGS: 			val = XQ64(ps->xq_inv_legs_1, ps->xq_inv_legs_2); 				break;
		case XQ_ITEM_SLOT_HANDS: 			val = XQ64(ps->xq_inv_hands_1, ps->xq_inv_hands_2);				break;
		case XQ_ITEM_SLOT_FEET: 			val = XQ64(ps->xq_inv_feet_1, ps->xq_inv_feet_2);				break;
		case XQ_ITEM_SLOT_SHOULDERS: 		val = XQ64(ps->xq_inv_shoulders_1, ps->xq_inv_shoulders_2); 	break;
		case XQ_ITEM_SLOT_BACK: 			val = XQ64(ps->xq_inv_back_1, ps->xq_inv_back_2); 				break;
		case XQ_ITEM_SLOT_NECK: 			val = XQ64(ps->xq_inv_neck_1, ps->xq_inv_neck_2); 				break;
		case XQ_ITEM_SLOT_LEFTFINGER: 		val = XQ64(ps->xq_inv_leftfinger_1, ps->xq_inv_leftfinger_2); 	break;
		case XQ_ITEM_SLOT_RIGHTFINGER: 		val = XQ64(ps->xq_inv_rightfinger_1, ps->xq_inv_rightfinger_2); break;
		case XQ_ITEM_SLOT_CARRY1: 			val = XQ64(ps->xq_inv_carry1_1, ps->xq_inv_carry1_2); 			break;
		case XQ_ITEM_SLOT_CARRY2: 			val = XQ64(ps->xq_inv_carry2_1, ps->xq_inv_carry2_2); 			break;
		case XQ_ITEM_SLOT_CARRY3: 			val = XQ64(ps->xq_inv_carry3_1, ps->xq_inv_carry3_2); 			break;
		case XQ_ITEM_SLOT_CARRY4: 			val = XQ64(ps->xq_inv_carry4_1, ps->xq_inv_carry4_2); 			break;
		case XQ_ITEM_SLOT_CARRY5: 			val = XQ64(ps->xq_inv_carry5_1, ps->xq_inv_carry5_2); 			break;
		case XQ_ITEM_SLOT_CARRY6: 			val = XQ64(ps->xq_inv_carry6_1, ps->xq_inv_carry6_2); 			break;
		case XQ_ITEM_SLOT_BANK1: 			val = XQ64(ps->xq_bank_slot_1_1, ps->xq_bank_slot_1_2); 		break;
		case XQ_ITEM_SLOT_BANK2: 			val = XQ64(ps->xq_bank_slot_2_1, ps->xq_bank_slot_2_2); 		break;
		case XQ_ITEM_SLOT_BANK3: 			val = XQ64(ps->xq_bank_slot_3_1, ps->xq_bank_slot_3_2); 		break;
		case XQ_ITEM_SLOT_BANK4: 			val = XQ64(ps->xq_bank_slot_4_1, ps->xq_bank_slot_4_2); 		break;


		case XQ_ITEM_SLOT_MERCHANT1: 		val = XQ64(ps->xq_merchant_slot_1_1, ps->xq_merchant_slot_1_2);	break;
		case XQ_ITEM_SLOT_MERCHANT2: 		val = XQ64(ps->xq_merchant_slot_2_1, ps->xq_merchant_slot_2_2);	break;
		case XQ_ITEM_SLOT_MERCHANT3: 		val = XQ64(ps->xq_merchant_slot_3_1, ps->xq_merchant_slot_3_2);	break;
		case XQ_ITEM_SLOT_MERCHANT4: 		val = XQ64(ps->xq_merchant_slot_4_1, ps->xq_merchant_slot_4_2);	break;
		case XQ_ITEM_SLOT_MERCHANT5: 		val = XQ64(ps->xq_merchant_slot_5_1, ps->xq_merchant_slot_5_2);	break;
		case XQ_ITEM_SLOT_MERCHANT6: 		val = XQ64(ps->xq_merchant_slot_6_1, ps->xq_merchant_slot_6_2);	break;
		case XQ_ITEM_SLOT_MERCHANT7: 		val = XQ64(ps->xq_merchant_slot_7_1, ps->xq_merchant_slot_7_2);	break;
		case XQ_ITEM_SLOT_MERCHANT8: 		val = XQ64(ps->xq_merchant_slot_8_1, ps->xq_merchant_slot_8_2);	break;
		case XQ_ITEM_SLOT_MERCHANT9: 		val = XQ64(ps->xq_merchant_slot_9_1, ps->xq_merchant_slot_9_2);	break;
		case XQ_ITEM_SLOT_MERCHANT10: 		val = XQ64(ps->xq_merchant_slot_10_1, ps->xq_merchant_slot_10_2);	break;
		case XQ_ITEM_SLOT_MERCHANT11: 		val = XQ64(ps->xq_merchant_slot_11_1, ps->xq_merchant_slot_11_2);	break;
		case XQ_ITEM_SLOT_MERCHANT12: 		val = XQ64(ps->xq_merchant_slot_12_1, ps->xq_merchant_slot_12_2);	break;
		case XQ_ITEM_SLOT_MERCHANT13: 		val = XQ64(ps->xq_merchant_slot_13_1, ps->xq_merchant_slot_13_2);	break;
		case XQ_ITEM_SLOT_MERCHANT14: 		val = XQ64(ps->xq_merchant_slot_14_1, ps->xq_merchant_slot_14_2);	break;
		case XQ_ITEM_SLOT_MERCHANT15: 		val = XQ64(ps->xq_merchant_slot_15_1, ps->xq_merchant_slot_15_2);	break;
		case XQ_ITEM_SLOT_MERCHANT16: 		val = XQ64(ps->xq_merchant_slot_16_1, ps->xq_merchant_slot_16_2);	break;
		case XQ_ITEM_SLOT_MERCHANT17: 		val = XQ64(ps->xq_merchant_slot_17_1, ps->xq_merchant_slot_17_2);	break;
		case XQ_ITEM_SLOT_MERCHANT18: 		val = XQ64(ps->xq_merchant_slot_18_1, ps->xq_merchant_slot_18_2);	break;
		case XQ_ITEM_SLOT_MERCHANT19: 		val = XQ64(ps->xq_merchant_slot_19_1, ps->xq_merchant_slot_19_2);	break;
		case XQ_ITEM_SLOT_MERCHANT20: 		val = XQ64(ps->xq_merchant_slot_20_1, ps->xq_merchant_slot_20_2);	break;

		case XQ_ITEM_SLOT_TRADE_GIVE1: 		val = XQ64(ps->xq_give_slot_1_1, ps->xq_give_slot_1_2);			break;
		case XQ_ITEM_SLOT_TRADE_GIVE2: 		val = XQ64(ps->xq_give_slot_2_1, ps->xq_give_slot_2_2);			break;
		case XQ_ITEM_SLOT_TRADE_GIVE3: 		val = XQ64(ps->xq_give_slot_3_1, ps->xq_give_slot_3_2);			break;
		case XQ_ITEM_SLOT_TRADE_GIVE4: 		val = XQ64(ps->xq_give_slot_4_1, ps->xq_give_slot_4_2);			break;
		case XQ_ITEM_SLOT_TRADE_RECEIVE1: 	val = XQ64(ps->xq_receive_slot_1_1, ps->xq_receive_slot_1_2);	break;
		case XQ_ITEM_SLOT_TRADE_RECEIVE2: 	val = XQ64(ps->xq_receive_slot_2_1, ps->xq_receive_slot_2_2);	break;
		case XQ_ITEM_SLOT_TRADE_RECEIVE3: 	val = XQ64(ps->xq_receive_slot_3_1, ps->xq_receive_slot_3_2);	break;
		case XQ_ITEM_SLOT_TRADE_RECEIVE4: 	val = XQ64(ps->xq_receive_slot_4_1, ps->xq_receive_slot_4_2);	break;

		case XQ_ITEM_SLOT_INCONTAINER:;
			xq_item_t *iinf_tmp = xq_ItemInfo(container_id);
			// If we didn't get the container item info from the server (yet), we can forget about its contents for now.
			if (iinf_tmp != NULL) {
				switch (container_slot) {
					case 1: val = iinf_tmp->container_slot_1; break;
					case 2: val = iinf_tmp->container_slot_2; break;
					case 3: val = iinf_tmp->container_slot_3; break;
					case 4: val = iinf_tmp->container_slot_4; break;
					case 5: val = iinf_tmp->container_slot_5; break;
					case 6: val = iinf_tmp->container_slot_6; break;
					case 7: val = iinf_tmp->container_slot_7; break;
					case 8: val = iinf_tmp->container_slot_8; break;
					case 9: val = iinf_tmp->container_slot_9; break;
					case 10: val = iinf_tmp->container_slot_10; break;
					default: val = 0; break;
				}
			}
			break;
		default:
			if (slot >= XQ_ITEM_SLOT_LOOT1 && slot <= XQ_ITEM_SLOT_LOOT1 + XQ_LOOT_MAX_SLOT - 1) {
				int tmp = slot - XQ_ITEM_SLOT_LOOT1 + 1;
				int *val1 = &ps->xq_loot_slot_1_1 + ((tmp - 1) * 2);
				int *val2 = &ps->xq_loot_slot_1_2 + ((tmp - 1) * 2);
				val = XQ64(*val1, *val2);
			} else {
				xq_clog(COLOR_RED, "Unknown inv slot #%i", slot);
			}
			break;
	}
	return val;
}
void xq_RefreshAllItems() {
	// Delete all item cache

	xq_CacheDelete(XQ_ITEM);
}

#include "../../../cg_local.h"

static int MoneyAmount(qw_obj_t *obj) {

	playerState_t *ps = &cg.snap->ps;

	if (
		obj->money_type != XQ_MONEY_COPPER &&
		obj->money_type != XQ_MONEY_SILVER &&
		obj->money_type != XQ_MONEY_GOLD &&
		obj->money_type != XQ_MONEY_PLATINUM
	) {
		xq_clog(COLOR_RED, "MoneyAmount: unknown money type %i", obj->money_type);
		return -1;
	}

	if (obj->money_source == XQ_MONEY_SOURCE_INV) {
		if (obj->money_type == XQ_MONEY_COPPER)		return ps->xq_copper;
		if (obj->money_type == XQ_MONEY_SILVER)		return ps->xq_silver;
		if (obj->money_type == XQ_MONEY_GOLD)		return ps->xq_gold;
		if (obj->money_type == XQ_MONEY_PLATINUM)	return ps->xq_platinum;
	} else if (obj->money_source == XQ_MONEY_SOURCE_BANK) {
		if (obj->money_type == XQ_MONEY_COPPER)		return ps->xq_bank_copper;
		if (obj->money_type == XQ_MONEY_SILVER)		return ps->xq_bank_silver;
		if (obj->money_type == XQ_MONEY_GOLD)		return ps->xq_bank_gold;
		if (obj->money_type == XQ_MONEY_PLATINUM)	return ps->xq_bank_platinum;
	} else if (obj->money_source == XQ_MONEY_SOURCE_GIVE) {
		if (obj->money_type == XQ_MONEY_COPPER)		return ps->xq_give_copper;
		if (obj->money_type == XQ_MONEY_SILVER)		return ps->xq_give_silver;
		if (obj->money_type == XQ_MONEY_GOLD)		return ps->xq_give_gold;
		if (obj->money_type == XQ_MONEY_PLATINUM)	return ps->xq_give_platinum;
	} else if (obj->money_source == XQ_MONEY_SOURCE_RECEIVE) {
		if (obj->money_type == XQ_MONEY_COPPER)		return ps->xq_receive_copper;
		if (obj->money_type == XQ_MONEY_SILVER)		return ps->xq_receive_silver;
		if (obj->money_type == XQ_MONEY_GOLD)		return ps->xq_receive_gold;
		if (obj->money_type == XQ_MONEY_PLATINUM)	return ps->xq_receive_platinum;
	} else {
		xq_clog(COLOR_RED, "MoneyAmount: unknown source %i", obj->money_source);
		return -1;
	}
	return -1;
}
void qw_Obj_Money_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	qw_obj_t *obj = &qws->obj[att->num];

	int amount = MoneyAmount(obj);
	if (amount == -1) return;

	char t[20] = {0};
	snprintf(t, 19, "%i", amount);

	obj->h = TINYCHAR_HEIGHT;
	obj->w = CG_DrawStrlen(t) * TINYCHAR_WIDTH;

	int x = att->x;
	int y = att->y;
	if (att->x_centered == 0) {
		if (x < 0) x = (win->w - obj->w) - QW_WIN_BORDER_WIDTH;
	} else {
		x = (win->w / 2) - obj->w / 2;
	}
	if (att->y_centered == 0) {
		if (y < 0) y = (win->h - TINYCHAR_HEIGHT) - QW_WIN_BORDER_WIDTH;
	} else {
		y = (win->h / 2) - TINYCHAR_HEIGHT / 2;
	}

	qw_AttachWithBorder(win, att, &x, &y);

	CG_DrawTinyString(win, x, y, t, 1.0F);
}
void qw_Obj_Money_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt) {
	playerState_t *ps = &cg.snap->ps;
	qw_window_t *w = &qws->win[win];
    qw_obj_t *o = &qws->obj[w->obj[obj].num];


	// Don't do anything if mouse cursor has an item on it
	if (xq_InvSlotItem(XQ_ITEM_SLOT_MOUSE1, 0, 0) != 0) {
		return;
	}


	if (ps->xq_mouse_money_amount == 0) {
		// Picking some money up - make sure it's available for picking
		// Make sure we're trying to pick money up from a valid place
		if (o->money_source != XQ_MONEY_SOURCE_INV && o->money_source != XQ_MONEY_SOURCE_BANK) {
			return;
		}

		// Make sure there's actually money available for picking
		int amount = MoneyAmount(o);
		if (amount < 1) return;


		// Decide on however much we're going to pick up
		if (!shift && !ctrl && !alt) {
			// no key mods - fire up the amount picker
			qw_Obj_AmountPicker(1, amount, 1, amount, xqst->mousex, xqst->mousey, "swapmoney", o->money_type, o->money_source, 0);
		} else if (!alt) {
			int pick_amount = 0;
			if (shift && !ctrl) {
				// grab the whole available amount
				pick_amount = amount;
			} else if (!shift && ctrl) {
				// pick one coin up
				pick_amount = 1;
			}
			xq_scmd(va("/swapmoney %i %i %i", o->money_type, o->money_source, pick_amount));
		}

	} else {
		// Putting some money somewhere
		// When banking, money type doesn't matter as it will be auto converted
		// Otherwise, gold must be deposited into gold etc.
		if (ps->xq_flags & XQ_BANKING || o->money_type == ps->xq_mouse_money_type) {
			xq_scmd(va("/swapmoney %i %i %i", o->money_type, o->money_source, 0));
		}
	}
}

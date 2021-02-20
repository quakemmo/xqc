#include "../../../cg_local.h"

void qw_Obj_Button_Click(int win, int button, int obj, int init_x, int init_y, int shift, int ctrl, int alt) {

	qw_window_t *w = &qws->win[win];
	qw_obj_t *o = &qws->obj[w->obj[obj].num];
	int maxval = 1, preset = 1, amount = 0;
	playerState_t *ps = &cg.snap->ps;

	if (o->action == QW_OBJ_BUTTON_ACTION_COMBINE) {
		xq_scmd(va("/combine %li", o->arg1));

	} else if (o->action == QW_OBJ_BUTTON_ACTION_PROMPT_NO) {
		qw_WindowDelete("prompt");
		switch (o->arg1) {
			case QW_PROMPT_ACTION_REZDECLINE:
				xq_scmd("/rezdecline");
				break;
		}

	} else if (o->action == QW_OBJ_BUTTON_ACTION_PROMPT_YES) {
		qw_WindowDelete("prompt");
		switch (o->arg1) {
			case QW_PROMPT_ACTION_DESTROY:
				xq_scmd("/destroy");
				break;
			case QW_PROMPT_ACTION_DROP:
				xq_scmd("/drop");
				break;
			case QW_PROMPT_ACTION_LOOT:
				xq_scmd(va("/lootslot %li %li", o->arg2, o->arg3));
				break;
			case QW_PROMPT_ACTION_SKILL_CAPUP:
				xq_scmd(va("/capup %li", o->arg2));
				break;
			case QW_PROMPT_ACTION_SCRIBE:
			case QW_PROMPT_ACTION_SKILLUP:
				xq_RunSlot((int)o->arg2);
				break;
			case QW_PROMPT_ACTION_REZACCEPT:
				xq_scmd("/rezaccept");
				break;
		}

	} else if (o->action == QW_OBJ_BUTTON_ACTION_DESTROY) {
		if (xq_InvSlotItem(XQ_ITEM_SLOT_MOUSE1, 0, 0) == 0 && (ps->xq_mouse_money_amount == 0 || ps->xq_mouse_money_type == 0)) {
			xq_clog(COLOR_RED, "To destroy an item, hold it on your mouse cursor and click the Destroy button.");
			return;
		}
		qw_Prompt("Destroy the item?", QW_PROMPT_ACTION_DESTROY, 0, 0, 0, 0);

	} else if (o->action == QW_OBJ_BUTTON_ACTION_SKILL_CAPUP) {
		qw_Prompt(
			va("Increase %s skill cap?", xq_skills_literal(o->arg1, 0, 0)),
			QW_PROMPT_ACTION_SKILL_CAPUP, 0, o->arg1, 0, 0
		);
		return;

	} else if (o->action == QW_OBJ_BUTTON_ACTION_TRADE) {
		if (qws->trading == 0) return;

		if (xq_InvSlotItem(XQ_ITEM_SLOT_MOUSE1, 0, 0) != 0) {
			xq_clog(COLOR_RED, "Your mouse cursor must be empty in order to complete the trade.");
			return;
		}
		xq_scmd("/tradeagree");

	} else if (o->action == QW_OBJ_BUTTON_ACTION_SPELLBOOK_SWITCH) {
		if (qws->looting || qws->buying || qws->banking || qws->trading) {
			return;
		}

		if (button == 0) {
			xq_scmd(va("/spellbookpage %li", o->arg1));
		}

	} else if (o->action == QW_OBJ_BUTTON_ACTION_BOOK_SWITCH) {
		if (qws->looting || qws->buying || qws->banking || qws->trading) {
			return;
		}

		if (button == 0) {
			qws->current_book_page = o->arg1;
			xqui_BookPageRefresh();
			xqui_BookButtons();
		}
	} else if (o->action == QW_OBJ_BUTTON_ACTION_BUY || o->action == QW_OBJ_BUTTON_ACTION_SELL) {
		if (qws->buying != 1) return;
		if (xq_InvSlotItem(XQ_ITEM_SLOT_MOUSE1, 0, 0) != 0) {
			xq_clog(COLOR_RED, "Your mouse cursor must be empty in order to purchase.");
			return;
		}

		xq_item_t *iinf = xq_ItemInfo(o->arg4);
		if (iinf == NULL) return;

		// Clicked Buy / Sell on a stackable item - ask for amount unless we have ctrl or shift held
		// (ctrl for buying just one stackable item, shift to buy a full stack)
		if (iinf->stackable > 0) {
			if (o->action == QW_OBJ_BUTTON_ACTION_SELL) {
				maxval = iinf->amount;
				preset = iinf->amount;
			} else {
				maxval = iinf->stackable;
				preset = 1;
			}
			if (!shift && !ctrl && !alt) {
				qw_Obj_AmountPicker(1, preset, 1, maxval, xqst->mousex, xqst->mousey,
					"swapwithmouseslot", o->arg1, o->arg2, o->arg3);
				return;
			} else if ((shift && !ctrl && !alt) || (!shift && ctrl && !alt)) {
				if (shift) {
					amount = iinf->stackable;
				} else if (ctrl) {
					amount = 1;
				}
				xq_SwapWithMouseSlot(o->arg1, o->arg2, o->arg3, amount, 0);
			}
		}

		// Clicked Buy / Sell on a non-stackable item - send immediate command
		xq_SwapWithMouseSlot(o->arg1, o->arg2, o->arg3, 0, 0);
	}
}
void qw_Obj_Button_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	char *t = NULL;
	int maxw = 0, maxh = 0;
	qw_obj_t *obj = &qws->obj[att->num];

	if (obj->text != NULL) {
		t = obj->text;
	} else if (obj->text_static[0] != 0) {
		t = obj->text_static;
	}

	if (!obj->bitmap[0]) {
		if (!t) return;
		if (!t[0]) return;
	}


	// See whether the text content or the bitmap image is bigger
	// and set button size according to the biggest dimensions
	if (t && t[0]) {
		maxw = TINYCHAR_HEIGHT + obj->padding_y * 2;
		maxh = CG_DrawStrlen(t) * TINYCHAR_WIDTH + obj->padding_x * 2;
	}
	if (obj->bitmap_w > maxw) {
		maxw = obj->bitmap_w;
		maxh = obj->bitmap_h;
	}
	obj->h = maxw;
	obj->w = maxh;


	// We might need to adjust coordinates because of what just happened
	int x = att->x;
	int y = att->y;
	if (att->x_centered == 0) {
		if (x < 0) x = (win->w - obj->w) - QW_WIN_BORDER_WIDTH + x;
	} else {
		x = (win->w / 2) - obj->w / 2;
	}
	if (att->y_centered == 0) {
		if (y < 0) y = (win->h - TINYCHAR_HEIGHT) - QW_WIN_BORDER_WIDTH + y;
	} else {
		y = (win->h / 2) - TINYCHAR_HEIGHT / 2;
	}

	qw_AttachWithBorder(win, att, &x, &y);


	// Draw the basic button structure
	if (obj->borderwidth > 0) {
		xq_DrawRect(
			x,
			y,
			obj->w,
			obj->h,
			obj->borderwidth,
			xq_Color2Vec(obj->bordercolor)
		);
	}

	if (!obj->nobgfill) {
		int bgcolor = qw_Hover(obj);
		xq_FillRect(
			x + obj->borderwidth,
			y + obj->borderwidth,
			obj->w - obj->borderwidth * 2,
			obj->h - obj->borderwidth * 2,
			xq_Color2Vec(bgcolor)
		);
	}

	// Draw the button bitmap background if any
	if (obj->bitmap[0]) {
		xq_DrawPic(x + 1, y + 1, obj->bitmap_w, obj->bitmap_h, xq_GfxShader(obj->bitmap, NULL));
	}

	// Draw the button text if any
	if (t && t[0]) {
		int drx = x + (obj->w / 2) - (CG_DrawStrlen(t) * TINYCHAR_WIDTH / 2);
		int dry = y + (obj->h / 2) - TINYCHAR_HEIGHT / 2;
		CG_DrawTinyString(win, drx, dry, t, 1.0F);
	}
}

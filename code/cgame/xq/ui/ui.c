/*	DESCRIPTION
	Functions that utilize the lower level QWIN subsystem in order to create
	player-facing windows such as the inventory window, the spell book, or container windows.
*/

#include "../../cg_local.h"
#include "style.h"

static void SpellsBar(int state) {
	// Creates or deletes the spell bar.


	if (state) {
		// Make the window
		qw_window_t w = {
			.wid		= "spells",
			.title		= "Spells",
			.w			= QW_OBJ_SPELL_GEM_WIDTH + 8,
			.h			= QW_OBJ_SPELL_GEM_HEIGHT * XQ_SPELL_SLOTS + 65, // leave some room for the book icon
		};
		qw_WinNew(w);


		// Add the spell gems
		for (int i = 1;  i <= XQ_SPELL_SLOTS;  i++) {
			qw_obj_t o;
			qw_ObjInit(&o, QW_OBJ_SPELL_GEM);
			o.slot = i;
			qw_ObjAttach("spells", qw_ObjCreate(&o), 0, 10 + (i - 1) * (QW_OBJ_SPELL_GEM_HEIGHT + 3), 0, 0);
		}
		qw_ObjAttach("spells", qws->objhandles[XQ_UI_OBJID_SPELLBOOK_SWITCH], 9, -25, 0, 0);
	} else {
		qw_WindowDelete("spells");
	}
}

void xqui_Init(void) {
	qw_obj_t obj_text, obj_perc, o;

	// Lower level QW init
	qw_Init();


	// Text strings
	strcpy(qws->strings.emptyslot, "Empty slot");
	strcpy(qws->strings.spellbook, "Spell book (B)");
	strcpy(qws->strings.nextpage, "Next page");
	strcpy(qws->strings.previouspage, "Previous page");
	strcpy(qws->strings.autoequip, "Auto-equip");
	strcpy(qws->strings.scribeTheSpell, "Scribe the spell?");
	strcpy(qws->strings.combine, "Combine");
	strcpy(qws->strings.lootTheNodropItem, "Loot the NODROP item?");


	// CHAT WINDOW
	qw_window_t w = {
		.wid		= "chat",
		.x			= 0,
		.y			= 500,
		.w			= 360,
		.h			= 266,
		.alpha_min	= 0.3f,
		.alpha_max	= 1.0f,
		.no_title	= 1,
		.resizable	= 1
	};
	xqst->ui_chat_window_x = w.x;
	xqst->ui_chat_window_y = w.y;
	xqst->ui_chat_window_w = w.w;
	xqst->ui_chat_window_h = w.h;

	qw_WinNew(w);

	// GROUP
	qw_window_t group_w = {
		.wid		= "group",
		.title		= "Group",
		.x			= 200,
		.y			= 0,
		.w			= 120,
		.h			= 260,
		.alpha_min	= 0.3f,
		.alpha_max	= 1.0f,
	};
	qw_WinNew(group_w);


	// Init objects once, then reuse them a few times as they get instanced
	// and are barely different one from another
	qw_ObjInit(&obj_perc, QW_OBJ_PERCBAR);
	obj_perc.filledcolor = XQUI_SMALLBARS_HP_COL;

	qw_ObjInit(&obj_text, QW_OBJ_TEXT);
	obj_text.capitalize = 1;

	int i;
	for (i = 0;  i <= 4;  i++) {

		obj_text.text = (char *)(&xqst->ui_group_member_1_name + i);
		qw_ObjAttach("group", qws->objhandles[XQ_UI_OBJID_GROUP_MEMBER_1_NAME + i] = qw_ObjCreate(&obj_text), 0, 20 + 40 * i, 0, 0);

		obj_perc.perc = &xqst->ui_group_member_1_hp_perc + i;
		qw_ObjAttach("group", qws->objhandles[XQ_UI_OBJID_GROUP_MEMBER_1_HP_PERC + i] = qw_ObjCreate(&obj_perc), 2, 30 + 40 * i, 0, 0);
	}



	// Create and save, but don't attach char name and level text objects.
	// These can be used in various places.
	qw_ObjInit(&obj_text, QW_OBJ_TEXT);
	obj_text.clickthrough = 1;

	obj_text.text = (char *)&xqst->ui_charname;
	qws->objhandles[XQ_UI_OBJID_CHAR_NAME] = qw_ObjCreate(&obj_text);

	obj_text.text = (char *)&xqst->ui_level;
	qws->objhandles[XQ_UI_OBJID_CHAR_LEVEL] = qw_ObjCreate(&obj_text);


	// INVENTORY
	qw_ObjInit(&obj_text, QW_OBJ_TEXT);
	obj_text.text = (char *)&xqst->ui_classrace;
	obj_text.clickthrough = 1;
	qws->objhandles[XQ_UI_OBJID_CHAR_CLASSRACE] = qw_ObjCreate(&obj_text);


	qw_ObjInit(&o, QW_OBJ_BUTTON);
	o.action = QW_OBJ_BUTTON_ACTION_SPELLBOOK_SWITCH;
	o.arg1 = 1;
	Q_strncpyz(o.bitmap, "gfx/2d/misc/spellbook.png", sizeof(o.bitmap));
	o.bitmap_w = 34;
	o.bitmap_h = 34;
	o.padding_x = 0;
	o.padding_y = 0;
	o.borderwidth = 0;
	o.nobgfill = 1;
	o.tooltip = qws->strings.spellbook;
	// we just create and save it here - it will get attached later if needed
	qws->objhandles[XQ_UI_OBJID_SPELLBOOK_SWITCH] = qw_ObjCreate(&o);


	// SPELL EFFECTS
	assert((QW_SPFX_COLS * QW_SPFX_ROWS >= XQ_SPFX_SLOTS));	// Make sure we never get affected by something
															// we cannot display.
	qw_window_t spells_w = {
		.wid		= "spfx",
		.title		= "Effects",
		.x			= 400,
		.y			= 400,
		.w			= QW_WIN_BORDER_WIDTH * 2 + QW_OBJ_SPELL_ICON_WIDTH * QW_SPFX_COLS,
		.h			= QW_WIN_STATUSBAR_HEIGHT + QW_WIN_BORDER_WIDTH * 2 + QW_OBJ_SPELL_ICON_HEIGHT * QW_SPFX_ROWS,
		.alpha_min	= 0.3f,
		.alpha_max	= 1.0f
	};
	qw_WinNew(spells_w);

	int num = 1;
	for (i = 1;  i <= QW_SPFX_COLS;  i++) {
		for (int j = 1;  j <= QW_SPFX_ROWS;  j++) {
			qw_ObjInit(&o, QW_OBJ_SPELL_ICON);
			o.tooltip = xqst->spfx_tooltips[num - 1];
			o.slot = num++;
			qw_ObjAttach("spfx", qws->objhandles[XQ_UI_OBJID_SPFX_SLOT_1 + num - 2] = qw_ObjCreate(&o),
				(i - 1) * QW_OBJ_SPELL_ICON_WIDTH,
				(j - 1) * QW_OBJ_SPELL_ICON_HEIGHT + QW_WIN_STATUSBAR_HEIGHT,
				0, 0
			);
		}
	}

	qw_MouseMove(); // Get initial fade out of alpha windows
}
void xqui_Frame(void) {
	// XQ UI-related Stuff that happens once every frame.
	// Mainly, we make sure here that some server-provided state changes such as
	// vendoring, trading, looting, etc are taken into account by the UI that will
	// create or destroy the relevant windows.

	playerState_t *ps = &cg.snap->ps;
	int *objh = qws->objhandles; // typing shortcut

	int i;
    char windowname[100];

	qw_Frame();


	// Loot window
	if (qws->looting == 0) {
		if (ps->xq_flags & XQ_LOOTING_PC || ps->xq_flags & XQ_LOOTING_NPC) {
			qws->looting = 1;
			xqui_LootOpen();
		}
	} else {
		if (!(ps->xq_flags & XQ_LOOTING_PC) && !(ps->xq_flags & XQ_LOOTING_NPC)) {
			qws->looting = 0;
			xqui_LootClose(NULL);
			qw_WindowDelete("loot");
			qw_MouseMove();
		}
	}


	// Bank window
	if (qws->banking == 0) {
		if (ps->xq_flags & XQ_BANKING) {
			qws->banking = 1;
			xqui_BankOpen();
		}
	} else {
		if (!(ps->xq_flags & XQ_BANKING)) {
			qws->banking = 0;
			qw_WindowDelete("bank");
			xqui_ContainerClose(XQ_ITEM_SLOT_BANK1, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_BANK2, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_BANK3, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_BANK4, 0);
			qw_MouseMove();
		}
	}


	// Merchant window
	if (qws->buying == 0) {
		if (ps->xq_flags & XQ_BUYING) {
			qws->buying = 1;
			xqui_MerchantOpen();
		}
	} else {
		if (!(ps->xq_flags & XQ_BUYING)) {

			xqui_InvSlotUnselectAll();
			qws->buying = 0;
			qw_WindowDelete("merchant");

			if (objh[XQ_UI_OBJID_BUTTON_BUY]) {
				qw_obj_t *buy_button = &qws->obj[objh[XQ_UI_OBJID_BUTTON_BUY]];
				buy_button->hidden = 1;
				buy_button->arg1 = 0;
				buy_button->arg2 = 0;
				buy_button->arg3 = 0;
				buy_button->arg4 = 0;
			}

			if (objh[XQ_UI_OBJID_BUTTON_SELL]) {
				qw_obj_t *sell_button = &qws->obj[objh[XQ_UI_OBJID_BUTTON_SELL]];
				sell_button->hidden = 1;
				sell_button->arg1 = 0;
				sell_button->arg2 = 0;
				sell_button->arg3 = 0;
				sell_button->arg4 = 0;
			}

			if (objh[XQ_UI_OBJID_MERCHANT_ITEMGFX]) {
				qws->obj[objh[XQ_UI_OBJID_MERCHANT_ITEMGFX]].item = 0;
			}
			if (objh[XQ_UI_OBJID_MERCHANT_ITEMTXT]) {
				qws->obj[objh[XQ_UI_OBJID_MERCHANT_ITEMTXT]].text_static[0] = 0;
			}

			qw_MouseMove();
		}
	}


	// Trade window
	if (qws->trading == 0) {
		if (ps->xq_flags & XQ_TRADING_PC || ps->xq_flags & XQ_TRADING_NPC) {
			qws->trading = (ps->xq_flags & XQ_TRADING_PC) ? 1 : 2;
			xqui_TradeOpen();
		}
	} else {
		if (!(ps->xq_flags & XQ_TRADING_PC) && !(ps->xq_flags & XQ_TRADING_NPC)) { 
			xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_GIVE1, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_GIVE2, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_GIVE3, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_GIVE4, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_RECEIVE1, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_RECEIVE2, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_RECEIVE3, 0);
			xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_RECEIVE4, 0);

			qws->trading = 0;
			qw_WindowDelete("give");
			qw_WindowDelete("receive");

			qw_obj_t *trade_button = &qws->obj[objh[XQ_UI_OBJID_BUTTON_TRADE_ME]];
			trade_button->arg1 = 0;
			trade_button->arg2 = 0;
			trade_button->arg3 = 0;

			trade_button = &qws->obj[objh[XQ_UI_OBJID_BUTTON_TRADE_THEM]];
			trade_button->arg1 = 0;
			trade_button->arg2 = 0;
			trade_button->arg3 = 0;

			qw_MouseMove();
		}
	}


	// Trade window buttons change colors if one of the parties clicks
	// "agree on trade". Any change in trade terms resets both buttons.
	if (qws->trading == 1) {
		if (ps->xq_trading_me_agree == 1) {
			qws->obj[objh[XQ_UI_OBJID_BUTTON_TRADE_ME]].bordercolor = QW_OBJ_BUTTON_TRADE_AGREE_YES;
		} else {
			qws->obj[objh[XQ_UI_OBJID_BUTTON_TRADE_ME]].bordercolor = QW_OBJ_BUTTON_TRADE_AGREE_NO;
		}

		if (ps->xq_trading_them_agree == 1) {
			qws->obj[objh[XQ_UI_OBJID_BUTTON_TRADE_THEM]].bordercolor = QW_OBJ_BUTTON_TRADE_AGREE_YES;
		} else {
			qws->obj[objh[XQ_UI_OBJID_BUTTON_TRADE_THEM]].bordercolor = QW_OBJ_BUTTON_TRADE_AGREE_NO;
		}
	}


	// Spell book
	if (qws->spellbook_page != ps->xq_spellbook_page) {
		qws->spellbook_page = ps->xq_spellbook_page;
		if (qws->spellbook_page > 0) {
			snprintf(qws->spellbook_page_txt, 20, "Page %i", qws->spellbook_page);
		}
		xqui_SpellBook();
	}


	// Close containers that poof from our inventory for whatever reason - mainly it should
	// only happen when we successfully combine something that consumes the container.
	for (i = 0;  i < 6;  i++) {
		if (xq_InvSlotItem(XQ_ITEM_SLOT_CARRY1 + i, 0, 0) == 0) {
			snprintf(windowname, 100, "cont_%i", i + 1);
			if (qw_WindowExists(windowname) != -1) {
				xqui_ContainerClose(XQ_ITEM_SLOT_CARRY1 + i, 0);
			}
		}
	}


	// Rez box
	if (ps->xq_rez_author_1 != 0) {
		if (xqst->rez_box == 0) {
			xqst->rez_box = 1;
			xqui_RezBox();
		}
	} else {
		if (xqst->rez_box == 1) {
			xqst->rez_box = 0;
		}
	}


	// See if some cg_ vars state change requires us to close/open some UI windows.
	xqui_VarsToWindows();


	// Make sure the spell bar is only there for magic users
	// (this allows for on-the-fly changing of PC class)
	if (!xq_is_magic_user(ps->xq_class)) {
		if (qw_WindowExists("spells") != -1 ) {
			SpellsBar(0);
		}
	} else {
		if (qw_WindowExists("spells") == -1) {
			SpellsBar(1);
		}
	}


	// See if the item currently selected for merchant transaction (in the merchant window or anywhere else)
	// is still there - clear the merchant's window's selected item icon/text, as well as buy/sell buttons if not.
	// It can disappear for many reasons, for example completing the merchant transaction, or someone else buying
	// the unique item from the merchant, etc.
	if (qws->buying) {
		int *h = qws->objhandles;
		int gfx_h = h[XQ_UI_OBJID_MERCHANT_ITEMGFX];
		int txt_h = h[XQ_UI_OBJID_MERCHANT_ITEMTXT];
		int bbut_h = h[XQ_UI_OBJID_BUTTON_BUY];
		int sbut_h = h[XQ_UI_OBJID_BUTTON_SELL];

		// The objects must have been created
		if (gfx_h && txt_h && bbut_h && sbut_h) {

			qw_obj_t *gfx = &qws->obj[gfx_h];
			qw_obj_t *txt = &qws->obj[txt_h];
			qw_obj_t *bbut = &qws->obj[bbut_h];
			qw_obj_t *sbut = &qws->obj[sbut_h];

			// If anything seems to show
			if (!bbut->hidden || !sbut->hidden) {
				int64_t item;
				if (!bbut->hidden) {
					item = xq_InvSlotItem(bbut->arg1, 0, 0);
				} else {
					item = xq_InvSlotItem(sbut->arg1, sbut->arg2, sbut->arg3);
				}
				if (!item) {
					bbut->hidden = sbut->hidden = 1;
					bbut->arg1 = bbut->arg2 = bbut->arg3 = bbut->arg4 = 0;
					sbut->arg1 = sbut->arg2 = sbut->arg3 = sbut->arg4 = 0;
					txt->text_static[0] = 0;
					gfx->item = 0;
				}
			}
		}
	}
}


// HELPER STUFF
void xqui_InvSlotUnselectAll(void) {
	for (int i = 1;  i < QW_MAX_OBJ;  i++) {
		qw_obj_t *o = &qws->obj[i];
		if (o->type == QW_OBJ_INVSLOT) {
			if (o->selected == 1) {	
				o->selected = 0;
			}
		}
	}
}


// BOOK
void 		xqui_ReadBook(int64_t item) {
	// Right click on a book will open and display it.

	if (item < 1) return;
	qw_obj_t o;

	// Don't do anything until we get the book data
	xq_item_t *iinf = xq_ItemInfo(item);
	if (iinf == NULL) {
		return;
	}


	// Replace existing book window
	if (qw_WindowExists("book") != -1) {
		qw_WindowDelete("book");
	}


	// Make the window
	qw_window_t w = {
		.wid		= "book",
		.x			= -1,
		.y			= -1,
		.w			= XQUI_BOOK_WIN_WIDTH,
		.h			= XQUI_BOOK_WIN_HEIGHT,
		.closex		= 1
	};
	int wid = qw_WinNew(w);
	if (wid > -1) {
		qw_WindowTitle(wid, iinf->name);
	} else {
		return;
	}

	qws->current_book_page = 1;


	// The actual book text
	qw_ObjInit(&o, QW_OBJ_TEXT);
	o.arg1 = item;
	qw_ObjAttach("book", qws->objhandles[XQ_UI_OBJID_BOOK_TEXT] = qw_ObjCreate(&o), 10, 10, 0, 0);
	xqui_BookPageRefresh();


	// The pagination buttons
	qw_ObjInit(&o, QW_OBJ_BUTTON);
	o.action = QW_OBJ_BUTTON_ACTION_BOOK_SWITCH;
	o.text = qws->strings.nextpage;
	qw_ObjAttach("book", qws->objhandles[XQ_UI_OBJID_BUTTON_BOOK_NEXT] = qw_ObjCreate(&o), -50, -25, 0, 0);

	o.text = qws->strings.previouspage;
	qw_ObjAttach("book", qws->objhandles[XQ_UI_OBJID_BUTTON_BOOK_PREV] = qw_ObjCreate(&o), 50, -25, 0, 0);


	// Set up initial button state
	xqui_BookButtons();
}
void 		xqui_BookPageRefresh(void) {
	int page = qws->current_book_page;
	qw_obj_t *o = &qws->obj[qws->objhandles[XQ_UI_OBJID_BOOK_TEXT]];

	if (page < 1) {
		xq_clog(COLOR_RED, "Current book page is %i - correcting to 1", page);
		page = 1;
	}
	if (page > XQ_BOOK_PAGES) {
		xq_clog(COLOR_RED, "Current book page is %i - correcting to %i", page, XQ_BOOK_PAGES);
		page = XQ_BOOK_PAGES;
	}


	xq_item_t *iinf = xq_ItemInfo(o->arg1);
	if (iinf == NULL) {
		xq_clog(COLOR_RED, "No iinf for item %li", o->arg1);
		return;
	}

	char tmp[XQ_ITEM_MAX_BOOK+1] = {0};
	Q_strncpyz(tmp, iinf->book, sizeof(tmp));

	char *tok = strtok(tmp, "$");
	int count = 0;
	while (tok) {
		count++;
		if (count == page) {
			Q_strncpyz(o->text_static, tok, sizeof(o->text_static));
			o->text_static[strlen(tok)] = 0;
			break;
		}
		tok = strtok(NULL, "$");
	}
}
void 		xqui_BookButtons() {
	int i = qws->objhandles[XQ_UI_OBJID_BUTTON_BOOK_NEXT];
	qw_obj_t *o;
	if (i > 0) {
		o = &qws->obj[i];
		o->hidden = (qws->current_book_page < XQ_BOOK_PAGES ? 0 : 1);
		o->arg1 = qws->current_book_page + 1;
	}

	i = qws->objhandles[XQ_UI_OBJID_BUTTON_BOOK_PREV];
	if (i > 0) {
		o = &qws->obj[i];
		o->hidden = (qws->current_book_page < 2 ? 1 : 0);
		o->arg1 = qws->current_book_page - 1;
	}
}


// INVENTORY
static void InventoryClose(void *win) {
	xqui_Inventory(0);
}
void xqui_Inventory_Toggle() {
	xqui_Inventory(2);
}
void xqui_Inventory(int open) {
	qw_obj_t o;
	int *h = qws->objhandles;
	int curwin = qw_WindowExists("inv");


	// Open = 2 means toggle its state
	if (open == 2) {
		if (curwin != -1) {
			open = 0;
		} else {
			open = 1;
		}
	}


	if (open == 1) {
		if (curwin != -1) {
			// Do nothing if the inv window is already open
			return;
		}

		// Create the window
		qw_window_t w = {
			.wid		= "inv",
			.title		= "Inventory",
			.x			= 100,
			.y			= 100,
			.w			= XQUI_INV_WIN_WIDTH,
			.h			= XQUI_INV_WIN_HEIGHT,
			.bg_shdr	= xq_GfxShader("gfx/2d/inventory/background.tga", NULL),
			.closefnc	= InventoryClose
		};
		qw_WinNew(w);


		// Add the already existing (also used in the "smallBars" window) character name, and character level objects
		if (h[XQ_UI_OBJID_CHAR_NAME] > 0) {
			qw_ObjAttach("inv", h[XQ_UI_OBJID_CHAR_NAME], 2, 20, 0, 0);
		}
		if (h[XQ_UI_OBJID_CHAR_LEVEL] > 0) {
			qw_ObjAttach("inv", h[XQ_UI_OBJID_CHAR_LEVEL], 2, 30, 0, 0);
		}
		if (h[XQ_UI_OBJID_CHAR_CLASSRACE] > 0) {
			qw_ObjAttach("inv", h[XQ_UI_OBJID_CHAR_CLASSRACE], 2, 40, 0, 0);
		}


		// Textual HP / Max HP representation
		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_hptxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 54, 90, 0, 0);


		// AC
		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_actxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 54, 110, 0, 0);


		// ATK
		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_atktxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 54, 180, 0, 0);


		// STR
		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_strtxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 54, 200, 0, 0);


		// Weight
		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_weighttxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 80, 354, 0, 0);


		// Resists
		int i = 400;
		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_mrtxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 20, i += 15, 0, 0);

		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_frtxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 20, i += 15, 0, 0);

		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_crtxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 20, i += 15, 0, 0);

		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_prtxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 20, i += 15, 0, 0);

		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_drtxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 20, i += 15, 0, 0);

		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text = (char *)&xqst->ui_psrtxt;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 20, i += 15, 0, 0);


		// Coin
		qw_ObjInit(&o, QW_OBJ_MONEY);
		o.money_source = XQ_MONEY_SOURCE_INV;
		o.money_type = XQ_MONEY_COPPER;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 340, 545, 0, 0);

		o.money_type = XQ_MONEY_SILVER;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 260, 545, 0, 0);

		o.money_type = XQ_MONEY_GOLD;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 200, 545, 0, 0);

		o.money_type = XQ_MONEY_PLATINUM;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 100, 545, 0, 0);



		// Set up some coordinate defines

		#define ISw (QW_OBJ_INVSLOT_WIDTH+XQUI_INV_OBJSLOT_SEPARATION) 		// invslot width + separation
		#define ISh (QW_OBJ_INVSLOT_HEIGHT+XQUI_INV_OBJSLOT_SEPARATION) 	// invslot height + separation

		#define ADD(s,id) o.slot = (s); h[id] = qw_ObjCreate(&o)	// create an invslot object
		#define ATT(id,x,y) qw_ObjAttach("inv", h[id], x, y, 0, 0)			// attach an invslot object


		// Bunch of coords
		#define OX (XQUI_INV_OBJSLOTS_X)
		#define OY (XQUI_INV_OBJSLOTS_Y)
		#define RIGHTBARX (OX + (QW_OBJ_INVSLOT_WIDTH+XQUI_INV_OBJSLOT_SEPARATION)*4)
		#define CENTERX ((OX + (RIGHTBARX - OX) / 2) + QW_OBJ_INVSLOT_WIDTH/2)
		#define CENTERX_P1 (CENTERX + (QW_OBJ_INVSLOT_WIDTH+XQUI_INV_OBJSLOT_SEPARATION))
		#define CENTERX_N1 (CENTERX - (QW_OBJ_INVSLOT_WIDTH+XQUI_INV_OBJSLOT_SEPARATION))
		#define CENTERX_N2 (CENTERX - (QW_OBJ_INVSLOT_WIDTH*2+XQUI_INV_OBJSLOT_SEPARATION*2))
		#define CARRYX (XQUI_INV_WIN_WIDTH - (QW_OBJ_INVSLOT_WIDTH+XQUI_INV_OBJSLOT_SEPARATION) * 2 - 10)
		#define CARRYY (XQUI_INV_WIN_HEIGHT - (QW_OBJ_INVSLOT_HEIGHT+XQUI_INV_OBJSLOT_SEPARATION) * 3 - 10)


		// Prepare some values
		if (h[XQ_UI_OBJID_INVSLOT_LEFTEAR] == 0) {

			// template object
			qw_ObjInit(&o, QW_OBJ_INVSLOT);
			o.tooltip = qws->strings.emptyslot;


			// TOP
			ADD(XQ_ITEM_SLOT_LEFTEAR,		XQ_UI_OBJID_INVSLOT_LEFTEAR);
			ADD(XQ_ITEM_SLOT_NECK,			XQ_UI_OBJID_INVSLOT_NECK);
			ADD(XQ_ITEM_SLOT_HEAD,			XQ_UI_OBJID_INVSLOT_HEAD);
			ADD(XQ_ITEM_SLOT_FACE,			XQ_UI_OBJID_INVSLOT_FACE);
			ADD(XQ_ITEM_SLOT_RIGHTEAR,		XQ_UI_OBJID_INVSLOT_RIGHTEAR);


			// LEFT
			ADD(XQ_ITEM_SLOT_CHEST,			XQ_UI_OBJID_INVSLOT_CHEST);
			ADD(XQ_ITEM_SLOT_ARMS,			XQ_UI_OBJID_INVSLOT_ARMS);
			ADD(XQ_ITEM_SLOT_LEFTWRIST,		XQ_UI_OBJID_INVSLOT_LEFTWRIST);
			ADD(XQ_ITEM_SLOT_WAIST,			XQ_UI_OBJID_INVSLOT_WAIST);
			ADD(XQ_ITEM_SLOT_LEFTFINGER,	XQ_UI_OBJID_INVSLOT_LEFTFINGER);


			// RIGHT
			ADD(XQ_ITEM_SLOT_BACK,			XQ_UI_OBJID_INVSLOT_BACK);
			ADD(XQ_ITEM_SLOT_SHOULDERS,		XQ_UI_OBJID_INVSLOT_SHOULDERS);
			ADD(XQ_ITEM_SLOT_RIGHTWRIST,	XQ_UI_OBJID_INVSLOT_RIGHTWRIST);
			ADD(XQ_ITEM_SLOT_HANDS,			XQ_UI_OBJID_INVSLOT_HANDS);
			ADD(XQ_ITEM_SLOT_RIGHTFINGER,	XQ_UI_OBJID_INVSLOT_RIGHTFINGER);


			// BOTTOM 1st row
			ADD(XQ_ITEM_SLOT_LEGS,			XQ_UI_OBJID_INVSLOT_LEGS);
			ADD(XQ_ITEM_SLOT_FEET,			XQ_UI_OBJID_INVSLOT_FEET);


			// BOTTOM 2nd row
			ADD(XQ_ITEM_SLOT_PRIMARY1,		XQ_UI_OBJID_INVSLOT_PRIMARY1);
			ADD(XQ_ITEM_SLOT_SECONDARY1,	XQ_UI_OBJID_INVSLOT_SECONDARY1);


			// BOTTOM 3nd row
			ADD(XQ_ITEM_SLOT_PRIMARY2,		XQ_UI_OBJID_INVSLOT_PRIMARY2);
			ADD(XQ_ITEM_SLOT_PRIMARY3,		XQ_UI_OBJID_INVSLOT_PRIMARY3);
			ADD(XQ_ITEM_SLOT_PRIMARY4,		XQ_UI_OBJID_INVSLOT_PRIMARY4);
			ADD(XQ_ITEM_SLOT_PRIMARY5,		XQ_UI_OBJID_INVSLOT_PRIMARY5);


			// BOTTOM 4nd row
			ADD(XQ_ITEM_SLOT_PRIMARY6,		XQ_UI_OBJID_INVSLOT_PRIMARY6);
			ADD(XQ_ITEM_SLOT_PRIMARY7,		XQ_UI_OBJID_INVSLOT_PRIMARY7);
			ADD(XQ_ITEM_SLOT_PRIMARY8,		XQ_UI_OBJID_INVSLOT_PRIMARY8);
			ADD(XQ_ITEM_SLOT_PRIMARY9,		XQ_UI_OBJID_INVSLOT_PRIMARY9);


			// CARRY slots
			ADD(XQ_ITEM_SLOT_CARRY1,		XQ_UI_OBJID_INVSLOT_CARRY1);
			ADD(XQ_ITEM_SLOT_CARRY2,		XQ_UI_OBJID_INVSLOT_CARRY2);
			ADD(XQ_ITEM_SLOT_CARRY3,		XQ_UI_OBJID_INVSLOT_CARRY3);
			ADD(XQ_ITEM_SLOT_CARRY4,		XQ_UI_OBJID_INVSLOT_CARRY4);
			ADD(XQ_ITEM_SLOT_CARRY5,		XQ_UI_OBJID_INVSLOT_CARRY5);
			ADD(XQ_ITEM_SLOT_CARRY6,		XQ_UI_OBJID_INVSLOT_CARRY6);
		}


		// Top
		ATT(XQ_UI_OBJID_INVSLOT_LEFTEAR,		OX,				OY);
		ATT(XQ_UI_OBJID_INVSLOT_NECK,			OX+ISw,			OY);
		ATT(XQ_UI_OBJID_INVSLOT_HEAD,			OX+ISw*2,		OY);
		ATT(XQ_UI_OBJID_INVSLOT_FACE,			OX+ISw*3,		OY);
		ATT(XQ_UI_OBJID_INVSLOT_RIGHTEAR,		OX+ISw*4,		OY);


		// Left
		ATT(XQ_UI_OBJID_INVSLOT_CHEST,			OX,				OY+ISh*1);
		ATT(XQ_UI_OBJID_INVSLOT_ARMS,			OX,				OY+ISh*2);
		ATT(XQ_UI_OBJID_INVSLOT_LEFTWRIST,		OX,				OY+ISh*3);
		ATT(XQ_UI_OBJID_INVSLOT_WAIST,			OX,				OY+ISh*4);
		ATT(XQ_UI_OBJID_INVSLOT_LEFTFINGER,		OX,				OY+ISh*5);


		// Right
		ATT(XQ_UI_OBJID_INVSLOT_BACK,			RIGHTBARX,		OY+ISh*1);
		ATT(XQ_UI_OBJID_INVSLOT_SHOULDERS,		RIGHTBARX,		OY+ISh*2);
		ATT(XQ_UI_OBJID_INVSLOT_RIGHTWRIST,		RIGHTBARX,		OY+ISh*3);
		ATT(XQ_UI_OBJID_INVSLOT_HANDS,			RIGHTBARX,		OY+ISh*4);
		ATT(XQ_UI_OBJID_INVSLOT_RIGHTFINGER,	RIGHTBARX,		OY+ISh*5);


		// Bottom row 1
		ATT(XQ_UI_OBJID_INVSLOT_LEGS,			CENTERX_N1,	OY+ISh*6);
		ATT(XQ_UI_OBJID_INVSLOT_FEET,			CENTERX,		OY+ISh*6);


		// Bottom row 2
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY1,		CENTERX_N1,	OY+ISh*7);
		ATT(XQ_UI_OBJID_INVSLOT_SECONDARY1,		CENTERX,		OY+ISh*7);


		// Bottom row 2
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY1,		CENTERX_N1,	OY+ISh*7);
		ATT(XQ_UI_OBJID_INVSLOT_SECONDARY1,		CENTERX,		OY+ISh*7);


		// Bottom row 3
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY2,		CENTERX_N2,	OY+ISh*8);
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY3,		CENTERX_N1,	OY+ISh*8);
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY4,		CENTERX,		OY+ISh*8);
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY5,		CENTERX_P1,	OY+ISh*8);


		// Bottom row 4
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY6,		CENTERX_N2,	OY+ISh*9);
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY7,		CENTERX_N1,	OY+ISh*9);
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY8,		CENTERX,		OY+ISh*9);
		ATT(XQ_UI_OBJID_INVSLOT_PRIMARY9,		CENTERX_P1,	OY+ISh*9);


		// Carry slots
		ATT(XQ_UI_OBJID_INVSLOT_CARRY1,			CARRYX,			CARRYY);
		ATT(XQ_UI_OBJID_INVSLOT_CARRY2,			CARRYX+ISw,		CARRYY);
		ATT(XQ_UI_OBJID_INVSLOT_CARRY3,			CARRYX,			CARRYY+ISh);
		ATT(XQ_UI_OBJID_INVSLOT_CARRY4,			CARRYX+ISw,		CARRYY+ISh);
		ATT(XQ_UI_OBJID_INVSLOT_CARRY5,			CARRYX,			CARRYY+ISh*2);
		ATT(XQ_UI_OBJID_INVSLOT_CARRY6,			CARRYX+ISw,		CARRYY+ISh*2);



		// Inv exp bar
		qw_ObjInit(&o, QW_OBJ_PERCBAR);
		o.perc = &xqst->ui_xp_perc;
		o.multiplier = 1000;
		o.filledcolor = XQUI_XP_BAR_GRADCOL;
		o.fastbar = 5;
		o.borderwidth = 1;
		qw_ObjAttach("inv", qw_ObjCreate(&o), 10, 155, 0, 0);


		// AUTOEQUIP AREA
		qw_ObjInit(&o, QW_OBJ_AUTOEQUIP);
		qw_ObjAttach("inv", qw_ObjCreate(&o),
			OX + ISw,
			OY + ISh,
			0, 0);


		// DESTROY BUTTON
		qw_ObjInit(&o, QW_OBJ_BUTTON);
		o.action = QW_OBJ_BUTTON_ACTION_DESTROY;
		o.padding_x = 15;
		o.padding_y = 15;
		Q_strncpyz(o.text_static, "Destroy", sizeof(o.text_static));
		qw_ObjAttach("inv", qw_ObjCreate(&o), 10, 280, 0, 0);


		// Mark inv as open
		xqst->ui_state_inv_open = 1;
	} else {
		// Destroy the inv window
		if (curwin == -1) {
			return;
		}
		qw_WindowDelete("inv");
		xqst->ui_state_inv_open = 0;
	}
}


// ITEM / SPELL INSPECTOR
void xqui_OpenInspector(int64_t item, int spell) {
	// We want to open the window a little off the mouse cursor
	int mx = xqst->mousex - 50;
	int my = xqst->mousey - 50;
	if (mx < 0) mx = 0;
	if (my < 0) my = 0;


	// Sanity checks
	if (item == 0 && spell == 0) {
		xq_clog(COLOR_RED, "xqui_OpenInspector: item and spell both null");
		return;
	} else if (item && spell) {
		xq_clog(COLOR_RED, "xqui_OpenInspector: item and spell both not null");
		return;
	}


	// Decide on what we're inspecting
	char buf[QW_MAX_WID+1] = {0};
	if (item) {
		snprintf((char *)buf, QW_MAX_WID, "iteminfo %li", item); 
	} else {
		snprintf((char *)buf, QW_MAX_WID, "spellinfo %i", spell);
	}


	// Remove existing window
	qw_WindowDelete(buf);


	// Create new one
	qw_window_t w = {
		.x			= mx,
		.y			= my,
		.w			= QW_OBJ_ITEMINSPECTOR_WIDTH,
		.h			= QW_OBJ_ITEMINSPECTOR_HEIGHT,
		.no_save	= 1,
		.closex		= 1
	};
	Q_strncpyz(w.wid, buf, sizeof(w.wid));
	int inspect_win = qw_WinNew(w);


	// Add object
	qw_obj_t o;
	qw_ObjInit(&o, QW_OBJ_ITEMINSPECTOR);
	o.item = item;
	o.spell = spell;
	o.title_window = inspect_win;
	qw_ObjAttach(buf, qw_ObjCreate(&o), 0, 10, 0, 0);
}


// CONTAINER
static void ContainerClose_Helper(void *w) {
	xqui_ContainerClose(((qw_window_t *)w)->closefarg, 0);
}
void xqui_Container_GetWID(int slot, char *buf) {

	if (slot >= XQ_ITEM_SLOT_CARRY1 && slot <= XQ_ITEM_SLOT_CARRY6) {
		snprintf(buf, QW_MAX_WID, "cont_%i", slot - XQ_ITEM_SLOT_CARRY1 + 1);
	} else if (slot >= XQ_ITEM_SLOT_BANK1 && slot <= XQ_ITEM_SLOT_BANK4) {
		snprintf(buf, QW_MAX_WID, "cont_bank_%i", slot - XQ_ITEM_SLOT_BANK1 + 1);
	} else if (slot >= XQ_ITEM_SLOT_TRADE_GIVE1 && slot <= XQ_ITEM_SLOT_TRADE_GIVE4) {
		snprintf(buf, QW_MAX_WID, "cont_give_%i", slot - XQ_ITEM_SLOT_TRADE_GIVE1 + 1);
	} else if (slot >= XQ_ITEM_SLOT_TRADE_RECEIVE1 && slot <= XQ_ITEM_SLOT_TRADE_RECEIVE4) {
		snprintf(buf, QW_MAX_WID, "cont_receive_%i", slot - XQ_ITEM_SLOT_TRADE_RECEIVE1 + 1);
	}
}
void xqui_ContainerOpen(int slot, int silent) {
	int i;
	int column = 0;
	int row = 0;
	qw_obj_t tmpobj;
	int top_marg = 50;
	int bot_marg = 50;
	int left_marg = 5;
	int right_marg = 5;


	char wid[QW_MAX_WID+1] = {0};
	xqui_Container_GetWID(slot, wid);
	if (!wid[0]) return; // make sure it's a valid slot for a container

	int64_t cont = xq_InvSlotItem(slot, 0, 0);
	if (!cont) return; // if the slot is empty, silently return

	xq_item_t *iinf = xq_ItemInfo(cont);
	if (!iinf) return; // don't have iteminfo yet? opening will just fail

	if (!iinf->container_slots) return; // slot doesn't have a container

	if (qw_WindowExists(wid) != -1) return; // container for the slot is already open

	// Open a container
	// Play the opening sound
	if (!silent) {
		trap_S_StartLocalSound(SOUNDH("sound/inv/container_open.wav"), CHAN_ANNOUNCER);
	}


	// Create the window
	int h = (QW_OBJ_INVSLOT_HEIGHT * iinf->container_slots / 2) + top_marg + bot_marg;
	int w = QW_OBJ_INVSLOT_WIDTH * 2 + 3 + left_marg + right_marg;
	qw_window_t win = {
		.w			= w,
		.h			= h,
		.no_title	= 1,
		.closefnc	= ContainerClose_Helper,
		.closefarg	= slot
	};
	Q_strncpyz(win.wid, wid, sizeof(win.wid));
	qw_WinNew(win);


	// Add the container's item name at the top of the window.
	// Status bar won't be large enough here.
	qw_ObjInit(&tmpobj, QW_OBJ_TEXT);
	Q_strncpyz(tmpobj.text_static, iinf->name, QW_OBJ_TEXT_MAX_TEXT_LEN);
	qw_ObjAttach(wid, qw_ObjCreate(&tmpobj), 0, 5, 1, 0);


	// Add the item slots
	qw_ObjInit(&tmpobj, QW_OBJ_INVSLOT);
	tmpobj.slot = XQ_ITEM_SLOT_INCONTAINER;
	for (i = 1;  i <= iinf->container_slots;  i++) {
		tmpobj.container_slot = i;
		tmpobj.container_id = iinf->id;
		tmpobj.item = *(&iinf->container_slot_1 + i - 1);
		tmpobj.parent_slot = slot; // this is needed for the invslot obj to find its new container_id after item rekeying

		qw_ObjAttach(wid,
			qw_ObjCreate(&tmpobj),
			column * QW_OBJ_INVSLOT_WIDTH + left_marg,
			top_marg + row * QW_OBJ_INVSLOT_HEIGHT,
			0, 0);

		if (column == 1) {
			row++;
		}
		column = (column == 0 ? 1 : 0);
	}


	// Add the optional combine button
	if (iinf->tradeskill == 1) {
		qw_ObjInit(&tmpobj, QW_OBJ_BUTTON);
		tmpobj.action = QW_OBJ_BUTTON_ACTION_COMBINE;
		tmpobj.arg1 = slot;
		Q_strncpyz(tmpobj.text_static, qws->strings.combine, sizeof(tmpobj.text_static));
		qw_ObjAttach(wid, qw_ObjCreate(&tmpobj), 10, -30, 0, 0);
	}


}
void xqui_ContainerClose(int slot, int silent) {
	// Figure out the window id
	char wid[QW_MAX_WID+1] = {0};
	xqui_Container_GetWID(slot, wid);

	// Remove the window and play the sound
	if (wid[0]) {
		if (qw_WindowExists(wid) != -1) {
			qw_WindowDelete(wid);
			if (!silent) {
				trap_S_StartLocalSound(SOUNDH("sound/inv/container_close.wav"), CHAN_ANNOUNCER);
			}
		}
	}
}
void xqui_Bags() {
	if (!xq_ShiftPressed()) {
		xqui_ContainerClose(XQ_ITEM_SLOT_CARRY1, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_CARRY2, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_CARRY3, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_CARRY4, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_CARRY5, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_CARRY6, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_BANK1, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_BANK2, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_BANK3, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_BANK4, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_GIVE1, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_GIVE2, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_GIVE3, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_GIVE4, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_RECEIVE1, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_RECEIVE2, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_RECEIVE3, 1);
		xqui_ContainerClose(XQ_ITEM_SLOT_TRADE_RECEIVE4, 1);
	} else {
		xqui_ContainerOpen(XQ_ITEM_SLOT_CARRY1, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_CARRY2, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_CARRY3, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_CARRY4, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_CARRY5, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_CARRY6, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_BANK1, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_BANK2, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_BANK3, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_BANK4, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_TRADE_GIVE1, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_TRADE_GIVE2, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_TRADE_GIVE3, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_TRADE_GIVE4, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_TRADE_RECEIVE1, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_TRADE_RECEIVE2, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_TRADE_RECEIVE3, 1);
		xqui_ContainerOpen(XQ_ITEM_SLOT_TRADE_RECEIVE4, 1);
	}
}


// CASTING TIME BAR
void xqui_StopCastingBar() {
	qw_WindowDelete("casting");
}
void xqui_StartCastingBar(int time) {

	// Make window
	qw_window_t w = {
		.wid		= "casting",
		.title		= "Casting Time",
		.w			= XQUI_CASTING_BAR_WIDTH + 5,
		.h			= XQUI_CASTING_BAR_HEIGHT + 10,
	};
	qw_WinNew(w);


	// Add time bar
	qw_obj_t o;
	qw_ObjInit(&o, QW_OBJ_TIMEBAR);
	o.tsecs = time;
	o.barwidth = XQUI_CASTING_BAR_WIDTH;
	o.barheight = XQUI_CASTING_BAR_HEIGHT;
	o.filledcolor = XQUI_CASTING_BAR_FILLEDCOLOR;
	o.emptycolor = XQUI_CASTING_BAR_EMPTYCOLOR;
	qw_ObjAttach("casting", qw_ObjCreate(&o), 0, 0, 0, 0);
}


// SPELLBOOK
static void SpellBookClose(void *win) {
	xq_scmd("/spellbookpage 0");
}
static void SpellBookButtons(void) {
	int i = qws->objhandles[XQ_UI_OBJID_BUTTON_SPELLBOOK_NEXT];
	int page = qws->spellbook_page;

	if (i > 0) {
		qws->obj[i].hidden = (page < XQ_SPELL_BOOK_PAGES ? 0 : 1);
		qws->obj[i].arg1 = page + 1;
	}

	i = qws->objhandles[XQ_UI_OBJID_BUTTON_SPELLBOOK_PREV];
	if (i > 0) {
		qws->obj[i].hidden = (page < 2 ? 1 : 0);
		qws->obj[i].arg1 = page - 1;
	}
}
void xqui_SpellBook_Toggle() {
	if (qws->spellbook_page == 0) {
		if (!xqst->mousefree) return;
		xq_scmd("/spellbookpage 1");
	} else {
		xq_scmd("/spellbookpage 0");
	}
}
void xqui_SpellBook() {
	int win = qw_WindowExists("spellbook");
	qw_obj_t *open_button = &qws->obj[qws->objhandles[XQ_UI_OBJID_SPELLBOOK_SWITCH]];
	int coords[] = {
		200, 50,
		200, 230,
		200, 410,
		-150, 50,
		-150, 230,
		-150, 410
	};

	if (win != -1) {
		if (qws->spellbook_page == 0) {
			// Close the book
			qws->spellbook_page_txt[0] = 0;
			qws->swap_spell = 0;
			qw_WindowDelete("spellbook");
			if (open_button) {
				open_button->arg1 = 1;
			}
		} else {
			// Changed page
			SpellBookButtons();
		}
		return;
	}
			
	// Need to open book
	qws->swap_spell = 0;
	qw_window_t w = {
		.wid		= "spellbook",
		.title		= "Spell Book",
		.x			= -1,
		.y			= -1,
		.w			= XQUI_SPELL_BOOK_WIDTH,
		.h			= XQUI_SPELL_BOOK_HEIGHT,
		.bg_shdr	= xq_GfxShader("gfx/2d/misc/spellbook_bg", NULL),
		.no_title	= 1,
		.closefnc	= SpellBookClose
	};
	qw_WinNew(w);


	// Toggle the spellbook open button action to close
	if (open_button) {
		open_button->arg1 = 0;
	}


	// Add the scribed spell slots
	qw_obj_t o;
	qw_ObjInit(&o, QW_OBJ_SPELLBOOK_SLOT);
	for (int i = 0;  i < XQ_SPELL_BOOK_PAGESPELLS;  i++) {
		o.slot = i + 1;
		qw_ObjAttach("spellbook", qw_ObjCreate(&o), coords[i*2], coords[i*2+1], 0, 0);
	}


	// Page # text
	qw_ObjInit(&o, QW_OBJ_TEXT);
	o.text = qws->spellbook_page_txt;
	qw_ObjAttach("spellbook", qw_ObjCreate(&o), 0, -12, 1, 0);


	// Next page button
	qw_ObjInit(&o, QW_OBJ_BUTTON);
	o.action = QW_OBJ_BUTTON_ACTION_SPELLBOOK_SWITCH;
	o.text = qws->strings.nextpage;
	qw_ObjAttach("spellbook", qws->objhandles[XQ_UI_OBJID_BUTTON_SPELLBOOK_NEXT] = qw_ObjCreate(&o), -200, -25, 0, 0);


	// Previous page button
	o.text = qws->strings.previouspage;
	qw_ObjAttach("spellbook", qws->objhandles[XQ_UI_OBJID_BUTTON_SPELLBOOK_PREV] = qw_ObjCreate(&o), 200, -25, 0, 0);


	// Run initial logic on hide/show pagination buttons
	SpellBookButtons();
}


// REZ BOX
void xqui_RezBox() {
	if (!cg.snap) return;

	char buf[30];
	snprintf(buf, 29, "%s", xq_RezName(&cg.snap->ps));
	buf[0] = toupper(buf[0]);

	qw_Prompt(
		va("Accept the %i%% experience ressurection from %s ?",
			cg.snap->ps.xq_rez_perc,
			buf
		),
		QW_PROMPT_ACTION_REZACCEPT,
		QW_PROMPT_ACTION_REZDECLINE,
		0, 0, 0
	);
}


// SKILLS WINDOW
void xqui_Skills() {
	int wid = qw_WindowExists("skills");
	if (wid != -1) {
		qw_WindowDelete("skills");
		return;
	}


	// Make the window
	qw_window_t w = {
		.wid		= "skills",
		.title		= "Skills",
		.x			= -1,
		.y			= -1,
		.w			= XQUI_SKILLS_WIN_WIDTH,
		.h			= XQUI_SKILLS_WIN_HEIGHT,
		.closex		= 1
	};
	qw_WinNew(w);

	qw_obj_t o;
	qw_ObjInit(&o, QW_OBJ_TEXT);


	// Add the skill levels
	o.text = (char *)&xqst->ui_skill_capups;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 200, 460, 0, 0);

	o.text = (char *)&xqst->ui_skill_tailoring;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 20, 0, 0);

	o.text = (char *)&xqst->ui_skill_cooking;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 40, 0, 0);

	o.text = (char *)&xqst->ui_skill_blacksmithing;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 60, 0, 0);

	o.text = (char *)&xqst->ui_skill_swimming;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 80, 0, 0);

	o.text = (char *)&xqst->ui_skill_channelling;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 100, 0, 0);

	o.text = (char *)&xqst->ui_skill_melee;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 120, 0, 0);

	o.text = (char *)&xqst->ui_skill_fire;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 140, 0, 0);

	o.text = (char *)&xqst->ui_skill_cold;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 160, 0, 0);

	o.text = (char *)&xqst->ui_skill_poison;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 180, 0, 0);

	o.text = (char *)&xqst->ui_skill_disease;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 200, 0, 0);

	o.text = (char *)&xqst->ui_skill_psy;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 220, 0, 0);

	o.text = (char *)&xqst->ui_skill_w2;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 240, 0, 0);

	o.text = (char *)&xqst->ui_skill_w3;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 260, 0, 0);

	o.text = (char *)&xqst->ui_skill_w4;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 280, 0, 0);

	o.text = (char *)&xqst->ui_skill_w5;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 300, 0, 0);

	o.text = (char *)&xqst->ui_skill_w6;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 320, 0, 0);

	o.text = (char *)&xqst->ui_skill_w7;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 340, 0, 0);

	o.text = (char *)&xqst->ui_skill_w8;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 360, 0, 0);

	o.text = (char *)&xqst->ui_skill_w9;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 380, 0, 0);

	o.text = (char *)&xqst->ui_skill_h2h;
	qw_ObjAttach("skills", qw_ObjCreate(&o), 10, 400, 0, 0);


	// Add the capup buttons
	for (int i = XQ_SKILL_TAILORING;  i <= XQ_SKILL_W9;  i++) {
		qw_ObjInit(&o, QW_OBJ_BUTTON);
		o.action = QW_OBJ_BUTTON_ACTION_SKILL_CAPUP;
		o.arg1 = i;
		o.padding_x = 2;
		o.padding_y = 2;
		Q_strncpyz(o.text_static, "Increase cap", sizeof(o.text_static));
		qw_ObjAttach("skills", qw_ObjCreate(&o), 200, 20 * i, 0, 0);
	}
}


// SPELL BAR
void xqui_SpellBar(void) {
	qw_window_t w = {
		.wid		= "spells",
		.w			= QW_OBJ_SPELL_GEM_WIDTH+8,
		.h			= QW_OBJ_SPELL_GEM_HEIGHT*XQ_SPELL_SLOTS+65,
		.no_title	= 1
	};
	qw_WinNew(w);

	qw_obj_t o;
	qw_ObjInit(&o, QW_OBJ_SPELL_GEM);
	for (int i = 1;  i <= XQ_SPELL_SLOTS;  i++) {
		o.slot = i;
		qw_ObjAttach("spells", qw_ObjCreate(&o), 0, 10 + (i - 1) * (QW_OBJ_SPELL_GEM_HEIGHT + 3), 0, 0);
	}
}


// LOOT
void xqui_LootOpen(void) {
	if (qw_WindowExists("loot") != -1) {
		xq_clog(COLOR_RED, "Loot window already open, weird");
		return;
	}


	// Make the window
	qw_window_t w = {
		.wid		= "loot",
		.title		= "Loot",
		.x			= 0,
		.y			= 0,
		.w			= QW_OBJ_INVSLOT_WIDTH * XQUI_LOOT_COLUMNS + 2 + XQUI_LOOT_COLUMNS * XQUI_LOOT_DIST,
		.h			= QW_OBJ_INVSLOT_HEIGHT * XQUI_LOOT_ROWS + 2 + XQUI_LOOT_ROWS * XQUI_LOOT_DIST,
		.no_save	= 1,
		.closefnc	= xqui_LootClose
	};
	qw_WinNew(w);


	// Add the loot slots
	qw_obj_t o;
	qw_ObjInit(&o, QW_OBJ_INVSLOT);

	int row = 0, col = 0;
	for (int i = 0;  i < XQ_LOOT_MAX_SLOT;  i++) {
		int64_t item = xq_InvSlotItem(XQ_ITEM_SLOT_LOOT1 + i, 0, 0);
		if (item != 0) {
			o.slot = XQ_ITEM_SLOT_LOOT1 + i;
			int x = 1 + (QW_OBJ_INVSLOT_WIDTH + XQUI_LOOT_DIST) * col;
			int y = 1 + QW_WIN_STATUSBAR_HEIGHT + row * (QW_OBJ_INVSLOT_HEIGHT + XQUI_LOOT_DIST);
			qw_ObjAttach("loot", qw_ObjCreate(&o), x, y, 0, 0);

			col++;
			if (col == XQUI_LOOT_COLUMNS) {
				col = 0;
				row++;
			}
		}
	}

	qw_MouseMove();


	// Make sure inv is open
	xqui_Inventory(1);
}
void xqui_LootClose(void *win) {
	xq_scmd("/lootoff");
}


// MERCHANT
void xqui_MerchantOpen(void) {
	if (qw_WindowExists("merchant") != -1) {
		xq_clog(COLOR_RED, "Merchant window already open, weird");
		return;
	}


	// Open the inventory
	xqui_Inventory(1);


	// Make the window
	qw_window_t w = {
		.wid		= "merchant",
		.title		= "Merchant",
		.w			= XQUI_MERCHANT_WIN_WIDTH,
		.h			= XQUI_MERCHANT_WIN_HEIGHT,
		.closefnc	= xqui_MerchantClose

	};
	qw_WinNew(w);


	// Add the item slots
	qw_obj_t o;
	for (int i = 0;  i < XQ_MERCHANT_MAX_SLOTS;  i++) {
		int y = 10 + i * (QW_OBJ_INVSLOT_HEIGHT - 1);
		// The clickable item graphic
		qw_ObjInit(&o, QW_OBJ_INVSLOT);
		o.slot = XQ_ITEM_SLOT_MERCHANT1 + i;
		qw_ObjAttach("merchant",  qw_ObjCreate(&o), 0, y, 0, 0);

		// The item name
		qw_ObjInit(&o, QW_OBJ_TEXT);
		o.text_itemname_invslot = XQ_ITEM_SLOT_MERCHANT1 + i;
		qw_ObjAttach("merchant", qw_ObjCreate(&o),
			QW_OBJ_INVSLOT_WIDTH + 10,
			y + (QW_OBJ_INVSLOT_HEIGHT / 2) - 2,
			0, 0
		);
	}

	int *h = qws->objhandles;

	// Create the buy and sell buttons if needed
	if (h[XQ_UI_OBJID_BUTTON_BUY] == 0) {
		qw_ObjInit(&o, QW_OBJ_BUTTON);
		o.action = QW_OBJ_BUTTON_ACTION_BUY;
		o.hidden = 1;
		Q_strncpyz(o.text_static, "Buy", sizeof(o.text_static));
		h[XQ_UI_OBJID_BUTTON_BUY] = qw_ObjCreate(&o);

		o.action = QW_OBJ_BUTTON_ACTION_SELL;
		Q_strncpyz(o.text_static, "Sell", sizeof(o.text_static));
		h[XQ_UI_OBJID_BUTTON_SELL] = qw_ObjCreate(&o);
	}
	qws->obj[h[XQ_UI_OBJID_BUTTON_BUY]].hidden = 1;
	qws->obj[h[XQ_UI_OBJID_BUTTON_SELL]].hidden = 1;


	// Attach them
	qw_ObjAttach("merchant", h[XQ_UI_OBJID_BUTTON_BUY],
		10,
		XQUI_MERCHANT_WIN_HEIGHT - 40,
		0, 0
	);
	qw_ObjAttach("merchant", h[XQ_UI_OBJID_BUTTON_SELL],
		50,
		XQUI_MERCHANT_WIN_HEIGHT - 40,
		0, 0
	);


	// Invoiced item icon
	if (h[XQ_UI_OBJID_MERCHANT_ITEMGFX] == 0) {
		qw_ObjInit(&o, QW_OBJ_ITEMGFX);
		h[XQ_UI_OBJID_MERCHANT_ITEMGFX] = qw_ObjCreate(&o);
	}
	qw_ObjAttach("merchant", h[XQ_UI_OBJID_MERCHANT_ITEMGFX],
		XQUI_MERCHANT_WIN_WIDTH - 40,
		XQUI_MERCHANT_WIN_HEIGHT - 40,
		0, 0
	);
	qws->obj[h[XQ_UI_OBJID_MERCHANT_ITEMGFX]].item = 0;


	// Invoiced item name
	if (h[XQ_UI_OBJID_MERCHANT_ITEMTXT] == 0) {
		qw_ObjInit(&o, QW_OBJ_TEXT);
		h[XQ_UI_OBJID_MERCHANT_ITEMTXT] = qw_ObjCreate(&o);
	}
	qw_ObjAttach("merchant", h[XQ_UI_OBJID_MERCHANT_ITEMTXT],
		120,
		XQUI_MERCHANT_WIN_HEIGHT - 30,
		0, 0
	);
	qws->obj[h[XQ_UI_OBJID_MERCHANT_ITEMTXT]].text_static[0] = 0;


	xqui_InvSlotUnselectAll();
	qw_MouseMove();
}
void xqui_MerchantClose(void *win) {
	xq_scmd("/merchantoff");
}


// BANK
void xqui_BankOpen(void) {
	int row = 0, col = 0;


	if (qw_WindowExists("bank") != -1) {
		xq_clog(COLOR_RED, "Bank window already open, weird");
		return;
	}
	xqui_Inventory(1);

	qw_window_t w = {
		.wid		= "bank",
		.title		= "Bank",
		.x			= 0,
		.y			= 0,
		.w			= 300,
		.h			= 300,
		.closefnc	= xqui_BankClose
	};
	qw_WinNew(w);

	qw_obj_t o;
	int i;


	// Item slots
	qw_ObjInit(&o, QW_OBJ_INVSLOT);
	for (i = 0;  i <= 3;  i++) {
		o.slot = XQ_ITEM_SLOT_BANK1 + i;
		qw_ObjAttach("bank",  qw_ObjCreate(&o), 1 + (QW_OBJ_INVSLOT_WIDTH + 2) * col, 10 + row * (QW_OBJ_INVSLOT_HEIGHT + 2), 0, 0);
		col++; if (col == 2) { col = 0;  row++; }
	}


	// Money
	qw_ObjInit(&o, QW_OBJ_MONEY);
	o.money_type = XQ_MONEY_PLATINUM;
	o.money_source = XQ_MONEY_SOURCE_BANK;
	qw_ObjAttach("bank", qw_ObjCreate(&o), 20, 120, 0, 0);

	o.money_type = XQ_MONEY_GOLD;
	o.money_source = XQ_MONEY_SOURCE_BANK;
	qw_ObjAttach("bank", qw_ObjCreate(&o), 80, 120, 0, 0);

	o.money_type = XQ_MONEY_SILVER;
	o.money_source = XQ_MONEY_SOURCE_BANK;
	qw_ObjAttach("bank", qw_ObjCreate(&o), 140, 120, 0, 0);

	o.money_type = XQ_MONEY_COPPER;
	o.money_source = XQ_MONEY_SOURCE_BANK;
	qw_ObjAttach("bank", qw_ObjCreate(&o), 200, 120, 0, 0);

	qw_MouseMove();
}
void xqui_BankClose(void *win) {
	xq_scmd("/bankoff");
}


// TRADE
void xqui_TradeOpen() {
	int pc = 0;
	playerState_t *ps = &cg.snap->ps;
	if (ps->xq_flags & XQ_TRADING_PC) {
		pc = 1;
	} else if (ps->xq_flags & XQ_TRADING_NPC) {
		pc = 0;
	} 


	if (qw_WindowExists("tradegive") != -1 || qw_WindowExists("tradereceive") != -1) {
		xq_clog(COLOR_RED, "Trade window already open, weird");
		return;
	}
	xqui_Inventory(1);


	qw_window_t give_w = {
		.wid		= "give",
		.title		= "Give",
		.w			= 300,
		.h			= 300,
		.closefnc	= xqui_TradeClose
	};
	qw_WinNew(give_w);




	if (pc) {
		qw_window_t receive_w = {
			.wid		= "receive",
			.title		= "Receive",
			.x			= 400,
			.w			= 300,
			.h			= 300,
			.closefnc	= xqui_TradeClose
		};
		qw_WinNew(receive_w);
	}


	// Give item slots
	qw_obj_t o;
	int i;
	int row = 0, col = 0;
	qw_ObjInit(&o, QW_OBJ_INVSLOT);

	for (i = 0;  i <= 3;  i++) {
		o.slot = XQ_ITEM_SLOT_TRADE_GIVE1 + i;
		qw_ObjAttach("give",  qw_ObjCreate(&o),
			1 + (QW_OBJ_INVSLOT_WIDTH + 2) * col,
			10 + row * (QW_OBJ_INVSLOT_HEIGHT + 2),
			0, 0
		);
		col++;
		if (col == 2) {
			col = 0;
			row++;
		}
	}


	// Money
	qw_ObjInit(&o, QW_OBJ_MONEY);
	o.money_type = XQ_MONEY_PLATINUM;
	o.money_source = XQ_MONEY_SOURCE_GIVE;
	qw_ObjAttach("give", qw_ObjCreate(&o), 20, 120, 0, 0);

	o.money_type = XQ_MONEY_GOLD;
	o.money_source = XQ_MONEY_SOURCE_GIVE;
	qw_ObjAttach("give", qw_ObjCreate(&o), 80, 120, 0, 0);

	o.money_type = XQ_MONEY_SILVER;
	o.money_source = XQ_MONEY_SOURCE_GIVE;
	qw_ObjAttach("give", qw_ObjCreate(&o), 140, 120, 0, 0);

	o.money_type = XQ_MONEY_COPPER;
	o.money_source = XQ_MONEY_SOURCE_GIVE;
	qw_ObjAttach("give", qw_ObjCreate(&o), 200, 120, 0, 0);



	if (pc) {
		// If we're trading with a PC, add receive item slots and money
		row = col = 0;
		qw_ObjInit(&o, QW_OBJ_INVSLOT);

		for (i = 0;  i <= 3;  i++) {
			o.slot = XQ_ITEM_SLOT_TRADE_RECEIVE1 + i;
			qw_ObjAttach("receive",  qw_ObjCreate(&o),
				100 + (QW_OBJ_INVSLOT_WIDTH + 2) * col,
				10 + row * (QW_OBJ_INVSLOT_HEIGHT + 2),
				0, 0
			);
			col++;
			if (col == 2) {
				col = 0;
				row++;
			}
		}


		// Money
		qw_ObjInit(&o, QW_OBJ_MONEY);
		o.money_type = XQ_MONEY_PLATINUM;
		o.money_source = XQ_MONEY_SOURCE_RECEIVE;
		qw_ObjAttach("receive", qw_ObjCreate(&o), 20, 120, 0, 0);

		o.money_type = XQ_MONEY_GOLD;
		o.money_source = XQ_MONEY_SOURCE_RECEIVE;
		qw_ObjAttach("receive", qw_ObjCreate(&o), 80, 120, 0, 0);

		o.money_type = XQ_MONEY_SILVER;
		o.money_source = XQ_MONEY_SOURCE_RECEIVE;
		qw_ObjAttach("receive", qw_ObjCreate(&o), 140, 120, 0, 0);

		o.money_type = XQ_MONEY_COPPER;
		o.money_source = XQ_MONEY_SOURCE_RECEIVE;
		qw_ObjAttach("receive", qw_ObjCreate(&o), 200, 120, 0, 0);
	}


	// Agree on trade viewer's button
	int *h = &qws->objhandles[XQ_UI_OBJID_BUTTON_TRADE_ME];
	if (*h == 0) {
		qw_ObjInit(&o, QW_OBJ_BUTTON);
		o.action = QW_OBJ_BUTTON_ACTION_TRADE;
		Q_strncpyz(o.text_static, "Agree on Trade", sizeof(o.text_static));
		qw_ObjAttach("give", *h = qw_ObjCreate(&o), 10, 250, 0, 0);
	} else {
		qw_ObjAttach("give", *h, 10, 250, 0, 0);
		if (ps->xq_trading_me_agree == 1) {
			qws->obj[*h].bordercolor = QW_OBJ_BUTTON_TRADE_AGREE_YES;
		} else {
			qws->obj[*h].bordercolor = QW_OBJ_BUTTON_TRADE_AGREE_NO;
		}

	}


	if (pc) {
		// Agree on trade other PC's button
		h = &qws->objhandles[XQ_UI_OBJID_BUTTON_TRADE_THEM];
		if (*h == 0) {
			qw_ObjInit(&o, QW_OBJ_BUTTON);

			// This button has no action defined as it only serves
			// the purpose of displaying other PC's trade agreement status.
			Q_strncpyz(o.text_static, "Agree on Trade", sizeof(o.text_static));
			qw_ObjAttach("receive", *h = qw_ObjCreate(&o), 50, 250, 0, 0);
		} else {
			qw_ObjAttach("receive", *h, 50, 250, 0, 0);
			if (ps->xq_trading_me_agree == 1) {
				qws->obj[*h].bordercolor = QW_OBJ_BUTTON_TRADE_AGREE_YES;
			} else {
				qws->obj[*h].bordercolor = QW_OBJ_BUTTON_TRADE_AGREE_NO;
			}
		}
	}

	qw_MouseMove();
}
void xqui_TradeClose(void *win) {
	xq_scmd("/tradeoff");
}


// TOOLTIPS
void xqui_Tooltips_On() {
	xqst->show_gem_tooltips = 1;
}
void xqui_Tooltips_Off() {
	xqst->show_gem_tooltips = 0;
}


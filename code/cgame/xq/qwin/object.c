#include "../../cg_local.h"

// This is used by qsort
qw_obj_attach_t *xqui_tmp_obj_att;

qw_obj_click_t *qw_ObjectFindXY(int win, int mx, int my, int skip_clickthrough, int skip_type) {
	// Find the highest zindex object at mx/my
	// optionally ignoring the clickthrough objects.

	int actual_x = 0, actual_y = 0;
	int objs[QW_OBJ_PER_WIN] = {0};
	qw_window_t *w = &qws->win[win];
	qw_obj_attach_t *att;


	// Make a list of all candidate objects at mx/my
	int i;
	int cnt = 0;
	for (i = 0;  i < QW_OBJ_PER_WIN;  i++) {
		att = &w->obj[i];
		if (att->num > 0) {
			qw_obj_t *o = &qws->obj[att->num];
			if (o->hidden == 1) {
				continue;
			}
			if (o->clickthrough == 1 && skip_clickthrough == 1) {
				continue;
			}
			if (o->type == skip_type) {
				continue;
			}
			actual_x = att->x;
			actual_y = att->y;
			if (actual_x < 0 || att->x_centered == 1) {
				actual_x = att->actual_x;
			}
			if (att->y < 0 || att->y_centered == 1) {
				actual_y = att->actual_y;
			}
			if (
					(w->x + actual_x) <= mx &&
					(w->x + actual_x + o->w) >= mx &&
					(w->y + actual_y) <= my &&
					(w->y + actual_y + o->h) >= my
				) {
				objs[cnt++] = i;
			}
		}
	}


	// Find the one with the highest zindex
	int topobj = -1;
	if (cnt > 0) {
		int topzi = 0;
		for (i = 0;  i < cnt;  i++) {
			att = &w->obj[objs[i]];
			if (att->zindex > topzi) {
				topzi = att->zindex;
				topobj = objs[i];
			}
		}
	}


	// Return info about the winner
	static qw_obj_click_t ret = {0};
	ret.objid = topobj;
	ret.actual_x = actual_x;
	ret.actual_y = actual_y;

	return &ret;
}
int qw_ObjZICompareFnc(const void *a, const void *b) {

	// See if we have explicit zindex values that must be honored
	int azi = xqui_tmp_obj_att[*(int*)a].zindex;
	int bzi = xqui_tmp_obj_att[*(int*)b].zindex;
	if (azi > bzi) return 1;
	if (azi < bzi) return -1;


	// same zindex for both elements, we untie by attach order, the object that was attached first is considered lower zindex
	int aoid = xqui_tmp_obj_att[*(int*)a].num;
	int boid = xqui_tmp_obj_att[*(int*)b].num;

	for (int i = 0;  i < QW_OBJ_PER_WIN;  i++) {
		if (xqui_tmp_obj_att[i].num == aoid) return -1;
		if (xqui_tmp_obj_att[i].num == boid) return 1;
	}

	// shouldn't be reached
	return 0;
}
void qw_ObjDetach(int win, int oid) {
	// Detaches an object from a window.
	// If it's not saved in qws->objhandles[] and use count is 0,
	// the object is also destroyed.


	for (int i = 0;  i < QW_OBJ_PER_WIN;  i++) {
		qw_obj_attach_t *att = &qws->win[win].obj[i];

		if (att->num == oid) {
			// We found the attachment slot of the object
			int preserve = 0;
			if (qws->obj[att->num].usecount > 1) {
				preserve = 1;
			}


			// The object isn't attached to another window - check qws->objhandles[]
			if (preserve == 0) {
				for (int k = 0;  k < QW_MAX_OBJ_SAVE;  k++) {
					if (qws->objhandles[k] == oid) {
						preserve = 1;
						break;
					}
				}
			}


			// Finish the job
			if (preserve == 0) {
				qws->obj[oid].type = QW_OBJ_NONE;
			} else {
				qws->obj[oid].usecount--;
			}
			att->num = 0;
		}
	}
}
static int qw_ObjIsAttached(int win, int oid) {
	int i;
	for (i = 0;  i < QW_OBJ_PER_WIN;  i++) {
		if (qws->win[win].obj[i].num == oid) {
			return 1;
		}
	}
	return 0;
}
void qw_ObjDetachFromAll(int oid) {
	int i;

	for (i = 0;  i < QW_MAX_WINDOWS;  i++) {
		if (qws->win[i].wid[0]) {
			if (qw_ObjIsAttached(i, oid)) {
				qw_ObjDetach(i, oid);
			}
		}
	}
}
void qw_ObjDraw(qw_window_t *win, int attachnum) {
	// This will draw the object referenced by the window and the object attachment

	qw_obj_attach_t *att = &win->obj[attachnum];
	qw_obj_t *obj = &qws->obj[att->num];
	void (*f)(qw_window_t *, qw_obj_attach_t *) = NULL;


	// Don't draw hidden objects
	if (obj->hidden == 1) return;


	// Don't draw the object hidden by reference to soem string
	// eg: Don't draw the "Target" text in the big HP bar if we have something targeted
	if (obj->hide_if_not_null && obj->hide_if_not_null[0]) return;

	switch (obj->type) {
		case QW_OBJ_PERCBAR:		f = qw_Obj_PercBar_Draw;		break;
		case QW_OBJ_TEXT:			f = qw_Obj_Text_Draw;			break;
		case QW_OBJ_INVSLOT:		f = qw_Obj_InvSlot_Draw;		break;
		case QW_OBJ_SPELLBOOK_SLOT:	f = qw_Obj_SpellBookSlot_Draw;	break;
		case QW_OBJ_TOOLTIP:		f = qw_Obj_ToolTip_Draw;		break;
		case QW_OBJ_ITEMINSPECTOR:	f = qw_Obj_ItemInspector_Draw;	break;
		case QW_OBJ_AMOUNTPICKER:	f = qw_Obj_AmountPicker_Draw;	break;
		case QW_OBJ_AUTOEQUIP:		f = qw_Obj_AutoEquip_Draw;		break;
		case QW_OBJ_MONEY:			f = qw_Obj_Money_Draw;			break;
		case QW_OBJ_BUTTON:			f = qw_Obj_Button_Draw;			break;
		case QW_OBJ_ITEMGFX:		f = qw_Obj_ItemGFX_Draw;		break;
		case QW_OBJ_TIMEBAR:		f = qw_Obj_TimeBar_Draw;		break;
		case QW_OBJ_SPELL_GEM:		f = qw_Obj_SpellGem_Draw;		break;
		case QW_OBJ_SPELL_ICON:		f = qw_Obj_SpellIcon_Draw;		break;
		default: break;
	}
	if (f) {
		f(win, att);
	}
}
int qw_ObjAttach(char *wid, int oid, int x, int y, int x_centered, int y_centered) {
	// This will attach an object to a particular window.
	// An object can be attached to 0, 1 or more windows.

	// An object that isn't attached to any windows should have their handle saved
	// in the qws->objhandles[] array to avoid leakage.

	// Objects whose handles are not saved in qws->objhandles[] will be deleted 
	// upon deletion of any window they are attached to.

	// Keeping track of (potentially) unattached objects or objects that may be attached
	// to several windows is the whole purpose of qws->objhandles[]


	for (int i = 0;  i < QW_MAX_WINDOWS;  i++) {

		if (!qws->win[i].wid[0]) {
			// No registered window found matching the given name
			xq_clog(COLOR_RED, "qw_ObjAttach: Window %s not found for attaching object %i", wid, oid);
			return -2;
		}


		if (xq_seq(qws->win[i].wid, wid)) {
			// We found the requested window

			for (int j = 0;  j < QW_OBJ_PER_WIN;  j++) {
				qw_obj_attach_t *att = &qws->win[i].obj[j];
				if (att->num == 0) {
					// We found a vacant attach slot.
					// Figure out max zindex for that window so we can attach on top of that
					int topzi = 0;
					for (int k = 0;  k < QW_OBJ_PER_WIN;  k++) {
						qw_obj_attach_t *tmpatt = &qws->win[i].obj[k];
						if (tmpatt->num > 0) {
							if (tmpatt->zindex > topzi) {
								topzi = tmpatt->zindex;
							}
						}
					}


					// Go ahead and attach our object
					att->num = oid;
					att->x = x;
					att->y = y;
					att->x_centered = x_centered;
					att->y_centered = y_centered;
					att->zindex = topzi + 1;
					qws->obj[oid].usecount++;


					// Returning the used attachment's index for the window
					return j;
				}
			}


			// We are out of available attachment slots
			xq_clog(COLOR_RED, "qw_ObjAttach: Window %s dropped object %i - too many attached", wid, oid);
			return -1;
		}
	}
	return -3; // Unreachable
}
int qw_ObjCreate(qw_obj_t *tpl) {
	// Create a new object by using the data in *tpl
	// and return the new object's index in qws->obj[], or -1 if we failed.

	for (int i = 1; i < QW_MAX_OBJ - 1;  i++) {
		qw_obj_t *o = &qws->obj[i];
		if (o->type == QW_OBJ_NONE) {
			memcpy(o, tpl, sizeof(*o));
			o->magics = o->magice = qw_obj_t_MAGIC;
			return i;
		}
	}
	xq_clog(COLOR_RED, "qw_ObjCreate: Out of object slots - requested objtype was %i", tpl->type);
	return -1; // Out of obj slots
}
void qw_ObjInit(qw_obj_t *obj, int class) {
	// Set some default values for a new object

	memset(obj, 0, sizeof(*obj));
	obj->type = class;

	switch (class) {
		case QW_OBJ_PERCBAR:
			obj->bordercolor = 		QW_OBJ_PERCBAR_BRDCOL;
			obj->barwidth = 		QW_OBJ_PERCBAR_WIDTH;
			obj->barheight = 		QW_OBJ_PERCBAR_HEIGHT;
			obj->filledcolor = 		QW_OBJ_PERCBAR_FILLEDCOL;
			obj->emptycolor = 		QW_OBJ_PERCBAR_EMPTYCOL;
			obj->notches = 			QW_OBJ_PERCBAR_NOTCHES;
			obj->smallnotchcolor = 	QW_OBJ_PERCBAR_SMALLNOTCHCOL;
			obj->bignotchcolor =	QW_OBJ_PERCBAR_BIGNOTCHCOL;
			obj->fastbar = 			QW_OBJ_PERCBAR_FASTBAR;
			obj->fastbarcolor = 	QW_OBJ_PERCBAR_FASTBARCOL;
			obj->disabledcolor = 	QW_OBJ_PERCBAR_DISABLEDCOL;
			obj->clickthrough = 	QW_OBJ_PERCBAR_CTR;
			break;
		case QW_OBJ_TEXT:
			obj->capitalize = 		QW_OBJ_TEXT_CAPITALIZE;
			obj->clickthrough = 	QW_OBJ_TEXT_CTR;
			break;
		case QW_OBJ_INVSLOT:
        	obj->onmousedown = 		qw_Obj_InvSlot_Click;
			obj->filledcolor = 		QW_OBJ_INVSLOT_BGCOL;
			obj->hoverbgcolor = 	QW_OBJ_INVSLOT_BGCOL_HOVER;
			break;
		case QW_OBJ_TOOLTIP:
			obj->bordercolor = 		QW_OBJ_TOOLTIP_BRDCOL;
			obj->filledcolor = 		QW_OBJ_TOOLTIP_BGCOL;
			break;
		case QW_OBJ_ITEMINSPECTOR:
			obj->clickthrough =		QW_OBJ_ITEMINSPECTOR_CTR;
			break;
		case QW_OBJ_AMOUNTPICKER:
			break;
		case QW_OBJ_AUTOEQUIP:
        	obj->onmousedown = 		qw_Obj_AutoEquip_Click;
			obj->bordercolor = 		QW_OBJ_AUTOEQUIP_BRDCOL;
			obj->filledcolor = 		QW_OBJ_AUTOEQUIP_BGCOL;
			obj->w = 				QW_OBJ_AUTOEQUIP_WIDTH;
			obj->h = 				QW_OBJ_AUTOEQUIP_HEIGHT;
			break;
		case QW_OBJ_MONEY:
			obj->onmousedown =		qw_Obj_Money_Click;
			break;
		case QW_OBJ_BUTTON:
			obj->onmousedown = 		qw_Obj_Button_Click;
			obj->filledcolor = 		QW_OBJ_BUTTON_BGCOL;
			obj->bordercolor = 		QW_OBJ_BUTTON_BRDCOL;
			obj->padding_x = 		QW_OBJ_BUTTON_PAD_X;
			obj->padding_y = 		QW_OBJ_BUTTON_PAD_Y;
			obj->capitalize = 		QW_OBJ_BUTTON_CAPITALIZE;
			obj->borderwidth =		QW_OBJ_BUTTON_BRDWIDTH;
			break;
		case QW_OBJ_ITEMGFX:
			break;
		case QW_OBJ_TIMEBAR:
			break;
		case QW_OBJ_SPELL_GEM:
        	obj->onmousedown = 		qw_Obj_SpellGem_Click;
			obj->borderwidth = 		QW_OBJ_SPELL_GEM_BRDWIDTH;
			obj->w =				QW_OBJ_SPELL_GEM_WIDTH + obj->borderwidth * 2;
			obj->h =				QW_OBJ_SPELL_GEM_HEIGHT + obj->borderwidth * 2;
			obj->filledcolor = 		QW_OBJ_SPELL_GEM_BGCOL;
			break;
		case QW_OBJ_SPELL_ICON:
        	obj->onmousedown = 		qw_Obj_SpellIcon_Click;
			break;
		case QW_OBJ_SPELLBOOK_SLOT:
        	obj->onmousedown = 		qw_Obj_SpellBookSlot_Click;
			break;

	}
	obj->creation_ts = cg.time;
}

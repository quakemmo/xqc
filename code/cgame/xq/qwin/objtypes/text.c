#include "../../../cg_local.h"

void qw_Obj_Text_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	char t[QW_OBJ_TEXT_MAX_TEXT_LEN+1] = {0};
	char t2[QW_OBJ_TEXT_MAX_TEXT_LEN+1] = {0};
	memset(t, 0, sizeof(t));
	memset(t2, 0, sizeof(t2));
	qw_obj_t *obj = &qws->obj[att->num];


	// Decide on the text size
	int width, height;
	switch (obj->charsize) {
		case 1:
			width = SMALLCHAR_WIDTH;
			height = SMALLCHAR_HEIGHT;
			break;

		case 2:
			width = BIGCHAR_WIDTH;
			height = BIGCHAR_HEIGHT;
			break;

		case 3:
			width = GIANTCHAR_WIDTH;
			height = GIANTCHAR_HEIGHT;
			break;

		default:
			width = TINYCHAR_WIDTH;
			height = TINYCHAR_HEIGHT;
			break;
	}


	// Figure out what text we're exactly supposed to display
	if (obj->text != NULL) {
		Q_strncpyz(t, obj->text, sizeof(t));
	} else if (obj->text_static[0]) {
		Q_strncpyz(t, obj->text_static, sizeof(t));
	} else if (obj->text_itemname_invslot) {
		int64_t item;
 		item = xq_InvSlotItem(obj->text_itemname_invslot, 0, 0);
		if (item) {
			xq_item_t *iinf = xq_ItemInfo(item);
			if (iinf) {
				snprintf(t, QW_OBJ_TEXT_MAX_TEXT_LEN, "%s", iinf->name);
			}
		}
	} else {
		return;
	}
	if (!t[0]) return;


	// Capitalize first letter if needed
	if (obj->capitalize) {
		t[0] = toupper(t[0]);
	}


	// Set object dimensions dynamically according to the text size
	obj->h = height;
	obj->w = CG_DrawStrlen(t) * width;

	int x = att->x;
	int y = att->y;

	if (att->x_centered == 0) {
		if (x < 0) x = (win->w - obj->w) - QW_WIN_BORDER_WIDTH + x;
	} else {
		x = (win->w / 2) - obj->w / 2;
		if (x < 0) x = 0;
	}
	if (att->y_centered == 0) {
		if (y < 0) y = (win->h - TINYCHAR_HEIGHT) - QW_WIN_BORDER_WIDTH + y;
	} else {
		y = (win->h / 2) - TINYCHAR_HEIGHT / 2;
	}

	qw_AttachWithBorder(win, att, &x, &y);


	// Special case for the group leader's name in the group window
	if (obj->group_leader) {
		Q_strncpyz(t2, t, sizeof(t2));
		snprintf(t, QW_OBJ_TEXT_MAX_TEXT_LEN, "%s (L)", t2);
	}


	// Decide on the color and alpha
	// if obj->coloralpha is 0, we simply draw full white with full visible alpha
	// otherwise the alpha value will apply and the color is taken from obj->colorvec
	vec4_t col;
	if (obj->coloralpha == 0) {
		col[0] = col[1] = col[2] = col[3] = 1;
	} else {
		Vector4Copy(xq_Color2Vec(obj->colorvec), col);
		col[3] = obj->coloralpha;
	}


	// Draw the actual text
	xq_DrawStringExt(win, x, y, t, col, qtrue, qfalse, width, height, 0 );
}

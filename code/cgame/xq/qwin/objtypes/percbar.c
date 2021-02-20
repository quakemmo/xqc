#include "../../../cg_local.h"

void qw_Obj_PercBar_Draw(qw_window_t *win, qw_obj_attach_t *att) {
	int disabled = 0;
	float perc;
	qw_obj_t *obj = &qws->obj[att->num];

	// we should probably just do nothing until we get some data to work with
	if (!cg.snap) return;


	// special value for not drawing anything
	if (*obj->perc == 200) return;


	if (*obj->perc == 201) {
		// special value for drawing a "disabled" bar
		// eg: group member's HP bar when they are in another zone
		disabled = 1;
		perc = 100.0f;
	} else {
		// All other drawable values
		perc = (float)*obj->perc;
		if (perc < 0) perc = 0;
		if (obj->multiplier > 0) {
			perc /= obj->multiplier;
		}
	}

	int x = att->x;
	int y = att->y;

	obj->w = obj->barwidth + obj->borderwidth*2;
	obj->h = obj->barheight + obj->borderwidth*2;

	if (att->x_centered == 0) {
		if (x < 0) x = (win->w - obj->w) - QW_WIN_BORDER_WIDTH;
	} else {
		x = (win->w / 2) - obj->w / 2;
	}
	if (att->y_centered == 0) {
		if (y < 0) y = (win->h - obj->h) - QW_WIN_BORDER_WIDTH;
	} else {
		y = (win->h / 2) - obj->h / 2;
	}

	att->actual_x = x;
	att->actual_y = y;


	// save typing
	float barwidth_1p = obj->barwidth / 100.0;
	int brdw = obj->borderwidth;
	int startx = win->x + QW_WIN_BORDER_WIDTH + x;
	int starty = win->y + QW_WIN_BORDER_WIDTH + y;


	// Bar border
	if (brdw > 0) {
		xq_DrawRect(
			startx,
			starty,
			obj->barwidth,
			obj->barheight,
			brdw,
			xq_Color2Vec(obj->bordercolor)
		);
	}



	// Filled bar part
	if (perc > 0) {
		int fill_x = startx + brdw;
		int fill_y = starty + brdw;
		int fill_w = barwidth_1p * perc - brdw*2;
		int fill_h = obj->barheight - brdw*2;
		vec4_t fill_col;
		Vector4Copy(xq_Color2Vec(obj->filledcolor), fill_col);

		if (obj->coloralpha == 0) {
			fill_col[3] = 1;
		} else {
			fill_col[3] = obj->coloralpha;
		}

		if (!obj->gradientcolor || disabled) {
			xq_FillRect(fill_x, fill_y, fill_w, fill_h,
				xq_Color2Vec(disabled ? obj->disabledcolor : obj->filledcolor));
		} else {
			vec4_t grad_col;
			Vector4Copy(xq_Color2Vec(obj->gradientcolor), grad_col);
			xq_FillRect_Gradient(fill_x, fill_y, fill_w, fill_h,
				fill_col, grad_col, obj->gradientmirror);
		}
	}


	// Empty bar part
	if (perc < 100) {
		xq_FillRect(
			startx + brdw + barwidth_1p * perc,
			starty + brdw,
			obj->barwidth - (barwidth_1p * perc) - brdw*2,
			obj->barheight - brdw*2,
			xq_Color2Vec(obj->emptycolor)
		);
	}


	// Optional notches every 20%
	if (obj->notches == 1) {
		int notchx, notchxx;
		for (
			notchx = (barwidth_1p * 20.0f);
			notchx <= (barwidth_1p * 100.0f);
			notchx += (barwidth_1p * 20.0f)
		) {

			// tiny notches every 5%
			for (
				notchxx = notchx - (barwidth_1p * 16.0f);
				notchxx < notchx;
				notchxx += (barwidth_1p * 4.0f)
			) {
				xq_FillRect(
					startx + notchxx,
					starty + obj->barheight - 3.0f,
					1.0f,
					1.0f,
					xq_Color2Vec(obj->smallnotchcolor)
				);
			}


			// Big 20% notch
			if (notchx != (obj->barwidth)) {
				xq_FillRect(
					startx + notchx,
					starty + obj->barheight - 3.0f,
					1.0f,
					2.0f,
					xq_Color2Vec(obj->bignotchcolor)
				);
			}
		}
	}


	// Optional "fast bar", eg: thin blue exp bar.
	if (obj->fastbar > 0) {
		float remainder = fmodf(perc, (100.0f / obj->fastbar));
		if (remainder > 0.0f) {

			int fastbar_y = starty + brdw + obj->barheight/2 - 1;
			fastbar_y -= (obj->fastbarheight / 2);
			float fastbar_filled_x = barwidth_1p * remainder * obj->fastbar - brdw*2;

			xq_FillRect(
				startx + brdw,
				fastbar_y,
				fastbar_filled_x,
				obj->fastbarheight ? obj->fastbarheight : 1,
				xq_Color2Vec(obj->fastbarcolor)
			);

		}
	}
}

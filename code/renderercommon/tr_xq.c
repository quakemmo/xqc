#include "tr_common.h"

void xq_print_surface(char *name, SDL_Surface *s, char *text) {
		char tmp[1000];
		ri.Printf(PRINT_DEVELOPER,	"%s for %s: "
									"w %i, "
									"h %i, "
									"pitch %i, "
									"format.BitsPerPixel %i, "
									"format.BytesPerPixel %i, "
									"format.Rmask %i, "
									"format.Gmask %i, "
									"format.Bmask %i, "
									"format.Amask %i, "
									"locked %%i, refcount %%i, offset %%i, flags %%i\n",
			name,
			text,
			s->w,
			s->h,
			s->pitch,
			s->format->BitsPerPixel,
			s->format->BytesPerPixel,
			s->format->Rmask,
			s->format->Gmask,
			s->format->Bmask,
			s->format->Amask
		);

		snprintf(tmp, 300, "/tmp/%s_%s.bmp", name, text);
		if (SDL_SaveBMP(s, tmp) == 0) {
			ri.Printf(PRINT_DEVELOPER, "SDL_SaveBMP() success\n");
		} else {
			ri.Printf(PRINT_DEVELOPER, "SDL_SaveBMP() failed: %s\n", SDL_GetError());
		}

}
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
    int bpp = surface->format->BytesPerPixel;
    // Here p is the address to the pixel we want to set
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}
SDL_Surface *tr_XQ_FBMP(const char *c, int level) {
	// Makes an SDL_Surface out of text. This is terrible but does the job for now.

	static TTF_Font *pcfont, *npcfont, *current_font, *outfont, *lvl_frame_font;
	SDL_Color color, black = {0,0,0};
	SDL_Surface *text_surface, *tmp_surface, *bg_lvl_surface;
	static int init_done = 0;
	char *text;
	int npc = 1;

	int final_w = 2048 * 1;
	int final_h = 256 * 1;

	SDL_Rect dstrect_name, dstrect_lvl, dstrect_lvl_frame;
	if (c[0] == '*') {
		// Color for NPC
		color.r = 42;
		color.g = 143;
		color.b = 129;
		color.a = 255;
		text = ((char *)c) + 1;
	} else {
		// Color for PC
		color.r = 64;
		color.g = 82;
		color.b = 178;
		color.a = 255;
		text = (char *)c;
		npc = 0;
	}

	SDL_Color lvl_color[] = {
		{150, 10, 50},
		{150, 30, 50},
		{150, 50, 50},
		{150, 70, 50},
		{150, 90, 50},
		{150, 110, 50},
		{150, 130, 50},
		{150, 150, 50},
		{150, 170, 50},
	};
	int max_levels = sizeof(lvl_color) / sizeof(lvl_color[0]);
		
 

	if (init_done == 0) {
		tr_XQ_Init(); // This isn't supposed to be here but it's here for now.

		if (TTF_Init() == -1) {
			ri.Printf(PRINT_DEVELOPER, "TTF_Init Failed\n");
			return NULL;
		} else {
			//ri.Printf(PRINT_DEVELOPER, "TTF_Init OK\n");
		}

		lvl_frame_font = TTF_OpenFont(BASEGAME "/fonts/levels.ttf", 110);
		if (!lvl_frame_font) {
			ri.Printf(PRINT_DEVELOPER, "TTF_OpenFont PC Failed: %s\n", TTF_GetError());
			return NULL;
		}

		pcfont = TTF_OpenFont(BASEGAME "/fonts/freesans.ttf", 80);
		if (!pcfont) {
			ri.Printf(PRINT_DEVELOPER, "TTF_OpenFont PC Failed: %s\n", TTF_GetError());
			return NULL;
		}

		npcfont = TTF_OpenFont(BASEGAME "/fonts/freesans.ttf", 80);
		if (!npcfont) {
			ri.Printf(PRINT_DEVELOPER, "TTF_OpenFont NPC Failed\n");
			return NULL;
		}

		outfont = TTF_OpenFont(BASEGAME "/fonts/freesans.ttf", 80);
		if (!outfont) {
			ri.Printf(PRINT_DEVELOPER, "TTF_OpenFont Outline Failed\n");
			return NULL;
		}
		TTF_SetFontStyle(pcfont, TTF_STYLE_BOLD);
		TTF_SetFontStyle(npcfont, TTF_STYLE_BOLD);
		TTF_SetFontStyle(outfont, TTF_STYLE_BOLD);
		TTF_SetFontOutline(outfont, 2);

		init_done = 1;
	}

	current_font = pcfont;
	if (npc == 1) {
		current_font = npcfont;
	}


	SDL_Surface *lvl_surface = NULL, *lvl_frame_surface = NULL;
	if (level > 0 && level <= max_levels) {
		char lvl_txt[20] = {0};
		snprintf(lvl_txt, 19, "%i", level);
		char lvl_frame_txt[20] = {0};
		lvl_frame_txt[0] = 'a' + level - 1;
		lvl_frame_txt[1] = 0;
		if (!(lvl_surface = TTF_RenderText_Solid(current_font, lvl_txt, lvl_color[level-1]))) {
			ri.Printf(PRINT_DEVELOPER, "TTF_RenderText_Solid Failed for level: %s\n", TTF_GetError());
			return NULL;
		}
		if (!(lvl_frame_surface = TTF_RenderText_Solid(lvl_frame_font, lvl_frame_txt, lvl_color[level-1]))) {
			ri.Printf(PRINT_DEVELOPER, "TTF_RenderText_Solid Failed for level: %s\n", TTF_GetError());
			return NULL;
		}
		if (!(bg_lvl_surface = TTF_RenderText_Solid(outfont, lvl_txt, black))) {
			ri.Printf(PRINT_DEVELOPER, "TTF_RenderText_Solid Failed for level: %s\n", TTF_GetError());
			return NULL;
		}
	} else {
		ri.Printf(PRINT_DEVELOPER, "Level is %i\n", level);
	}



	if (!(text_surface=TTF_RenderText_Solid(current_font, text, color))) {
		ri.Printf(PRINT_DEVELOPER, "TTF_RenderText_Solid Failed: %s\n", TTF_GetError());
		return NULL;
	} else {
		// center rendered text on a ^2 sized surface before returning
        tmp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, final_w, final_h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		if (tmp_surface == NULL) {
			ri.Printf(PRINT_DEVELOPER, "tmp_surface creation failed: %s\n", SDL_GetError());
			SDL_FreeSurface(text_surface);
			return NULL;
		}


		dstrect_name.x = (final_w / 2) - (text_surface->w / 2); // Center rendered text on the destination surface
		dstrect_name.y = (final_h / 2) - (text_surface->h / 2); // Vertically, too
		dstrect_name.h = -1;
		dstrect_name.w = -1;
		if (lvl_surface) {
			dstrect_name.x += lvl_frame_surface->w / 2;

			dstrect_lvl.x = dstrect_name.x - lvl_surface->w * 2;
			dstrect_lvl.y = dstrect_name.y;
			dstrect_lvl.h = dstrect_name.h;
			dstrect_lvl.w = dstrect_name.w;

			dstrect_lvl_frame.x = dstrect_name.x - (lvl_frame_surface->w + 25);
			dstrect_lvl_frame.y = dstrect_name.y - 12;
			dstrect_lvl_frame.h = dstrect_name.h;
			dstrect_lvl_frame.w = dstrect_name.w;
		}

		// Black outline of the text so it's more visible on a bright background
		SDL_Surface *bg_surface = TTF_RenderText_Blended(outfont, text, black);

	
		// See the whole surface as bright red for debugging purposes
		#if 0
		SDL_Rect fillrect;
		fillrect.x = 0;
		fillrect.y = 0;
		fillrect.w = final_w;
		fillrect.h = final_h;
		SDL_FillRect(tmp_surface, &fillrect, SDL_MapRGB(tmp_surface->format, 255, 0, 0));
		#endif

		if (SDL_BlitSurface(bg_surface, NULL, tmp_surface, &dstrect_name)) {
			ri.Printf(PRINT_DEVELOPER, "SDL_BlitSurface failed (outline): %s\n", SDL_GetError());
			SDL_FreeSurface(text_surface);
			SDL_FreeSurface(tmp_surface);
			SDL_FreeSurface(bg_surface);
			if (lvl_surface) SDL_FreeSurface(lvl_surface);
			if (lvl_frame_surface) SDL_FreeSurface(lvl_frame_surface);
			if (bg_lvl_surface) SDL_FreeSurface(bg_lvl_surface);
			return NULL;
		}

		if (SDL_BlitSurface(text_surface, NULL, tmp_surface, &dstrect_name)) {
			ri.Printf(PRINT_DEVELOPER, "SDL_BlitSurface failed: %s\n", SDL_GetError());
			SDL_FreeSurface(text_surface);
			SDL_FreeSurface(tmp_surface);
			SDL_FreeSurface(bg_surface);
			if (lvl_surface) SDL_FreeSurface(lvl_surface);
			if (lvl_frame_surface) SDL_FreeSurface(lvl_frame_surface);
			if (bg_lvl_surface) SDL_FreeSurface(bg_lvl_surface);
			return NULL;
		}

		if (lvl_frame_surface) {
			if (SDL_BlitSurface(lvl_frame_surface, NULL, tmp_surface, &dstrect_lvl_frame)) {
				ri.Printf(PRINT_DEVELOPER, "SDL_BlitSurface failed: %s\n", SDL_GetError());
				SDL_FreeSurface(text_surface);
				SDL_FreeSurface(tmp_surface);
				SDL_FreeSurface(bg_surface);
				if (lvl_surface) SDL_FreeSurface(lvl_surface);
				if (lvl_frame_surface) SDL_FreeSurface(lvl_frame_surface);
				if (bg_lvl_surface) SDL_FreeSurface(bg_lvl_surface);
				return NULL;
			}
		}

		if (lvl_surface) {
			if (SDL_BlitSurface(bg_lvl_surface, NULL, tmp_surface, &dstrect_lvl)) {
				ri.Printf(PRINT_DEVELOPER, "SDL_BlitSurface failed (outline): %s\n", SDL_GetError());
				SDL_FreeSurface(text_surface);
				SDL_FreeSurface(tmp_surface);
				SDL_FreeSurface(bg_surface);
				if (lvl_surface) SDL_FreeSurface(lvl_surface);
				if (lvl_frame_surface) SDL_FreeSurface(lvl_frame_surface);
				if (bg_lvl_surface) SDL_FreeSurface(bg_lvl_surface);
				return NULL;
			}
			if (SDL_BlitSurface(lvl_surface, NULL, tmp_surface, &dstrect_lvl)) {
				ri.Printf(PRINT_DEVELOPER, "SDL_BlitSurface failed: %s\n", SDL_GetError());
				SDL_FreeSurface(text_surface);
				SDL_FreeSurface(tmp_surface);
				SDL_FreeSurface(bg_surface);
				if (lvl_surface) SDL_FreeSurface(lvl_surface);
				if (lvl_frame_surface) SDL_FreeSurface(lvl_frame_surface);
				if (bg_lvl_surface) SDL_FreeSurface(bg_lvl_surface);
				return NULL;
			}
		}

		if (r_xqdebugTShader->integer) {
			xq_print_surface("text_surface", text_surface, text);
			xq_print_surface("tmp_surface", tmp_surface, text);
		}
		SDL_FreeSurface(text_surface);
		return tmp_surface;
	}
}
void tr_XQ_Init() {
}

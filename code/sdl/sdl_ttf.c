#include <stdlib.h>
#include <stdio.h>
#include "../client/client.h"

#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#   include <SDL_ttf.h>
#else
#	include <SDL.h>
#   include <SDL2/SDL_ttf.h>
#endif


void *CL_xq_txt2sdl(const char *c, int level) {
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

		if (TTF_Init() == -1) {
			Com_Printf("TTF_Init Failed\n");
			return NULL;
		} else {
			//Com_Printf("TTF_Init OK\n");
		}

		lvl_frame_font = TTF_OpenFont(BASEGAME "/fonts/levels.ttf", 110);
		if (!lvl_frame_font) {
			Com_Printf("TTF_OpenFont PC Failed: %s\n", TTF_GetError());
			return NULL;
		}

		pcfont = TTF_OpenFont(BASEGAME "/fonts/freesans.ttf", 80);
		if (!pcfont) {
			Com_Printf("TTF_OpenFont PC Failed: %s\n", TTF_GetError());
			return NULL;
		}

		npcfont = TTF_OpenFont(BASEGAME "/fonts/freesans.ttf", 80);
		if (!npcfont) {
			Com_Printf("TTF_OpenFont NPC Failed\n");
			return NULL;
		}

		outfont = TTF_OpenFont(BASEGAME "/fonts/freesans.ttf", 80);
		if (!outfont) {
			Com_Printf("TTF_OpenFont Outline Failed\n");
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
			Com_Printf("TTF_RenderText_Solid Failed for level: %s\n", TTF_GetError());
			return NULL;
		}
		if (!(lvl_frame_surface = TTF_RenderText_Solid(lvl_frame_font, lvl_frame_txt, lvl_color[level-1]))) {
			Com_Printf("TTF_RenderText_Solid Failed for level: %s\n", TTF_GetError());
			return NULL;
		}
		if (!(bg_lvl_surface = TTF_RenderText_Solid(outfont, lvl_txt, black))) {
			Com_Printf("TTF_RenderText_Solid Failed for level: %s\n", TTF_GetError());
			return NULL;
		}
	} else {
		if (level > max_levels) {
			Com_Printf("CL_xq_txt2sdl: Level is %i for %s\n", level, c);
		}
	}



	if (!(text_surface=TTF_RenderText_Solid(current_font, text, color))) {
		Com_Printf("TTF_RenderText_Solid Failed: %s\n", TTF_GetError());
		return NULL;
	} else {
		// center rendered text on a ^2 sized surface before returning
        tmp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, final_w, final_h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		if (tmp_surface == NULL) {
			Com_Printf("tmp_surface creation failed: %s\n", SDL_GetError());
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
			Com_Printf("SDL_BlitSurface failed (outline): %s\n", SDL_GetError());
			SDL_FreeSurface(text_surface);
			SDL_FreeSurface(tmp_surface);
			SDL_FreeSurface(bg_surface);
			if (lvl_surface) SDL_FreeSurface(lvl_surface);
			if (lvl_frame_surface) SDL_FreeSurface(lvl_frame_surface);
			if (bg_lvl_surface) SDL_FreeSurface(bg_lvl_surface);
			return NULL;
		}

		if (SDL_BlitSurface(text_surface, NULL, tmp_surface, &dstrect_name)) {
			Com_Printf("SDL_BlitSurface failed: %s\n", SDL_GetError());
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
				Com_Printf("SDL_BlitSurface failed: %s\n", SDL_GetError());
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
				Com_Printf("SDL_BlitSurface failed (outline): %s\n", SDL_GetError());
				SDL_FreeSurface(text_surface);
				SDL_FreeSurface(tmp_surface);
				SDL_FreeSurface(bg_surface);
				if (lvl_surface) SDL_FreeSurface(lvl_surface);
				if (lvl_frame_surface) SDL_FreeSurface(lvl_frame_surface);
				if (bg_lvl_surface) SDL_FreeSurface(bg_lvl_surface);
				return NULL;
			}
			if (SDL_BlitSurface(lvl_surface, NULL, tmp_surface, &dstrect_lvl)) {
				Com_Printf("SDL_BlitSurface failed: %s\n", SDL_GetError());
				SDL_FreeSurface(text_surface);
				SDL_FreeSurface(tmp_surface);
				SDL_FreeSurface(bg_surface);
				if (lvl_surface) SDL_FreeSurface(lvl_surface);
				if (lvl_frame_surface) SDL_FreeSurface(lvl_frame_surface);
				if (bg_lvl_surface) SDL_FreeSurface(bg_lvl_surface);
				return NULL;
			}
		}

		//xq_print_surface("text_surface", text_surface, text);
		//xq_print_surface("tmp_surface", tmp_surface, text);
		SDL_FreeSurface(text_surface);
		return (void *)tmp_surface;
	}
}

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

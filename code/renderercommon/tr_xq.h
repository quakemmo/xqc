#ifdef USE_LOCAL_HEADERS
#	include <SDL_ttf.h>
#else
#	include <SDL2/SDL_ttf.h>
#endif

SDL_Surface *tr_XQ_FBMP(const char *c, int level);
void tr_XQ_Init(void);
void tr_XQ_SetShaderFlags(qhandle_t);
void RE_XQ_ClearTShaderCache(void);


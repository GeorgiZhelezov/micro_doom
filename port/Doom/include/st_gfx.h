#ifndef ST_GFX_H
#define ST_GFX_H

extern const unsigned int gfx_stbar_len;
#if SCREENWIDTH == 320
extern const unsigned char *gfx_stbar;
#else
extern const unsigned char gfx_stbar[] __attribute ((aligned(4)));
#endif

#endif // ST_GFX_H

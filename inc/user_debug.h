#ifndef _USER_DEBUG_H
#define _USER_DEBUG_H

void doomk(const char *fmt, ...);

#define debugi(...)
// #define debugi(...) doomk(__VA_ARGS__)

#endif
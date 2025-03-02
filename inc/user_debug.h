#ifndef _USER_DEBUG_H
#define _USER_DEBUG_H

void printk(const char *fmt, ...);

#define debugi(...) 
// #define debugi(...) printk(__VA_ARGS__)

#endif
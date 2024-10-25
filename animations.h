#include <unistd.h>
#include <stdbool.h>
#include <notcurses/notcurses.h>
#include <math.h>

#define NUM_OF_ANIMATIONS 2
extern struct notcurses* nc;
extern struct ncplane* stdplane;
extern struct ncplane* barplane;
extern struct ncplane* barsplane;
extern int animation_on;
extern int paused;

void* animation_sine(void* );
void* animation_bars(void* );
void draw_sine_bar(double* amplitude,int row);

extern void* (*animation[NUM_OF_ANIMATIONS])(void*);


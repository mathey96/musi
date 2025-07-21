#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <notcurses/notcurses.h>
#include <math.h>

#define NUM_OF_ANIMATIONS 4

enum animations {
	ANIMATION_FFT = 0,
	ANIMATION_DOTS,
	ANIMATION_SINE,
	ANIMATION_RANDOM,
};

extern struct notcurses* nc;
extern struct ncplane* stdplane;
extern struct ncplane* barplane;
extern struct ncplane* barsplane;
extern int animation_on;
extern int paused;

static void* animation_fft(void* );
static void* animation_dots(void* );
static void* animation_sine(void* );
static void* animation_random(void* );
static void draw_sine_bar(double* amplitude,int row);

extern void* (*animation[NUM_OF_ANIMATIONS])(void*);


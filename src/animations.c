#include "animations.h"
#include <signal.h>

extern float amps[50];
volatile sig_atomic_t wakeup_flag = 0;
void signal_handler(int sig) {
    // Set the flag when the signal is received
    wakeup_flag = 1;
}


void* (*animation[])(void* )  = {animation_fft, animation_dots, animation_sine, animation_random};

void*
animation_sine(void* ){

	unsigned r = 250, b = 125, g = 200;
	ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);


		while(animation_on == 1){
			int row = 0;
			r = 250, b = 125, g = 200; //reset to default after every loop
			ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
			if(paused == false){// need to check this after every sleep because if we pause playback while this thread is sleeping, the barsplane will will not clear
			/* fprintf(stderr,"r: %d, g: %d, b: %d\n",r,g,b); */
				for(row = 0; row < 50;){
					double sin_amp = sin((double)row*7.2);
					draw_sine_bar(&sin_amp, row);
					r -=4;
					g -=6;
					b -=6;
					ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
					usleep(20000);
					row = row + 2;
					if(animation_on == 0){
						ncplane_erase(barsplane);
						pthread_exit(NULL);
				}
				/* pthread_mutex_unlock(&mutex_erase); */
			}
				ncplane_erase(barsplane);
		}
	}
	ncplane_erase(barsplane);
	pthread_exit(NULL);
}

void*
animation_random(void* ){
	while(animation_on){
		double randheight = 0;
		unsigned r = 250, b = 125, g = 200; //reset to default after every loop
		ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
		usleep(20000);
		ncplane_erase(barsplane);
		for(int i=0;i<50;){
			randheight = (double) rand() / RAND_MAX;
			r -=4;
			g -=6;
			b -=6;
			ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
			draw_sine_bar(&randheight,i);
		i  = i + 2;
		}
	}
	usleep(200);
	ncplane_erase(barsplane);
	pthread_exit(NULL);
}

void*
animation_dots(void* ){

	int i = 4;

	unsigned r = 250, b = 125, g = 200;
	ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
	signal(SIGUSR1, signal_handler);

	while(animation_on){

		while(paused == false){

			if(i == 4){
				r = 250, b = 125, g = 200; //reset to default after every loop
				ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
			}
			i--;
			if (!wakeup_flag) {
				usleep(200000);
			}
			if(paused == false){// need to check this after every sleep because if we pause playback while this thread is sleeping, the barsplane will will not clear
			ncplane_putwc_yx(barsplane, i + 1, 0, L'▄');
			r -=20; g -=35; b -=35;
			}
			if(i > 1){
				r -=10; g -=25; b -=25;
				ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);

			if (!wakeup_flag) {
				usleep(200000);
			}
			if(paused == false)
				ncplane_putwc_yx(barsplane, i + 1, 2, L'▄');
			}
			if(i > 2){
				r -=20; g -=35; b -=35;

			if (!wakeup_flag) {
				usleep(200000);
			}
				if(paused == false)
				ncplane_putwc_yx(barsplane, i + 1, 4, L'▄');
			}
			if(i == 0){
				i = 4;
				ncplane_erase(barsplane);
			}

			if(animation_on == 0){
				ncplane_erase(barsplane);
				wakeup_flag = 0;
				pthread_exit(NULL);

			}
		}
		wakeup_flag = 0;
		pthread_exit(NULL);
	}
}

void
draw_sine_bar(double* amplitude,int row){
	int height = 4;
	if (*amplitude < 0) {
		*amplitude = fabs(*amplitude);
	};

	if(*amplitude < 1){
		height = 0;
		}
	if(*amplitude < 0.75){
		height = 1;
		}

	if(*amplitude < 0.50){
		height = 2;
		}
	if(*amplitude < 0.25){
		height = 3;
		}

	// y = 4 is the base height
		for(int y= 4; y >= height;y--){
			ncplane_putwc_yx(barsplane, y, row, L'█');
	}
}

void
draw_fft_bar(float* amplitude,int row){
	int height = 4;
	if (*amplitude < 0) {
		*amplitude = fabs(*amplitude);
	};

	if(*amplitude < 1){
		height = 0;
		}
	if(*amplitude < 0.25){
		height = 1;
		}
	if(*amplitude < 0.03){
		height = 2;
		}
	if(*amplitude < 0.01){
		height = 3;
		}
	/* if(*amplitude < 0.01){ */
	/* 	height = 4; */
	/* 	} */
	// y = 4 is the base height
		for(int y= 4; y > height;y--){
			ncplane_putwc_yx(barsplane, y, row, L'█');
	}
}


void*
animation_fft(void* ){
	while(animation_on){
		double randheight = 0;
		unsigned r = 250, b = 125, g = 200; //reset to default after every loop
		ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
		usleep(20000);
		ncplane_erase(barsplane);
		for(int i=0;i<50;){
			randheight = (double) rand() / RAND_MAX;
			r -=4;
			g -=6;
			b -=6;
			ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
			draw_fft_bar(&amps[i],i);
		i  = i + 2;
		}
	}
	ncplane_erase(barsplane);
	pthread_exit(NULL);
}

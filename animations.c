#include "animations.h"

void* (*animation[])(void* )  = {animation_bars, animation_sine};

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
					r -=2;
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
	pthread_exit(NULL);
}

void*
animation_bars(void* ){

	int i = 4;

	unsigned r = 250, b = 125, g = 200;
	ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);

	while(animation_on){

		while(paused == false){

			if(i == 4){
				r = 250, b = 125, g = 200; //reset to default after every loop
				ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
			}
			i--;
			usleep(200000);
			if(paused == false){// need to check this after every sleep because if we pause playback while this thread is sleeping, the barsplane will will not clear
			/* fprintf(stderr,"r: %d, g: %d, b: %d\n",r,g,b); */
			ncplane_putwc_yx(barsplane, i + 1, 0, L'▄');
			r -=20; g -=35; b -=35;
			}
			if(i > 1){
				r -=10; g -=25; b -=25;
				ncplane_set_fg_rgb8_clipped(barsplane, r, g, b);
				usleep(200000);
			if(paused == false)
				ncplane_putwc_yx(barsplane, i + 1, 2, L'▄');
			}
			if(i > 2){
				r -=20; g -=35; b -=35;
				usleep(200000);
				if(paused == false)
				ncplane_putwc_yx(barsplane, i + 1, 4, L'▄');
				/* fprintf(stderr,"r: %d, g: %d, b: %d\n",r,g,b); */
			}
			if(i == 0){
				i = 4;
				/* pthread_mutex_lock(&mutex_erase); */
				ncplane_erase(barsplane);
				/* pthread_mutex_unlock(&mutex_erase); */
			}

			if(animation_on == 0){
				ncplane_erase(barsplane);
				pthread_exit(NULL);

			}
		}
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




#include <stdio.h>
#include <notcurses/notcurses.h>
#include <complex.h>
#include <assert.h>

#include <stdio.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "animations.h"

#define FIVE_SEC_IN_FRAME 44100 * 5

ma_uint64 total_length_in_sec = 0;
ma_uint64 total_length_in_min = 0;
ma_uint64 total_length_in_sec_curr_min = 0;
ma_uint64 totalFrames = 0;
ma_uint64 cur_time;
float current_bar_pos = 0;
int curr_ms = 0;
ma_uint64 curr_sec = 0;
ma_uint64 cursor;
int ystd = 0, xstd = 0;

typedef enum{
	PROGRESS_BAR = 0,
	HELP
} display_status;

int display_state = PROGRESS_BAR;

ma_engine engine;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

struct ncplane* stdplane;
struct ncplane* barplane;
struct ncplane* barsplane;
struct notcurses* nc;

bool thread_done = false;

int paused = 0;
ma_sound sound;

int animation_on = 1;
static pthread_t thread_id_input;
static pthread_t thread_animation;
int cur_animation_thread = 0;


void
display_bar(struct ncplane* n, ma_sound* sound){

	unsigned r = 250, b = 125, g = 200;
	cur_time = ma_sound_get_time_in_milliseconds(sound);

	curr_ms = cur_time % 1000;
	curr_sec = cur_time / 1000;

	current_bar_pos = ((float)curr_sec / (float)total_length_in_sec)*50; // there are 50 horizontal bar characters rendered at the end of the song

		for(int i=0;i < current_bar_pos; i++){
			ncplane_set_fg_rgb8_clipped(n, r, g, b);
			ncplane_putwc_yx(barplane, 0, i, L'▄');
			r -=1;
			g -=3;
			b -=3;
		}
		ncplane_printf_yx(barplane	, 1, 0, "%lld.%d" ,curr_sec,curr_ms);
		ncplane_printf_yx(barplane	, 1,47, "%lld.%lld" ,total_length_in_sec ,total_length_in_sec%1000);
}

static void*
handle_input(void* arg){
	ncinput ni;
    struct notcurses* nc = (struct notcurses*)arg;
	uint32_t id;

    if(pthread_create(&thread_animation, NULL, animation[0], nc) != 0){
		exit(-1);
	}

	while((id = notcurses_get_blocking(nc, &ni)) != (uint32_t)-1){
		if(id == 0){
			continue;
		}
		if(id == 'q'){
			animation_on = 0;
			if(pthread_join(thread_animation, NULL) != 0){
				fprintf(stderr, "unsuccessful exit from animation thread\n");
				exit(-1);
			}
			thread_done = true;
			pthread_exit(NULL);
			break;
		}
		if(id == ' '){
			if (paused == false){
				paused = true;
				ncplane_erase(barsplane);
				ma_sound_stop(&sound);
			}
			else {
				paused = false;
				ncplane_erase(barsplane);
				ncplane_erase(barplane);
				ma_sound_start(&sound);
			}
		}
		if(id == 'k'){
			ma_sound_get_cursor_in_pcm_frames(&sound,&cursor);
				ma_sound_seek_to_pcm_frame(&sound, cursor + FIVE_SEC_IN_FRAME);
		}
		if(id == 'j'){
			ma_sound_get_cursor_in_pcm_frames(&sound,&cursor);
			if( cursor > FIVE_SEC_IN_FRAME){
				ma_sound_seek_to_pcm_frame(&sound, cursor - FIVE_SEC_IN_FRAME);
			}
			else
				ma_sound_seek_to_pcm_frame(&sound, 0);
		}
		if(id == 'r'){
			ma_sound_seek_to_pcm_frame(&sound, 0);
		}
		if(id == 'a'){
			if(cur_animation_thread < NUM_OF_ANIMATIONS)
				cur_animation_thread ++;
			if(cur_animation_thread == NUM_OF_ANIMATIONS) cur_animation_thread = 0;
			animation_on = 0;
			if(pthread_join(thread_animation, NULL) != 0){
				notcurses_stop(nc);
			}
			animation_on = 1;
			if(pthread_create(&thread_animation, NULL, animation[cur_animation_thread], nc)){
				exit(-1);
			}
		}
		if(id == 'h'){
		/* 	if(display_state == HELP) */
		/* 		display_state = PROGRESS_BAR; */
		/* 	else if(display_state == PROGRESS_BAR) */
		/* 		display_state = HELP; */
		/* } */
			switch(display_state){
				case PROGRESS_BAR: display_state = HELP;
				case HELP: display_state = PROGRESS_BAR;
			}
		}
	}
	return NULL;
}

void display_help(struct ncplane* plane){
	ncplane_putstr_yx(plane, 1, 1, "k - skip 5 sec forward");
	ncplane_putstr_yx(plane, 2, 1, "j - skip 5 sec backward");
	ncplane_putstr_yx(plane, 3, 1, "r - restart audio");
	ncplane_putstr_yx(plane, 4, 1, "a - change animation style");
	ncplane_putstr_yx(plane, 5, 1, "SPC - pause");
	ncplane_putstr_yx(plane, 6, 1, "h - display help");
	ncplane_putstr_yx(plane, 7, 1, "q - exit application");
};

int resize_cb(struct ncplane* plane){
	notcurses_stddim_yx(nc, &ystd, &xstd);
	int x_center = xstd/3 ;
	int y_center = ystd/3 + 1;
	int y_center_barsplane = ystd/3 - 4;
	if( ystd > 15 && xstd > 70){
		ncplane_move_yx(barplane, y_center, x_center);
		ncplane_move_yx(barsplane, y_center_barsplane, x_center);
	}
	if(xstd < 70 ){
		ncplane_move_yx(barplane,  y_center, 0);
		ncplane_move_yx(barsplane, y_center_barsplane, 0);
	}
	if( ystd < 15 && xstd > 70){
		ncplane_move_yx(barplane,  0, x_center);
		ncplane_move_yx(barsplane, 0, x_center);
	}
	if( ystd < 15 && xstd < 70){
		ncplane_move_yx(plane, 0, 0);
		ncplane_move_yx(plane, 0, 0);
	}
	return 0;
}

int main(int argc, const char* argv[]) {
    struct notcurses_options opts = {0}; // man notcurses_init(3)
	if(argc < 2){
		fprintf(stderr,"provide an input file\n");
		exit(-1);
		}
    nc = notcurses_init(&opts, stdout);
    if(nc == NULL){
    	return EXIT_FAILURE;
    }
    stdplane = notcurses_stdplane(nc);

    int ystd = 0, xstd = 0;
    notcurses_stddim_yx(nc, &ystd, &xstd);
    struct ncplane_options nopts = {
    	.y = ystd/3 +1,
    	.x = xstd/3 +1,
    	.rows = 150,
    	.cols = 150,
    	.name = "plot",
    	.resizecb = resize_cb,
    	/* .flags = NCPLANE_OPTION_FIXED, */
    	.margin_b = 0,
    	.margin_r = 0,
    };

	struct ncplane_options nopts2 = {
		.y = ystd/3 -4,
		.x = xstd/3 +1,
		.rows = 5,
		.cols = 50,
		.name = "plot",
		.resizecb = resize_cb,
		/* .flags = NCPLANE_OPTION_FIXED, */
		.margin_b = 0,
		.margin_r = 0,
	};

    barplane = ncplane_create(stdplane, &nopts);
	barsplane = ncplane_create(stdplane, &nopts2);

    ma_device_config deviceConfig;
    ma_device device;
    ma_result result;
    ma_decoder decoder;

    // Initialize the Miniaudio device configuration
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 2; // Stereo
    deviceConfig.sampleRate        = 44100;
    deviceConfig.dataCallback      = data_callback;

    // Initialize the audio device
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
    	printf("Failed to initialize playback device.\n");
    	return -1;
    }

    // Start the audio device
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
    	printf("Failed to start playback device.\n");
    	ma_device_uninit(&device);
    	return -1;
    }

    ma_engine_config engineConfig;


    engineConfig = ma_engine_config_init();
    /* engineConfig.pResourceManager = &myCustomResourceManager;   // <-- Initialized as some earlier stage. */
    result = ma_engine_init(&engineConfig, &engine);

    /* ma_engine_play_sound(&engine, "darkorundek.mp3", NULL); */

    result = ma_decoder_init_file(argv[1], NULL, &decoder);
    if (result != MA_SUCCESS) {
		fprintf(stderr,"ybg\n");
        return false;   // An error occurred.
    }
	ma_decoder_get_length_in_pcm_frames(&decoder,&totalFrames);
	total_length_in_sec = (ma_uint64) totalFrames / 44100;
    result = ma_sound_init_from_file(&engine, argv[1], MA_SOUND_FLAG_DECODE, NULL, NULL, &sound);

    if (result != MA_SUCCESS) {
        return result;
    }
    if(pthread_create(&thread_id_input, NULL, &handle_input, nc)){
		exit(-1);
		return -1;
	}
    ma_sound_start(&sound);

	while(thread_done == false){
		ncplane_erase(barplane);
		if(display_state == PROGRESS_BAR){
			display_bar(barplane, &sound);
			}
		if(display_state == HELP){
			ncplane_erase(barplane);
			display_help(barplane);
		}
		notcurses_render(nc);
	}

	if(pthread_join(thread_id_input, NULL) == 0){
		notcurses_stop(nc);
		return 1;
	}

	ma_sound_uninit(&sound);
    // Stop and uninitialize the audio device
    ma_device_stop(&device);
    ma_device_uninit(&device);

    return 0;
}

#include <stdio.h>
#include <notcurses/notcurses.h>
#include <complex.h>
#include <assert.h>

#include <stdio.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

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
struct notcurses* nc;

bool thread_done = false;

int paused = 0;
ma_sound sound;

static void*
handle_input(void* arg){
	ncinput ni;
	struct notcurses* nc = (struct notcurses*)arg;
	uint32_t id;

	while((id = notcurses_get_blocking(nc, &ni)) != (uint32_t)-1){
		if(id == 0){
			continue;
		}
		if(id == 'q'){
			thread_done = true;
			pthread_exit(NULL);
			break;
		}
		if(id == ' '){
			if (paused == false){
				paused = true;
				ma_sound_stop(&sound);
			}
			else {
				paused = false;
				ma_sound_start(&sound);
			}
		}

		if(id == NCKEY_UP){
			fprintf(stderr,"CAOOO\n");
			ncplane_move_rel(barplane, -1, 0);
		}
		if(id == NCKEY_DOWN){
			ncplane_move_rel(barplane, 1, 0);
		}
		if(id == NCKEY_LEFT){
			ncplane_move_rel(barplane, 0, -1);
		}
		if(id == NCKEY_RIGHT){
			ncplane_move_rel(barplane, 0, 1);
		}
		}
	return NULL;
}

static pthread_t thread_id_input;


int main() {

 	struct notcurses_options opts = {0}; // man notcurses_init(3)
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
 		/* .resizecb = resize_cb, */
 		/* .flags = NCPLANE_OPTION_FIXED, */
 		.margin_b = 0,
 		.margin_r = 0,
 	};
 	barplane = ncplane_create(stdplane, &nopts);

    ma_device_config deviceConfig;
    ma_device device;
    ma_result result;

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

    ma_engine engine;
    ma_engine_config engineConfig;


    engineConfig = ma_engine_config_init();
    /* engineConfig.pResourceManager = &myCustomResourceManager;   // <-- Initialized as some earlier stage. */
    result = ma_engine_init(&engineConfig, &engine);

   /* ma_engine_play_sound(&engine, "darkorundek.mp3", NULL); */

    result = ma_sound_init_from_file(&engine, "darkorundek.mp3", 0, NULL, NULL, &sound);
	if (result != MA_SUCCESS) {
        return result;
    }
	if(pthread_create(&thread_id_input, NULL, &handle_input, nc)){
		exit(-1);
		return -1;
	}

    ma_sound_start(&sound);

	while(thread_done == false){ // main loop

	}

	if(pthread_join(thread_id_input, NULL) == 0){
		notcurses_stop(nc);
		return 1;
	}
    ma_sound_stop(&sound);

    // Stop and uninitialize the audio device
    ma_device_stop(&device);
    ma_device_uninit(&device);
	ma_sound_uninit(&sound);

    return 0;
}

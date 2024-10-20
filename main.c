#include <stdio.h>
#include <notcurses/notcurses.h>
#include <complex.h>
#include <assert.h>

#include <stdio.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

ma_uint64 total_length_in_sec = 0;
ma_uint64 total_length_in_min = 0;
ma_uint64 total_length_in_sec_curr_min = 0;
float current_bar_pos = 0;
int curr_ms = 0;
int curr_sec = 0;
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
struct notcurses* nc;

bool thread_done = false;

int paused = 0;
ma_sound sound;

void display_bar(const struct ncplane* n, int length, ma_sound* sound){
	const nccell c;
	nccell_load(barplane, &c, "h");

	if(nccell_wide_right_p(&c)){ // not really a character
		return;
	}

	float song_length = 0;
	ma_uint64 cur_time = ma_sound_get_time_in_milliseconds(sound);

	curr_ms = cur_time % 1000;
	curr_sec = cur_time / 1000;
	unsigned r = 250, b = 28, g = 255;

	current_bar_pos = ((float)curr_sec / (float)total_length_in_sec)*50; // there are 50 horizontal bar characters rendered at the end of the song



	for(int i=0;i < current_bar_pos; i++){
		ncplane_set_fg_rgb8_clipped(n, r, g, b);
		/* ncplane_putstr_yx(barplane,0,i, "hello"); */ // maybe put some time note here
		ncplane_putwc_yx(barplane, 0, i, L'▄');
		/* usleep(50000); */
		r -=5;
		/* g -=5; */
		b -=5;
	}
	ncplane_printf_yx(barplane, 1,0, "%d.%d",curr_sec,curr_ms, total_length_in_sec);
	ncplane_printf_yx(barplane, 1,47, "/%d", total_length_in_sec);
}

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
    		ma_sound_stop(&sound);
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
const uint64_t sec = 5;


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
    ma_decoder decoder;

    time_t start_time = time(NULL);

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

    ma_uint64 totalFrames = 0;
    result = ma_decoder_init_file("darkorundek.mp3", NULL, &decoder);
    if (result != MA_SUCCESS) {
		fprintf(stderr,"ybg\n");
        return false;   // An error occurred.
    }
	ma_decoder_get_length_in_pcm_frames(&decoder,&totalFrames);
	total_length_in_sec = (ma_uint64) totalFrames / 44100;
    result = ma_sound_init_from_file(&engine, "darkorundek.mp3", MA_SOUND_FLAG_DECODE, NULL, NULL, &sound);

    if (result != MA_SUCCESS) {
        return result;
    }
    if(pthread_create(&thread_id_input, NULL, &handle_input, nc)){
		exit(-1);
		return -1;
	}

    ma_sound_start(&sound);

	while(thread_done == false){ // main loop
		display_bar(barplane,50,&sound);
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

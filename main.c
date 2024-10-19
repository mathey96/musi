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
    ma_sound sound;

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

    ma_sound_start(&sound);

    // Stop and uninitialize the audio device
    ma_device_stop(&device);
    ma_device_uninit(&device);
	ma_sound_uninit(&sound);

    return 0;
}

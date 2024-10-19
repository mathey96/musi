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

    printf("Playback started. Press Enter to exit...\n");
    ma_engine_play_sound(&engine, "darkorundek.mp3", NULL);
    getchar();

    // Stop and uninitialize the audio device
    ma_device_stop(&device);
    ma_device_uninit(&device);

    return 0;
}

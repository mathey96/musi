#include <math.h>
#include <complex.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    int16_t L;
    int16_t R;
} sample_16b_2ch_t;

typedef struct {
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;    // Bytes per second
    uint16_t blockAlign;  // Bytes per sample, bitePerSample * numChannels / 8
    uint16_t bitsPerSample;
    uint32_t dataSize;
    ma_uint64 numSamples;
    sample_16b_2ch_t* sample;
} wav_t;   // struct and some code in this commit is taken from https://github.com/RoboBachelor/Music-Visualizer-Piano

void fft(float in[], size_t stride, float complex out[], size_t n);
void update_vars();

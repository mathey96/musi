#include <math.h>
#include <complex.h>
#include <stdio.h>
#include "fft.h"
#include "miniaudio.h"

#define N 1024

extern uint32_t sampleIndex;
extern float in[N];  // Initialize complex array
extern float complex out[N];
extern wav_t wav;
extern float amps[50]; // final vector of values to be rendered

float pi = 0;

// fft algorithm taken from tsoding's musializer https://github.com/tsoding/musializer
void fft(float in[], size_t stride, float complex out[], size_t n)
{
	//  pi = 4*atanf(1.0f);   // < -- maybe include this if you want different outptu at the ridges and
    assert(n > 0);

    if (n == 1) {
		out[0] = crealf(in[0]) + 0.0 * I;

        return;
    }

    fft(in, stride*2, out, n/2);
    fft(in + stride, stride*2,  out + n/2, n/2);

    for (size_t k = 0; k < n/2; ++k) {
        float t = (float)k/n;
        float complex v = cexp(-2*I*pi*t)*out[k + n/2];
        float complex e = out[k];
        out[k]       = e + v;
        out[k + n/2] = e - v;
    }
}

float amp(float complex z)
{
    float a = fabsf(crealf(z));
    float b = fabsf(cimagf(z));
    if (a < b) return b;
    return a;
}

void update_vars() {
	/* fprintf(stderr," \ninput values:\n"); */
	for (int32_t i = 0; i < 1024; ++i) {
		if (i + sampleIndex >= 0 && i + sampleIndex < wav.numSamples){
			in[i] = (float) wav.sample[i + sampleIndex].L/32768.f;
		}
		else{
			in[i] = 0;
		}
	}

	fft(in,1, out, 1024);

    float max_amp = 0.0f;

    for (size_t i = 0; i < N; ++i) {
        float a = amp(out[i]);
        if (max_amp < a) max_amp = a;

    float step = 1.06;
    size_t m = 0;
    for (float f = 20.0f; (size_t) f < N; f *= step) {
        m += 1;
    }

    m = 0;
    for (float f = 20.0f; (size_t) f < N; f += 20) {
        float f1 = f*step;
        float a = 0.0f;
        for (size_t q = (size_t) f; q < N && q < (size_t) f1; ++q) {
            a += amp(out[q]);
        }
        a /= (size_t) f1 - (size_t) f + 1;
        float t = a/max_amp;
		amps[m] = t;
        m += 1;
		}
	}
}

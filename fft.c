
#include <complex.h>
#include <stdio.h>
#include "fft.h"
#include "miniaudio.h"

#define N_FFT 1024
#define N 1024

extern uint32_t sampleIndex;
extern float values[N_FFT];  // Initialize complex array
extern float complex out[N_FFT];
extern wav_t wav;
extern float amps[50];

float pi;

// fft algorithm taken from tsoding's musializer https://github.com/tsoding/musializer
void fft(float in[], size_t stride, float complex out[], size_t n)
{
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
			values[i] = (float) wav.sample[i + sampleIndex].L/32768.f;
			/* fprintf(stderr," %d: %f | ",i, values[i]); */
		}
		else{
			values[i] = 0;
			/* fprintf(stderr,"else values[i]: %d\n", values[i]); */
		}
	}

    /* int w = 960; */
    /* int h = 540; */

	fft(values,1, out, 1024);
		/* fprintf(stderr,"\noutput values:\n " ); */
	/* for(int f=0;f < 255;f ++){ */
    /*     printf("%i: %.2f, %.2f | ", f, creal(out[f]), cimag(out[f])); */
	/* } */
		/* fprintf(stderr,"\n" ); */


    float max_amp = 0.0f;
	/* fprintf(stderr,"max_amp: \n"); */

    for (size_t i = 0; i < N; ++i) {
        float a = amp(out[i]);
        if (max_amp < a) max_amp = a;
	/* 	fprintf(stderr,"in[%zu]: %f | ",i, values[i]); */
	/* } */
	/* fprintf(stderr,"\n"); */
	  /* fprintf(stderr,"%f\n", max_amp); */



    float step = 1.06;
    size_t m = 0;
    for (float f = 20.0f; (size_t) f < N; f *= step) {
        m += 1;
    }
	/* fprintf(stderr, "\n"); */

    m = 0;
	/* fprintf(stderr,"heights:\n"); */
    /* for (float f = 20.0f; (size_t) f < N; f *= step) { */
    for (float f = 20.0f; (size_t) f < N; f += 20) {
        float f1 = f*step;
        float a = 0.0f;
        for (size_t q = (size_t) f; q < N && q < (size_t) f1; ++q) {
            a += amp(out[q]);
        }
        a /= (size_t) f1 - (size_t) f + 1;
        float t = a/max_amp;
		amps[m] = t;
		/* fprintf(stderr,"amps[%d]: %f\n", m, amps[m] ); */
        m += 1;
    }
	}
	/* fprintf(stderr,"\n"); */
}

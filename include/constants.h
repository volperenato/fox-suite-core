#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#define _TEMPLATE_SAMPLERATE 44100					// [Hz]
#define _TEMPLATE_DELAY_BUFFER_LENGTH 1000.0		// [milliseconds]
#define _SEED_FOR_RAND_GENERATION 4231
#define MAX_LPF_FREQUENCY 20000.0
#define MIN_LPF_FREQUENCY 20.0
#define MAX_HPF_FREQUENCY 17000.0
#define MIN_HPF_FREQUENCY 20.0
#define MAX_FREQUENCY 20000.0
#define MIN_FREQUENCY 10.0

// const parameters
const float MAX_LPF_FREQUENCY_LOG = log(MAX_LPF_FREQUENCY);
const float MIN_LPF_FREQUENCY_LOG = log(MIN_LPF_FREQUENCY);
const float MAX_HPF_FREQUENCY_LOG = log(MAX_HPF_FREQUENCY);
const float MIN_HPF_FREQUENCY_LOG = log(MIN_HPF_FREQUENCY);
const float MAX_FREQUENCY_LOG = log(MAX_FREQUENCY);
const float MIN_FREQUENCY_LOG = log(MIN_FREQUENCY);

// constants for parabolic sine
const float PARABOLIC_SINE_B = 4.0 / M_PI;
const float PARABOLIC_SINE_C = -4.0 / (M_PI * M_PI);
const float PARABOLIC_SINE_P = 0.225;


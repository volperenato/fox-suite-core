#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "LPFButterworth.h"
#include "constants.h"

LPFButterworth::LPFButterworth() {
	lpf_a0 = 0.0;
	lpf_a1 = 0.0;
	lpf_a2 = 0.0;
	lpf_b1 = 0.0;
	lpf_b2 = 0.0;
	lpf_cutoffFreq = MAX_LPF_FREQUENCY;
	lpf_sampleRate = _TEMPLATE_SAMPLERATE;
	lpf_xn_1 = 0.0;
	lpf_xn_2 = 0.0;
	lpf_yn_1 = 0.0;
	lpf_yn_2 = 0.0;
}

LPFButterworth::~LPFButterworth() {}

void LPFButterworth::init(int sampleRate)
{
	// set internal sample rate
	lpf_sampleRate = sampleRate;
}

void LPFButterworth::setSampleRate(int sampleRate)
{
	// allocate internal sample rate
	lpf_sampleRate = sampleRate;

	// update lpf gains
	updateGains();
}


void LPFButterworth::setCutoffFrequency(float cutoffFreq)
{
	// allocate cutoff frequency value
	lpf_cutoffFreq = cutoffFreq;

	// update lpf gains
	updateGains();
}

void LPFButterworth::updateGains()
{
	// define lpf fb and ff gains
	float C = 1.0 / tan(( M_PI * lpf_cutoffFreq) / (float)lpf_sampleRate );
	lpf_a0 = 1.0 / (1.0 + sqrt(2.0) * C + C * C);
	lpf_a1 = 2.0 * lpf_a0;
	lpf_a2 = lpf_a0;
	lpf_b1 = 2.0 * lpf_a0 * (1 - C * C);
	lpf_b2 = lpf_a0 * (1.0 - sqrt(2.0) * C + C * C);
}

void LPFButterworth::updateBuffers(float xn, float yn)
{
	lpf_xn_2 = lpf_xn_1;
	lpf_yn_2 = lpf_yn_1;
	lpf_xn_1 = xn;
	lpf_yn_1 = yn;
}

float LPFButterworth::processAudio(float xn)
{
	// compute filtered output
	float yn = lpf_a0 * xn + lpf_a1 * lpf_xn_1 + lpf_a2 * lpf_xn_2 - lpf_b1 * lpf_yn_1 - lpf_b2 * lpf_yn_2;

	// update buffers
	updateBuffers(xn, yn);

	return yn;
}

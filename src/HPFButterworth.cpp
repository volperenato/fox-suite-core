#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "HPFButterworth.h"

HPFButterworth::HPFButterworth() {
	hpf_a0 = 0.0;
	hpf_a1 = 0.0;
	hpf_a2 = 0.0;
	hpf_b1 = 0.0;
	hpf_b2 = 0.0;
	hpf_cutoffFreq = 0.0;
	hpf_xn_1 = 0.0;
	hpf_xn_2 = 0.0;
	hpf_yn_1 = 0.0;
	hpf_yn_2 = 0.0;
}

HPFButterworth::~HPFButterworth() {}

void HPFButterworth::init(int sampleRate)
{
	// set internal sample rate
	hpf_sampleRate = sampleRate;
}

void HPFButterworth::setSampleRate(int sampleRate)
{
	// allocate internal sample rate
	hpf_sampleRate = sampleRate;

	// update hpf gains
	updateGains();
}


void HPFButterworth::setCutoffFrequency(float cutoffFreq)
{
	// allocate cutoff frequency value
	hpf_cutoffFreq = cutoffFreq;

	// update hpf gains
	updateGains();
}

void HPFButterworth::updateGains()
{
	// define hpf fb and ff gains
	float C = tan(( M_PI * hpf_cutoffFreq) / (float)hpf_sampleRate );
	hpf_a0 = 1.0 / (1.0 + sqrt(2.0) * C + C * C);
	hpf_a1 = -2.0 * hpf_a0;
	hpf_a2 = hpf_a0;
	hpf_b1 = 2.0 * hpf_a0 * (C * C - 1.0);
	hpf_b2 = hpf_a0 * (1.0 - sqrt(2.0) * C + C * C);
}

void HPFButterworth::updateBuffers(float xn, float yn)
{
	hpf_xn_2 = hpf_xn_1;
	hpf_yn_2 = hpf_yn_1;
	hpf_xn_1 = xn;
	hpf_yn_1 = yn;
}

float HPFButterworth::processAudio(float xn)
{

	// compute filtered output
	float yn = hpf_a0 * xn + hpf_a1 * hpf_xn_1 + hpf_a2 * hpf_xn_2 - hpf_b1 * hpf_yn_1 - hpf_b2 * hpf_yn_2;

	// update buffers
	updateBuffers(xn, yn);

	return yn;
}

#pragma once
#include "LPCombFilter.h"
#include<math.h>
#define MAX_CUTOFF_FREQUENCY 20000


LPCombFilter::LPCombFilter() : CombFilter()
{
	lpcf_feedbackLPF = new LPFButterworth();
	setCutoffFrequency(MAX_CUTOFF_FREQUENCY);
}


LPCombFilter::~LPCombFilter() {
	lpcf_feedbackLPF->~LPFButterworth();
}

void LPCombFilter::init(float maxDelayInmsec, int sampleRate)
{
	// initialize delay line
	Delay::init(maxDelayInmsec, sampleRate);

	// initialize LPF
	lpcf_feedbackLPF->init(sampleRate);
}

void LPCombFilter::setCutoffFrequency(float cutoffFreq)
{
	// set LPCF cutoff frequency to the inserted value
	lpcf_cutoffFreq = cutoffFreq;

	// set LPF cutoff frequency to the inserted value
	lpcf_feedbackLPF->setCutoffFrequency(lpcf_cutoffFreq);

}

float LPCombFilter::processAudio(float xn)
{
	// Extract value from delay buffer
	float yn = readFromDelayLine();
	
	// process output with LPF
	float yn_lpf = lpcf_feedbackLPF->processAudio(yn);

	// compute value to be stored in delay buffer
	float buff = xn + cf_feedbackGain * yn_lpf;

	// write the value to delay buffer
	writeToDelayLine(buff);

	// aggiorna gli indici
	updateIndices();

	// return output value
	return yn * dly_makeUpGain;
}



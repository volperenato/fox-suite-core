#pragma once
#include "LPCombFilter.h"
#include <math.h>


LPCombFilter::LPCombFilter() : CombFilter() {
	lpcf_feedbackLPF = new LowPassFilter();	
}

LPCombFilter::~LPCombFilter() {
	delete lpcf_feedbackLPF;
}

void LPCombFilter::init(float maxDelayInmsec, int sampleRate) {
	// initialize delay line
	Delay::init(maxDelayInmsec, sampleRate);

	// initialize LPF
	lpcf_feedbackLPF->init(sampleRate);
	setCutoffFrequency(MAX_LPF_FREQUENCY);
}

void LPCombFilter::setCutoffFrequency(float cutoffFreq) {
	// set LPCF cutoff frequency to the inserted value
	lpcf_cutoffFreq = cutoffFreq;

	// set LPF cutoff frequency to the inserted value
	lpcf_feedbackLPF->setCutoffFrequency(lpcf_cutoffFreq);
}

void LPCombFilter::setFilterType(LPFilterType type) {
	lpcf_feedbackLPF->setFilterType(type);
}

float LPCombFilter::processLowPass(float xn) {
	return lpcf_feedbackLPF->processAudio(xn);
}

float LPCombFilter::processAudio(float xn) {
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



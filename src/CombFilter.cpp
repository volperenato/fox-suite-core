#pragma once
#include "CombFilter.h"
#include <math.h>


CombFilter::CombFilter() : Delay() {
	cf_feedbackGain     = 0.0;
	cf_decayInSeconds   = 0.0;
	cf_feedbackGainSign = 1;
}


CombFilter::~CombFilter() {}

void CombFilter::setFeedback(float g) {
	cf_feedbackGain = g;
}

void CombFilter::setFeedbackFromDecay(float decayInSeconds) {
	// set decay
	cf_decayInSeconds = decayInSeconds;
	
	// compute comb filter gain module according to given decay value (in seconds)
	float feedbackModule = (cf_decayInSeconds > 0.0) ? pow(10, -3.0 * dly_delayInmsec / (cf_decayInSeconds * 1000.0)) : 0.0;

	// allocate comb filter gain value retaining its sign
	cf_feedbackGain = cf_feedbackGainSign * feedbackModule;
}

float CombFilter::processAudio(float xn) {
	// Extract value from delay buffer
	float yn = readFromDelayLine();
	
	// compute value to be stored in delay buffer
	float buff = xn + cf_feedbackGain * yn;

	// write the value to delay buffer
	writeToDelayLine(buff);

	// aggiorna gli indici
	updateIndices();

	// return output value
	return yn * dly_makeUpGain;
}

void CombFilter::setFeedbackToNegative() {
	cf_feedbackGainSign = -1;
}

void CombFilter::setFeedbackToPositive() {	
	cf_feedbackGainSign = 1;
}

float CombFilter::getFeedback() {
	return cf_feedbackGain;
}



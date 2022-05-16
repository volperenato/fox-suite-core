#pragma once
#include "AllPassFilter.h"
#include <math.h>

AllPassFilter::AllPassFilter() : Delay() {
	ap_feedbackGain = 0.0;
	ap_decayInSeconds = 0.0;
}

AllPassFilter::~AllPassFilter() {}

void AllPassFilter::setFeedback(float g)
{
	ap_feedbackGain = g;	
}

void AllPassFilter::setFeedbackFromDecay(float decay)
{
	// set decay
	ap_decayInSeconds = decay;

	// compute comb filter gain. Decay value must be in seconds
	ap_feedbackGain = pow(10, -3 * dly_delayInmsec / (ap_decayInSeconds * 1000));
}


float AllPassFilter::processAudio(float xn) {
	
	// read sample from delay line
	float buf = readFromDelayLine();

	// define output
	float yn = buf - ap_feedbackGain * xn;

	// define sample to be written to delay line
	float samp = xn + ap_feedbackGain * yn;

	// write sample to delay line
	writeToDelayLine(samp);
	
	// update indices
	updateIndices();

	return yn;

}

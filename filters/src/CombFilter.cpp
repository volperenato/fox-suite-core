#pragma once
#include "CombFilter.h"
#include<math.h>



CombFilter::CombFilter() : Delay()
{
	cf_feedbackGain = 0.0;
	cf_decayInSeconds = 0.0;
}


CombFilter::~CombFilter() {}

void CombFilter::setFeedback(float g)
{
	cf_feedbackGain = g;
}

void CombFilter::setFeedbackFromDecay(float decayInSeconds)
{
	// set decay
	cf_decayInSeconds = decayInSeconds;
	
	// compute comb filter gain. Decay value must be in seconds
	cf_feedbackGain = pow(10, -3 * dly_delayInmsec / (cf_decayInSeconds * 1000));
}

float CombFilter::processAudio(float xn)
{
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

void CombFilter::setFeedbackToNegative()
{
	if (cf_feedbackGain > 0)
		cf_feedbackGain *= -1;
}

void CombFilter::setFeedbackToPositive()
{
	if (cf_feedbackGain < 0)
		cf_feedbackGain *= -1;
}



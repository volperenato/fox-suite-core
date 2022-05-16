#pragma once
#include "CombFilter.h"
#include "LPFButterworth.h"

class LPCombFilter : public CombFilter
{
protected:

	// lpf Butterworth object for the feedback path
	LPFButterworth* lpcf_feedbackLPF;

	// lpf cutoff frequency
	float lpcf_cutoffFreq;

public:

	LPCombFilter();
	~LPCombFilter();
	void init(float maxDelayInmsec, int sampleRate) override;
	void setCutoffFrequency(float cutoffFreq);
	virtual float processAudio(float xn) override;
};


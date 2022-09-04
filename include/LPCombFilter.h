#pragma once
#include "CombFilter.h"
#include "LowPassFilter.h"

class LPCombFilter : public CombFilter
{
protected:

	// lpf Butterworth object for the feedback path
	LowPassFilter* lpcf_feedbackLPF;

	// lpf cutoff frequency
	float lpcf_cutoffFreq;

public:

	LPCombFilter();
	~LPCombFilter();
	void init(float maxDelayInmsec, int sampleRate) override;
	void setCutoffFrequency(float cutoffFreq);
	void setFilterType(LPFilterType type);
	float processLowPass(float xn);
	virtual float processAudio(float xn) override;
};


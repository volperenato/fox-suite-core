#pragma once
#include "AllPassFilter.h"
#include "LFO.h"

class AllPassFilterModulated : public AllPassFilter
{
protected:

	// Modulation Rate [hz]
	float apfm_modRateHz;

	// Modulation depth applied [msec]
	float apfm_modDepthmsec;

	// Minimum delay mod time [msec]
	float apfm_minDelaymSec;

	// Maximum delay mod time [msec]
	float apfm_maxDelaymSec;

	// Maximum delay mod depth [msec]
	float apfm_maxDepthmSec;

	// Mean modulation amplitude [msec]
	float apfm_meanModAmplitudemsec;

	// Delta Modulation amplitude [msec]
	float apfm_deltaModAmplitudemsec;

	// Center total delay value in msec [msec]
	float apfm_centerDelayValueInmSec;

	// LFO object
	LFO* apfm_LFOOscillator;
		
public:

	AllPassFilterModulated();
	~AllPassFilterModulated();
	void init(float delayLineLengthInmsec, int sampleRate, OscillatorType waveform, float LFOFrequency);
	void setModDepth(float modDepth);
	void setModRate(float modRate);
	void setModWaveform(OscillatorType waveform);
	void setDelayInmsec(float dlyInmSec);
	float getModFrequency() const;
	virtual float processAudio(float xn) override;
	virtual void setSampleRate(int sampleRate) override;


};


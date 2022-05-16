#pragma once
#include "Delay.h"
#include "LFO.h"

class Chorus : public Delay
{
protected:

	// Modulation Rate [hz]
	float chorus_modRateHz;

	// Modulation depth applied [msec]
	float chorus_modDepthmsec;

	// Minimum delay mod time [msec]
	float chorus_minDelaymSec;

	// Maximum delay mod time [msec]
	float chorus_maxDelaymSec;

	// Maximum delay mod depth [msec]
	float chorus_maxDepthmSec;

	// Mean modulation amplitude [msec]
	float chorus_meanModAmplitudemsec;

	// Delta Modulation amplitude [msec]
	float chorus_deltaModAmplitudemsec;

	// LFO object
	LFO* chorus_LFOOscillator;
		
public:

	Chorus();
	~Chorus();
	void init(float delayLineLengthInmsec, int sampleRate, OscillatorType waveform, float LFOFrequency);
	void setModDepth(float modDepth);
	void setModRate(float modRate);
	void setModWaveform(OscillatorType waveform);
	float getModFrequency() const;
	virtual float processAudio(float xn) override;
	virtual void setSampleRate(int sampleRate) override;


};


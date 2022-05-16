#pragma once
#include "LFO.h"

class Tremolo
{
protected:

	// Modulation Rate [hz]
	float trem_modRateHz;

	// Modulation depth applied [-]
	float trem_modDepth;

	// Modulation waveform
	OscillatorType trem_modWave;

	// LFO object
	LFO* trem_LFOOscillator;
		
public:

	Tremolo();
	~Tremolo();
	void init(int sampleRate, OscillatorType waveform, float LFOFrequency, float LFOAmplitude);
	void setModDepth(float modDepth);
	void setModRate(float modRate);
	void setModWaveform(OscillatorType waveform);
	void setModBipolar(bool bipolar);
	float getModFrequency() const;
	virtual float processAudio(float xn);
	virtual void setSampleRate(int sampleRate);


};


#pragma once
#include "LPCombFilter.h"
#include "LFO.h"
#include "utils.h"

#define DEFAULT_OSC_TYPE OscillatorType::Sine

class ModDelay : public LPCombFilter
{
public:

	// LFO object
	LFO* mdly_LFO;

	// LFO unipolar
	bool mdly_isUnipolar;
	
	// LFO type
	OscillatorType mdly_lfoWaveform;

	// mean delay modulation value in msec
	float mdly_meanDelayValue;

	// delta delay modulation value in msec
	float mdly_deltaDelayValue;	

	float mdly_minDelayValue, mdly_maxDelayValue;
	float mdly_rate;

private:

	void updateDelays() {
		mdly_deltaDelayValue = mdly_maxDelayValue - mdly_minDelayValue;
		if (!mdly_isUnipolar)
			mdly_deltaDelayValue /= 2.0;
	}

public:

	ModDelay() : LPCombFilter() {
		mdly_LFO = new LFO();
		mdly_minDelayValue = 0.0;
		mdly_maxDelayValue = 0.0;
		mdly_rate = 0.0;
		mdly_deltaDelayValue = 0.0;
		mdly_meanDelayValue = 0.0;
		mdly_lfoWaveform = DEFAULT_OSC_TYPE;
	}

	~ModDelay() {
		delete mdly_LFO;
	}

	void init(float bufferLengthMs, int sampleRate, OscillatorType type = DEFAULT_OSC_TYPE) {
		bool isunipolar = false;
		
		// initialize delay line
		LPCombFilter::init(bufferLengthMs, sampleRate);

		// initialize LFO
		mdly_lfoWaveform = type;
		mdly_LFO->init(type, sampleRate);
		setLFOUnipolar(isunipolar);
	}

	void setMinDelay(float minDel) {
		mdly_minDelayValue = minDel;
		updateDelays();
	}

	void setMaxDelayValue(float maxDel) {
		mdly_maxDelayValue = maxDel;
		updateDelays();
	}

	void setDeltaDelayValue(float delta) {
		mdly_deltaDelayValue = delta;
	}

	void setDelayInmsec(float delay) {
		mdly_meanDelayValue = delay;
	}

	void setModRate(float modRate) {
		mdly_rate = modRate;
		mdly_LFO->setLFOfrequency(mdly_rate);
	}		

	void setLFOWaveform(OscillatorType wave) {
		mdly_lfoWaveform = wave;
		mdly_LFO->setLFOWaveform(wave);
	}

	void setLFOUnipolar(bool isUnipolar) {
		mdly_isUnipolar = isUnipolar;
		mdly_LFO->setLFOunipolar(isUnipolar);
	}	

	void setSampleRate(int sampleRate) {
		mdly_LFO->setSampleRate(sampleRate);
		LPCombFilter::setSampleRate(sampleRate);
	}

	float processAudio(float xn) {
		// Compute the total delay value in milliseconds
		float newDelayInmsec = mdly_meanDelayValue + mdly_deltaDelayValue * mdly_LFO->processAudio();
		
		// Set the delay value to the delay line
		LPCombFilter::setDelayInmsec(newDelayInmsec);

		// comb filter processing
		float yn = LPCombFilter::processAudio(xn);

		return yn;
	}

};


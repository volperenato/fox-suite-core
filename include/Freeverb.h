#pragma once
#define NUM_COMB_FILTERS 8
#define NUM_ALLPASS_FILTERS_IN 3
#define NUM_ALLPASS_FILTERS_OUT 3
#define MAX_LPF_FREQUENCY 20000.0
#define MIN_LPF_FREQUENCY 20.0
#define MAX_HPF_FREQUENCY 17000.0
#define MIN_HPF_FREQUENCY 10.0
#include <math.h>
#include "AllPassFilter.h"
#include "Delay.h"
#include "LPCombFilter.h"

class Freeverb
{

protected:

	// Reverb User Parameters
	float freeverb_wet, freeverb_smearing, freeverb_decaySeconds, freeverb_dampingFrequency, freeverb_preDelayMilliSeconds, freeverb_spread;

	// Reverb Internal parameters
	float wet1, wet2, dry;

	// pre-delay module
	CombFilter* preDelayModule;

	// Comb Filters
	LPCombFilter* combFiltersL;
	LPCombFilter* combFiltersR;

	float combFilterDlymsRight[NUM_COMB_FILTERS], combFilterDlymsLeft[NUM_COMB_FILTERS];

	// All pass filters
	AllPassFilter* apFiltersL_input;
	AllPassFilter* apFiltersR_input;
	AllPassFilter* apFiltersL_output;
	AllPassFilter* apFiltersR_output;

	float allPassDlymsRight_input[NUM_ALLPASS_FILTERS_IN], allPassDlymsLeft_input[NUM_ALLPASS_FILTERS_IN];
	float allPassDlymsRight_output[NUM_ALLPASS_FILTERS_OUT], allPassDlymsLeft_output[NUM_ALLPASS_FILTERS_OUT];
	
public:

	Freeverb();
	~Freeverb();
	void init(int sampleRate, float rwet, float rdecay, float rdamping, float rsmearing, float rspread, float rpredelay);
	float mapValueIntoRange(float value, float minvalue, float maxValue);
	float mapValueOutsideRange(float value, float minValue, float maxValue);
	void setCombFiltersDelay();
	void setAllPassFiltersDelay();
	void updateReverbParameters();

	// Process Audio
	void processAudio(float* in, float* out);

	// Setter methods
	void setSampleRate(int sampleRate);
	void setReverbWet(float wet);
	void setReverbSmearing(float smearing);
	void setReverbDecayInSeconds(float decay);
	void setReverbDampingFrequency(float damping);
	void setReverbPreDelayInMilliseconds(float predelay);
	void setReverbSpread(float spread);

	// Getter methods
	float getReverbWet();
	float getReverbSmearing();
	float getReverbDecayInSeconds();
	float getReverbDampingFrequency();
	float getReverbPreDelayInMilliseconds();
	float getReverbSpread();
};


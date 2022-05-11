#pragma once
#include "CombFilter.h"
#include "LFO.h"


enum class ModulationType {Chorus=0, Vibrato, Flanger};

class ModDelay : protected CombFilter
{
protected:

	// LFO object
	LFO* mdly_lfo;

	// LFO rate
	float mdly_modRate;

	// LFO depth
	float mdly_modDepth;

	// LFO unipolar
	bool mdly_isUnipolar;
	
	// LFO type
	OscillatorType mdly_modLFO;

	// Modulation type
	ModulationType mdly_modType;

	// minimum delay in msec
	float mdly_minDelaymSec;

	// maximum delay in msec
	float mdly_maxDelaymSec;

	// delay feedback
	float mdly_feedback;

	// wet level
	float mdly_wet;

	// dry level
	float mdly_dry;

	// mean delay modulation value in msec
	float mdly_meanMod;

	// delta delay modulation value in msec
	float mdly_deltaMod;	

public:

	ModDelay();
	~ModDelay();	
	virtual void init(int sampleRate, ModulationType modType, float modRate, float ModDepth);
	void setModRate(float modRate);
	void setModDepth(float modDepth);
	void setModType(ModulationType modType);
	void setSampleRate(int sampleRate);
	void setFeedback(float feedback);
	virtual float processAudio(float xn);
};


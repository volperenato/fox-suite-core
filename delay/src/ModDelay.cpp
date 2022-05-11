#pragma once
#include "ModDelay.h"
#include <string>


ModDelay::ModDelay() : CombFilter() , mdly_lfo(new LFO)
{
	mdly_modType = ModulationType::Chorus;	
	mdly_feedback = 0.0;
}


ModDelay::~ModDelay()
{
	delete mdly_lfo;
}

void ModDelay::init(int sampleRate, ModulationType modType, float modRate, float modDepth)
{
	// set modulation type
	setModType(modType);

	// initialize delay line
	CombFilter::init(100.0, sampleRate);
	setDelayInmsec(mdly_meanMod);
	//setFeedback(mdly_feedback);

	// initialize LFO
	mdly_modRate = modRate;
	mdly_lfo->init(mdly_modLFO, mdly_modRate, sampleRate);
	setModDepth(modDepth);
	mdly_lfo->setLFOunipolar(mdly_isUnipolar);
}

void ModDelay::setModRate(float modRate)
{
	mdly_modRate = modRate;
	mdly_lfo->setLFOfrequency(mdly_modRate);
}

void ModDelay::setModDepth(float modDepth)
{
	mdly_modDepth = modDepth;
	//mdly_lfo->setLFOAmplitude(mdly_modDepth);
}
 
void ModDelay::setModType(ModulationType modType)
{
	mdly_modType = modType;
	switch (mdly_modType)
	{
	case ModulationType::Flanger:
	{
		mdly_minDelaymSec = 1.0;
		mdly_maxDelaymSec = 7.0;
		mdly_wet = 0.5;
		mdly_dry = 0.5;
		mdly_feedback = 0.0;
		mdly_modLFO = OscillatorType::Triangular;	
		mdly_isUnipolar = true;	
		mdly_meanMod  = mdly_minDelaymSec;
		mdly_deltaMod = (mdly_maxDelaymSec - mdly_minDelaymSec);
		break;
	}

	case ModulationType::Vibrato:
	{
		mdly_minDelaymSec = 0.0;
		mdly_maxDelaymSec = 7.0;
		mdly_wet = 1.0;
		mdly_dry = 0.0;
		mdly_feedback = 0.0;
		mdly_modLFO = OscillatorType::Sine;
		mdly_isUnipolar = true;
		mdly_meanMod = mdly_minDelaymSec;
		mdly_deltaMod = (mdly_maxDelaymSec - mdly_minDelaymSec);
		break;
	}

	case ModulationType::Chorus:
	{
		mdly_minDelaymSec = 5.0;
		mdly_maxDelaymSec = 30.0;
		mdly_wet = 0.5;
		mdly_dry = 1.0;
		mdly_feedback = 0.0;
		mdly_modLFO = OscillatorType::Triangular;
		mdly_isUnipolar = false;
		mdly_meanMod = (mdly_maxDelaymSec + mdly_minDelaymSec) / 2.0;
		mdly_deltaMod = (mdly_maxDelaymSec - mdly_minDelaymSec) / 2.0;
		break;
	}

	default: // Chorus
	{
		mdly_minDelaymSec = 5.0;
		mdly_maxDelaymSec = 30.0;
		mdly_wet = 0.5;
		mdly_dry = 1.0;
		mdly_feedback = 0.0;
		mdly_modLFO = OscillatorType::Triangular;
		mdly_isUnipolar = false;
		mdly_meanMod = (mdly_maxDelaymSec + mdly_minDelaymSec) / 2.0;
		mdly_deltaMod = (mdly_maxDelaymSec - mdly_minDelaymSec) / 2.0;
		break;
	}
	}			
}

void ModDelay::setSampleRate(int sampleRate)
{
	mdly_lfo->setSampleRate(sampleRate);
	CombFilter::setSampleRate(sampleRate);
}

void ModDelay::setFeedback(float feedback)
{
	setFeedback(feedback);
}

float ModDelay::processAudio(float xn)
{
	// Compute the total delay value in milliseconds
	float newDelayInmsec = mdly_meanMod +mdly_modDepth * mdly_deltaMod * mdly_lfo->processAudio();

	// Set the delay value to the delay line
	this->setDelayInmsec(newDelayInmsec);

	// comb filter processing
	float yn = CombFilter::processAudio(xn);

	//// read from delay line
	//float buf = readFromDelayLine();
	//
	//// write do delay line
	//writeToDelayLine(mdly_feedback * buf + xn);

	//// update delay indices
	//updateIndices();

	return mdly_dry * xn + mdly_wet * yn;
}

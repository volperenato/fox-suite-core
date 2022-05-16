#include "Tremolo.h"
#include <math.h>

Tremolo::Tremolo() {	

	// initialize modulation constants (max, min modulation depth in milliseconds, mod rate in hz)
	trem_LFOOscillator = new LFO();

}

Tremolo::~Tremolo() {
	trem_LFOOscillator->~LFO();
}

void Tremolo::init(int sampleRate, OscillatorType waveform, float LFOFrequency, float LFOAmplitude)
{		
	// init LFO object
	trem_LFOOscillator->init(waveform, LFOFrequency, sampleRate);
	
	// Set LFO amplitude	
	trem_LFOOscillator->setLFOAmplitude(LFOAmplitude);

	// Set LFO unipolar
	trem_LFOOscillator->setLFOunipolar(true);

	// update tremolo parameters
	trem_modDepth = LFOAmplitude;
	trem_modRateHz = LFOFrequency;
	trem_modWave = waveform;
}

void Tremolo::setModDepth(float modDepth)
{
	trem_modDepth = modDepth;
	trem_LFOOscillator->setLFOAmplitude(trem_modDepth);

}

void Tremolo::setModRate(float modRate)
{
	trem_modRateHz = modRate;
	trem_LFOOscillator->setLFOfrequency(trem_modRateHz);
}

void Tremolo::setModWaveform(OscillatorType waveform)
{
	trem_LFOOscillator->setLFOWaveform(waveform);
}

float Tremolo::getModFrequency() const
{
	return trem_LFOOscillator->getLFOFrequency();
}

void Tremolo::setModBipolar(bool bipolar)
{
	trem_LFOOscillator->setLFOunipolar(!bipolar);
}


float Tremolo::processAudio(float xn) 
{	
	// Compute LFO value
	float lfoValue = trem_LFOOscillator->processAudio();

	// All Pass Filter processing
	return xn * lfoValue + xn * (0.99 - trem_modDepth);
}

void Tremolo::setSampleRate(int sampleRate)
{
	trem_LFOOscillator->setSampleRate(sampleRate);
}

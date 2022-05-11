#include "AllPassFilterModulated.h"
#include <math.h>

AllPassFilterModulated::AllPassFilterModulated() : AllPassFilter() , apfm_LFOOscillator(new LFO) {	

	// initialize modulation constants (max, min modulation depth in milliseconds, mod rate in hz)
	apfm_minDelaymSec = 0.0;
	apfm_maxDepthmSec = 0.5;
	apfm_maxDelaymSec = apfm_minDelaymSec + apfm_maxDepthmSec;
	apfm_modRateHz    = 1.0;

	// compute derived quantities
	apfm_meanModAmplitudemsec   = (apfm_maxDelaymSec + apfm_minDelaymSec) / 2;
	apfm_deltaModAmplitudemsec  = (apfm_maxDelaymSec - apfm_minDelaymSec) / 2;
	apfm_modDepthmsec		    = apfm_deltaModAmplitudemsec * 1.0;
	apfm_centerDelayValueInmSec = apfm_meanModAmplitudemsec + dly_delayInmsec;
}

AllPassFilterModulated::~AllPassFilterModulated() {
	apfm_LFOOscillator->~LFO();
}

void AllPassFilterModulated::init(float delayLineLengthInmsec, int sampleRate, OscillatorType waveform, float LFOFrequency)
{	
	Delay::init(delayLineLengthInmsec, sampleRate);
	apfm_LFOOscillator->init(waveform, LFOFrequency, sampleRate);
	
	// Set LFO amplitude
	apfm_LFOOscillator->setLFOAmplitude(apfm_modDepthmsec);
}

void AllPassFilterModulated::setModDepth(float modDepth)
{
	apfm_modDepthmsec = apfm_deltaModAmplitudemsec * modDepth;
	apfm_LFOOscillator->setLFOAmplitude(apfm_modDepthmsec);

}

void AllPassFilterModulated::setModRate(float modRate)
{
	apfm_modRateHz = modRate;
	apfm_LFOOscillator->setLFOfrequency(apfm_modRateHz);
}

void AllPassFilterModulated::setModWaveform(OscillatorType waveform)
{
	apfm_LFOOscillator->setLFOWaveform(waveform);
}

void AllPassFilterModulated::setDelayInmsec(float dlyInmSec)
{
	Delay::setDelayInmsec(dlyInmSec);
	apfm_centerDelayValueInmSec = apfm_meanModAmplitudemsec + dlyInmSec;
}

float AllPassFilterModulated::getModFrequency() const
{
	return apfm_LFOOscillator->getLFOFrequency();
}



float AllPassFilterModulated::processAudio(float xn) {
	
	// Compute the total delay value in milliseconds
	float newDelayInmsec = apfm_LFOOscillator->processAudio() + apfm_centerDelayValueInmSec;

	// Set the delay value to the delay line
	Delay::setDelayInmsec(newDelayInmsec);
	
	// All Pass Filter processing
	float yn = AllPassFilter::processAudio(xn);

	return yn;
}

void AllPassFilterModulated::setSampleRate(int sampleRate)
{
	apfm_LFOOscillator->setSampleRate(sampleRate);
	Delay::setSampleRate(sampleRate);
}

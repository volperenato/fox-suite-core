#include "Chorus.h"
#include <math.h>

Chorus::Chorus() : Delay() , chorus_LFOOscillator(new LFO) {	

	// initialize modulation constants (max, min modulation depth in milliseconds, mod rate in hz)
	chorus_minDelaymSec = 10.0;
	chorus_maxDepthmSec = 30.0;
	chorus_maxDelaymSec = chorus_minDelaymSec + chorus_maxDepthmSec;
	chorus_modRateHz    = 1.0;

	// compute derived quantities
	chorus_meanModAmplitudemsec  = (chorus_maxDelaymSec + chorus_minDelaymSec) / 2;
	chorus_deltaModAmplitudemsec = (chorus_maxDelaymSec - chorus_minDelaymSec) / 2;
	chorus_modDepthmsec		     = chorus_deltaModAmplitudemsec * 1.0;
}

Chorus::~Chorus() {
	chorus_LFOOscillator->~LFO();
}

void Chorus::init(float delayLineLengthInmsec, int sampleRate, OscillatorType waveform, float LFOFrequency)
{	
	Delay::init(delayLineLengthInmsec, sampleRate);
	chorus_LFOOscillator->init(waveform, LFOFrequency, sampleRate);
	
	// set delay time
	setDelayInmsec(0.0);

	// Set LFO amplitude
	chorus_LFOOscillator->setLFOAmplitude(chorus_modDepthmsec);
}

void Chorus::setModDepth(float modDepth)
{
	chorus_modDepthmsec = chorus_deltaModAmplitudemsec * modDepth;
	chorus_LFOOscillator->setLFOAmplitude(chorus_modDepthmsec);

}

void Chorus::setModRate(float modRate)
{
	chorus_modRateHz = modRate;
	chorus_LFOOscillator->setLFOfrequency(chorus_modRateHz);
}

void Chorus::setModWaveform(OscillatorType waveform)
{
	chorus_LFOOscillator->setLFOWaveform(waveform);
}

float Chorus::getModFrequency() const
{
	return chorus_LFOOscillator->getLFOFrequency();
}



float Chorus::processAudio(float xn) {
	
	// Compute LFO value
	float lfoValue = chorus_LFOOscillator->processAudio() + chorus_meanModAmplitudemsec;
	//float lfoValue = chorus_meanModAmplitudemsec;

	// Compute the total delay value in milliseconds
	float newDelayInmsec = dly_delayInmsec + lfoValue;

	// Set the delay value to the delay line
	setDelayInmsec(newDelayInmsec);

	// All Pass Filter processing
	return processAudio(xn);

	//// read sample from delay line
	//float buf = readFromDelayLine();	

	//// define output
	//float yn = buf - ap_feedbackGain * xn; // ISSUE TO BE SOLVED: buf value make plugin crash if used to compute yn

	//// define sample to be written to delay line
	//float samp = xn + ap_feedbackGain * yn;

	//// write sample to delay line
	//writeToDelayLine(samp);
	//
	//// update indices
	//updateIndices();


}

void Chorus::setSampleRate(int sampleRate)
{
	chorus_LFOOscillator->setSampleRate(sampleRate);
	Delay::setSampleRate(sampleRate);
}

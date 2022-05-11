#pragma once
#define _USE_MATH_DEFINES
#include "LFO_2.h"
#include <string>
#include <math.h>

void LFO_2::init(int waveform, float frequency, int sampleRate)
{
	lfo_waveform = waveform;
	lfo_frequency = frequency;
	lfo_sampleRate = sampleRate;
	computeIncrement();
	lfo_counter = 0.0;
}

void LFO_2::setLFOfrequency(float frequency)
{
	lfo_frequency = frequency;
	computeIncrement();
}

void LFO_2::setLFOAmplitude(float amplitude)
{
	lfo_amplitude = amplitude;
}

void LFO_2::setLFOWaveform(int waveform)
{
	lfo_waveform = waveform;
}

void LFO_2::reset()
{
	lfo_counter = 0.0;
}

void LFO_2::increaseLFOCounter()
{
	lfo_counter += lfo_inc;
	if (lfo_counter >= 1)
		lfo_counter = 0.0;
}

void LFO_2::setSampleRate(int sampleRate)
{
	lfo_sampleRate = sampleRate;
	computeIncrement();
	lfo_counter = 0.0;
}

float LFO_2::processAudio()
{
	// Compute LFO value
	float yn;
	switch (lfo_waveform) {

		case 0: {
			yn = 2.0 * lfo_counter - 1.0;
			break;
		}
		case 1: {
			if (lfo_counter == 0)
				yn = 0.0;
			else
				yn = sin(lfo_counter * 2.0 * M_PI);
			break;
		}
		case 2: {
			if (lfo_counter <= 0.5)
				yn = 4.0 * lfo_counter - 1.0;
			else
				yn = 3.0 - 4.0 * lfo_counter;
			break;
		}
		default: {
			yn = 0.0;
			break;
		}

	}
		
	// Increase lfo counter
	increaseLFOCounter();

	// Return LFO value
	return yn;

}

void LFO_2::computeIncrement()
{
	lfo_inc = lfo_frequency / (float)lfo_sampleRate;
}

LFO_2::LFO_2()
{
	lfo_counter = 0.0;
	lfo_sampleRate = 44100;
	lfo_inc = 0.0;
	lfo_frequency = 0.0;
	lfo_waveform = 1;
	lfo_amplitude = 1.0;
}

LFO_2::~LFO_2() {}

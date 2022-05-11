#pragma once
#define _USE_MATH_DEFINES
#define WAVETABLE_SIZE 2048
#include "LFO.h"
#include <string>
#include <math.h>

LFO::LFO()
{
	lfo_counter = 0;
	lfo_sampleRate = 44100;	
	lfo_frequency = 0.0;
	lfo_amplitude = 1.0;
	lfo_inc = 0.0;
	lfo_waveform = OscillatorType::Sine;
	lfo_unipolar = false;
	createWavetables();
}

//LFO::LFO(OscillatorType waveform, float frequency, int sampleRate, float amplitude)
//{
//	createWavetables();
//	init(waveform, frequency, sampleRate);
//	setLFOAmplitude(amplitude);
//}

LFO::~LFO()
{
	delete(lfo_sine);
	delete(lfo_saw);
	delete(lfo_triangular);
	delete(lfo_pulse);
	lfo_sine = lfo_saw = lfo_triangular = lfo_pulse = nullptr;
}

void LFO::init(OscillatorType waveform, float frequency, int sampleRate)
{	
	lfo_counter = 0;
	lfo_amplitude = 1.0;
	lfo_waveform = waveform;
	lfo_frequency = frequency;
	lfo_sampleRate = sampleRate;
	computeIncrement();	
}

void LFO::setLFOfrequency(float frequency)
{
	lfo_frequency = frequency;
	computeIncrement();
}

void LFO::setLFOWaveform(OscillatorType waveform)
{
	lfo_waveform = waveform;
}

void LFO::setLFOAmplitude(float amplitude)
{
	lfo_amplitude = amplitude;
}

void LFO::setLFOunipolar(bool isUnipolar)
{
	lfo_unipolar = isUnipolar;
}

float LFO::getLFOFrequency() const
{
	return lfo_frequency;
}

void LFO::computeIncrement()
{
	lfo_inc = WAVETABLE_SIZE * lfo_frequency / (float) lfo_sampleRate;
}

void LFO::increaseLFOCounter()
{
	lfo_counter += lfo_inc;
	if (lfo_counter >= WAVETABLE_SIZE)
		lfo_counter -= WAVETABLE_SIZE;
}

void LFO::setSampleRate(int sampleRate)
{
	lfo_sampleRate = sampleRate;
	computeIncrement();
}

float LFO::linearInterp(float x1, float x2, float y1, float y2, float x)
{
	float denom = x2 - x1;
	if (denom == 0)
		return y1; // should not ever happen

	// calculate decimal position of x
	float dx = (x - x1) / (x2 - x1);

	// use weighted sum method of interpolating
	float result = dx * y2 + (1 - dx) * y1;

	return result;
}


float LFO::processAudio()
{		
	// Read the LFO value to be returned
	float yn;
	int readIndex = (int)lfo_counter;
	float frac = lfo_counter - readIndex;
	int readIndexNext = (readIndex + 1 >= WAVETABLE_SIZE) ? 0 : readIndex + 1;

	switch (lfo_waveform) {
	case OscillatorType::Saw:
		yn = linearInterp(0, 1, lfo_saw[readIndex], lfo_saw[readIndexNext], frac);		
		break;	
	case OscillatorType::Sine:
		yn = linearInterp(0, 1, lfo_sine[readIndex], lfo_sine[readIndexNext], frac);
		break;	
	case OscillatorType::Triangular:
		yn = linearInterp(0, 1, lfo_triangular[readIndex], lfo_triangular[readIndexNext], frac);
		break;	
	case OscillatorType::Pulse:
		yn = linearInterp(0, 1, lfo_pulse[readIndex], lfo_pulse[readIndexNext], frac);		
		break;
	default:
		yn = 0.0;
	}

	// Increase lfo counter
	increaseLFOCounter();

	// unipolar lfo
	if (lfo_unipolar) {
		yn /= 2.0;
		yn += 0.5;
	}
	// Return LFO value
	return yn * lfo_amplitude;

}

void LFO::createWavetables()
{
	// allocate wavetables memory
	lfo_saw = lfo_sine = lfo_triangular = lfo_pulse = nullptr;
	lfo_saw = (float*)malloc(WAVETABLE_SIZE * sizeof(float));
	lfo_sine = (float*)malloc(WAVETABLE_SIZE * sizeof(float));
	lfo_triangular = (float*)malloc(WAVETABLE_SIZE * sizeof(float));
	lfo_pulse = (float*)malloc(WAVETABLE_SIZE * sizeof(float));
	memset(lfo_saw, 0, WAVETABLE_SIZE * sizeof(float));
	memset(lfo_sine, 0, WAVETABLE_SIZE * sizeof(float));
	memset(lfo_triangular, 0, WAVETABLE_SIZE * sizeof(float));
	memset(lfo_pulse, 0, WAVETABLE_SIZE * sizeof(float));
	
	// define wavetables	
	float step;
	int halfWave = WAVETABLE_SIZE / 2;
	for (int i = 0; i < WAVETABLE_SIZE; i++) {

		// define step reached
		step = (float)i / (float)WAVETABLE_SIZE;

		// saw wave
		lfo_saw[i] = 2.0 * step - 1.0;

		// sine wave
		if (i != 0)
			lfo_sine[i] = sin(step * 2.0 * M_PI);
		else
			lfo_sine[i] = 0.0;

		// triangular and pulse waves
		if (i <= halfWave) {
			lfo_triangular[i] = 4.0 * step - 1.0;
			lfo_pulse[i] = 1.0;
		}
		else {
			lfo_triangular[i] = 3.0 - 4.0 * step;
			lfo_pulse[i] = 0.0;
		}		
	}
}

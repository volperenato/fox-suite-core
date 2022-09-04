#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include "utils.h"
#include "constants.h"

#define DEFAULT_OSC_TYPE OscillatorType::Sine
#define WAVETABLE_SIZE 4096


class LFO {

protected:

	// LFO frequency
	float lfo_frequency;

	// internal sample rate
	int lfo_sampleRate;

	// LFO amplitude
	float lfo_amplitude;

	// LFO waveform
	OscillatorType lfo_waveform;

	// LFO counter
	float lfo_counter;

	// LFO reading increment
	float lfo_inc;

	// wavetables
	std::vector<float> lfo_table;

	// unipolar flag
	bool lfo_unipolar;

public:

	LFO(OscillatorType type = DEFAULT_OSC_TYPE)
	{
		lfo_counter = 0.0;
		lfo_sampleRate = _TEMPLATE_SAMPLERATE;
		lfo_frequency = 0.0;
		lfo_amplitude = 1.0;
		lfo_inc = 0.0;
		lfo_waveform = type;
		lfo_unipolar = false;
		createTable();
	}

	~LFO()
	{
		clearTable();
	}

	void init(OscillatorType waveform, int sampleRate) {
		lfo_counter = 0.0;
		lfo_amplitude = 1.0;
		lfo_waveform = waveform;
		lfo_frequency = 0.0;
		lfo_sampleRate = sampleRate;		
		computeIncrement();
		clearTable();
		createTable();
	}

	void setLFOfrequency(float frequency) {
		lfo_frequency = frequency;
		computeIncrement();
	}

	void setLFOWaveform(OscillatorType waveform) {
		lfo_waveform = waveform;
		clearTable();
		createTable();
	}

	void setLFOAmplitude(float amplitude) {
		lfo_amplitude = amplitude;
	}

	void setLFOunipolar(bool isUnipolar) {
		lfo_unipolar = isUnipolar;
	}

	float getLFOFrequency() const {
		return lfo_frequency;
	}

	void setSampleRate(int sampleRate) {
		lfo_sampleRate = sampleRate;
		computeIncrement();
	}


	float processAudio() {
		// Increase lfo counter
		increaseLFOCounter();

		// Read the LFO value to be returned
		int readIndex = (int)lfo_counter;
		float frac = lfo_counter - readIndex;
		int readIndexNext = (readIndex + 1 >= WAVETABLE_SIZE) ? 0 : readIndex + 1;
		float yn = linearInterp(0.0, 1.0, lfo_table[readIndex], lfo_table[readIndexNext], frac);
		
		// unipolar lfo
		if (lfo_unipolar) {
			yn /= 2.0;
			yn += 0.5;
		}
		// Return LFO value
		return yn * lfo_amplitude;

	}

private:

	void computeIncrement() {
		lfo_inc = WAVETABLE_SIZE * lfo_frequency / (float)lfo_sampleRate;
	}

	void increaseLFOCounter() {
		lfo_counter += lfo_inc;
		if (lfo_counter >= WAVETABLE_SIZE)
			lfo_counter -= WAVETABLE_SIZE;
	}

	void clearTable() {
		lfo_table.clear();
	}

	void createTable() {
	
		lfo_table.reserve(WAVETABLE_SIZE);	

		// define wavetables	
		float step;
		int halfWave = WAVETABLE_SIZE / 2;
		switch (lfo_waveform) {

		// Create Sine oscillator wave
		case OscillatorType::Sine: {
			for (int i = 0; i < WAVETABLE_SIZE; i++) {
				step = (float)i / (float)WAVETABLE_SIZE;
				if (i != 0)
					lfo_table[i] = sin(step * 2.0 * M_PI);
				else
					lfo_table[i] = 0.0;
			}
			break;
		}

		// Create Saw oscillator wave
		case OscillatorType::Saw: {
			for (int i = 0; i < WAVETABLE_SIZE; i++) {
				step = (float)i / (float)WAVETABLE_SIZE;
				lfo_table[i] = 2.0 * step - 1.0;
			}
			break;
		}

		// Create Triangular oscillator wave
		case OscillatorType::Triangular: {
			for (int i = 0; i < WAVETABLE_SIZE; i++) {
				step = (float)i / (float)WAVETABLE_SIZE;	
				lfo_table[i] = 2.0 * abs(2.0 * step - 1.0) - 1.0;				
			}
			break;
		}

		// Create Pulse oscillator wave
		case OscillatorType::Pulse: {
			for (int i = 0; i < WAVETABLE_SIZE; i++) {
				step = (float)i / (float)WAVETABLE_SIZE;
				if (i <= halfWave)
					lfo_table[i] = 1.0;
				else
					lfo_table[i] = 0.0;
			}
			break;
		}
		}
	}
};


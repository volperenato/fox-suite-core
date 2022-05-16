#pragma once

enum class OscillatorType {Saw = 0, Sine, Triangular};

class LFO_2
{
protected:

	// LFO frequency
	float lfo_frequency;

	// LFO amplitude
	float lfo_amplitude;

	// internal sample rate
	int lfo_sampleRate;

	// LFO waveform
	int lfo_waveform;

	// LFO counter
	float lfo_counter;

	// LFO reading increment
	float lfo_inc;

public:

	LFO_2();
	~LFO_2();
	virtual void init(int waveform, float frequency, int sampleRate);
	void setLFOfrequency(float frequency);
	void setLFOAmplitude(float amplitude);
	void setLFOWaveform(int waveform);
	void reset();
	void increaseLFOCounter();
	virtual void setSampleRate(int sampleRate);
	virtual float processAudio();

private:

	void computeIncrement();
};


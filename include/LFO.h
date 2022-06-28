#pragma once
enum class OscillatorType {Saw=0, Sine, Triangular, Pulse};

class LFO
{
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
	float* lfo_saw, * lfo_sine, * lfo_triangular, * lfo_pulse;

	// unipolar flag
	bool lfo_unipolar;

public:
	LFO();
	//LFO(OscillatorType waveform, float frequency, int sampleRate, float amplitude);
	~LFO();
	void createWavetables();
	virtual void init(OscillatorType waveform, float frequency, int sampleRate);
	void setLFOfrequency(float frequency);
	void setLFOWaveform(OscillatorType waveform);
	void setLFOAmplitude(float amplitude);	
	void setLFOunipolar(bool isUnipolar);
	float getLFOFrequency() const;	
	virtual void setSampleRate(int sampleRate);
	virtual float processAudio();

private:
	void computeIncrement();
	void increaseLFOCounter();
};


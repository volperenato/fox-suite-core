#pragma once

class LPFButterworth {

protected:

	// lpf cutoff frequency
	float lpf_cutoffFreq;

	// internal sample rate
	int lpf_sampleRate;

	// lpf gains
	float lpf_a0;
	float lpf_a1;
	float lpf_a2;
	float lpf_b1;
	float lpf_b2;

	// input buffers
	float lpf_xn_1;
	float lpf_xn_2;

	// output buffers
	float lpf_yn_1;
	float lpf_yn_2;

public:

	LPFButterworth();
	~LPFButterworth();
	void init(int sampleRate);
	void setSampleRate(int sampleRate);
	void setCutoffFrequency(float cutoffFreq);
	void updateGains();
	void updateBuffers(float xn, float yn);
	virtual float processAudio(float xn);
};

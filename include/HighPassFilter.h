#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "constants.h"
#include "utils.h"

#define DEFAULT_HIGH_PASS_FILTER_TYPE HPFilterType::Butterworth
#define DEFAULT_SHELVING_GAIN -6.0
#define DEFAULT_RESONANCE 0.707

class HighPassFilter {

protected:

	// lp filter type
	HPFilterType hpf_type;

	// lpf cutoff frequency
	float hpf_cutoffFreq;

	// lpf sample rate
	int hpf_sampleRate;

	// lpf gain attenutation (for shelving filters)
	float hpf_shelvingGaindB;

	// lpf resonance factor Q
	float hpf_Q;

	// lpf gains
	float hpf_a0, hpf_a1, hpf_a2, hpf_b1, hpf_b2, hpf_c0;

	// lpf buffers
	float hpf_xn_1, hpf_xn_2, hpf_yn_1, hpf_yn_2;

public:

	HighPassFilter(int sampleRate = _TEMPLATE_SAMPLERATE, float freq = MAX_HPF_FREQUENCY, HPFilterType type = DEFAULT_HIGH_PASS_FILTER_TYPE) {
		hpf_cutoffFreq = freq;
		hpf_sampleRate = sampleRate;
		hpf_type = type;
		hpf_shelvingGaindB = DEFAULT_SHELVING_GAIN;
		hpf_Q = DEFAULT_RESONANCE;
		hpf_xn_1 = 0.0;
		hpf_xn_2 = 0.0;
		hpf_yn_1 = 0.0;
		hpf_yn_2 = 0.0;
		updateGains();
	}

	~HighPassFilter() {}

	void init(int sampleRate) {
		// set internal sample rate
		hpf_sampleRate = sampleRate;
	}

	void setSampleRate(int sampleRate) {
		// allocate internal sample rate
		hpf_sampleRate = sampleRate;

		// update hpf gains
		updateGains();
	}

	void setCutoffFrequency(float cutoffFreq) {
		// allocate cutoff frequency value
		hpf_cutoffFreq = cutoffFreq;

		// update hpf gains
		updateGains();
	}

	void setFilterType(HPFilterType type) {
		// allocate filter type
		hpf_type = type;

		// update hpf gains
		updateGains();
	}

	void setShelvingGain(float gain) {
		hpf_shelvingGaindB = gain;
		updateGains();
	}

	void setQualityFactor(float Q) {
		hpf_Q = Q;
		updateGains();
	}

	void updateGains() {
		switch (hpf_type) {
		case HPFilterType::Butterworth: {
			// define hpf fb and ff gains
			float C = tan((M_PI * hpf_cutoffFreq) / (float)hpf_sampleRate);
			hpf_a0 = 1.0 / (1.0 + sqrt(2.0) * C + C * C);
			hpf_a1 = -2.0 * hpf_a0;
			hpf_a2 = hpf_a0;
			hpf_b1 = 2.0 * hpf_a0 * (C * C - 1.0);
			hpf_b2 = hpf_a0 * (1.0 - sqrt(2.0) * C + C * C);
			hpf_c0 = 1.0;
			break;
		}
		case HPFilterType::LinkwitzRiley: {
			float omegac = M_PI * hpf_cutoffFreq;
			float thetac = omegac / (float)hpf_sampleRate;
			float k = omegac / tan(thetac);
			float delta = k * k + omegac * omegac + 2 * k * omegac;
			hpf_a0 = k * k / delta;
			hpf_a1 = -2 * hpf_a0;
			hpf_a2 = hpf_a0;
			hpf_b1 = (-2 * k * k + 2 * omegac * omegac) / delta;
			hpf_b2 = (-2 * k * omegac + k * k + omegac * omegac) / delta;
			hpf_c0 = 1.0;
			break;
		}
		case HPFilterType::Shelving: {
			float thetac = 2.0 * M_PI * hpf_cutoffFreq / (float)hpf_sampleRate;
			float mi = pow(10.0, hpf_shelvingGaindB / 20.0);
			float beta = (1.0 + mi) / 4.0;
			float delta = beta * tan(thetac / 2.0);
			float gamma = (1.0 - delta) / (1.0 + delta);
			hpf_a0 = (1.0 + gamma) / 2.0;
			hpf_a1 = -hpf_a0;
			hpf_a2 = 0.0;
			hpf_b1 = -gamma;
			hpf_b2 = 0.0;
			hpf_c0 = 1.0;// mi - 1.0;
			break;
		}
		case HPFilterType::DigitalFirstOrder: {
			float thetac = 2.0 * M_PI * hpf_cutoffFreq / (float)hpf_sampleRate;
			float gamma = cos(thetac) / (1.0 + sin(thetac));
			hpf_a0 = (1.0 + gamma) / 2.0;
			hpf_a1 = -hpf_a0;
			hpf_a2 = 0.0;
			hpf_b1 = -gamma;
			hpf_b2 = 0.0;
			hpf_c0 = 1.0;
			break;
		}		
		}
	}

	void updateBuffers(float xn, float yn) {
		hpf_xn_2 = hpf_xn_1;
		hpf_yn_2 = hpf_yn_1;
		hpf_xn_1 = xn;
		hpf_yn_1 = yn;
	}

	float processAudio(float xn) {
		// compute filtered output
		float yn = hpf_a0 * xn + hpf_a1 * hpf_xn_1 + hpf_a2 * hpf_xn_2 - hpf_b1 * hpf_yn_1 - hpf_b2 * hpf_yn_2;

		// update buffers
		updateBuffers(xn, yn);

		return hpf_c0 * yn;
	}

	float getCutoffFrequency() {
		return hpf_cutoffFreq;
	}

	HPFilterType getFilterType() {
		return hpf_type;
	}

	float getShelvingGain() {
		return hpf_shelvingGaindB;
	}

	float getQualityFactor() {
		return hpf_Q;
	}

private:

	void constructLPF(int sampleRate = _TEMPLATE_SAMPLERATE, float freq = MAX_HPF_FREQUENCY, HPFilterType type = DEFAULT_HIGH_PASS_FILTER_TYPE) {
		hpf_cutoffFreq = freq;
		hpf_sampleRate = sampleRate;
		hpf_type = type;
		hpf_xn_1 = 0.0;
		hpf_xn_2 = 0.0;
		hpf_yn_1 = 0.0;
		hpf_yn_2 = 0.0;
		updateGains();
	}
};

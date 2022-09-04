#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "constants.h"
#include "utils.h"

#define DEFAULT_LOW_PASS_FILTER_TYPE LPFilterType::Butterworth
#define DEFAULT_SHELVING_GAIN -6.0
#define DEFAULT_RESONANCE 0.707

class LowPassFilter {

protected:

	// lp filter type
	LPFilterType lpf_type;

	// lpf cutoff frequency
	float lpf_cutoffFreq;

	// lpf sample rate
	int lpf_sampleRate;

	// lpf gain attenutation (for shelving filters)
	float lpf_shelvingGaindB;

	// lpf resonance factor Q
	float lpf_Q;

	// lpf gains
	float lpf_a0, lpf_a1, lpf_a2, lpf_b1, lpf_b2, lpf_c0;

	// lpf buffers
	float lpf_xn_1, lpf_xn_2, lpf_yn_1, lpf_yn_2;

public: 

	LowPassFilter(int sampleRate = _TEMPLATE_SAMPLERATE, float freq = MAX_LPF_FREQUENCY, LPFilterType type = DEFAULT_LOW_PASS_FILTER_TYPE) {
			lpf_cutoffFreq = freq;
			lpf_sampleRate = sampleRate;
			lpf_type = type;
			lpf_shelvingGaindB = DEFAULT_SHELVING_GAIN;
			lpf_Q = DEFAULT_RESONANCE;
			lpf_xn_1 = 0.0;
			lpf_xn_2 = 0.0;
			lpf_yn_1 = 0.0;
			lpf_yn_2 = 0.0;
			updateGains();
	}

	~LowPassFilter() {}

	void init(int sampleRate) {
		// set internal sample rate
		lpf_sampleRate = sampleRate;
	}

	void setSampleRate(int sampleRate) {
		// allocate internal sample rate
		lpf_sampleRate = sampleRate;

		// update lpf gains
		updateGains();
	}

	void setCutoffFrequency(float cutoffFreq) {
		// allocate cutoff frequency value
		lpf_cutoffFreq = cutoffFreq;

		// update lpf gains
		updateGains();
	}

	void setFilterType(LPFilterType type) {
		// allocate filter type
		lpf_type = type;

		// update lpf gains
		updateGains();
	}

	void setShelvingGain(float gain) {
		lpf_shelvingGaindB = gain;
		updateGains();
	}

	void setQualityFactor(float Q) {
		lpf_Q = Q;
		updateGains();
	}

	void updateGains() {
		switch (lpf_type) {
		case LPFilterType::Butterworth: {
			// define lpf fb and ff gains
			float C = 1.0 / tan((M_PI * lpf_cutoffFreq) / (float)lpf_sampleRate);
			lpf_a0 = 1.0 / (1.0 + sqrt(2.0) * C + C * C);
			lpf_a1 = 2.0 * lpf_a0;
			lpf_a2 = lpf_a0;
			lpf_b1 = 2.0 * lpf_a0 * (1 - C * C);
			lpf_b2 = lpf_a0 * (1.0 - sqrt(2.0) * C + C * C);
			lpf_c0 = 1.0;
			break;
		}
		case LPFilterType::LinkwitzRiley: {
			float omegac = M_PI * lpf_cutoffFreq;
			float thetac = omegac / (float)lpf_sampleRate;
			float k = omegac / tan(thetac);
			float delta = k * k + omegac * omegac + 2 * k * omegac;
			lpf_a0 = omegac * omegac / delta;
			lpf_a1 = 2 * lpf_a0;
			lpf_a2 = lpf_a0;
			lpf_b1 = (-2 * k * k + 2 * omegac * omegac) / delta;
			lpf_b2 = (-2 * k * omegac + k * k + omegac * omegac) / delta;
			lpf_c0 = 1.0;
			break;
		}
		case LPFilterType::Shelving: {
			float thetac = 2.0 * M_PI * lpf_cutoffFreq / (float)lpf_sampleRate;
			float mi = pow(10.0, lpf_shelvingGaindB / 20.0);
			float beta = 4.0 / (1.0 + mi);
			float delta = beta * tan(thetac / 2.0);
			float gamma = (1.0 - delta) / (1.0 + delta);
			lpf_a0 = (1.0 - gamma) / 2.0;
			lpf_a1 = lpf_a0;
			lpf_a2 = 0.0;
			lpf_b1 = -gamma;
			lpf_b2 = 0.0;
			lpf_c0 = 1.0;// mi - 1.0;
			break;
		}
		case LPFilterType::DigitalFirstOrder: {
			float thetac = 2.0 * M_PI * lpf_cutoffFreq / (float)lpf_sampleRate;
			float gamma = cos(thetac) / (1.0 + sin(thetac));
			lpf_a0 = (1.0 - gamma) / 2.0;
			lpf_a1 = lpf_a0;
			lpf_a2 = 0.0;
			lpf_b1 = -gamma;
			lpf_b2 = 0.0;
			lpf_c0 = 1.0;
			break;
		}
		case LPFilterType::AllPoleFirstOrder: {
			float thetac = 2.0 * M_PI * lpf_cutoffFreq / (float)lpf_sampleRate;
			float gamma = 2.0 - cos(thetac);
			lpf_b1 = sqrt(gamma * gamma - 1.0) - gamma;
			lpf_a0 = 1.0 + lpf_b1;
			lpf_a1 = 0.0;
			lpf_a2 = 0.0;
			lpf_b2 = 0.0;
			lpf_c0 = 1.0;
			break;
		}
		case LPFilterType::AllPoleMMA: {
			float thetac = 2.0 * M_PI * lpf_cutoffFreq / (float)lpf_sampleRate;
			float resonance;
			if (lpf_Q <= 0.707)
				resonance = 0.0;
			else
				resonance = 20.0 * log10(lpf_Q * lpf_Q / sqrt(lpf_Q * lpf_Q - 0.25));
			float r = (cos(thetac) + sin(thetac) * sqrt(pow(10.0, resonance / 10.0) - 1.0)) / (pow(10.0, resonance / 20.0) * sin(thetac) + 1.0);
			float g = pow(10.0, -resonance / 40.0);
			lpf_b1 = -2.0 * r * cos(thetac);
			lpf_b2 = r * r;
			lpf_a0 = g * (1.0 + lpf_b1 + lpf_b2);
			lpf_a1 = 0.0;
			lpf_a2 = 0.0;
			lpf_c0 = 1.0;
			break;
		}
		case LPFilterType::Vicanek: {
			float omegac = 2.0 * M_PI * lpf_cutoffFreq / (float)lpf_sampleRate;
			float f0 = omegac / M_PI;
			float q = 1.0 / (2.0 * lpf_Q);
			if (q <= 1.0)
				lpf_b1 = -2.0 * exp(-q * omegac) * cos(sqrt(1.0 - q * q) * omegac);
			else
				lpf_b1 = -2.0 * exp(-q * omegac) * cosh(sqrt(q * q - 1.0) * omegac);
			lpf_b2 = exp(-2.0 * q * omegac);
			float r1 = ((1.0 - lpf_b1 + lpf_b2) * f0 * f0) / sqrt((1.0 - f0 * f0) * (1.0 - f0 * f0) + f0 * f0 / (lpf_Q * lpf_Q));
			float r0 = 1.0 + lpf_b1 + lpf_b2;
			lpf_a0 = (r0 + r1) / 2.0;
			lpf_a1 = r0 - lpf_a0;
			lpf_a2 = 0.0;
			lpf_c0 = 1.0;
			break;
		}
		}		
	}

	void updateBuffers(float xn, float yn) {
		lpf_xn_2 = lpf_xn_1;
		lpf_yn_2 = lpf_yn_1;
		lpf_xn_1 = xn;
		lpf_yn_1 = yn;
	}

	float processAudio(float xn) {
		// compute filtered output
		float yn = lpf_a0 * xn + lpf_a1 * lpf_xn_1 + lpf_a2 * lpf_xn_2 - lpf_b1 * lpf_yn_1 - lpf_b2 * lpf_yn_2;

		// update buffers
		updateBuffers(xn, yn);

		return lpf_c0 * yn;
	}

	float getCutoffFrequency() {
		return lpf_cutoffFreq;
	}	

	LPFilterType getFilterType() {
		return lpf_type;
	}

	float getShelvingGain() {
		return lpf_shelvingGaindB;
	}

	float getQualityFactor() {
		return lpf_Q;
	}

private:

	void constructLPF(int sampleRate = _TEMPLATE_SAMPLERATE, float freq = MAX_LPF_FREQUENCY, LPFilterType type = DEFAULT_LOW_PASS_FILTER_TYPE) {
		lpf_cutoffFreq = freq;
		lpf_sampleRate = sampleRate;
		lpf_type = type;
		lpf_xn_1 = 0.0;
		lpf_xn_2 = 0.0;
		lpf_yn_1 = 0.0;
		lpf_yn_2 = 0.0;
		updateGains();
	}
};

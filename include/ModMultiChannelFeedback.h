#pragma once
#include "ModDelay.h"
#include "Householder.h"
#include <stdlib.h>

#define DEFAULT_NUMBER_OF_CHANNELS_MCF 4
#define _DELAY_MAKEUP_GAIN_VALUE 1.5

using namespace std;

class ModMultiChannelFeedback {

protected:

	int mcf_numberOfChannels;
	int mcf_sampleRate;
	float mcf_minDelayLength, mcf_maxDelayLength;
	float mcf_delayBufferSizeMs;
	float mcf_decay;
	DelayDistribution mcf_delayDistribution;
	Householder* mcf_Householder;
	vector<ModDelay*> mcf_DelayLines;
	float mcf_modValmsec;

public:

	ModMultiChannelFeedback() {
		constructMCF(DEFAULT_NUMBER_OF_CHANNELS_MCF);
	}

	ModMultiChannelFeedback(int numCH) {
		constructMCF(numCH);
	}

	~ModMultiChannelFeedback() {
		deleteDelayLines();
		delete mcf_Householder;
	}

	void setNumberOfChannels(int numCh) {
		mcf_numberOfChannels = numCh;
		mcf_Householder->setNumberOfChannels(numCh);
		deleteDelayLines();
		allocateDelayLines();
		init(mcf_delayBufferSizeMs, mcf_sampleRate);
		setDelayLengths(mcf_minDelayLength, mcf_maxDelayLength, mcf_delayDistribution);
		setDecayInSeconds(mcf_decay);
	}

	void setDecayInSeconds(float decay) {
		mcf_decay = decay;
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setFeedbackFromDecay(decay);
	}

	void init(float delayMs, int sampleRate) {
		mcf_delayBufferSizeMs = delayMs;
		mcf_sampleRate = sampleRate;
		for (int i = 0; i < mcf_numberOfChannels; i++) {
			mcf_DelayLines[i]->init(delayMs, sampleRate);
			mcf_DelayLines[i]->setMakeUpGaindB(_DELAY_MAKEUP_GAIN_VALUE);
		}
	}

	void setSampleRate(int sampleRate) {
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setSampleRate(sampleRate);
	}

	void setDelayLengths(float minDelay, float maxDelay = 0.0, DelayDistribution distr = DelayDistribution::Exponential) {
		mcf_minDelayLength = minDelay;
		if (maxDelay != 0.0)
			mcf_maxDelayLength = maxDelay;
		mcf_delayDistribution = distr;
		switch (distr) {
		case DelayDistribution::Exponential: {
			setDelayExponential();
			break;
		}
		case DelayDistribution::RandomInRange: {
			setRandomInRangeDelayLines();
			break;
		}
		}
	}

	void setDampingFrequency(float freq) {
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setCutoffFrequency(freq);
	}

	void setFilterType(LPFilterType type) {
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setFilterType(type);
	}

	void setModValue(float val) {
		mcf_modValmsec = val;
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setDeltaDelayValue(val);
	}
	void setModDepth(float depth) {
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setDeltaDelayValue(depth* mcf_modValmsec);
	}

	void setModRate(float freq) {
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setModRate(freq);
	}

	void setOscillatorType(OscillatorType type) {
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setLFOWaveform(type);
	}

	void setOscillatorIsUnipolar(bool isUnipolar) {
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setLFOUnipolar(isUnipolar);
	}

	vector<float> getDelayLengths() {
		vector<float> delays(mcf_numberOfChannels);
		for (int i = 0; i < delays.size(); i++)
			delays[i] = mcf_DelayLines[i]->getDelayLength();
		return delays;
	}

	float getMeanDelayLength() {
		float del = 0.0;
		for (int i = 0; i < mcf_numberOfChannels; i++)
			del += mcf_DelayLines[i]->getDelayLength();
		return del / mcf_numberOfChannels;
	}

	void processAudio(float* in, float* out) {
		vector<float> houseout(mcf_numberOfChannels);
		vector<float> housein(mcf_numberOfChannels);				

		// Read value from delay line and allocate it to output
		for (int i = 0; i < mcf_numberOfChannels; i++) {
			float newDelayInmsec = mcf_DelayLines[i]->mdly_meanDelayValue + mcf_DelayLines[i]->mdly_deltaDelayValue * mcf_DelayLines[i]->mdly_LFO->processAudio();

			// Set the delay value to the delay line
			mcf_DelayLines[i]->LPCombFilter::setDelayInmsec(newDelayInmsec);

			// Compute the total delay value in milliseconds
			out[i] = mcf_DelayLines[i]->readFromDelayLine();
		}

		// Write the processed feedback value to delay lines
		for (int i = 0; i < mcf_numberOfChannels; i++) {
			housein[i] = mcf_DelayLines[i]->processLowPass(out[i]) * mcf_DelayLines[i]->getFeedback();
		}

		// Apply householder transformation
		mcf_Householder->processAudio(&housein[0], &houseout[0]);

		for (int i = 0; i < mcf_numberOfChannels; i++) {
			mcf_DelayLines[i]->writeToDelayLine(in[i] + houseout[i]);
			mcf_DelayLines[i]->updateIndices();
		}
	}

private:

	void constructMCF(int numCh) {
		mcf_Householder = new Householder(numCh);
		setNumberOfChannels(numCh);
		mcf_numberOfChannels = numCh;
		mcf_Householder->setNumberOfChannels(numCh);
		allocateDelayLines();
	}

	void deleteDelayLines() {
		if (!mcf_DelayLines.empty())
			for (int i = 0; i < mcf_DelayLines.size(); i++)
				delete mcf_DelayLines[i];
		mcf_DelayLines.clear();
	}

	void allocateDelayLines() {
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines.push_back(new ModDelay);
	}

	void setDelayExponential() {
		vector<float> expo(mcf_numberOfChannels);
		expo = exponentialVector(mcf_minDelayLength, mcf_maxDelayLength, mcf_numberOfChannels);
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setDelayInmsec(expo[i]);
	}

	void setRandomInRangeDelayLines() {
		float step = (mcf_maxDelayLength - mcf_minDelayLength) / mcf_numberOfChannels;
		float dlyLength;
		for (int i = 0; i < mcf_numberOfChannels; i++) {
			dlyLength = randomInRange(mcf_minDelayLength + step * i, mcf_minDelayLength + step * (i + 1));
			mcf_DelayLines[i]->setDelayInmsec(dlyLength);
		}
	}

};

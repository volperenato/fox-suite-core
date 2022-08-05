#pragma once
#include "CombFilter.h"
#include "Householder.h"
#include <stdlib.h>

#define DEFAULT_NUMBER_OF_CHANNELS_MCF 4

using namespace std;

class MultiChannelFeedback {

protected:

	int mcf_numberOfChannels;
	float mcf_minDelayLength, mcf_maxDelayLength;
	DelayDistribution mcf_distributionDelay;
	Householder* mcf_Householder;
	vector<CombFilter*> mcf_DelayLines;

public:

	MultiChannelFeedback() { 
		constructMCF(DEFAULT_NUMBER_OF_CHANNELS_MCF); 
	}

	MultiChannelFeedback(int numCH) { 
		constructMCF(numCH);
	}

	~MultiChannelFeedback() { 
		deleteDelayLines();  
		delete mcf_Householder;
	}

	void setNumberOfChannels(int numCh) {
		mcf_numberOfChannels = numCh;
		mcf_Householder->setNumberOfChannels(numCh);
		deleteDelayLines();
		allocateDelayLines();
	}

	void setDecayInSeconds(float decay) {
		for (int i = 0; i < mcf_numberOfChannels; i++) 
			mcf_DelayLines[i]->setFeedbackFromDecay(decay);
	}	

	void initDelayLines(float delayMs, int sampleRate) {
		for (int i = 0; i < mcf_numberOfChannels; i++) 
			mcf_DelayLines[i]->init(delayMs, sampleRate);
	}

	void setSampleRate(int sampleRate) {
		for (int i = 0; i < mcf_numberOfChannels; i++) 
			mcf_DelayLines[i]->setSampleRate(sampleRate);
	}

	void setDelayLengths(float minDelay, float maxDelay, DelayDistribution distr = DelayDistribution::Exponential) {
		mcf_minDelayLength = minDelay;
		mcf_maxDelayLength = maxDelay;
		mcf_distributionDelay = distr;
		switch (distr) {
		case DelayDistribution::Exponential: {
			setDelayExponential();
			break;
		}
		}
	}

	void processAudio(float* in, float* out) {
		vector<float> houseout(mcf_numberOfChannels);
		vector<float> housein(mcf_numberOfChannels);

		// Read value from delay line and allocate it to output
		for (int i = 0; i < mcf_numberOfChannels; i++) 
			housein[i] = mcf_DelayLines[i]->readFromDelayLine();

		// Apply householder transformation
		mcf_Householder->processAudio(&housein[0], &houseout[0]);
		
		// Write the processed feedback value to delay lines
		for (int i = 0; i < mcf_numberOfChannels; i++) {
			float tmp = in[i] + houseout[i] * mcf_DelayLines[i]->getFeedback();
			mcf_DelayLines[i]->writeToDelayLine(tmp);
			mcf_DelayLines[i]->updateIndices();
		}
		std::copy(housein.begin(), housein.end(), out);
	}

private:

	void constructMCF(int numCh) {
		mcf_Householder = new Householder(numCh);
		setNumberOfChannels(numCh);
	}

	void deleteDelayLines() {
		if (!mcf_DelayLines.empty())
			for (int i = 0; i < mcf_DelayLines.size(); i++) 
				delete mcf_DelayLines[i];
		mcf_DelayLines.clear();
	}

	void allocateDelayLines() {
		for (int i = 0; i < mcf_numberOfChannels; i++) 
			mcf_DelayLines.push_back(new CombFilter);
	}

	void setDelayExponential() {
		std::vector<float> expo(mcf_numberOfChannels);
		expo = exponentialVector(mcf_minDelayLength, mcf_maxDelayLength, mcf_numberOfChannels);
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setDelayInmsec(expo[i]);
	}

};

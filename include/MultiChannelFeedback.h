#pragma once
#include "LPCombFilter.h"
#include "Householder.h"
#include <stdlib.h>

#define DEFAULT_NUMBER_OF_CHANNELS_MCF 4

using namespace std;

class MultiChannelFeedback {

protected:

	int mcf_numberOfChannels;
	int mcf_sampleRate;
	float mcf_minDelayLength, mcf_maxDelayLength;
	float mcf_delayBufferSizeMs;
	float mcf_decay;
	DelayDistribution mcf_delayDistribution;
	Householder* mcf_Householder;
	vector<LPCombFilter*> mcf_DelayLines;

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
		initDelayLines(mcf_delayBufferSizeMs, mcf_sampleRate);
		setDelayLengths(mcf_minDelayLength, mcf_maxDelayLength, mcf_delayDistribution);
		setDecayInSeconds(mcf_decay);
	}

	void setDecayInSeconds(float decay) {
		mcf_decay = decay;
		for (int i = 0; i < mcf_numberOfChannels; i++) 
			mcf_DelayLines[i]->setFeedbackFromDecay(decay);
	}	

	void initDelayLines(float delayMs, int sampleRate) {
		mcf_delayBufferSizeMs = delayMs;
		mcf_sampleRate = sampleRate;
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
		mcf_delayDistribution = distr;
		switch (distr) {
		case DelayDistribution::Exponential: {
			setDelayExponential();
			break;
		}
		}
	}

	void setDampingFrequency(float freq) {
		for (int i = 0; i < mcf_numberOfChannels; i++) 
			mcf_DelayLines[i]->setCutoffFrequency(freq);
	}

	//void processAudio(float* in, float* out) {
	//	vector<float> houseout(mcf_numberOfChannels);
	//	vector<float> housein(mcf_numberOfChannels);

	//	// Read value from delay line and allocate it to output
	//	for (int i = 0; i < mcf_numberOfChannels; i++) 
	//		housein[i] = mcf_DelayLines[i]->readFromDelayLine();

	//	// Apply householder transformation
	//	mcf_Householder->processAudio(&housein[0], &houseout[0]);
	//	
	//	// Write the processed feedback value to delay lines
	//	for (int i = 0; i < mcf_numberOfChannels; i++) {
	//		float tmp = in[i] + mcf_DelayLines[i]->processLowPass(houseout[i]) * mcf_DelayLines[i]->getFeedback();
	//		mcf_DelayLines[i]->writeToDelayLine(tmp);
	//		mcf_DelayLines[i]->updateIndices();
	//	}
	//	std::copy(housein.begin(), housein.end(), out);
	//}

	void processAudio(float* in, float* out) {
		vector<float> houseout(mcf_numberOfChannels);
		vector<float> housein(mcf_numberOfChannels);

		// Read value from delay line and allocate it to output
		for (int i = 0; i < mcf_numberOfChannels; i++)
			out[i] = mcf_DelayLines[i]->readFromDelayLine();		

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

		//std::copy(housein.begin(), housein.end(), out);
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
			mcf_DelayLines.push_back(new LPCombFilter);
	}

	void setDelayExponential() {
		std::vector<float> expo(mcf_numberOfChannels);
		expo = exponentialVector(mcf_minDelayLength, mcf_maxDelayLength, mcf_numberOfChannels);
		for (int i = 0; i < mcf_numberOfChannels; i++)
			mcf_DelayLines[i]->setDelayInmsec(expo[i]);
	}

};

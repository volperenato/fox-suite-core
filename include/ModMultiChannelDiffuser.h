#pragma once
#include "ModMultiChannelDelay.h"
#include "Hadamard.h"
#include "FlipPolarity.h"
#include "utils.h"
#include <string>

#define DEFAULT_NUMBER_OF_CHANNELS_MDIFF 4
#define _DELAY_MAKEUP_GAIN_VALUE 1.5

class ModMultiChannelDiffuser {

protected:

	int mdiff_numberOfChannels;
	int mdiff_sampleRate;
	float* mdiff_outMultiChDel, * mdiff_outHadamard;
	ModMultiChannelDelay* mdiff_MultiChDelay;
	Hadamard* mdiff_Hadamard;
	FlipPolarity* mdiff_Polarity;

public:

	ModMultiChannelDiffuser() {
		constructMCDF(DEFAULT_NUMBER_OF_CHANNELS_MDIFF);
	}

	ModMultiChannelDiffuser(int numChInt) {
		constructMCDF(numChInt);
	}

	~ModMultiChannelDiffuser() {
		deleteBlocks();
		deleteInternalArrays();
	}

	void init(float bufferLengthMs, int sampleRate) {
		mdiff_sampleRate = sampleRate;
		mdiff_MultiChDelay->initDelayLines(bufferLengthMs, sampleRate);
		setMakeUpGainDB(_DELAY_MAKEUP_GAIN_VALUE);
	}

	void setNumberOfInternalChannels(int numChInt) {
		mdiff_numberOfChannels = numChInt;
		mdiff_MultiChDelay->setNumberOfChannels(numChInt);
		mdiff_Hadamard->setNumberOfChannels(numChInt);
		mdiff_Polarity->setNumberOfChannels(numChInt);
		resetInternalArrays();
	}

	void setDelayLinesLength(float minDelayMs, float maxDelayMs, DelayDistribution distr = DelayDistribution::RandomInRange) {
		mdiff_MultiChDelay->setDelayLinesLength(minDelayMs, maxDelayMs, distr);
	}

	void setAllDelayLenghtsMs(float* delayLenghts) { mdiff_MultiChDelay->setSpecificDelayLengthsMs(delayLenghts); }

	void setMakeUpGainDB(float makeUpGain) { mdiff_MultiChDelay->setMakeUpGaindB(makeUpGain); }

	void setModDepth(float depth) {
		mdiff_MultiChDelay->setModDepth(depth);
	}

	void setModRate(float rate) {
		mdiff_MultiChDelay->setModRate(rate);
	}

	void setModValue(float val) {
		mdiff_MultiChDelay->setModValueInMsec(val);
	}

	void setOscillatorType(OscillatorType type) {
		mdiff_MultiChDelay->setOscillatorType(type);
	}

	void setOscillatorIsUnipolar(bool isUnipolar) {
		mdiff_MultiChDelay->setOscillatorIsUnipolar(isUnipolar);
	}

	void setSampleRate(int sampleRate) {
		mdiff_sampleRate = sampleRate;
		mdiff_MultiChDelay->setSampleRate(sampleRate);
	}

	void processAudio(float* in, float* out) {
		mdiff_MultiChDelay->processAudio(in, mdiff_outMultiChDel);
		mdiff_Polarity->processAudio(mdiff_outMultiChDel, mdiff_outMultiChDel);
		mdiff_Hadamard->processAudio(mdiff_outMultiChDel, out);
	}

private:

	void constructMCDF(int numCh) {
		mdiff_numberOfChannels = numCh;
		initPointers();
		constructBlocks();
		initInternalArrays();
	}

	void initInternalArrays() {
		int lenghtInBytes = mdiff_numberOfChannels * sizeof(float);
		mdiff_outMultiChDel = (float*)malloc(lenghtInBytes);
		mdiff_outHadamard = (float*)malloc(lenghtInBytes);
		memset(mdiff_outMultiChDel, 0, lenghtInBytes);
		memset(mdiff_outHadamard, 0, lenghtInBytes);
	}

	void resetInternalArrays() {
		deleteInternalArrays();
		initInternalArrays();
	}

	void deleteInternalArrays() {
		if (mdiff_outMultiChDel)
			free(mdiff_outMultiChDel);
		if (mdiff_outHadamard)
			free(mdiff_outHadamard);
		initPointers();
	}

	void initPointers() {
		mdiff_outHadamard = nullptr;
		mdiff_outMultiChDel = nullptr;
	}

	void deleteBlocks() {
		delete mdiff_MultiChDelay;
		delete mdiff_Hadamard;
		delete mdiff_Polarity;
	}

	void constructBlocks() {
		mdiff_MultiChDelay = new ModMultiChannelDelay(mdiff_numberOfChannels);
		mdiff_Hadamard = new Hadamard(mdiff_numberOfChannels);
		mdiff_Polarity = new FlipPolarity(mdiff_numberOfChannels);
	}

};


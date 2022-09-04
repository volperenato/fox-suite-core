#pragma once
#include "ModDelay.h"
#include "utils.h"
#include "constants.h"
#include <vector>

#define DEFAULT_NUM_OF_CHANNELS_MDEL 4
#define DEFAULT_DELAY_BUFFER_LENGTHS 500.0
#define MINIMUM_DELAY_LENGTH_MCD 0.1
#define DEFAULT_MOD_VALUE 30.0

using namespace std;

class ModMultiChannelDelay {

protected:

	int mdel_numberOfChannels;
	vector<ModDelay*> mdel_DelayLines;
	float mdel_minDelayMs, mdel_maxDelayMs;
	int mdel_sampleRate;
	DelayDistribution mdel_delayDistr;
	float mdel_modValmsec;

public:

	ModMultiChannelDelay() {
		constructMCDL(DEFAULT_NUM_OF_CHANNELS_MDEL);
	}

	ModMultiChannelDelay(int numCh) {
		constructMCDL(numCh);
	}

	~ModMultiChannelDelay() { deleteDelayLines(); }

	void initDelayLines(float bufferLengthMs, int sampleRate) {
		mdel_sampleRate = sampleRate;
		if (!mdel_DelayLines.empty()) {
			for (int i = 0; i < mdel_numberOfChannels; i++)
				mdel_DelayLines[i]->init(bufferLengthMs, sampleRate);
		}
	}

	void setNumberOfChannels(int numCH) {
		float dlyBufferSize = mdel_DelayLines[0]->getBufferSizeMs();
		int sampleRate = mdel_DelayLines[0]->getSampleRate();
		deleteDelayLines();
		mdel_numberOfChannels = numCH;
		constructDelayObjects();
		initDelayLines(dlyBufferSize, sampleRate);
		setDelayLinesLength(mdel_minDelayMs, mdel_maxDelayMs, mdel_delayDistr);
	}

	void setDelayLinesLength(float dlyMinLengthMs, float dlyMaxLengthMs, DelayDistribution distr = DelayDistribution::RandomInRange) {
		mdel_minDelayMs = dlyMinLengthMs;
		mdel_maxDelayMs = dlyMaxLengthMs;
		/*if (mdel_maxDelayMs / mdel_numberOfChannels < MINIMUM_DELAY_LENGTH_MCD)
			mdel_maxDelayMs = 2 * MINIMUM_DELAY_LENGTH_MCD;*/
		mdel_delayDistr = distr;
		switch (distr) {
		case DelayDistribution::RandomInRange: {
			setRandomInRangeDelayLines();
			break;
		}
		case DelayDistribution::Exponential: {
			setExponentialDelayLengths();
			break;
		}
		case DelayDistribution::Equal: {
			setEqualDelayLines();
			break;
		}
		}
	}

	void setSpecificDelayLengthsMs(float* lenghts) {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setDelayInmsec(lenghts[i]);
	}

	void setMakeUpGaindB(float makeUpGain) {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setMakeUpGaindB(makeUpGain);
	}

	void setModDepth(float depth) {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setDeltaDelayValue(depth*mdel_modValmsec);
	}

	void setModRate(float freq) {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setModRate(freq);
	}	
	
	void setModValueInMsec(float modVal) {
		mdel_modValmsec = modVal;
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setDeltaDelayValue(modVal);
	}

	void setOscillatorType(OscillatorType type) {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setLFOWaveform(type);
	}

	void setOscillatorIsUnipolar(bool isUnipolar) {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setLFOUnipolar(isUnipolar);
	}

	void setSampleRate(int sampleRate) {
		for (int i = 0; i < mdel_numberOfChannels; i++) {
			mdel_sampleRate = sampleRate;
			mdel_DelayLines[i]->setSampleRate(sampleRate);
		}
	}

	void processAudio(float* in, float* out) {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			out[i] = mdel_DelayLines[i]->processAudio(in[i]);
	}

private:

	void setEqualDelayLines() {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setDelayInmsec(mdel_maxDelayMs);
	}

	void setRandomInRangeDelayLines() {
		float step = (mdel_maxDelayMs - MINIMUM_DELAY_LENGTH_MCD) / mdel_numberOfChannels;
		float dlyLength;
		for (int i = 0; i < mdel_numberOfChannels; i++) {
			dlyLength = randomInRange(MINIMUM_DELAY_LENGTH_MCD + step * i, MINIMUM_DELAY_LENGTH_MCD + step * (i + 1));
			mdel_DelayLines[i]->setDelayInmsec(dlyLength);
		}
	}

	void setExponentialDelayLengths() {
		vector<float> dly(mdel_numberOfChannels);
		dly = exponentialVector(0.0, mdel_maxDelayMs, mdel_numberOfChannels);
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines[i]->setDelayInmsec(dly[i]);
		dly.clear();
	}

	void deleteDelayLines() {
		if (!mdel_DelayLines.empty()) {
			for (int i = 0; i < mdel_DelayLines.size(); i++)
				delete mdel_DelayLines[i];
			mdel_DelayLines.clear();
		}
	}

	void constructDelayObjects() {
		for (int i = 0; i < mdel_numberOfChannels; i++)
			mdel_DelayLines.push_back(new ModDelay);
	}

	void constructMCDL(int numCh) {
		mdel_numberOfChannels = numCh;
		mdel_minDelayMs = 0.0;
		mdel_maxDelayMs = 0.0;
		constructDelayObjects();
		setModValueInMsec(DEFAULT_MOD_VALUE);
	}


};

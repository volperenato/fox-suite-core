#pragma once
#include "Delay.h"
#define DEFAULT_NUM_OF_CHANNELS_MDEL 4

class MultiChannelDelay {

protected:

	int mdel_numberOfChannels;
	Delay* mdel_DelayLines;

public:

	MultiChannelDelay() {
		mdel_DelayLines = nullptr;
		setNumberOfChannels(DEFAULT_NUM_OF_CHANNELS_MDEL); 
	}

	MultiChannelDelay(int numCh) { 
		mdel_DelayLines = nullptr;
		setNumberOfChannels(numCh); 
	}

	~MultiChannelDelay() { deleteDelayLines(); }

	void deleteDelayLines() {
		if (mdel_DelayLines) {
			for (int i = 0; i < mdel_numberOfChannels; i++) mdel_DelayLines[i].~Delay();
			delete[] mdel_DelayLines;
		}
		mdel_DelayLines = nullptr;
	}

	void setNumberOfChannels(int numCH) {		
		deleteDelayLines();
		mdel_numberOfChannels = numCH;
		mdel_DelayLines = new Delay[mdel_numberOfChannels];
	}

	void init(float dlyBufferLength, int sampleRate) {
		for (int i = 0; i < mdel_numberOfChannels; i++) {
			mdel_DelayLines[i].init(dlyBufferLength, sampleRate);
		}
	}

	void setDelayLengthMs(float* lenghts) {
		for (int i = 0; i < mdel_numberOfChannels; i++)  mdel_DelayLines[i].setDelayInmsec(lenghts[i]);
	}

	void setMakeUpGaindB(float* makeUpGains) {
		for (int i = 0; i < mdel_numberOfChannels; i++) mdel_DelayLines[i].setMakeUpGaindB(makeUpGains[i]);
	}

	void setSampleRate(int sampleRate) {
		for (int i = 0; i < mdel_numberOfChannels; i++) mdel_DelayLines[i].setSampleRate(sampleRate);
	}

	void processAudio(float* in, float* out) {
		for (int i = 0; i < mdel_numberOfChannels; i++) out[i] = mdel_DelayLines[i].processAudio(in[i]);
	}
	
};

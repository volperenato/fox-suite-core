#pragma once
#include "MultiChannelDiffuser.h"

class FDN {

protected:

	MultiChannelDiffuser* fdn_diffuser;
	int fdn_numberOfInputChannels, fdn_numberOfInternalChannels, fdn_numberOfOutputChannels;	

private:

	void initPointers() {
		fdn_diffuser = nullptr;
	}

	void allocateBlocks() {
		fdn_diffuser = new MultiChannelDiffuser();
	}

public:

	FDN() { 
		initPointers(); 
		allocateBlocks();
	};

	FDN(int numChIn, int numChInt, int numChOut) {
		initPointers();
		allocateBlocks();
		setNumberOfInputChannels(numChIn);
		setNumberOfInternalChannels(numChInt);
		setNumberOfOutputChannels(numChOut);
	}

	~FDN() { fdn_diffuser->~MultiChannelDiffuser(); }

	void init(float delayMaxLength, int sampleRate) {
		fdn_diffuser->init(delayMaxLength, sampleRate);
	};

	void setNumberOfInputChannels(int numChIn) {
		fdn_numberOfInputChannels = numChIn;
		fdn_diffuser->setNumberOfInputChannels(numChIn);
	}

	void setNumberOfInternalChannels(int numChInt) {
		fdn_numberOfInternalChannels = numChInt;
		fdn_diffuser->setNumberOfInternalChannels(numChInt);
	}

	void setNumberOfOutputChannels(int numChOut) {
		fdn_numberOfOutputChannels = numChOut;
		fdn_diffuser->setNumberOfOutputChannels(numChOut);
	}

	void setSampleRate(int sampleRate) {
		fdn_diffuser->setSampleRate(sampleRate);
	}
	
	void setDiffuserDelayLengths(float* delayLengths) {
		fdn_diffuser->setDelayLenghts(delayLengths);
	}	

	void processAudio(float* in, float* out) {
		fdn_diffuser->processAudio(in, out);
	};

};
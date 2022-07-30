#pragma once
#include "ChannelSplitter.h"
#include "MultiChannelDelay.h"
#include "Hadamard.h"
#include "ChannelMixer.h"
#include <string>

#define DEFAULT_NUMBER_OF_CHANNELS_MDIFF_IN 1
#define DEFAULT_NUMBER_OF_CHANNELS_MDIFF_INTERNAL 4
#define DEFAULT_NUMBER_OF_CHANNELS_MDIFF_OUT 1

class MultiChannelDiffuser {

protected:

	int mdiff_numberOfInputChannels, mdiff_numberOfInternalChannels, mdiff_numberOfOutputChannels;
	float* mdiff_outSplitter, *mdiff_outMultiChDel, * mdiff_outHadamard;
	ChannelSplitter* mdiff_Splitter;
	MultiChannelDelay* mdiff_MultiChDelay;
	Hadamard* mdiff_Hadamard;
	ChannelMixer* mdiff_Mixer;

public:

	MultiChannelDiffuser() { 
		initPointers();
		allocateBlocks();
		setNumberOfInputChannels(DEFAULT_NUMBER_OF_CHANNELS_MDIFF_IN);
		setNumberOfInternalChannels(DEFAULT_NUMBER_OF_CHANNELS_MDIFF_INTERNAL);
		setNumberOfOutputChannels(DEFAULT_NUMBER_OF_CHANNELS_MDIFF_OUT);
	}

	MultiChannelDiffuser(int numChIn, int numChInt, int numChOut) { 
		initPointers();
		allocateBlocks();
		setNumberOfInputChannels(numChIn);
		setNumberOfInternalChannels(numChInt);
		setNumberOfOutputChannels(numChOut);
	}

	~MultiChannelDiffuser() { 
		deleteBlocks();
		deleteInternalArrays();
	}	

	void setNumberOfInputChannels(int numChIn) { 
		mdiff_numberOfInputChannels = numChIn;
		mdiff_Splitter->setNumberOfChannelsIn(numChIn); 
	}

	void setNumberOfInternalChannels(int numChInt) {	
		mdiff_numberOfInternalChannels = numChInt;
		mdiff_Splitter->setNumberOfChannelsOut(numChInt);
		mdiff_MultiChDelay->setNumberOfChannels(numChInt);
		mdiff_Hadamard->setNumberOfChannels(numChInt);
		mdiff_Mixer->setNumberOfInputChannels(numChInt);
		initInternalArrays();
	}

	void setNumberOfOutputChannels(int numChOut) { 
		mdiff_numberOfOutputChannels = numChOut;
		mdiff_Mixer->setNumberOfOutputChannels(numChOut); 
	}	

	void init(float delayMaxLength, int sampleRate) { mdiff_MultiChDelay->init(delayMaxLength, sampleRate);	}

	void setDelayLenghts(float* delayLenghts) {	mdiff_MultiChDelay->setDelayLengthMs(delayLenghts);	}

	void setMakeUpGainDB(float* makeUpGains) { mdiff_MultiChDelay->setMakeUpGaindB(makeUpGains); }

	void setSampleRate(int sampleRate) { mdiff_MultiChDelay->setSampleRate(sampleRate); }

	void processAudio(float* in, float* out) {
		mdiff_Splitter->processAudio(in, mdiff_outSplitter);		
		mdiff_MultiChDelay->processAudio(mdiff_outSplitter, mdiff_outMultiChDel);
		mdiff_Hadamard->processAudio(mdiff_outMultiChDel, mdiff_outHadamard);
		mdiff_Mixer->processAudio(mdiff_outHadamard, out);
	}

private:

	void initInternalArrays() {
		int lenghtInBytes = mdiff_numberOfInternalChannels * sizeof(float);
		if (!mdiff_outSplitter || !mdiff_outMultiChDel)
			allocateInternalArrays(lenghtInBytes);

		resetInternalArrays(lenghtInBytes);
	}

	void allocateInternalArrays(int lenghtInBytes) {
		// allocate memory
		mdiff_outSplitter = (float*)malloc(lenghtInBytes);
		mdiff_outMultiChDel = (float*)malloc(lenghtInBytes);
		mdiff_outHadamard = (float*)malloc(lenghtInBytes);
	}

	void resetInternalArrays(int lenghtInBytes) {
		// set the allocated memory at zero
		memset(mdiff_outSplitter, 0, lenghtInBytes);
		memset(mdiff_outMultiChDel, 0, lenghtInBytes);
		memset(mdiff_outHadamard, 0, lenghtInBytes);
	}

	void deleteInternalArrays() {
		if (mdiff_outSplitter)
			delete mdiff_outSplitter;
		if (mdiff_outMultiChDel)
			delete mdiff_outMultiChDel;
		if (mdiff_outHadamard)
			delete mdiff_outHadamard;
		mdiff_outSplitter = nullptr;
		mdiff_outMultiChDel = nullptr;
		mdiff_outHadamard = nullptr;
	}

	void initPointers() {
		mdiff_Splitter = nullptr;
		mdiff_MultiChDelay = nullptr;
		mdiff_Hadamard = nullptr;
		mdiff_Mixer = nullptr;
		mdiff_outSplitter = nullptr;
		mdiff_outMultiChDel = nullptr;
	}

	void deleteBlocks() {
		if (mdiff_Splitter) {
			mdiff_Splitter->~ChannelSplitter();
			delete mdiff_Splitter;
			mdiff_Splitter = nullptr;
		}

		if (mdiff_MultiChDelay) {
			mdiff_MultiChDelay->~MultiChannelDelay();
			delete mdiff_MultiChDelay;
			mdiff_MultiChDelay = nullptr;
		}

		if (mdiff_Hadamard) {
			mdiff_Hadamard->~Hadamard();
			delete mdiff_Hadamard;
			mdiff_Hadamard = nullptr;
		}

		if (mdiff_Mixer) {
			mdiff_Mixer->~ChannelMixer();
			delete mdiff_Mixer;
			mdiff_Mixer = nullptr;
		}
	}

	void allocateBlocks() {
		mdiff_Splitter = new ChannelSplitter();
		mdiff_MultiChDelay = new MultiChannelDelay();
		mdiff_Hadamard = new Hadamard();
		mdiff_Mixer = new ChannelMixer();
	}

};


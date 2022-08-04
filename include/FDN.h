#pragma once
#include "MultiChannelDiffuser.h"
#include "ChannelSplitter.h"
#include "ChannelMixer.h"
#include "constants.h"

#define DEFAULT_DIFFUSION_STEPS 1
#define DEFAULT_NUMBER_INPUT_CHANNELS_FDN 1
#define DEFAULT_NUMBER_DIFFUSION_CHANNELS_FDN 8
#define DEFAULT_NUMBER_OUTPUT_CHANNELS_FDN 1
#define DEFAULT_DIFFUSER_DELAY_DISTRIBUTION DelayDistribution::RandomInRange
#define DEFAULT_DIFFUSER_DELAY_LOGIC DiffuserDelayLogic::Doubled

using namespace std;

enum class DiffuserDelayLogic {
	Doubled,
	Equal
};

class FDN {

protected:

	ChannelSplitter* fdn_splitter;
	ChannelMixer* fdn_mixer;
	vector<MultiChannelDiffuser*> fdn_diffuser;
	int fdn_inputChannels, fdn_internalChannels, fdn_outputChannels;
	int fdn_diffusionSteps;
	float* fdn_spOut, *fdn_dlOut;
	int fdn_sampleRate;	

public:

	// Class constructors
	FDN() { 				
		constructFDN(DEFAULT_NUMBER_INPUT_CHANNELS_FDN, DEFAULT_NUMBER_DIFFUSION_CHANNELS_FDN, DEFAULT_NUMBER_OUTPUT_CHANNELS_FDN, DEFAULT_DIFFUSION_STEPS, _TEMPLATE_SAMPLERATE);
	}

	FDN(int numChIn, int numChInt, int numChOut) {
		constructFDN(numChIn, numChInt, numChOut, DEFAULT_DIFFUSION_STEPS, _TEMPLATE_SAMPLERATE);
	}

	FDN(int numChIn, int numChInt, int numChOut, int numDiffStep) {						
		constructFDN(numChIn, numChInt, numChOut, numDiffStep, _TEMPLATE_SAMPLERATE);
	}

	FDN(int numChIn, int numChInt, int numChOut, int numDiffStep, int sampleRate) {
		constructFDN(numChIn, numChInt, numChOut, numDiffStep, sampleRate);
	}

	// Class destructor
	~FDN() { 
		deleteInterfaceBlocks();
		deleteDiffusionBlocks();		
		deleteInternalArrays();
	}	

	// Initialize FDN elements
	void initialize(float diffusionMaximumLength, int sampleRate) {
		for (int i = 0; i < fdn_diffusionSteps; i++)
			fdn_diffuser[i]->init(diffusionMaximumLength, sampleRate);
	}

	// Set the length of delay lines in the diffusion blocks
	void setDiffuserDelayLengths(float delayMaxLength, DiffuserDelayLogic logic = DEFAULT_DIFFUSER_DELAY_LOGIC, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		switch (logic) {
		case DiffuserDelayLogic::Doubled: {
			setDoubledDiffuserDelayLengths(delayMaxLength, distr);
			break;
		}
		case DiffuserDelayLogic::Equal: {
			setEqualDiffuserDelayLengths(delayMaxLength, distr);
			break;
		}
		}
	}

	// Set the number of input channels
	void setNumberOfInputChannels(int numChIn) {
		fdn_inputChannels = numChIn;
		fdn_splitter->setNumberOfChannelsIn(fdn_inputChannels);
		fdn_splitter->setNumberOfChannelsOut(fdn_internalChannels);
	}

	// Set the number of internal channels (for processing purposes)
	void setNumberOfInternalChannels(int numChInt) {
		fdn_internalChannels = numChInt;
		for (int i = 0; i < fdn_diffusionSteps; i++) fdn_diffuser[i]->setNumberOfInternalChannels(numChInt);
	}

	// Set the number of output channels
	void setNumberOfOutputChannels(int numChOut) {
		fdn_outputChannels = numChOut;
		fdn_mixer->setNumberOfInputChannels(fdn_internalChannels);
		fdn_mixer->setNumberOfOutputChannels(fdn_outputChannels);
	}
		
	// Set the sample rate
	void setSampleRate(int sampleRate) { 
		fdn_sampleRate = sampleRate;
		for (int i = 0; i < fdn_diffusionSteps; i++) fdn_diffuser[i]->setSampleRate(sampleRate); 
	}	

	// Set output mixing mode
	void setMixMode(MixMode mode) { fdn_mixer->setMixMode(mode); }

	// Process Audio
	void processAudio(float* in, float* out) { 
		fdn_splitter->processAudio(in, fdn_spOut);
		for (int i = 0; i < fdn_diffusionSteps; i++) fdn_diffuser[i]->processAudio(fdn_spOut, fdn_spOut);
		fdn_mixer->processAudio(fdn_spOut, out);
	};

private:

	// Set the length of diffusion steps each equal to double the previous
	void setDoubledDiffuserDelayLengths(float delayMaxLength, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		for (int i = fdn_diffusionSteps - 1; i >= 0; i--) {
			fdn_diffuser[i]->setDelayLinesLength(delayMaxLength, distr);
			delayMaxLength *= 0.5;
		}
	}

	// Set the length of diffusion steps each equal to the previous
	void setEqualDiffuserDelayLengths(float delayLengths, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		for (int i = 0; i < fdn_diffusionSteps; i++) fdn_diffuser[i]->setDelayLinesLength(delayLengths, distr);
	}

	// Function called in the class constructors
	void constructFDN(int numChIn, int numChInt, int numChOut, int numDiffStep, int sampleRate) {
		srand(_SEED_FOR_RAND_GENERATION);
		fdn_inputChannels = numChIn;
		fdn_internalChannels = numChInt;
		fdn_outputChannels = numChOut;
		fdn_diffusionSteps = numDiffStep;		
		constructInterfaceBlocks();
		constructDiffusionBlocks();
		initInternalArrays();
		setSampleRate(sampleRate);
	}	

	// Construct objects used for in&out interfaces purposes (ChannelSplitter and ChannelMixer)
	void constructInterfaceBlocks() {
		fdn_splitter = new ChannelSplitter(fdn_inputChannels, fdn_internalChannels);
		fdn_mixer = new ChannelMixer(fdn_internalChannels, fdn_outputChannels);
	}

	// Construct objects used for diffusion purposes and set their internal channels correctly (MultiChannelDiffuser)
	void constructDiffusionBlocks() {
		deleteDiffusionBlocks();				

		// Create each MultiChannelDiffuser object, with custom constructor)
		for (int i = 0; i < fdn_diffusionSteps; i++) {
			fdn_diffuser.push_back(new MultiChannelDiffuser(fdn_internalChannels));
		}
	}

	// Delete objects used for in&out interfaces (ChannelSplitter and ChannelMixer)
	void deleteInterfaceBlocks() {
		delete fdn_splitter;
		delete fdn_mixer;
	}

	// Delete objects used for diffusion purposes (MultiChannelDiffuser)
	void deleteDiffusionBlocks() {
		if (!fdn_diffuser.empty()) {
			// Each element of the vector is a point, thus it has to be de-allocated
			for (int i = 0; i < fdn_diffuser.size(); i++) delete fdn_diffuser[i];

			// "clear" calls distructors as well (if present)
			fdn_diffuser.clear();
		}
	}

	// Initialize internal arrays for audio processing
	void initInternalArrays() {
		initPointers();
		int lenghtInBytes = fdn_internalChannels * sizeof(float);
		fdn_spOut = (float*)malloc(lenghtInBytes);
		fdn_dlOut = (float*)malloc(lenghtInBytes);
		memset(fdn_spOut, 0, lenghtInBytes);
		memset(fdn_dlOut, 0, lenghtInBytes);
	}	

	// Delete internal arrays (free memory + set null pointers)
	void deleteInternalArrays() {
		if (fdn_spOut)
			free(fdn_spOut);
		if (fdn_dlOut)
			free(fdn_dlOut);
		initPointers();
	}

	// Set internal array pointers to null
	void initPointers() {
		fdn_spOut = nullptr;
		fdn_dlOut = nullptr;
	}
};
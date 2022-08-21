#pragma once
#include "MultiChannelDiffuser.h"
#include "MultiChannelFeedback.h"
#include "ChannelSplitter.h"
#include "ChannelMixer.h"
#include "LowPassFilter.h"
#include "constants.h"
#include "utils.h"


#define DEFAULT_DIFFUSION_STEPS 4
#define DEFAULT_NUMBER_INPUT_CHANNELS_FDN 1
#define DEFAULT_NUMBER_INTERNAL_CHANNELS_FDN 16
#define DEFAULT_NUMBER_OUTPUT_CHANNELS_FDN 1
#define DEFAULT_DIFFUSER_DELAY_DISTRIBUTION DelayDistribution::RandomInRange
#define DEFAULT_DIFFUSER_DELAY_LOGIC DiffuserDelayLogic::Doubled
#define DEFAULT_FEEDBACK_DELAY_LOGIC DelayDistribution::Exponential
#define DEFAULT_DIFFUSER_MIN_DELAY 0.0
#define DEFAULT_EARLYREFL_WEIGHT 0.5
#define DEFAULT_DIFFUSED_WEIGHT 0.0

using namespace std;

class FDN {

protected:

	// Reverb blocks (class objects)
	ChannelSplitter* fdn_Splitter;
	ChannelMixer* fdn_Mixer;
	vector<MultiChannelDelay*> fdn_EarlyReflections;
	vector<MultiChannelDiffuser*> fdn_Diffuser;
	MultiChannelFeedback* fdn_Feedback;
	vector<LowPassFilter*> fdn_LPFOutput;
	vector<LowPassFilter*> fdn_LPFDiffusion;

	// Channel numbers
	int fdn_inputChannels, fdn_internalChannels, fdn_outputChannels;

	// Diffusion steps
	int fdn_diffusionSteps;

	// Auxiliary arrays
	vector<float> fdn_tmpDiffuser;
	vector<float> fdn_tmpFeedback;
	vector<vector<float>> fdn_outEarly;

	// Sample Rate
	int fdn_sampleRate;	

	// Diffusion and feedback delay maximum lengths
	float fdn_diffMaxLength, fdn_feedMaxLength;

	// Weights
	float fdn_earlyWeight;
	float fdn_diffusedWeight;

public:

	// Class constructors
	FDN() { 				
		constructFDN(DEFAULT_NUMBER_INPUT_CHANNELS_FDN, DEFAULT_NUMBER_INTERNAL_CHANNELS_FDN, DEFAULT_NUMBER_OUTPUT_CHANNELS_FDN, DEFAULT_DIFFUSION_STEPS);
	}

	FDN(int numChIn, int numChInt, int numChOut) {
		constructFDN(numChIn, numChInt, numChOut, DEFAULT_DIFFUSION_STEPS);
	}

	FDN(int numChIn, int numChInt, int numChOut, int numDiffStep) {						
		constructFDN(numChIn, numChInt, numChOut, numDiffStep);
	}

	FDN(int numChIn, int numChInt, int numChOut, int numDiffStep, int sampleRate) {
		constructFDN(numChIn, numChInt, numChOut, numDiffStep);
	}

	// Class destructor
	~FDN() { 
		deleteInterfaceBlocks();
		deleteDiffusionBlocks();	
		deleteFeedbackBlock();
		deleteEarlyReflBlock();
		deleteInternalArrays();
		deleteLowPass();
		deleteDiffusionLowPassFilters();
	}	

	// Initialize FDN elements
	void initialize(float diffusionMaximumLength, float feedbackMaxLength, int sampleRate) {
		fdn_sampleRate = sampleRate;
		fdn_diffMaxLength = diffusionMaximumLength;
		fdn_feedMaxLength = feedbackMaxLength;
		fdn_Feedback->initDelayLines(feedbackMaxLength, sampleRate);
		for (int i = 0; i < fdn_diffusionSteps; i++) {
			fdn_Diffuser[i]->init(diffusionMaximumLength, sampleRate);
			fdn_EarlyReflections[i]->initDelayLines(diffusionMaximumLength, sampleRate);
		}
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_LPFOutput[i]->init(sampleRate);
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_LPFDiffusion[i]->init(sampleRate);
	}

	// Set the length of delay lines in the diffusion blocks
	void setDiffuserDelayLengths(float delayMaxLength, DiffuserDelayLogic logic = DEFAULT_DIFFUSER_DELAY_LOGIC, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		switch (logic) {
		case DiffuserDelayLogic::Doubled: {
			setDoubledDiffuserDelayLengths(DEFAULT_DIFFUSER_MIN_DELAY, delayMaxLength, distr);
			break;
		}
		case DiffuserDelayLogic::Equal: {
			setEqualDiffuserDelayLengths(DEFAULT_DIFFUSER_MIN_DELAY, delayMaxLength, distr);
			break;
		}
		}
	}

	// Set the length of delay lines in the feedback block
	void setFeedbackDelayLengths(float mindelay, float maxDelay, DelayDistribution distr = DEFAULT_FEEDBACK_DELAY_LOGIC) {
		fdn_Feedback->setDelayLengths(mindelay, maxDelay, distr);
	}

	// Set the length of delay lines for the early reflections
	void setEarlyReflDelayLengths(float maxDelayMs, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		for (int i = 0; i < fdn_diffusionSteps; i++)
			fdn_EarlyReflections[i]->setDelayLinesLength(DEFAULT_DIFFUSER_MIN_DELAY, maxDelayMs, distr);
	}

	// Set the weight used to sum early reflections to reverberated signal
	void setEarlyReflWeight(float weight) {
		fdn_earlyWeight = weight;
	}

	// Set the weight used to sum diffused signal (only) to reverberated signal
	void setDiffusedWeight(float weight) {
		fdn_diffusedWeight = weight;
	}

	// Set reverb decay in seconds (it modifies the multi channel feedback gain value)
	void setDecayInSeconds(float decaySeconds) {
		fdn_Feedback->setDecayInSeconds(decaySeconds);
	}

	// Set the damping frequency of the low pass filter in the multi channel feedback loop
	void setDampingFrequency(float freq) {
		fdn_Feedback->setDampingFrequency(freq);
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_LPFDiffusion[i]->setCutoffFrequency(freq);
	}

	// Set Damping low pass filters type
	void setDampingType(FilterType type) {
		fdn_Feedback->setFilterType(type);
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_LPFDiffusion[i]->setFilterType(type);
	}

	// Set output LowPassFilter frequency
	void setLowPassFrequency(float freq) {
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_LPFOutput[i]->setCutoffFrequency(freq);
	}

	// Set output low pass filter type
	void setLowPassType(FilterType type) {
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_LPFOutput[i]->setFilterType(type);
	}

	// Set the number of input channels
	void setNumberOfInputChannels(int numChIn) {
		fdn_inputChannels = numChIn;
		fdn_Splitter->setNumberOfChannelsIn(fdn_inputChannels);
	}

	// Set the number of internal channels (for processing purposes)
	void setNumberOfInternalChannels(int numChInt) {
		fdn_internalChannels = numChInt;
		fdn_Splitter->setNumberOfChannelsOut(fdn_internalChannels);		
		for (int i = 0; i < fdn_diffusionSteps; i++) {
			fdn_Diffuser[i]->setNumberOfInternalChannels(fdn_internalChannels);
			fdn_EarlyReflections[i]->setNumberOfChannels(fdn_internalChannels);
		}
		fdn_Feedback->setNumberOfChannels(fdn_internalChannels);
		fdn_Mixer->setNumberOfInputChannels(fdn_internalChannels);
		float freq = fdn_LPFDiffusion[0]->getCutoffFrequency();
		FilterType type = fdn_LPFDiffusion[0]->getFilterType();
		float Q = fdn_LPFDiffusion[0]->getQualityFactor();
		float g = fdn_LPFOutput[0]->getShelvingGain();
		deleteDiffusionLowPassFilters();
		constructDiffusionLowPassFilters();
		for (int i = 0; i < fdn_internalChannels; i++) {
			fdn_LPFDiffusion[i]->init(fdn_sampleRate);
			fdn_LPFDiffusion[i]->setCutoffFrequency(freq);
			fdn_LPFDiffusion[i]->setQualityFactor(Q);
			fdn_LPFDiffusion[i]->setShelvingGain(g);
			fdn_LPFDiffusion[i]->setFilterType(type);
		}
		deleteInternalArrays();
		initInternalArrays();
	}

	// Set the number of output channels
	void setNumberOfOutputChannels(int numChOut) {
		// reset Mixer's number of channels
		fdn_outputChannels = numChOut;
		fdn_Mixer->setNumberOfOutputChannels(fdn_outputChannels);

		// reset number of low pass filters and set back the same freq and sample rate
		float freq = fdn_LPFOutput[0]->getCutoffFrequency();		
		float g = fdn_LPFOutput[0]->getShelvingGain();
		float Q = fdn_LPFOutput[0]->getQualityFactor();
		FilterType type = fdn_LPFOutput[0]->getFilterType();
		deleteLowPass();
		constructLowPass();		
		for (int i = 0; i < fdn_outputChannels; i++) {
			fdn_LPFOutput[i]->init(fdn_sampleRate);
			fdn_LPFOutput[i]->setCutoffFrequency(freq);
			fdn_LPFOutput[i]->setQualityFactor(Q);
			fdn_LPFOutput[i]->setShelvingGain(g);
			fdn_LPFOutput[i]->setFilterType(type);
		}
	}
		
	// Set the sample rate
	void setSampleRate(int sampleRate) { 
		fdn_sampleRate = sampleRate;		
		for (int i = 0; i < fdn_diffusionSteps; i++) {
			fdn_Diffuser[i]->setSampleRate(sampleRate);
			fdn_EarlyReflections[i]->setSampleRate(sampleRate);
		}
		fdn_Feedback->setSampleRate(sampleRate);
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_LPFOutput[i]->setSampleRate(sampleRate);
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_LPFDiffusion[i]->setSampleRate(sampleRate);
	}	

	// Set output mixing mode
	void setMixMode(MixMode mode) { fdn_Mixer->setMixMode(mode); }

	// Set number of diffusion steps
	void setDiffusionSteps(int numSteps) {
		deleteDiffusionBlocks();
		fdn_diffusionSteps = numSteps;
		constructDiffusionBlocks();
		for (int i = 0; i < fdn_diffusionSteps; i++)
			fdn_Diffuser[i]->init(fdn_diffMaxLength, fdn_sampleRate);
		deleteInternalArrays();
		initInternalArrays();
	}

	// Process Audio
	void processAudio(float* in, float* out) { 
		// Split input into N internal channels
		fdn_Splitter->processAudio(in, &fdn_tmpDiffuser[0]);

		// Send input signal into diffusion blocks and spill-out for early reflections
		for (int i = 0; i < fdn_diffusionSteps; i++) {
			fdn_EarlyReflections[i]->processAudio(&fdn_tmpDiffuser[0], &fdn_outEarly[i][0]);
			fdn_Diffuser[i]->processAudio(&fdn_tmpDiffuser[0], &fdn_tmpDiffuser[0]);
		}

		// Apply Low pass filters after diffusion
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_tmpDiffuser[i] = fdn_LPFDiffusion[i]->processAudio(fdn_tmpDiffuser[i]);

		// Send diffused signal into feedback lines
		fdn_Feedback->processAudio(&fdn_tmpDiffuser[0], &fdn_tmpFeedback[0]);

		// Sum-up early reflections, diffused and reverbered signals
		for (int i = 0; i < fdn_internalChannels; i++) {
			for (int k = 0; k < fdn_diffusionSteps; k++)
				fdn_tmpFeedback[i] += fdn_earlyWeight * fdn_outEarly[k][i] + fdn_diffusedWeight * fdn_tmpDiffuser[i];
		}

		// Mix-down N internal channels into output channels
		fdn_Mixer->processAudio(&fdn_tmpFeedback[0], out);

		// Apply Low Pass filtering
		for (int i = 0; i < fdn_outputChannels; i++)
			out[i] = fdn_LPFOutput[i]->processAudio(out[i]);

	};

private:

	// Set the length of diffusion steps each equal to double the previous
	void setDoubledDiffuserDelayLengths(float minDelayMs, float maxDelayMs, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		for (int i = fdn_diffusionSteps - 1; i >= 0; i--) {
			fdn_Diffuser[i]->setDelayLinesLength(minDelayMs, maxDelayMs, distr);
			maxDelayMs *= 0.5;
		}
	}

	// Set the length of diffusion steps each equal to the previous
	void setEqualDiffuserDelayLengths(float minDelayMs, float maxDelayMs, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		for (int i = 0; i < fdn_diffusionSteps; i++) 
			fdn_Diffuser[i]->setDelayLinesLength(minDelayMs, maxDelayMs, distr);
	}

	// Function called in the class constructors
	void constructFDN(int numChIn, int numChInt, int numChOut, int numDiffStep) {
		srand(_SEED_FOR_RAND_GENERATION);
		fdn_inputChannels = numChIn;
		fdn_internalChannels = numChInt;
		fdn_outputChannels = numChOut;
		fdn_diffusionSteps = numDiffStep;	
		setEarlyReflWeight(DEFAULT_EARLYREFL_WEIGHT);
		setDiffusedWeight(DEFAULT_DIFFUSED_WEIGHT);

		constructDiffusionBlocks();

		// Construct objects used for in&out interfaces purposes (ChannelSplitter and ChannelMixer)
		fdn_Splitter = new ChannelSplitter(fdn_inputChannels, fdn_internalChannels);
		fdn_Mixer = new ChannelMixer(fdn_internalChannels, fdn_outputChannels);
		
		// Construct objects used for feedback delay purposes and set their internal channels correctly (MultiChannelFeedback)
		fdn_Feedback = new MultiChannelFeedback(fdn_internalChannels);
		
		// Construct objects used for early reflection purposes and set their internal channels correctly (MultiChannelDelay)
		for (int i = 0; i < fdn_diffusionSteps; i++)
			fdn_EarlyReflections.push_back(new MultiChannelDelay(fdn_internalChannels));

		constructLowPass();
		constructDiffusionLowPassFilters();
		initInternalArrays();
	}	

	// Construct objects used for diffusion purposes and set their internal channels correctly (MultiChannelDiffuser)
	void constructDiffusionBlocks() {		
		for (int i = 0; i < fdn_diffusionSteps; i++)
			fdn_Diffuser.push_back(new MultiChannelDiffuser(fdn_internalChannels));		
	}

	// Construct objects used for early reflection purposes and set their internal channels correctly (MultiChannelDelay)
	void constructEarlyReflBlock() {
		for (int i = 0; i < fdn_diffusionSteps; i++)
			fdn_EarlyReflections.push_back(new MultiChannelDelay(fdn_internalChannels));
	}
	
	// Construct object for low pass filter (LowPassFilter)
	void constructLowPass() {
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_LPFOutput.push_back(new LowPassFilter());
	}

	// Construct object for low pass filter after diffusion (LowPassFilter)
	void constructDiffusionLowPassFilters() {
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_LPFDiffusion.push_back(new LowPassFilter());
	}

	// Delete objects used for in&out interfaces (ChannelSplitter and ChannelMixer)
	void deleteInterfaceBlocks() {
		delete fdn_Splitter;
		delete fdn_Mixer;
	}

	// Delete objects used for diffusion purposes (MultiChannelDiffuser)
	void deleteDiffusionBlocks() {
		if (!fdn_Diffuser.empty()) {
			// Each element of the vector is a point, thus it has to be de-allocated
			for (int i = 0; i < fdn_Diffuser.size(); i++) 
				delete fdn_Diffuser[i];

			// "clear" calls distructors as well (if present)
			fdn_Diffuser.clear();
		}
	}

	// Delete objects used for feedback purposes (MultiChannelFeedback)
	void deleteFeedbackBlock() {
		delete fdn_Feedback;
	}

	// Delete objects used for early reflections (MultiChannelDelay)
	void deleteEarlyReflBlock() {
		if (!fdn_EarlyReflections.empty()) {
			// Each element of the vector is a point, thus it has to be de-allocated
			for (int i = 0; i < fdn_EarlyReflections.size(); i++)
				delete fdn_EarlyReflections[i];

			// "clear" calls distructors as well (if present)
			fdn_EarlyReflections.clear();
		}
	}

	// Delete objects used for low pass filter (LowPassFilter)
	void deleteLowPass() {
		if (!fdn_LPFOutput.empty()) {
			for (int i = 0; i < fdn_LPFOutput.size(); i++)
				delete fdn_LPFOutput[i];
			fdn_LPFOutput.clear();
		}
	}

	// Delete objects used for low pass filter after diffusion (LowPassFilter)
	void deleteDiffusionLowPassFilters() {
		if (!fdn_LPFDiffusion.empty()) {
			for (int i = 0; i < fdn_LPFDiffusion.size(); i++)
				delete fdn_LPFDiffusion[i];
			fdn_LPFDiffusion.clear();
		}
	}

	// Initialize internal arrays for audio processing
	void initInternalArrays() {
		fdn_tmpDiffuser.resize(fdn_internalChannels);
		fdn_tmpFeedback.resize(fdn_internalChannels);
		fdn_outEarly.resize(fdn_diffusionSteps, vector<float>(fdn_internalChannels));
	}	

	// Delete internal arrays (free memory + set null pointers)
	void deleteInternalArrays() {
		if (!fdn_tmpDiffuser.empty())
			fdn_tmpDiffuser.clear();
		if (!fdn_tmpFeedback.empty())
			fdn_tmpFeedback.clear();
		if (!fdn_outEarly.empty()) {
			for (int i = 0; i < fdn_outEarly.size(); i++) {
				if (!fdn_outEarly[i].empty())
					fdn_outEarly[i].clear();
			}
		}
				
	}
};
#pragma once
#include "MultiChannelDiffuser.h"
#include "MultiChannelFeedback.h"
#include "ModMultiChannelDiffuser.h"
#include "ModMultiChannelFeedback.h"
#include "ChannelSplitter.h"
#include "ChannelMixer.h"
#include "LowPassFilter.h"
#include "HighPassFilter.h"
#include "Modulation.h"
#include "constants.h"
#include "utils.h"

// Channels constants
#define DEFAULT_NUMBER_INPUT_CHANNELS_FDN 1
#define DEFAULT_NUMBER_INTERNAL_CHANNELS_FDN 16
#define DEFAULT_NUMBER_OUTPUT_CHANNELS_FDN 1

// Diffusion constants
#define DEFAULT_DIFFUSION_STEPS 4
#define DEFAULT_NUMBER_MOD_DIFFUSER 1
#define DEFAULT_DIFFUSER_DELAY_DISTRIBUTION DelayDistribution::RandomInRange
#define DEFAULT_DIFFUSER_DELAY_LOGIC DiffuserDelayLogic::Doubled
#define DEFAULT_DIFFUSER_MIN_DELAY 30.0
#define DEFAULT_DIFFUSED_WEIGHT 0.0
#define _DIFFUSER_MODULATION_VALUE 1.0 // [ms]
#define _DIFFUSER_MODULATION_RATE 1.0 // [Hz]
#define _DIFFUSER_MODULATION_TYPE OscillatorType::Sine
#define _DIFFUSER_MODULATION_POLARITY false

// Early reflection constants
#define DEFAULT_EARLYREFL_WEIGHT 1.0
#define DEFAULT_EARLYREFL_MIN_DELAY 2.0
#define DEFAULT_EARLYREFL_DELAY_DISTRIBUTION DelayDistribution::Exponential
#define DEFAULT_EARLYREFL_BUFFER_LENGTH 20.0 // [ms]

// Feedback constants
#define DEFAULT_FEEDBACK_MIN_DELAY 13.0
#define DEFAULT_FEEDBACK_DELAY_DISTRIBUTION DelayDistribution::RandomInRange
#define _FEEDBACK_MAX_MODULATION_VALUE 4.0 // [ms]
#define _FEEDBACK_MAX_MODULATION_RATE 0.4 // [hz]
#define _FEEDBACK_MODULATION_TYPE OscillatorType::Sine
#define _FEEDBACK_MODULATION_POLARITY false

// Modulation constants
#define DEFAULT_MODULATION_TYPE ModulationType::Vibrato

// Filters
#define LPF_FILTER_TYPE LPFilterType::Vicanek
#define HPF_FILTER_TYPE HPFilterType::Shelving
#define DAMPING_FILTER_TYPE LPFilterType::Vicanek


using namespace std;

class FDN {

protected:

	// Reverb blocks (class objects)
	ChannelSplitter* fdn_Splitter;
	ChannelMixer* fdn_Mixer;
	MultiChannelDelay* fdn_EarlyReflections;
	vector<MultiChannelDiffuser*> fdn_Diffuser;
	vector<ModMultiChannelDiffuser*> fdn_ModDiffuser;
	ModMultiChannelFeedback* fdn_Feedback;
	vector<LowPassFilter*> fdn_LPFOutput;
	vector<Modulation*> fdn_Modulation;
	Hadamard* fdn_MixMatrix;
	MultiChannelDiffuser* fdn_OutputDiffusion;
	vector<HighPassFilter*> fdn_HPFOutput;

	// Channel numbers
	int fdn_inputChannels, fdn_internalChannels, fdn_outputChannels;

	// Diffusion steps
	int fdn_diffusionSteps;
	int fdn_numModDiffuser;
	float type;

	// Auxiliary arrays
	vector<float> fdn_tmpDiffuser;
	vector<float> fdn_tmpFeedback;
	vector<float> fdn_outEarly;

	// Sample Rate
	int fdn_sampleRate;	

	// Early reflections, diffusion and feedback buffer lengths
	float fdn_earlBufferLength , fdn_diffBufferLength, fdn_feedBufferLength;

	// Diffusion, feedback and early reflections delay distributions
	DelayDistribution fdn_diffDelDistr, fdn_feedDelDistr, fdn_reflDistr;

	// Diffusion logic
	DiffuserDelayLogic fdn_diffLogic;
	
	// Reverb decay in seconds
	float fdn_decay, fdn_decayMultiplier;

	// Weights
	float fdn_earlyWeight;
	float fdn_stereoSpread;
	float fdn_modWet, fdn_modDry;

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

	FDN(int numChIn, int numChInt, int numChOut, int numDiffStep, int numModDiffusers) {
		constructFDN(numChIn, numChInt, numChOut, numDiffStep, numModDiffusers);
	}

	// Class destructor
	~FDN() { 
		deleteInterfaceBlocks();
		deleteDiffusionBlocks();	
		deleteFeedbackBlock();
		deleteInternalArrays();
		deleteFilters();
		deleteChorus();

		// Delete early reflection block
		delete fdn_EarlyReflections;

		// Delete mix matrix for output mixing after feedback stage
		delete fdn_MixMatrix;
		
	}	

	// Initialize FDN elements
	void initialize(float diffusionMaximumLength, float feedbackMaxLength, int sampleRate, float earlyrefBuffLength = DEFAULT_EARLYREFL_BUFFER_LENGTH) {
		fdn_sampleRate = sampleRate;
		fdn_earlBufferLength = earlyrefBuffLength;
		fdn_diffBufferLength = diffusionMaximumLength;
		fdn_feedBufferLength = feedbackMaxLength;
		fdn_diffLogic = DEFAULT_DIFFUSER_DELAY_LOGIC;
		fdn_diffDelDistr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION;
		fdn_feedDelDistr = DEFAULT_FEEDBACK_DELAY_DISTRIBUTION;
		fdn_reflDistr = DEFAULT_EARLYREFL_DELAY_DISTRIBUTION;
		fdn_decay = 0.0;
		fdn_decayMultiplier = 1.0;
		
		// Early reflections
		fdn_EarlyReflections->initDelayLines(earlyrefBuffLength, sampleRate);
		setEarlyReflWeight(DEFAULT_EARLYREFL_WEIGHT);

		// Diffusers
		for (int i = 0; i < fdn_diffusionSteps - fdn_numModDiffuser; i++)
			fdn_Diffuser[i]->init(diffusionMaximumLength, sampleRate);

		for (int i = 0; i < fdn_numModDiffuser; i++) {
			fdn_ModDiffuser[i]->init(diffusionMaximumLength, sampleRate);
			fdn_ModDiffuser[i]->setModValue(_DIFFUSER_MODULATION_VALUE);
			fdn_ModDiffuser[i]->setModRate(_DIFFUSER_MODULATION_RATE);
			fdn_ModDiffuser[i]->setOscillatorType(_DIFFUSER_MODULATION_TYPE);
			fdn_ModDiffuser[i]->setOscillatorIsUnipolar(_DIFFUSER_MODULATION_POLARITY);
		}

		// Modulation
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_Modulation[i]->init(DEFAULT_MODULATION_TYPE, sampleRate);
		setModDepth(0.0);
		setModRate(0.0);
		setModFeedback(0.0);
		setModMix(1.0);

		// Feedback
		fdn_Feedback->init(feedbackMaxLength, sampleRate);
		fdn_Feedback->setModValue(_FEEDBACK_MAX_MODULATION_VALUE);
		fdn_Feedback->setModRate(_FEEDBACK_MAX_MODULATION_RATE);
		fdn_Feedback->setOscillatorType(_FEEDBACK_MODULATION_TYPE);
		fdn_Feedback->setOscillatorIsUnipolar(_FEEDBACK_MODULATION_POLARITY);
		setDampingType(DAMPING_FILTER_TYPE);
		setDampingFrequency(MAX_LPF_FREQUENCY);

		// Output diffusion
		fdn_OutputDiffusion->init(diffusionMaximumLength, sampleRate);

		// LPF & HPF
		for (int i = 0; i < fdn_outputChannels; i++) {
			fdn_LPFOutput[i]->init(sampleRate);
			fdn_HPFOutput[i]->init(sampleRate);
		}	
		setLowPassType(LPF_FILTER_TYPE);
		setHighPassType(HPF_FILTER_TYPE);
		setLowPassFrequency(MAX_LPF_FREQUENCY);
		setHighPassFrequency(MIN_HPF_FREQUENCY);
	}	
	
	// Set reverb decay in seconds (it modifies the multi channel feedback gain value)
	void setDecayInSeconds(float decaySeconds) {	
		fdn_decay = decaySeconds;
		fdn_Feedback->setDecayInSeconds(fdn_decay * fdn_decayMultiplier);
	}	

	// Set room size by changing diffusion and feedback delay lenghts
	void setRoomSize(float size, DiffuserDelayLogic logic = DiffuserDelayLogic::Empty, DelayDistribution diffDistr = DelayDistribution::Empty, DelayDistribution feedDistr = DelayDistribution::Empty) {
		if (logic == DiffuserDelayLogic::Empty)
			logic = fdn_diffLogic;
		if (diffDistr == DelayDistribution::Empty)
			diffDistr = fdn_diffDelDistr;
		if (feedDistr == DelayDistribution::Empty)
			feedDistr = fdn_feedDelDistr;

		float diffMaxdel = mapValueIntoRange(0.5 * size, DEFAULT_DIFFUSER_MIN_DELAY, fdn_diffBufferLength);
		float feedMaxdel = mapValueIntoRange(size, diffMaxdel + 20.0, fdn_feedBufferLength);
		setDiffuserDelayLengths(diffMaxdel, logic, diffDistr, DEFAULT_DIFFUSER_MIN_DELAY);
		setFeedbackDelayLengths(0.5 * diffMaxdel, feedMaxdel, feedDistr);
		fdn_decayMultiplier = mapValueIntoRange(size, 0.1, 2.0);
		setDecayInSeconds(fdn_decay);
		fdn_Feedback->setModDepth(size);
		fdn_Feedback->setModRate(mapValueIntoRange(1.0 - size, 0.0, _FEEDBACK_MAX_MODULATION_RATE));
	}	

	// Set chorus rate
	void setModRate(float rate) {	
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_Modulation[i]->setModRate(rate);
	}

	// Set chorus depth
	void setModDepth(float depth) {		
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_Modulation[i]->setModDepth(depth);	
	}

	// Set modulation feedback value
	void setModFeedback(float fb) {
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_Modulation[i]->setModFeedback(fb);
	}
		
	// Set modulated reverb mix with non-modulated
	void setModMix(float mix) {
		fdn_modWet = sin(mix * 0.5 * M_PI);
		fdn_modDry = cos(mix * 0.5 * M_PI);
	}

	void setModulationType(ModulationType type) {		
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_Modulation[i]->setModType(type);
	}

	// Set the damping frequency of the low pass filter in the multi channel feedback loop
	void setDampingFrequency(float freq) {
		fdn_Feedback->setDampingFrequency(freq);
	}

	// Set Damping low pass filters type
	void setDampingType(LPFilterType type) {
		fdn_Feedback->setFilterType(type);
	}

	// Set output LowPassFilter frequency
	void setLowPassFrequency(float freq) {
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_LPFOutput[i]->setCutoffFrequency(freq);
	}

	// Set output HighPassFilter frequency
	void setHighPassFrequency(float freq) {
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_HPFOutput[i]->setCutoffFrequency(freq);
	}

	// Set output low pass filter type
	void setLowPassType(LPFilterType type) {
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_LPFOutput[i]->setFilterType(type);
	}

	// Set output high pass filter type
	void setHighPassType(HPFilterType type) {
		for (int i = 0; i < fdn_outputChannels; i++)
			fdn_HPFOutput[i]->setFilterType(type);
	}

	// Set stereo spreading coefficient
	void setStereoSpread(float spread) {
		fdn_stereoSpread = spread;
		for (int i = 0; i < fdn_numModDiffuser; i++)
			fdn_ModDiffuser[i]->setModDepth(spread);		
	}	

	// Set the length of delay lines for the early reflections
	void setEarlyReflDelayLengths(float maxDelayMs, DelayDistribution distr = DEFAULT_EARLYREFL_DELAY_DISTRIBUTION, float delayMinLength = DEFAULT_EARLYREFL_MIN_DELAY) {
		fdn_reflDistr = distr;
		fdn_EarlyReflections->setDelayLinesLength(delayMinLength, maxDelayMs, distr);
	}

	// Set the length of delay lines in the diffusion blocks
	void setDiffuserDelayLengths(float delayMaxLength, DiffuserDelayLogic logic = DEFAULT_DIFFUSER_DELAY_LOGIC, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION, float delayMinLength = DEFAULT_DIFFUSER_MIN_DELAY) {
		fdn_diffLogic = logic;
		fdn_diffDelDistr = distr;
		switch (logic) {
		case DiffuserDelayLogic::Doubled: {
			setDoubledDiffuserDelayLengths(delayMinLength, delayMaxLength, distr);
			break;
		}
		case DiffuserDelayLogic::Equal: {
			setEqualDiffuserDelayLengths(delayMinLength, delayMaxLength, distr);
			break;
		}
		}
	}

	// Set the length of delay lines in the feedback block
	void setFeedbackDelayLengths(float mindelay, float maxDelay, DelayDistribution distr = DEFAULT_FEEDBACK_DELAY_DISTRIBUTION) {
		fdn_feedDelDistr = distr;
		fdn_Feedback->setDelayLengths(mindelay, maxDelay, distr);
		setDecayInSeconds(fdn_decay);
	}

	// Set the weight used to sum early reflections to reverberated signal
	void setEarlyReflWeight(float weight) {
		fdn_earlyWeight = weight;
	}

	// Set the number of input channels
	void setNumberOfInputChannels(int numChIn) {
		fdn_inputChannels = numChIn;
		fdn_Splitter->setNumberOfChannelsIn(fdn_inputChannels);
	}

	// Set the number of internal channels (for processing purposes)
	void setNumberOfInternalChannels(int numChInt) {
		fdn_internalChannels = numChInt;

		// set number of channels to classes
		fdn_Splitter->setNumberOfChannelsOut(fdn_internalChannels);		
		for (int i = 0; i < fdn_diffusionSteps - fdn_numModDiffuser; i++)
			fdn_Diffuser[i]->setNumberOfInternalChannels(fdn_internalChannels);
		for (int i = 0; i < fdn_numModDiffuser; i++)
			fdn_ModDiffuser[i]->setNumberOfInternalChannels(fdn_internalChannels);
		fdn_EarlyReflections->setNumberOfChannels(fdn_internalChannels);
		fdn_OutputDiffusion->setNumberOfInternalChannels(fdn_internalChannels);
		fdn_Feedback->setNumberOfChannels(fdn_internalChannels);
		fdn_Mixer->setNumberOfInputChannels(fdn_internalChannels);		
		fdn_MixMatrix->setNumberOfChannels(fdn_internalChannels);

		// reset chorus
		float depth = fdn_Modulation[0]->getModDepth();
		float rate = fdn_Modulation[0]->getModRate();
		deleteChorus();
		constructChorus();
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_Modulation[i]->init(DEFAULT_MODULATION_TYPE, fdn_sampleRate);
		setModDepth(depth);
		setModRate(rate);

		// reset internal arrays
		deleteInternalArrays();
		initInternalArrays();
	}

	// Set the number of output channels
	void setNumberOfOutputChannels(int numChOut) {
		// reset Mixer's number of channels
		fdn_outputChannels = numChOut;
		fdn_Mixer->setNumberOfOutputChannels(fdn_outputChannels);

		// reset number of low and high pass filters and set back the same freq and sample rate
		float LPfreq = fdn_LPFOutput[0]->getCutoffFrequency();		
		float LPg = fdn_LPFOutput[0]->getShelvingGain();
		float LPQ = fdn_LPFOutput[0]->getQualityFactor();
		LPFilterType LPtype = fdn_LPFOutput[0]->getFilterType();
		float HPfreq = fdn_HPFOutput[0]->getCutoffFrequency();
		float HPg = fdn_HPFOutput[0]->getShelvingGain();
		float HPQ = fdn_HPFOutput[0]->getQualityFactor();
		HPFilterType HPtype = fdn_HPFOutput[0]->getFilterType();
		deleteFilters();
		constructFilters();
		for (int i = 0; i < fdn_outputChannels; i++) {
			// LPF
			fdn_LPFOutput[i]->init(fdn_sampleRate);
			fdn_LPFOutput[i]->setCutoffFrequency(LPfreq);
			fdn_LPFOutput[i]->setQualityFactor(LPQ);
			fdn_LPFOutput[i]->setShelvingGain(LPg);
			fdn_LPFOutput[i]->setFilterType(LPtype);
			// HPF
			fdn_HPFOutput[i]->init(fdn_sampleRate);
			fdn_HPFOutput[i]->setCutoffFrequency(HPfreq);
			fdn_HPFOutput[i]->setQualityFactor(HPQ);
			fdn_HPFOutput[i]->setShelvingGain(HPg);
			fdn_HPFOutput[i]->setFilterType(HPtype);
		}
	}
		
	// Set the sample rate
	void setSampleRate(int sampleRate) { 
		fdn_sampleRate = sampleRate;		
		fdn_EarlyReflections->setSampleRate(sampleRate);
		for (int i = 0; i < fdn_diffusionSteps - fdn_numModDiffuser; i++)
			fdn_Diffuser[i]->setSampleRate(sampleRate);		
		for (int i = 0; i < fdn_numModDiffuser; i++)
			fdn_ModDiffuser[i]->setSampleRate(sampleRate);
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_Modulation[i]->setSampleRate(sampleRate);
		fdn_OutputDiffusion->setSampleRate(sampleRate);
		fdn_Feedback->setSampleRate(sampleRate);
		for (int i = 0; i < fdn_outputChannels; i++) {
			fdn_LPFOutput[i]->setSampleRate(sampleRate);
			fdn_HPFOutput[i]->setSampleRate(sampleRate);
		}
	}	

	// Set output mixing mode
	void setMixMode(MixMode mode) { 
		fdn_Mixer->setMixMode(mode); 
	}

	// Set number of diffusion steps
	void setDiffusionSteps(int numSteps) {
		deleteDiffusionBlocks();
		fdn_diffusionSteps = numSteps;
		constructDiffusionBlocks();
		for (int i = 0; i < fdn_diffusionSteps - fdn_numModDiffuser; i++)
			fdn_Diffuser[i]->init(fdn_diffBufferLength, fdn_sampleRate);
		for (int i = 0; i < fdn_numModDiffuser; i++)
			fdn_ModDiffuser[i]->init(fdn_diffBufferLength, fdn_sampleRate);
		fdn_OutputDiffusion->init(fdn_diffBufferLength, fdn_sampleRate);
		deleteInternalArrays();
		initInternalArrays();
	}

	// Process Audio
	void processAudio(float* in, float* out) { 
		// Split input into N internal channels
		fdn_Splitter->processAudio(in, &fdn_tmpDiffuser[0]);

		// Send input signal into diffusion blocks
		for (int i = 0; i < fdn_diffusionSteps - fdn_numModDiffuser; i++)
			fdn_Diffuser[i]->processAudio(&fdn_tmpDiffuser[0], &fdn_tmpDiffuser[0]);

		// Send input signal into modulated diffusion blocks
		for (int i = 0; i < fdn_numModDiffuser; i++)
			fdn_ModDiffuser[i]->processAudio(&fdn_tmpDiffuser[0], &fdn_tmpDiffuser[0]);
						
		// Spill-out the diffused signal and send to multi-channel delay for early reflections
		fdn_EarlyReflections->processAudio(&fdn_tmpDiffuser[0], &fdn_outEarly[0]);

		// Feedback lines
		fdn_Feedback->processAudio(&fdn_tmpDiffuser[0], &fdn_tmpFeedback[0]);
		
		// Apply Hadamard mixing matrix to "align" channels
		fdn_MixMatrix->processAudio(&fdn_tmpFeedback[0], &fdn_tmpFeedback[0]);
		
		// Apply the shortest diffusion step to the aligned channels to spread out the reverb
		fdn_OutputDiffusion->processAudio(&fdn_tmpFeedback[0], &fdn_tmpFeedback[0]);

		// Sum-up early reflections and reverbered signals
		for (int i = 0; i < fdn_internalChannels; i++)		
			fdn_tmpFeedback[i] += fdn_earlyWeight * fdn_outEarly[i];		

		// Modulate reverb and mix it with non modulated signal
		for (int i = 0; i < fdn_internalChannels; i++) {
			float mod = fdn_Modulation[i]->processAudio(fdn_tmpFeedback[i]);
			fdn_tmpFeedback[i] = fdn_modWet * mod + fdn_modDry * fdn_tmpFeedback[i];
		}

		// Mix-down N internal channels into output channels
		fdn_Mixer->processAudio(&fdn_tmpFeedback[0], out);

		// Apply mid-side processing for stereo spreading
		if (fdn_outputChannels == 2) {
			float mid = 0.5*(out[0] + out[1]);
			float side = 0.5*(out[0] - out[1]);
			side *= fdn_stereoSpread;
			mid *= (1.0 - fdn_stereoSpread);
			out[0] = mid + side;
			out[1] = mid - side;
		}
		
		// Apply Low & High Pass filtering
		for (int i = 0; i < fdn_outputChannels; i++) {
			out[i] = fdn_LPFOutput[i]->processAudio(out[i]);
			out[i] = fdn_HPFOutput[i]->processAudio(out[i]);
		}

	};

private:

	// Set the length of diffusion steps each equal to double the previous
	void setDoubledDiffuserDelayLengths(float minDelayMs, float maxDelayMs, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		for (int i = fdn_numModDiffuser - 1; i >=0; i--) {
			fdn_ModDiffuser[i]->setDelayLinesLength(minDelayMs, maxDelayMs, distr);
			maxDelayMs *= 0.5;
		}
		for (int i = fdn_diffusionSteps - fdn_numModDiffuser - 1; i >= 0; i--) {
			fdn_Diffuser[i]->setDelayLinesLength(minDelayMs, maxDelayMs, distr);
			maxDelayMs *= 0.5;
		}		
		fdn_OutputDiffusion->setDelayLinesLength(minDelayMs, maxDelayMs * 2.0, distr);
	}

	// Set the length of diffusion steps each equal to the previous
	void setEqualDiffuserDelayLengths(float minDelayMs, float maxDelayMs, DelayDistribution distr = DEFAULT_DIFFUSER_DELAY_DISTRIBUTION) {
		for (int i = 0; i < fdn_diffusionSteps - fdn_numModDiffuser; i++)
			fdn_Diffuser[i]->setDelayLinesLength(minDelayMs, maxDelayMs, distr);
		for (int i = 0; i < fdn_numModDiffuser; i++)
			fdn_ModDiffuser[i]->setDelayLinesLength(minDelayMs, maxDelayMs, distr);
		fdn_OutputDiffusion->setDelayLinesLength(minDelayMs, maxDelayMs, distr);
	}

	// Function called in the class constructors
	void constructFDN(int numChIn, int numChInt, int numChOut, int numDiffStep, int numModDiffuser = DEFAULT_NUMBER_MOD_DIFFUSER) {
		srand(_SEED_FOR_RAND_GENERATION);
		fdn_inputChannels = numChIn;
		fdn_internalChannels = numChInt;
		fdn_outputChannels = numChOut;
		fdn_diffusionSteps = numDiffStep;	
		fdn_numModDiffuser = numModDiffuser;		

		constructDiffusionBlocks();

		// Construct objects used for in&out interfaces purposes (ChannelSplitter and ChannelMixer)
		fdn_Splitter = new ChannelSplitter(fdn_inputChannels, fdn_internalChannels);
		fdn_Mixer = new ChannelMixer(fdn_internalChannels, fdn_outputChannels);
		
		// Construct objects used for feedback delay purposes and set their internal channels correctly (MultiChannelFeedback)
		fdn_Feedback = new ModMultiChannelFeedback(fdn_internalChannels);
		
		// Construct objects used for early reflection purposes and set their internal channels correctly (MultiChannelDelay)
		fdn_EarlyReflections = new MultiChannelDelay(fdn_internalChannels);

		// Construct filters
		constructFilters();
		constructChorus();

		// Construct mix matrix for output mixing after feedback stage
		fdn_MixMatrix = new Hadamard(fdn_internalChannels);
		
		// Init vectors for tmp storage
		initInternalArrays();
	}	

	// Construct objects used for diffusion purposes and set their internal channels correctly (MultiChannelDiffuser)
	void constructDiffusionBlocks() {		
		for (int i = 0; i < fdn_diffusionSteps-fdn_numModDiffuser; i++)
			fdn_Diffuser.push_back(new MultiChannelDiffuser(fdn_internalChannels));		
		for (int i = 0; i < fdn_numModDiffuser; i++)
			fdn_ModDiffuser.push_back(new ModMultiChannelDiffuser(fdn_internalChannels));
		fdn_OutputDiffusion = new MultiChannelDiffuser(fdn_internalChannels);
	}	
	
	// Construct object for low pass filter (LowPassFilter)
	void constructFilters() {
		for (int i = 0; i < fdn_outputChannels; i++) {
			fdn_LPFOutput.push_back(new LowPassFilter());
			fdn_HPFOutput.push_back(new HighPassFilter());
		}
	}

	// Construct Chorus modulation objects (Modulation)
	void constructChorus() {
		for (int i = 0; i < fdn_internalChannels; i++)
			fdn_Modulation.push_back(new Modulation());
	}

	// Delete objects used for in&out interfaces (ChannelSplitter and ChannelMixer)
	void deleteInterfaceBlocks() {
		delete fdn_Splitter;
		delete fdn_Mixer;
	}

	// Delete objects used for diffusion purposes (MultiChannelDiffuser)
	void deleteDiffusionBlocks() {
		// Delete diffusers
		if (!fdn_Diffuser.empty()) {
			// Each element of the vector is a point, thus it has to be de-allocated
			for (int i = 0; i < fdn_Diffuser.size(); i++) 
				delete fdn_Diffuser[i];

			// "clear" calls distructors as well (if present)
			fdn_Diffuser.clear();
		}

		// Delete modulated diffusers
		if (!fdn_ModDiffuser.empty()) {
			for (int i = 0; i < fdn_ModDiffuser.size(); i++)
				delete fdn_ModDiffuser[i];
			fdn_ModDiffuser.clear();
		}

		// Delete output diffuser
		delete fdn_OutputDiffusion;
	}

	// Delete objects used for feedback purposes (MultiChannelFeedback)
	void deleteFeedbackBlock() {
		delete fdn_Feedback;
	}	

	// Delete objects used for low pass filter (LowPassFilter)
	void deleteFilters() {
		if (!fdn_LPFOutput.empty()) {
			for (int i = 0; i < fdn_LPFOutput.size(); i++)
				delete fdn_LPFOutput[i];
			fdn_LPFOutput.clear();
		}
		if (!fdn_HPFOutput.empty()) {
			for (int i = 0; i < fdn_HPFOutput.size(); i++)
				delete fdn_HPFOutput[i];
			fdn_HPFOutput.clear();
		}
	}

	// Delete chorus object
	void deleteChorus() {
		if (!fdn_Modulation.empty()) {
			for (int i = 0; i < fdn_Modulation.size(); i++)
				delete fdn_Modulation[i];
			fdn_Modulation.clear();
		}
	}

	// Initialize internal arrays for audio processing
	void initInternalArrays() {
		fdn_tmpDiffuser.resize(fdn_internalChannels);
		fdn_tmpFeedback.resize(fdn_internalChannels);
		fdn_outEarly.resize(fdn_internalChannels);
	}	

	// Delete internal arrays (free memory + set null pointers)
	void deleteInternalArrays() {
		if (!fdn_tmpDiffuser.empty())
			fdn_tmpDiffuser.clear();
		if (!fdn_tmpFeedback.empty())
			fdn_tmpFeedback.clear();
		if (!fdn_outEarly.empty())		
			fdn_outEarly.clear();				
	}
};
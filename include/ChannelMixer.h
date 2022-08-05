#pragma once
#include "utils.h"

#define DEFAULT_NUMBER_INPUT_CHANNELS 4
#define DEFAULT_NUMBER_OUTPUT_CHANNELS 1
#define DEFAULT_MIXING_LOGIC MixMode::WeightedSum


class ChannelMixer {

protected:
	
	int mix_numberOfChannelsIn;
	int mix_numberOfChannelsOut;
	MixMode mix_mode;

public:

	ChannelMixer() {
		setNumberOfInputChannels(DEFAULT_NUMBER_INPUT_CHANNELS);
		setNumberOfOutputChannels(DEFAULT_NUMBER_OUTPUT_CHANNELS);
		mix_mode = DEFAULT_MIXING_LOGIC;
	}

	ChannelMixer(int numChIn, int numChOut) {
		setNumberOfInputChannels(numChIn);
		setNumberOfOutputChannels(numChOut);
		mix_mode = DEFAULT_MIXING_LOGIC;
	}

	~ChannelMixer() {}

	void setNumberOfInputChannels(int numChIn) { mix_numberOfChannelsIn = numChIn; }

	void setNumberOfOutputChannels(int numChOut) { mix_numberOfChannelsOut = numChOut; }

	void setMixMode(MixMode mode) { mix_mode = mode; }

	void processAudio(float* in, float* out) {
		switch (mix_mode) {
		case MixMode::First: {
			if (mix_numberOfChannelsOut == 1)
				out[0] = in[0];
			else if(mix_numberOfChannelsOut == 2) {
				out[0] = in[0];
				out[1] = in[1];
			}
			break;
		}
		case MixMode::WeightedSum: {
			float dummyOut = 0.0;
			for (int i = 0; i < mix_numberOfChannelsIn; i++) dummyOut += in[i];
			if (mix_numberOfChannelsOut == 1)
				out[0] = dummyOut / mix_numberOfChannelsIn;
			else {
				out[0] = dummyOut / (0.5 * mix_numberOfChannelsIn);
				out[1] = out[0];
			}
			break;
		}
		}					
	}

};
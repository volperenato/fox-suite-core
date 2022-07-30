#pragma once
#define DEFAULT_NUMBER_INPUT_CHANNELS 4
#define DEFAULT_NUMBER_OUTPUT_CHANNELS 1

class ChannelMixer {

protected:
	
	int mix_numberOfChannelsIn;
	int mix_numberOfChannelsOut;

public:

	ChannelMixer::ChannelMixer() {
		setNumberOfInputChannels(DEFAULT_NUMBER_INPUT_CHANNELS);
		setNumberOfOutputChannels(DEFAULT_NUMBER_OUTPUT_CHANNELS);
	}

	ChannelMixer::ChannelMixer(int numChIn, int numChOut) {
		setNumberOfInputChannels(numChIn);
		setNumberOfOutputChannels(numChOut);
	}

	ChannelMixer::~ChannelMixer() {}

	void setNumberOfInputChannels(int numChIn) { mix_numberOfChannelsIn = numChIn; }

	void setNumberOfOutputChannels(int numChOut) { mix_numberOfChannelsOut = numChOut; }

	void processAudio(float* in, float* out) {
		float dummyOut = 0.0;
		for (int i = 0; i < mix_numberOfChannelsIn; i++) dummyOut += in[i];
		if (mix_numberOfChannelsOut == 1)
			out[0] = dummyOut / mix_numberOfChannelsIn;
		else {
			out[0] = dummyOut / (0.5 * mix_numberOfChannelsIn);
			out[1] = out[0];
		}			
	}

};
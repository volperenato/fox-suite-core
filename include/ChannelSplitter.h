#pragma once
#define DEFAULT_NUMBER_OF_CHANNELS_SPLITTER_IN 1
#define DEFAULT_NUMBER_OF_CHANNELS_SPLITTER_OUT 4

class ChannelSplitter {

protected:

	int cspl_numberOfChannelsIn;
	int cspl_numberOfChannelsOut;

public:

	ChannelSplitter() {
		setNumberOfChannelsIn(DEFAULT_NUMBER_OF_CHANNELS_SPLITTER_IN);
		setNumberOfChannelsOut(DEFAULT_NUMBER_OF_CHANNELS_SPLITTER_OUT);
	}

	ChannelSplitter(int numChIn, int numChOut) { 
		setNumberOfChannelsIn(numChIn); 
		setNumberOfChannelsOut(numChOut); 
	}

	~ChannelSplitter() { };

	void setNumberOfChannelsIn(int numChIn) { cspl_numberOfChannelsIn = numChIn; }

	void setNumberOfChannelsOut(int numCh) { cspl_numberOfChannelsOut = numCh; }

	int getInputChannels() { return cspl_numberOfChannelsIn;  }

	int getOutputChannels() { return cspl_numberOfChannelsOut; }

	void processAudio(float* in, float* out) { 
		float input;
		if (cspl_numberOfChannelsIn != 1) {
			input = 0;
			for (int i = 0; i < cspl_numberOfChannelsIn; i++) {
				input += in[i];
			}
			input = input / cspl_numberOfChannelsIn;
		}
		else
			input = in[0];

		for (int i = 0; i < cspl_numberOfChannelsOut; i++) out[i] = input; }

};

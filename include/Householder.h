#pragma once
#define DEFAULT_NUMBER_OF_CHANNELS_HOUSEHOLDER 4

class Householder {

protected:

	int hou_numberOfChannels;
	float hou_multiplier;

public:

	Householder() { setNumberOfChannels(DEFAULT_NUMBER_OF_CHANNELS_HOUSEHOLDER); }

	Householder(int numCh) { setNumberOfChannels(numCh); }
	
	void setNumberOfChannels(int numCh) {
		hou_numberOfChannels = numCh;
		hou_multiplier = -2.0 / hou_numberOfChannels;
	}

	void processAudio(float* in, float* out) {	
		float sum = 0.0;
		for (int i = 0; i < hou_numberOfChannels; i++)
			sum += in[i];

		sum *= hou_multiplier;

		for (int i = 0; i < hou_numberOfChannels; i++)
			out[i] = in[i] + sum;
			//arr[i] += sum;
	}
};

#pragma once
#include "CombFilter.h"
#include "Householder.h"

class MultiChannelFeedback {

protected:

	int mcf_numberOfChannels;
	MultiChannelDelay* mcf_MultiChDelay;
	Householder* mcf_Householder;


public:

	void setNumberOfChannels(int numCh);
	void processAudio(float* in, float* out);


};

#pragma once
#include "Delay.h"

class AllPassFilter : public Delay 
{
protected:
	float ap_feedbackGain;
	float ap_decayInSeconds;
public:
	AllPassFilter();
	~AllPassFilter();
	void setFeedback(float g);
	void setFeedbackFromDecay(float decay);
	virtual float processAudio(float xn) override;
};


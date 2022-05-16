#pragma once
#include "Delay.h"

class CombFilter : public Delay
{
protected:
	float cf_feedbackGain;
	float cf_decayInSeconds;
public:
	CombFilter();
	~CombFilter();
	void setFeedback(float g);
	void setFeedbackFromDecay(float decay);
	virtual float processAudio(float xn) override;
	void setFeedbackToNegative();
	void setFeedbackToPositive();
};


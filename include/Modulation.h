#pragma once
#include "ModDelay.h"
#include "utils.h"

#define DEFAULT_MOD_TYPE ModulationType::Chorus
#define DEFAULT_MOD_BUFFER_LENGTH 40.0

class Modulation : private ModDelay {

protected:

	// Modulation type
	ModulationType mod_type;

	// wet level
	float mod_wet;

	// dry level
	float mod_dry;

	float mod_mix;

	// Mod feedback
	float mod_fb;

	// Modulation rate
	float mod_rate;

	// Modulation depth
	float mod_depth;

	// Modulation value in msec
	float mod_modValue;

private:

	void setModulationValue(float modVal) {
		mod_modValue = modVal;
		ModDelay::setDeltaDelayValue(modVal);
	}

public:

	Modulation() {}

	~Modulation() {}

	void init(ModulationType type, int sampleRate) {
		ModDelay::init(DEFAULT_MOD_BUFFER_LENGTH, sampleRate);
		setModType(type);				
	}

	void setModDry(float dry) {
		mod_dry = dry;
	}

	void setModWet(float wet) {
		mod_wet = wet;
	}

	void setModDepth(float depth) {
		mod_depth = depth;
		ModDelay::setDeltaDelayValue(mod_modValue * mod_depth);
	}

	void setModRate(float rate) {
		mod_rate = rate;
		ModDelay::setModRate(rate);
	}

	void setModFeedback(float fb) {
		mod_fb = fb;
		ModDelay::setFeedback(fb);
	}

	void setModMix(float mix) {
		mod_mix = mix;
	}

	void setSampleRate(int sampleRate) {
		ModDelay::setSampleRate(sampleRate);
	}

	void setModType(ModulationType modType) {
		mod_type = modType;
		OscillatorType lfo_type = OscillatorType::Triangular;

		float feedback = 0.0;
		float wet = 1.0;
		float dry = 0.0;
		bool isUnipolar = true;
		float meanDel = 0.0;
		float deltaDel = 0.0;
		float minDel, maxDel;
		float mix = 0.0;

		switch (mod_type) {
		case ModulationType::Flanger: {
			minDel = 1.0;
			maxDel = 7.0;
			wet = 0.5;
			dry = 0.5;
			mix = 0.5;
			feedback = 0.0;
			lfo_type = OscillatorType::Triangular;
			isUnipolar = true;
			meanDel = minDel;
			deltaDel = (maxDel - minDel);
			break;
		}

		case ModulationType::Vibrato: {
			minDel = 0.1;
			maxDel = 5.0;
			wet = 1.0;
			dry = 0.0;
			feedback = 0.0;
			lfo_type = OscillatorType::Sine;
			isUnipolar = true;
			meanDel = minDel;
			deltaDel = (maxDel - minDel);
			break;
		}

		case ModulationType::Chorus: {
			minDel = 1.0;
			maxDel = 20.0;
			wet = 0.5;
			dry = 1.0;
			feedback = 0.0;
			lfo_type = OscillatorType::Triangular;
			isUnipolar = false;
			deltaDel = (maxDel - minDel) / 2.0;
			meanDel = minDel + deltaDel;
			break;
		}
		case ModulationType::WhiteChorus: {
			minDel = 1.0;
			maxDel = 20.0;
			wet = 0.5;
			dry = 1.0;
			feedback = 0.7;
			lfo_type = OscillatorType::Triangular;
			isUnipolar = false;
			deltaDel = (maxDel - minDel) / 2.0;
			meanDel = minDel + deltaDel;
			break;
		}
		}

		ModDelay::setLFOWaveform(lfo_type);
		ModDelay::setLFOUnipolar(isUnipolar);
		ModDelay::setDelayInmsec(meanDel);
		setModulationValue(deltaDel);
		setModMix(mix);
		setModDry(dry);
		setModWet(wet);
		setModFeedback(feedback);
	}

	float processAudio(float xn) {	
		float yn = ModDelay::processAudio(xn);
		return mod_dry * xn + mod_wet * yn;
	}

	// Getter methods
	float getModDepth() {
		return mod_depth;
	}

	float getModRate() {
		return mod_rate;
	}

	float getModValue() {
		return mod_modValue;
	}

};
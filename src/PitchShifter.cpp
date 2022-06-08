#include "PitchShifter.h"

void PitchShifter::processAudio(float* input, float* output) {
	float inL = input[0];
	float inR = input[1];

	output[0] = inL;
	output[1] = inR;
}

void PitchShifter::setSampleRate(int sampleRate) {

}
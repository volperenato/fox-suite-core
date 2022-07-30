#pragma once
#include <math.h>
#define DEFAULT_NUMBER_OF_CHANNELS_HADAMARD 4

class Hadamard {

protected:

	int had_numberOfChannels;

public:

	Hadamard() { setNumberOfChannels(DEFAULT_NUMBER_OF_CHANNELS_HADAMARD); }

	Hadamard(int numCh) { setNumberOfChannels(numCh); }

	~Hadamard() {}

	void setNumberOfChannels(int numCh) { had_numberOfChannels = numCh; }

	static void hadamardUnscaled(float* data, int size) {
		if (size <= 1) return;
		int hSize = size / 2;

		// Two (unscaled) Hadamards of half the size
		hadamardUnscaled(data, hSize);
		hadamardUnscaled(data + hSize, hSize);

		// Combine the two halves using sum/difference
		for (int i = 0; i < hSize; ++i) {
			double a = data[i];
			double b = data[i + hSize];
			data[i] = (a + b);
			data[i + hSize] = (a - b);
		}
	}

	static void hadamardScaled(float* data, int size) {
		hadamardUnscaled(data, size);

		float scalingFactor = sqrt(1.0 / size);
		for (int c = 0; c < size; ++c) {
			data[c] *= scalingFactor;
		}
	}

	void processAudio(float* in, float* out) {
		// Allocate the input into the output
		out = in;

		// Process output with hadamard
		hadamardScaled(out, had_numberOfChannels);
	}

};

#pragma once
#include <math.h>
#include <string>
#define DEFAULT_NUMBER_OF_CHANNELS_HADAMARD 4

class Hadamard {

protected:

	int had_numberOfChannels;
	float had_scalingFactor;
	int** had_matrix;

public:

	Hadamard() { 
		had_matrix = nullptr;
		setNumberOfChannels(DEFAULT_NUMBER_OF_CHANNELS_HADAMARD); 
	}

	Hadamard(int numCh) { 
		had_matrix = nullptr;
		setNumberOfChannels(numCh); 
	}

	~Hadamard() { deleteMatrix(); }

	void deleteMatrix() {
		if (had_matrix) {
			for (int i = 0; i < had_numberOfChannels; i++) {
				delete[] had_matrix[i];
			}
			delete[] had_matrix;
		}
		had_matrix = nullptr;
	}

	void setNumberOfChannels(int numCh) {
		deleteMatrix();
		had_numberOfChannels = numCh;	
		had_scalingFactor = sqrt(1.0 / had_numberOfChannels);
		had_matrix = new int* [had_numberOfChannels];
		for (int i = 0; i < had_numberOfChannels; i++) had_matrix[i] = new int[had_numberOfChannels];
		had_matrix[0][0] = 1;
		for (int x = 1;x < had_numberOfChannels;x += x) {
			for (int i = 0;i < x;i++) {
				for (int j = 0;j < x;j++) {
					had_matrix[i + x][j] = had_matrix[i][j];
					had_matrix[i][j + x] = had_matrix[i][j];
					had_matrix[i + x][j + x] = -had_matrix[i][j];
				}
			}
		}		
	}

	void hadamardUnscaled(float* data, int size) {
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

	void hadamardScaled(float* data, int size) {
		hadamardUnscaled(data, size);

		float scalingFactor = sqrt(1.0 / size);
		for (int c = 0; c < size; ++c) {
			data[c] *= scalingFactor;
		}
	}

	void multiplyHadamard(float* in, float* out) {
		for (int i = 0; i < had_numberOfChannels; i++) {			
			out[i] = 0.0;
			for (int j = 0; j < had_numberOfChannels; j++) {
				out[i] += had_matrix[i][j] * in[j];
			}
			out[i] *= had_scalingFactor;
		}
	}

	void processAudio(float* in, float* out) {
		// Allocate the input into the output
		for (int i = 0; i < had_numberOfChannels; i++) out[i] = in[i];

		// Process output with hadamard
		hadamardScaled(out, had_numberOfChannels);
		//multiplyHadamard(in, out);
	}

};

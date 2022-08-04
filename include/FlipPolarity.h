#pragma once

class FlipPolarity {

protected:

	std::vector<int> fp_flipPolarity;
	int fp_numofChannels;

public:

	FlipPolarity(int numCh) { 
		setNumberOfChannels(numCh); 
	}

	~FlipPolarity() { deleteFlipVector(); }

	void setNumberOfChannels(int numCh) { 
		fp_numofChannels = numCh; 
		deleteFlipVector();			
		for (int i = 0; i < fp_numofChannels; i++)
			if (rand() % 2)
				fp_flipPolarity.push_back(1);
			else
				fp_flipPolarity.push_back(-1);
	}

	void processAudio(float* in, float* out) {
		for (int i = 0; i < fp_numofChannels; i++) {			
			out[i] = in[i] * fp_flipPolarity[i];
		}
	}

private:

	void deleteFlipVector() {
		if (!fp_flipPolarity.empty())
			fp_flipPolarity.clear();
	}
};

class HPFButterworth {

protected:

	// hpf cutoff frequency
	float hpf_cutoffFreq;

	// internal sample rate
	int hpf_sampleRate;

	// hpf gains
	float hpf_a0;
	float hpf_a1;
	float hpf_a2;
	float hpf_b1;
	float hpf_b2;

	// input buffers
	float hpf_xn_1;
	float hpf_xn_2;

	// output buffers
	float hpf_yn_1;
	float hpf_yn_2;

public:

	HPFButterworth();
	~HPFButterworth();
	void init(int sampleRate);
	void setSampleRate(int sampleRate);
	void setCutoffFrequency(float cutoffFreq);
	void updateGains();
	void updateBuffers(float xn, float yn);
	virtual float processAudio(float xn);
};

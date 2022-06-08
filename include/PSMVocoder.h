#pragma once
#include <memory>
#include <algorithm>
#include <math.h>
#include "guiconstants.h"
#include <time.h>       /* time */

#define HAVE_FFTW 1

/**
@getMagnitude
\ingroup FX-Functions

@brief calculates magnitude of a complex numb er

\param re - complex number real part
\param im - complex number imaginary part
\return the magnitude value
*/
inline double getMagnitude(double re, double im)
{
	return sqrt((re * re) + (im * im));
}

/**
@getPhase
\ingroup FX-Functions

@brief calculates phase of a complex numb er

\param re - complex number real part
\param im - complex number imaginary part
\return the phase value
*/
inline double getPhase(double re, double im)
{
	return atan2(im, re);
}

/**
@principalArg
\ingroup FX-Functions

@brief calculates proncipal argument of a phase value; this is the wrapped value on the range
of [-pi, +pi]

\param phaseIn - value to convert
\return the phase value on the range of [-pi, +pi]
*/
inline double principalArg(double phaseIn)
{
	if (phaseIn >= 0)
		return fmod(phaseIn + kPi, kTwoPi) - kPi;
	else
		return fmod(phaseIn + kPi, -kTwoPi) + kPi;
}


enum class interpolation { kLinear, kLagrange4 };


/**
@doLinearInterpolation
\ingroup FX-Functions

@brief performs linear interpolation of fractional x distance between two adjacent (x,y) points;
returns interpolated value

\param y1 - the y coordinate of the first point
\param y2 - the 2 coordinate of the second point
\param x - the interpolation location as a fractional distance between x1 and x2 (which are not needed)
\return the interpolated value or y2 if the interpolation is outside the x interval
*/
inline double doLinearInterpolation(double y1, double y2, double fractional_X)
{
	// --- check invalid condition
	if (fractional_X >= 1.0) return y2;

	// --- use weighted sum method of interpolating
	return fractional_X * y2 + (1.0 - fractional_X) * y1;
}

/**
@doLagrangeInterpolation
\ingroup FX-Functions

@brief implements n-order Lagrange Interpolation

\param x - Pointer to an array containing the x-coordinates of the input values
\param y - Pointer to an array containing the y-coordinates of the input values
\param n - the order of the interpolator, this is also the length of the x,y input arrays
\param xbar - The x-coorinates whose y-value we want to interpolate
\return the interpolated value
*/
inline double doLagrangeInterpolation(double* x, double* y, int n, double xbar)
{
	int i, j;
	double fx = 0.0;
	double l = 1.0;
	for (i = 0; i < n; i++)
	{
		l = 1.0;
		for (j = 0; j < n; j++)
		{
			if (j != i)
				l *= (xbar - x[j]) / (x[i] - x[j]);
		}
		fx += l * y[i];
	}
	return (fx);
}

/**
@resample
\ingroup FX-Functions

@brief function that resamples an input array of length N into an output array of
length M. You can set the interpolation type (linear or lagrange) and you can
supply an optional window array that the resampled array will be processed through.

\param input - input array
\param output - output array
\param inLength - length N of input buffer
\param outLength - length M of output buffer
\param interpType - linear or lagrange interpolation
\param scalar - output scaling value (optional)
\param outWindow - output windowing buffer (optional)
\return true if resampling was sucessful
*/
inline bool resample(double* input, double* output, uint32_t inLength, uint32_t outLength,
	interpolation interpType = interpolation::kLinear,
	double scalar = 1.0, double* outWindow = nullptr)
{
	if (inLength <= 1 || outLength <= 1) return false;
	if (!input || !output) return false;

	double x[4] = { 0.0, 0.0, 0.0, 0.0 };
	double y[4] = { 0.0, 0.0, 0.0, 0.0 };

	// --- inc
	double inc = (double)(inLength - 1) / (double)(outLength - 1);

	// --- first point
	if (outWindow)
		output[0] = outWindow[0] * scalar * input[0];
	else
		output[0] = scalar * input[0];

	if (interpType == interpolation::kLagrange4)
	{
		for (unsigned int i = 1; i < outLength; i++)
		{
			// --- find interpolation location
			double xInterp = i * inc;
			uint32_t x1 = (uint32_t)xInterp; // floor?

			if (xInterp > 1.0 && x1 < inLength - 2)
			{
				x[0] = x1 - 1;
				y[0] = input[(int)x[0]];

				x[1] = x1;
				y[1] = input[(int)x[1]];

				x[2] = x1 + 1;
				y[2] = input[(int)x[2]];

				x[3] = x1 + 2;
				y[3] = input[(int)x[3]];

				if (outWindow)
					output[i] = outWindow[i] * scalar * doLagrangeInterpolation(x, y, 4, xInterp);
				else
					output[i] = scalar * doLagrangeInterpolation(x, y, 4, xInterp);
			}
			else // --- linear for outer 2 end pts
			{
				uint32_t x2 = x1 + 1;
				if (x2 >= outLength)
					x2 = x1;
				double y1 = input[x1];
				double y2 = input[x2];

				if (outWindow)
					output[i] = outWindow[i] * scalar * doLinearInterpolation(y1, y2, xInterp - x1);
				else
					output[i] = scalar * doLinearInterpolation(y1, y2, xInterp - x1);
			}
		}
	}
	else // must be linear
	{
		// --- LINEAR INTERP
		for (uint32_t i = 1; i < outLength; i++)
		{
			double xInterp = i * inc;
			uint32_t x1 = (uint32_t)xInterp; // floor?
			uint32_t x2 = x1 + 1;
			if (x2 >= outLength)
				x2 = x1;
			double y1 = input[x1];
			double y2 = input[x2];

			if (outWindow)
				output[i] = outWindow[i] * scalar * doLinearInterpolation(y1, y2, xInterp - x1);
			else
				output[i] = scalar * doLinearInterpolation(y1, y2, xInterp - x1);
		}
	}

	return true;
}



// ------------------------------------------------------------------ //
// --- INTERFACES --------------------------------------------------- //
// ------------------------------------------------------------------ //

/**
\class IAudioSignalProcessor
\ingroup Interfaces
\brief
Use this interface for objects that process audio input samples to produce audio output samples. A derived class must implement the three abstract methods. The others are optional.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class IAudioSignalProcessor
{
public:
	// --- pure virtual, derived classes must implement or will not compile
	//     also means this is a pure abstract base class and is incomplete,
	//     so it can only be used as a base class
	//
	/** initialize the object with the new sample rate */
	virtual bool reset(double _sampleRate) = 0;

	/** process one sample in and out */
	virtual double processAudioSample(double xn) = 0;

	/** return true if the derived object can process a frame, false otherwise */
	virtual bool canProcessAudioFrame() = 0;

	/** set or change the sample rate; normally this is done during reset( ) but may be needed outside of initialzation */
	virtual void setSampleRate(double _sampleRate) {}

	/** switch to enable/disable the aux input */
	virtual void enableAuxInput(bool enableAuxInput) {}

	/** for processing objects with a sidechain input or other necessary aux input
			the return value is optional and will depend on the subclassed object */
	virtual double processAuxInputAudioSample(double xn)
	{
		// --- do nothing
		return xn;
	}

	/** for processing objects with a sidechain input or other necessary aux input
	--- optional processing function
		e.g. does not make sense for some objects to implement this such as inherently mono objects like Biquad
			 BUT a processor that must use both left and right channels (ping-pong delay) would require it */
	virtual bool processAudioFrame(const float* inputFrame,		/* ptr to one frame of data: pInputFrame[0] = left, pInputFrame[1] = right, etc...*/
		float* outputFrame,
		uint32_t inputChannels,
		uint32_t outputChannels)
	{
		// --- do nothing
		return false; // NOT handled
	}
};

// ------------------------------------------------------------------ //
// --- OBJECTS REQUIRING FFTW --------------------------------------- //
// ------------------------------------------------------------------ //

/**
\enum windowType
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the windowing type for FFT algorithms that use it.

- enum class windowType {kNoWindow, kRectWindow, kHannWindow, kBlackmanHarrisWindow, kHammingWindow };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum class windowType { kNoWindow, kRectWindow, kHannWindow, kBlackmanHarrisWindow, kHammingWindow };

/**
@makeWindow
\ingroup FX-Functions

@brief  creates a new std::unique_ptr<double[]> array for a given window lenght and type.

\param windowLength - length of window array (does NOT need to be power of 2)
\param hopSize - hopSize for vococerf applications, may set to 0 for non vocoder use
\param gainCorrectionValue - return variable that contains the window gain correction value
*/
inline std::unique_ptr<double[]> makeWindow(unsigned int windowLength, unsigned int hopSize, windowType window, double& gainCorrectionValue)
{
	std::unique_ptr<double[]> windowBuffer;
	windowBuffer.reset(new double[windowLength]);

	if (!windowBuffer) return nullptr;

	double overlap = hopSize > 0.0 ? 1.0 - (double)hopSize / (double)windowLength : 0.0;
	gainCorrectionValue = 0.0;

	for (uint32_t n = 0; n < windowLength; n++)
	{
		if (window == windowType::kRectWindow)
		{
			if (n >= 1 && n <= windowLength - 1)
				windowBuffer[n] = 1.0;
		}
		else if (window == windowType::kHammingWindow)
		{
			windowBuffer[n] = 0.54 - 0.46 * cos((n * 2.0 * kPi) / (windowLength));
		}
		else if (window == windowType::kHannWindow)
		{
			windowBuffer[n] = 0.5 * (1 - cos((n * 2.0 * kPi) / (windowLength)));
		}
		else if (window == windowType::kBlackmanHarrisWindow)
		{
			windowBuffer[n] = (0.42323 - (0.49755 * cos((n * 2.0 * kPi) / (windowLength))) + 0.07922 * cos((2 * n * 2.0 * kPi) / (windowLength)));
		}
		else if (window == windowType::kNoWindow)
		{
			windowBuffer[n] = 1.0;
		}

		gainCorrectionValue += windowBuffer[n];
	}

	// --- calculate gain correction factor
	if (window != windowType::kNoWindow)
		gainCorrectionValue = (1.0 - overlap) / gainCorrectionValue;
	else
		gainCorrectionValue = 1.0 / gainCorrectionValue;

	return windowBuffer;
}

#ifdef HAVE_FFTW
#include "fftw3.h"

const unsigned int PSM_FFT_LEN = 4096;

/**
\class FastFFT
\ingroup FFTW-Objects
\brief
The FastFFT provides a simple wrapper for the FFTW FFT operation - it is ultra-thin and simple to use.

Audio I/O:
- processes mono inputs into FFT outputs.

Control I/F:
- none.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class FastFFT
{
public:
	FastFFT() {}		/* C-TOR */
	~FastFFT() {
		if (windowBuffer) delete[] windowBuffer;
		destroyFFTW();
	}	/* D-TOR */

	/** setup the FFT for a given framelength and window type*/
	void initialize(unsigned int _frameLength, windowType _window);

	/** destroy FFTW objects and plans */
	void destroyFFTW();

	/** do the FFT and return real and imaginary arrays */
	fftw_complex* doFFT(double* inputReal, double* inputImag = nullptr);

	/** do the IFFT and return real and imaginary arrays */
	fftw_complex* doInverseFFT(double* inputReal, double* inputImag);

	/** get the current FFT length */
	unsigned int getFrameLength() { return frameLength; }

protected:
	// --- setup FFTW
	fftw_complex* fft_input = nullptr;		///< array for FFT input
	fftw_complex* fft_result = nullptr;		///< array for FFT output
	fftw_complex* ifft_input = nullptr;		///< array for IFFT input
	fftw_complex* ifft_result = nullptr;		///< array for IFFT output
	fftw_plan       plan_forward = nullptr;		///< FFTW plan for FFT
	fftw_plan		plan_backward = nullptr;	///< FFTW plan for IFFT

	double* windowBuffer = nullptr;				///< buffer for window (naked)
	double windowGainCorrection = 1.0;			///< window gain correction
	windowType window = windowType::kHannWindow; ///< window type
	unsigned int frameLength = 0;				///< current FFT length
};



/**
\struct PSMVocoderParameters
\ingroup FFTW-Objects
\brief
Custom parameter structure for the Biquad object. Default version defines the biquad structure used in the calculation.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct PSMVocoderParameters
{
	PSMVocoderParameters() {}
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	PSMVocoderParameters& operator=(const PSMVocoderParameters& params)	// need this override for collections to work
	{
		if (this == &params)
			return *this;

		pitchShiftSemitones = params.pitchShiftSemitones;
		enablePeakPhaseLocking = params.enablePeakPhaseLocking;
		enablePeakTracking = params.enablePeakTracking;

		return *this;
	}

	// --- params
	double pitchShiftSemitones = 0.0;	///< pitch shift in half-steps
	bool enablePeakPhaseLocking = false;///< flag to enable phase lock
	bool enablePeakTracking = false;	///< flag to enable peak tracking
};

/**
\struct BinData
\ingroup FFTW-Objects
\brief
Custom structure that holds information about each FFT bin. This includes all information
needed to perform pitch shifting and time stretching with phase locking (optional)
and peak tracking (optional).

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct BinData
{
	BinData() {}
	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	BinData& operator=(const BinData& params)	// need this override for collections to work
	{
		if (this == &params)
			return *this;

		isPeak = params.isPeak;
		magnitude = params.magnitude;
		phi = params.phi;

		psi = params.psi;
		localPeakBin = params.localPeakBin;
		previousPeakBin = params.previousPeakBin;
		updatedPhase = params.updatedPhase;

		return *this;
	}

	/** reset all variables to 0.0 */
	void reset()
	{
		isPeak = false;
		magnitude = 0.0;
		phi = 0.0;

		psi = 0.0;
		localPeakBin = 0;
		previousPeakBin = -1; // -1 is flag
		updatedPhase = 0.0;
	}

	bool isPeak = false;	///< flag for peak bins
	double magnitude = 0.0; ///< bin magnitude angle
	double phi = 0.0;		///< bin phase angle
	double psi = 0.0;		///< bin phase correction
	unsigned int localPeakBin = 0; ///< index of peak-boss
	int previousPeakBin = -1; ///< index of peak bin in previous FFT
	double updatedPhase = 0.0; ///< phase update value
};



/**
\class PhaseVocoder
\ingroup FFTW-Objects
\brief
The PhaseVocoder provides a basic phase vocoder that is initialized to N = 4096 and
75% overlap; the de-facto standard for PSM algorithms. The analysis and sythesis
hop sizes are identical.

Audio I/O:
- processes mono input into mono output.

Control I/F:
- none.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PhaseVocoder
{
public:
	PhaseVocoder() {}		/* C-TOR */
	~PhaseVocoder() {
		if (inputBuffer) delete[] inputBuffer;
		if (outputBuffer) delete[] outputBuffer;
		if (windowBuffer) delete[] windowBuffer;
		destroyFFTW();
	}	/* D-TOR */

	/** setup the FFT for a given framelength and window type*/
	void initialize(unsigned int _frameLength, unsigned int _hopSize, windowType _window);

	/** destroy FFTW objects and plans */
	void destroyFFTW();

	/** process audio sample through vocode; check fftReady flag to access FFT output */
	double processAudioSample(double input, bool& fftReady);

	/** add zero-padding without advancing output read location, for fast convolution */
	bool addZeroPad(unsigned int count);

	/** increment the FFT counter and do the FFT if it is ready */
	bool advanceAndCheckFFT();

	/** get FFT data for manipulation (yes, naked pointer so you can manipulate) */
	fftw_complex* getFFTData() { return fft_result; }

	/** get IFFT data for manipulation (yes, naked pointer so you can manipulate) */
	fftw_complex* getIFFTData() { return ifft_result; }

	/** do the inverse FFT (optional; will be called automatically if not used) */
	void doInverseFFT();

	/** do the overlap-add operation */
	void doOverlapAdd(double* outputData = nullptr, int length = 0);

	/** get current FFT length */
	unsigned int getFrameLength() { return frameLength; }

	/** get current hop size ha = hs */
	unsigned int getHopSize() { return hopSize; }

	/** get current overlap as a raw value (75% = 0.75) */
	double getOverlap() { return overlap; }

	/** set the vocoder for overlap add only without hop-size */
	// --- for fast convolution and other overlap-add algorithms
	//     that are not hop-size dependent
	void setOverlapAddOnly(bool b) { bool overlapAddOnly = b; }

protected:
	// --- setup FFTW
	fftw_complex* fft_input = nullptr;		///< array for FFT input
	fftw_complex* fft_result = nullptr;		///< array for FFT output
	fftw_complex* ifft_result = nullptr;		///< array for IFFT output
	fftw_plan       plan_forward = nullptr;		///< FFTW plan for FFT
	fftw_plan		plan_backward = nullptr;	///< FFTW plan for IFFT

	// --- linear buffer for window
	double* windowBuffer = nullptr;		///< array for window

	// --- circular buffers for input and output
	double* inputBuffer = nullptr;		///< input timeline (x)
	double* outputBuffer = nullptr;		///< output timeline (y)

	// --- index and wrap masks for input and output buffers
	unsigned int inputWriteIndex = 0;			///< circular buffer index: input write
	unsigned int outputWriteIndex = 0;			///< circular buffer index: output write
	unsigned int inputReadIndex = 0;			///< circular buffer index: input read
	unsigned int outputReadIndex = 0;			///< circular buffer index: output read
	unsigned int wrapMask = 0;					///< input wrap mask
	unsigned int wrapMaskOut = 0;				///< output wrap mask

	// --- amplitude correction factor, aking into account both hopsize (overlap)
	//     and the window power itself
	double windowHopCorrection = 1.0;			///< window correction including hop/overlap

	// --- these allow a more robust combination of user interaction
	bool needInverseFFT = false;				///< internal flag to signal IFFT required
	bool needOverlapAdd = false;				///< internal flag to signal overlap/add required

	// --- our window type; you can add more windows if you like
	windowType window = windowType::kHannWindow;///< window type

	// --- counters
	unsigned int frameLength = 0;				///< current FFT length
	unsigned int fftCounter = 0;				///< FFT sample counter

	// --- hop-size and overlap (mathematically related)
	unsigned int hopSize = 0;					///< hop: ha = hs
	double overlap = 1.0;						///< overlap as raw value (75% = 0.75)

	// --- flag for overlap-add algorithms that do not involve hop-size, other
	//     than setting the overlap
	bool overlapAddOnly = false;				///< flag for overlap-add-only algorithms

};


/**
\class PSMVocoder
\ingroup FFTW-Objects
\brief
The PSMVocoder object implements a phase vocoder pitch shifter. Phase locking and peak tracking
are optional.

Audio I/O:
- Processes mono input to mono output.

Control I/F:
- Use PSMVocoderParameters structure to get/set object params.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PSMVocoder : public IAudioSignalProcessor
{
public:
	PSMVocoder() {
		vocoder.initialize(PSM_FFT_LEN, PSM_FFT_LEN / 4, windowType::kHannWindow);  // 75% overlap
	}		/* C-TOR */
	~PSMVocoder() {
		if (windowBuff) delete[] windowBuff;
		if (outputBuff) delete[] outputBuff;

	}	/* D-TOR */

	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		memset(&phi[0], 0, sizeof(double) * PSM_FFT_LEN);
		memset(&psi[0], 0, sizeof(double) * PSM_FFT_LEN);
		if (outputBuff)
			memset(outputBuff, 0, sizeof(double) * outputBufferLength);

		for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
		{
			binData[i].reset();
			binDataPrevious[i].reset();

			peakBins[i] = -1;
			peakBinsPrevious[i] = -1;
		}

		return true;
	}

	/** return false: this object only processes samples */
	virtual bool canProcessAudioFrame() { return false; }

	/** set the pitch shift in semitones (note that this can be fractional too)*/
	void setPitchShift(double semitones)
	{
		// --- this is costly so only update when things changed
		double newAlpha = pow(2.0, semitones / 12.0);
		double newOutputBufferLength = round((1.0 / newAlpha) * (double)PSM_FFT_LEN);

		// --- check for change
		if (newOutputBufferLength == outputBufferLength)
			return;

		// --- new stuff
		alphaStretchRatio = newAlpha;
		ha = hs / alphaStretchRatio;

		// --- set output resample buffer
		outputBufferLength = newOutputBufferLength;

		// --- create Hann window
		if (windowBuff) delete[] windowBuff;
		windowBuff = new double[outputBufferLength];
		windowCorrection = 0.0;
		for (unsigned int i = 0; i < outputBufferLength; i++)
		{
			windowBuff[i] = 0.5 * (1.0 - cos((i * 2.0 * kPi) / (outputBufferLength)));
			windowCorrection += windowBuff[i];
		}
		windowCorrection = 1.0 / windowCorrection;

		// --- create output buffer
		if (outputBuff) delete[] outputBuff;
		outputBuff = new double[outputBufferLength];
		memset(outputBuff, 0, sizeof(double) * outputBufferLength);
	}

	/** find bin index of nearest peak bin in previous FFT frame */
	int findPreviousNearestPeak(int peakIndex)
	{
		if (peakBinsPrevious[0] == -1) // first run, there is no peak
			return -1;

		int delta = -1;
		int previousPeak = -1;
		for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
		{
			if (peakBinsPrevious[i] < 0)
				break;

			int dist = abs(peakIndex - peakBinsPrevious[i]);
			if (dist > PSM_FFT_LEN / 4)
				break;

			if (i == 0)
			{
				previousPeak = i;
				delta = dist;
			}
			else if (dist < delta)
			{
				previousPeak = i;
				delta = dist;
			}
		}

		return previousPeak;
	}

	/** identify peak bins and tag their respective regions of influence */
	void findPeaksAndRegionsOfInfluence()
	{
		// --- FIND PEAKS --- //
		//
		// --- find local maxima in 4-sample window
		double localWindow[4] = { 0.0, 0.0, 0.0, 0.0 };
		int m = 0;
		for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
		{
			if (i == 0)
			{
				localWindow[0] = 0.0;
				localWindow[1] = 0.0;
				localWindow[2] = binData[i + 1].magnitude;
				localWindow[3] = binData[i + 2].magnitude;
			}
			else  if (i == 1)
			{
				localWindow[0] = 0.0;
				localWindow[1] = binData[i - 1].magnitude;
				localWindow[2] = binData[i + 1].magnitude;
				localWindow[3] = binData[i + 2].magnitude;
			}
			else  if (i == PSM_FFT_LEN - 1)
			{
				localWindow[0] = binData[i - 2].magnitude;
				localWindow[1] = binData[i - 1].magnitude;
				localWindow[2] = 0.0;
				localWindow[3] = 0.0;
			}
			else  if (i == PSM_FFT_LEN - 2)
			{
				localWindow[0] = binData[i - 2].magnitude;
				localWindow[1] = binData[i - 1].magnitude;
				localWindow[2] = binData[i + 1].magnitude;
				localWindow[3] = 0.0;
			}
			else
			{
				localWindow[0] = binData[i - 2].magnitude;
				localWindow[1] = binData[i - 1].magnitude;
				localWindow[2] = binData[i + 1].magnitude;
				localWindow[3] = binData[i + 2].magnitude;
			}

			// --- found peak bin!
			if (binData[i].magnitude > 0.00001 &&
				binData[i].magnitude > localWindow[0]
				&& binData[i].magnitude > localWindow[1]
				&& binData[i].magnitude > localWindow[2]
				&& binData[i].magnitude > localWindow[3])
			{
				binData[i].isPeak = true;
				peakBins[m++] = i;

				// --- for peak bins, assume that it is part of a previous, moving peak
				if (parameters.enablePeakTracking)
					binData[i].previousPeakBin = findPreviousNearestPeak(i);
				else
					binData[i].previousPeakBin = -1;
			}
		}

		// --- assign peak bosses
		if (m > 0)
		{
			int n = 0;
			int bossPeakBin = peakBins[n];
			double nextPeak = peakBins[++n];
			int midBoundary = (nextPeak - (double)bossPeakBin) / 2.0 + bossPeakBin;

			if (nextPeak >= 0)
			{
				for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
				{
					if (i <= bossPeakBin)
					{
						binData[i].localPeakBin = bossPeakBin;
					}
					else if (i < midBoundary)
					{
						binData[i].localPeakBin = bossPeakBin;
					}
					else // > boundary, calc next set
					{
						bossPeakBin = nextPeak;
						nextPeak = peakBins[++n];
						if (nextPeak > bossPeakBin)
							midBoundary = (nextPeak - (double)bossPeakBin) / 2.0 + bossPeakBin;
						else // nextPeak == -1
							midBoundary = PSM_FFT_LEN;

						binData[i].localPeakBin = bossPeakBin;
					}
				}
			}
		}
	}

	/** process input sample through PSM vocoder */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double input)
	{
		bool fftReady = false;
		double output = 0.0;

		// --- normal processing
		output = vocoder.processAudioSample(input, fftReady);

		// --- if FFT is here, GO!
		if (fftReady)
		{
			// --- get the FFT data
			fftw_complex* fftData = vocoder.getFFTData();

			if (parameters.enablePeakPhaseLocking)
			{
				// --- get the magnitudes for searching
				for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
				{
					binData[i].reset();
					peakBins[i] = -1;

					// --- store mag and phase
					binData[i].magnitude = getMagnitude(fftData[i][0], fftData[i][1]);
					binData[i].phi = getPhase(fftData[i][0], fftData[i][1]);
				}

				findPeaksAndRegionsOfInfluence();

				// --- each bin data should now know its local boss-peak
				//
				// --- now propagate phases accordingly
				//
				//     FIRST: set PSI angles of bosses
				for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
				{
					double mag_k = binData[i].magnitude;
					double phi_k = binData[i].phi;

					// --- horizontal phase propagation
					//
					// --- omega_k = bin frequency(k)
					double omega_k = kTwoPi * i / PSM_FFT_LEN;

					// --- phase deviation is actual - expected phase
					//     = phi_k -(phi(last frame) + wk*ha
					double phaseDev = phi_k - phi[i] - omega_k * ha;

					// --- unwrapped phase increment
					double deltaPhi = omega_k * ha + principalArg(phaseDev);

					// --- save for next frame
					phi[i] = phi_k;

					// --- if peak, assume it could have hopped from a different bin
					if (binData[i].isPeak)
					{
						// --- calculate new phase based on stretch factor; save phase for next time
						if (binData[i].previousPeakBin < 0)
							psi[i] = principalArg(psi[i] + deltaPhi * alphaStretchRatio);
						else
							psi[i] = principalArg(psi[binDataPrevious[i].previousPeakBin] + deltaPhi * alphaStretchRatio);
					}

					// --- save peak PSI (new angle)
					binData[i].psi = psi[i];

					// --- for IFFT
					binData[i].updatedPhase = binData[i].psi;
				}

				// --- now set non-peaks
				for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
				{
					if (!binData[i].isPeak)
					{
						int myPeakBin = binData[i].localPeakBin;

						double PSI_kp = binData[myPeakBin].psi;
						double phi_kp = binData[myPeakBin].phi;

						// --- save for next frame
						// phi[i] = binData[myPeakBin].phi;

						// --- calculate new phase, locked to boss peak
						psi[i] = principalArg(PSI_kp - phi_kp - binData[i].phi);
						binData[i].updatedPhase = psi[i];// principalArg(PSI_kp - phi_kp - binData[i].phi);
					}
				}

				for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
				{
					double mag_k = binData[i].magnitude;

					// --- convert back
					fftData[i][0] = mag_k * cos(binData[i].updatedPhase);
					fftData[i][1] = mag_k * sin(binData[i].updatedPhase);

					// --- save for next frame
					binDataPrevious[i] = binData[i];
					peakBinsPrevious[i] = peakBins[i];

				}
			}// end if peak locking

			else // ---> old school
			{
				for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
				{
					double mag_k = getMagnitude(fftData[i][0], fftData[i][1]);
					double phi_k = getPhase(fftData[i][0], fftData[i][1]);

					// --- horizontal phase propagation
					//
					// --- omega_k = bin frequency(k)
					double omega_k = kTwoPi * i / PSM_FFT_LEN;

					// --- phase deviation is actual - expected phase
					//     = phi_k -(phi(last frame) + wk*ha
					double phaseDev = phi_k - phi[i] - omega_k * ha;

					// --- unwrapped phase increment
					double deltaPhi = omega_k * ha + principalArg(phaseDev);

					// --- save for next frame
					phi[i] = phi_k;

					// --- calculate new phase based on stretch factor; save phase for next time
					psi[i] = principalArg(psi[i] + deltaPhi * alphaStretchRatio);

					// --- convert back
					fftData[i][0] = mag_k * cos(psi[i]);
					fftData[i][1] = mag_k * sin(psi[i]);
				}
			}


			// --- manually so the IFFT (OPTIONAL)
			vocoder.doInverseFFT();

			// --- can get the iFFT buffers
			fftw_complex* inv_fftData = vocoder.getIFFTData();

			// --- make copy (can speed this up)
			double ifft[PSM_FFT_LEN] = { 0.0 };
			for (uint32_t i = 0; i < PSM_FFT_LEN; i++)
				ifft[i] = inv_fftData[i][0];

			// --- resample the audio as if it were stretched
			resample(&ifft[0], outputBuff, PSM_FFT_LEN, outputBufferLength, interpolation::kLinear, windowCorrection, windowBuff);

			// --- overlap-add the interpolated buffer to complete the operation
			vocoder.doOverlapAdd(&outputBuff[0], outputBufferLength);
		}

		return output;
	}

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return PSMVocoderParameters custom data structure
	*/
	PSMVocoderParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param PSMVocoderParameters custom data structure
	*/
	void setParameters(const PSMVocoderParameters& params)
	{
		if (params.pitchShiftSemitones != parameters.pitchShiftSemitones)
		{
			setPitchShift(params.pitchShiftSemitones);
		}

		// --- save
		parameters = params;
	}

protected:
	PSMVocoderParameters parameters;	///< object parameters
	PhaseVocoder vocoder;				///< vocoder to perform PSM
	double alphaStretchRatio = 1.0;		///< alpha stretch ratio = hs/ha

	// --- FFT is 4096 with 75% overlap
	const double hs = PSM_FFT_LEN / 4;	///< hs = N/4 --- 75% overlap
	double ha = PSM_FFT_LEN / 4;		///< ha = N/4 --- 75% overlap
	double phi[PSM_FFT_LEN] = { 0.0 };	///< array of phase values for classic algorithm
	double psi[PSM_FFT_LEN] = { 0.0 };	///< array of phase correction values for classic algorithm

	// --- for peak-locking
	BinData binData[PSM_FFT_LEN];			///< array of BinData structures for current FFT frame
	BinData binDataPrevious[PSM_FFT_LEN];	///< array of BinData structures for previous FFT frame

	int peakBins[PSM_FFT_LEN] = { -1 };		///< array of current peak bin index values (-1 = not peak)
	int peakBinsPrevious[PSM_FFT_LEN] = { -1 }; ///< array of previous peak bin index values (-1 = not peak)

	double* windowBuff = nullptr;			///< buffer for window
	double* outputBuff = nullptr;			///< buffer for resampled output
	double windowCorrection = 0.0;			///< window correction value
	unsigned int outputBufferLength = 0;	///< lenght of resampled output array
};



#endif //HAVE_FFTW
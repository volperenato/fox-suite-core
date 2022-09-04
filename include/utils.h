#pragma once
#include <math.h>
#include <vector>
#include "constants.h"

enum class LPFilterType {
	Butterworth = 0,
	LinkwitzRiley,
	Shelving,
	DigitalFirstOrder,
	AllPoleFirstOrder,
	AllPoleMMA,
	Vicanek
};

enum class HPFilterType {
	Butterworth = 0,
	LinkwitzRiley,
	Shelving,
	DigitalFirstOrder
};

enum class MixMode {
	WeightedSum,
	First
};

enum class DelayDistribution {
	Exponential,
	RandomInRange,
	Equal,
	Empty
};

enum class DiffuserDelayLogic {
	Doubled,
	Equal,
	Empty
};

enum class ModulationType { 
	Chorus = 0, 
	Vibrato, 
	Flanger,
	WhiteChorus
};

enum class OscillatorType { 
	Saw = 0, 
	Sine, 
	Triangular, 
	Pulse 
};


/*--------------------------------------------------------------------*/
// Convert a value from interval [minValue, maxValue] to [0,1]
inline float mapValueIntoRange(float value, float minvalue, float maxValue)
{
    return minvalue + value * (maxValue - minvalue);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// Convert a value from interval [0,1] to [minValue, maxValue]
inline float mapValueOutsideRange(float value, float minValue, float maxValue)
{
    return (value - minValue) / (maxValue - minValue);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
inline float linearInterp(float x1, float x2, float y1, float y2, float x)
{
	float denom = x2 - x1;
	if (denom == 0)
		return y1; // should not ever happen

	// calculate decimal position of x
	float dx = (x - x1) / (x2 - x1);

	// use weighted sum method of interpolating
	float result = dx * y2 + (1 - dx) * y1;

	return result;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
inline std::vector<float> exponentialVector(float min, float max, int n) {
    std::vector<float> out(n);
    float rate;
    float step = (max - min) / n;
    for (int i = 0; i < n; i++) {
        rate = ((float)i + 1.0) * 2.0 / n;
		out[i] = min + step * (exp(rate) - 0.5);
    }
    return out;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
inline float randomInRange(float min, float max) {
	float unitRand = rand() / float(RAND_MAX);
	return min + unitRand * (max - min);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
inline float parabolicSine(float x) {
	float y = PARABOLIC_SINE_B * x + PARABOLIC_SINE_C * x * abs(x);
	y = PARABOLIC_SINE_P * (y * abs(y) - y) + y;
	return y;
}
/*--------------------------------------------------------------------*/

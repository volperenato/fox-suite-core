#pragma once
#include <math.h>

#define MAX_LPF_FREQUENCY 19000.0
#define MIN_LPF_FREQUENCY 20.0
#define MAX_HPF_FREQUENCY 17000.0
#define MIN_HPF_FREQUENCY 10.0
#define MAX_FREQUENCY 20000.0
#define MIN_FREQUENCY 20.0

// const parameters
const float MAX_LPF_FREQUENCY_LOG = log(MAX_LPF_FREQUENCY);
const float MIN_LPF_FREQUENCY_LOG = log(MIN_LPF_FREQUENCY);
const float MAX_HPF_FREQUENCY_LOG = log(MAX_HPF_FREQUENCY);
const float MIN_HPF_FREQUENCY_LOG = log(MIN_HPF_FREQUENCY);
const float MAX_FREQUENCY_LOG = log(MAX_FREQUENCY);
const float MIN_FREQUENCY_LOG = log(MIN_FREQUENCY);

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
#include "Freeverb.h"

/*--------------------------------------------------------------------*/
#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#define MAX_COMB_FILTER_LENGTH_IN_MS 100.0
#define MAX_PREDELAY_VALUE_IN_MS 300.0
#define MAX_AP_FILTER_LENGTH_IN_MS 50.0
#define MAX_REVERB_DECAY_IN_SECONDS 5.0
#define MAX_SMEARING_VALUE 0.97
#define MIN_MOD_RATE_IN_HZ 0.1
#define MAX_MOD_RATE_IN_HZ 10.0
#define STEREO_SPREAD_COEFFICIENT_IN_MS 1.0
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
// Freeverb constructor
Freeverb::Freeverb() {
    freeverb_wet      = 0.0;
    freeverb_smearing = 0.0;
    freeverb_decaySeconds    = 0.0;
    freeverb_dampingFrequency  = 0.0;
    freeverb_preDelayMilliSeconds = 0.0;
    freeverb_spread   = 0.0;

    /*.......................................*/
    // allocate pre-delay module
    preDelayModule = new CombFilter();

    /*.......................................*/
    // allocate comb filters
    combFiltersL = new LPCombFilter[NUM_COMB_FILTERS];
    combFiltersR = new LPCombFilter[NUM_COMB_FILTERS];

    /*.......................................*/
    // allocate all pass lines
    apFiltersL_input  = new AllPassFilter[NUM_ALLPASS_FILTERS_IN];
    apFiltersR_input  = new AllPassFilter[NUM_ALLPASS_FILTERS_IN];
    apFiltersL_output = new AllPassFilter[NUM_ALLPASS_FILTERS_OUT];
    apFiltersR_output = new AllPassFilter[NUM_ALLPASS_FILTERS_OUT];

    /*.......................................*/
    // init CombFilters delay values
    combFilterDlymsLeft[0] = 25.31; // 29.7;
    combFilterDlymsLeft[1] = 26.94; // 37.1;
    combFilterDlymsLeft[2] = 28.96; // 41.1;
    combFilterDlymsLeft[3] = 30.75; // 43.7;
    combFilterDlymsLeft[4] = 32.24; // 31.3;
    combFilterDlymsLeft[5] = 33.81; // 31.7;
    combFilterDlymsLeft[6] = 35.31; // 37.7;
    combFilterDlymsLeft[7] = 36.70; // 41.7;

    /*.......................................*/
    // init AllPass delay values
    allPassDlymsLeft_input[0] = 1.1;
    allPassDlymsLeft_input[1] = 2.3;
    allPassDlymsLeft_input[2] = 4.7;
    allPassDlymsLeft_output[0] = 7.73; // 1.1;
    allPassDlymsLeft_output[1] = 10.00; // 2.3;
    allPassDlymsLeft_output[2] = 12.61; // 4.7;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// Freeverb distructor
Freeverb::~Freeverb() {
    // Free pre-delay object
    preDelayModule->~CombFilter();

    // Free CombFilter objects
    for (int i = 0; i < NUM_COMB_FILTERS; i++) {
        combFiltersL[i].~LPCombFilter();
        combFiltersR[i].~LPCombFilter();
    }

    // Free AllPass objects
    for (int i = 0; i < NUM_ALLPASS_FILTERS_IN; i++) {
        apFiltersL_input[i].~AllPassFilter();
        apFiltersR_input[i].~AllPassFilter();
    }

    for (int i = 0; i < NUM_ALLPASS_FILTERS_OUT; i++) {
        apFiltersL_output[i].~AllPassFilter();
        apFiltersR_output[i].~AllPassFilter();
    }    
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// Initialize all the objects and parameters
void Freeverb::init(int sampleRate, float rwet, float rdecay, float rdamping, float rsmearing, float rspread, float rpredelay)
{    
    /*.......................................*/
    // init pre-delay
    preDelayModule->init(MAX_PREDELAY_VALUE_IN_MS, sampleRate);
    preDelayModule->setFeedback(0.0);

    // init comb filters
    for (int i = 0; i < NUM_COMB_FILTERS; i++) {
        combFiltersL[i].init(MAX_COMB_FILTER_LENGTH_IN_MS, sampleRate);
        combFiltersL[i].setMakeUpGaindB(-12.0);

        combFiltersR[i].init(MAX_COMB_FILTER_LENGTH_IN_MS, sampleRate);
        combFiltersR[i].setMakeUpGaindB(-12.0);

        // set a negative feedback for comb filters successive to first
        if (i > 0) {
            combFiltersL[i].setFeedbackToNegative();
            combFiltersR[i].setFeedbackToNegative();
        }
    }

    // set comb filters delays
    setCombFiltersDelay();
    
    // init all pass lines      
    for (int i = 0; i < NUM_ALLPASS_FILTERS_IN; i++) {
        apFiltersL_input[i].init(MAX_AP_FILTER_LENGTH_IN_MS, sampleRate);
        apFiltersR_input[i].init(MAX_AP_FILTER_LENGTH_IN_MS, sampleRate);
    }

    for (int i = 0; i < NUM_ALLPASS_FILTERS_OUT; i++) {
        apFiltersL_output[i].init(MAX_AP_FILTER_LENGTH_IN_MS, sampleRate);
        apFiltersR_output[i].init(MAX_AP_FILTER_LENGTH_IN_MS, sampleRate);
    }

    // set all pass filters delays
    setAllPassFiltersDelay();

    /*.......................................*/
    // initialize reverb plug-in parameters
    setReverbWet(rwet);
    setReverbDecayInSeconds(rdecay);
    setReverbDampingFrequency(rdamping);
    setReverbSmearing(rsmearing);
    setReverbSpread(rspread);
    setReverbPreDelayInMilliseconds(rpredelay);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// replace the "setSampleRate" method with user-defined one
void Freeverb::setSampleRate(int sampleRate)
{
    // Call setSampleRate methods on all needed delay lines
    preDelayModule->setSampleRate(sampleRate);

    for (int i = 0; i < NUM_COMB_FILTERS; i++) {
        combFiltersL[i].setSampleRate(sampleRate);
        combFiltersR[i].setSampleRate(sampleRate);
    }

    for (int i = 0; i < NUM_ALLPASS_FILTERS_IN; i++) {
        apFiltersL_input[i].setSampleRate(sampleRate);
        apFiltersR_input[i].setSampleRate(sampleRate);
    }

    for (int i = 0; i < NUM_ALLPASS_FILTERS_OUT; i++) {
        apFiltersL_output[i].setSampleRate(sampleRate);
        apFiltersR_output[i].setSampleRate(sampleRate);
    }
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void Freeverb::setReverbWet(float wet)
{   
    freeverb_wet = wet;
    updateReverbParameters();
}

void Freeverb::setReverbSmearing(float smearing)
{
    freeverb_smearing = smearing * MAX_SMEARING_VALUE;
    for (int i = 0; i < NUM_ALLPASS_FILTERS_IN; i++) {
        apFiltersL_input[i].setFeedback(freeverb_smearing);
        apFiltersR_input[i].setFeedback(freeverb_smearing);
    }

    for (int i = 0; i < NUM_ALLPASS_FILTERS_OUT; i++) {
        apFiltersL_output[i].setFeedback(freeverb_smearing);
        apFiltersR_output[i].setFeedback(freeverb_smearing);
    }
}

void Freeverb::setReverbDecayInSeconds(float decay)
{
    freeverb_decaySeconds = decay;
    for (int i = 0; i < NUM_COMB_FILTERS; i++) {
        combFiltersL[i].setFeedbackFromDecay(freeverb_decaySeconds);
        combFiltersR[i].setFeedbackFromDecay(freeverb_decaySeconds);
    }
}

void Freeverb::setReverbDampingFrequency(float damping)
{
    freeverb_dampingFrequency = damping;
    for (int i = 0; i < NUM_COMB_FILTERS; i++) {
        combFiltersL[i].setCutoffFrequency(freeverb_dampingFrequency);
        combFiltersR[i].setCutoffFrequency(freeverb_dampingFrequency);
    }
}

void Freeverb::setReverbPreDelayInMilliseconds(float predelay)
{
    freeverb_preDelayMilliSeconds = predelay;
    preDelayModule->setDelayInmsec(freeverb_preDelayMilliSeconds);
}

void Freeverb::setReverbSpread(float spread)
{
    freeverb_spread = spread;
    updateReverbParameters();
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
float Freeverb::getReverbWet()
{
    return freeverb_wet;
}
float Freeverb::getReverbSmearing()
{
    return freeverb_smearing;
}
float Freeverb::getReverbDecayInSeconds()
{
    return freeverb_decaySeconds;
}
float Freeverb::getReverbDampingFrequency()
{
    return freeverb_dampingFrequency;
}
float Freeverb::getReverbPreDelayInMilliseconds()
{
    return freeverb_preDelayMilliSeconds;
}
float Freeverb::getReverbSpread()
{
    return freeverb_spread;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// Convert a value from interval [minValue, maxValue] to [0,1]
float Freeverb::mapValueIntoRange(float value, float minvalue, float maxValue)
{
    return minvalue + value * (maxValue - minvalue);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// Convert a value from interval [0,1] to [minValue, maxValue]
float Freeverb::mapValueOutsideRange(float value, float minValue, float maxValue)
{
    return (value - minValue) / (maxValue - minValue);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// set Left and Right All Pass Filters delay lengths in milliseconds
void Freeverb::setCombFiltersDelay()
{    
    //float delay_tmp;
    for (int i = 0; i < NUM_COMB_FILTERS; i++) {
        combFilterDlymsRight[i] = combFilterDlymsLeft[i] + STEREO_SPREAD_COEFFICIENT_IN_MS;
        combFiltersL[i].setDelayInmsec(combFilterDlymsLeft[i]);
        combFiltersR[i].setDelayInmsec(combFilterDlymsRight[i]);
    }
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// set Left and Right All Pass Filters delay lengths in milliseconds
void Freeverb::setAllPassFiltersDelay()
{
    //float delay_tmp;
    for (int i = 0; i < NUM_ALLPASS_FILTERS_IN; i++) {
        allPassDlymsRight_input[i] = allPassDlymsLeft_input[i] + STEREO_SPREAD_COEFFICIENT_IN_MS;
        apFiltersL_input[i].setDelayInmsec(allPassDlymsLeft_input[i]);
        apFiltersR_input[i].setDelayInmsec(allPassDlymsRight_input[i]);
    }

    for (int i = 0; i < NUM_ALLPASS_FILTERS_OUT; i++) {
        allPassDlymsRight_output[i] = allPassDlymsLeft_output[i] + STEREO_SPREAD_COEFFICIENT_IN_MS;
        apFiltersL_output[i].setDelayInmsec(allPassDlymsLeft_output[i]);
        apFiltersR_output[i].setDelayInmsec(allPassDlymsRight_output[i]);
    }
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// Update Left and Right wet coefficients when rev_wet or freeverb_spread change
void Freeverb::updateReverbParameters() {
    float width = mapValueIntoRange(freeverb_spread, -1.0, 1.0);
    wet1 = freeverb_wet * (width / 2.0 + 0.5);
    wet2 = freeverb_wet * (1 - width) / 2.0;
    dry = (1.0 - freeverb_wet);
}
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
// processAudio method. Accepts stereo input
void Freeverb::processAudio(float* in, float* out)
{
    float inL = in[0];
    float inR = in[1];

    // Init output values
    float outputL = 0;
    float outputR = 0;

    // Init input values
    float inputL = 0;
    float inputR = 0;

    // Compute mono input from Left and Right inputs
    float input_mono = (inL + inR) / 2.0;

    // Pre-delay processing 
    inputL = preDelayModule->processAudio(input_mono);
    inputR = preDelayModule->processAudio(input_mono);

    // Input AllPass Filters series processing
    for (int j = 0; j < NUM_ALLPASS_FILTERS_IN; j++) {
        inputL = apFiltersL_input[j].processAudio(inputL);
        inputR = apFiltersR_input[j].processAudio(inputR);
    }

    // Comb Filters parallel processing
    for (int j = 0; j < NUM_COMB_FILTERS; j++) {
        outputL += combFiltersL[j].processAudio(inputL);
        outputR += combFiltersR[j].processAudio(inputR);
    }

    // Output AllPass Filters series processing
    for (int j = 0; j < NUM_ALLPASS_FILTERS_OUT; j++) {
        outputL = apFiltersL_output[j].processAudio(outputL);
        outputR = apFiltersR_output[j].processAudio(outputR);
    }    

    // Stereo spread processing + output allocation
    out[0] = wet1 * outputL + wet2 * outputR + dry * inL;
    out[1] = wet1 * outputR + wet2 * outputL + dry * inR;
}
/*--------------------------------------------------------------------*/

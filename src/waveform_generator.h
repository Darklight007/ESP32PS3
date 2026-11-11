#pragma once

#include "globals.h"
#include "device.hpp"
#include "config.hpp"

#define PI 3.14159265358979323846
#define CHART_POINTS 20

// FUN peaks of set and measured voltage, max-held on Core 0 and taken (reset)
// by the output-collapse check in enforceSoftwareLimits() on Core 1.
extern volatile float g_funSetPeakV, g_funMeasPeakV;

// Waveform function declarations
double sineWave(double t);
double squareWave(double t);
double triangularWave(double t);
double pulseWave(double t);
double sawtoothWave(double t);
double invertedSawtoothWave(double t);
double exponentialDecay(double t);
double exponentialRise(double t);
double logarithmicCurve(double t);
double pwmWave(double t);
double linearChirp(double t);
double cosineWave(double t);
double halfSineWave(double t);
double fullWaveRectifiedSine(double t);
double stepFunction(double t);
double parabolicWave(double t);
double gaussianPulse(double t);
double sincFunction(double t);
double randomNoise(double t);

// Arbitrary waveform functions
double tablePoint(double t);
double arbitraryBank0(double t);
double arbitraryBank1(double t);

// Waveform array
extern Waveform waveforms[];
extern int numWaveforms;

// Function generator functions
bool functionGenerator();
void functionGenerator_demo();

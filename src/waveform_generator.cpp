#include "waveform_generator.h"
#include <math.h>
#include <stdlib.h>

double sineWave(double t)
{
    // sin(...) ranges from -1 to 1, shift to [0,1]
    return (sin(2.0 * PI * t) + 1.0) / 2.0;
}

double squareWave(double t)
{
    // Original: (t < 0.5) ? 1.0 : -1.0 => replace -1.0 with 0.0
    return (t < 0.5) ? 1.0 : 0.0;
}

double triangularWave(double t)
{
    // Original: goes from -1 to 1, now shift to [0,1]
    double val = (t < 0.5) ? (4.0 * t - 1.0) : (-4.0 * t + 3.0);
    return (val + 1.0) / 2.0;
}

double pulseWave(double t)
{
    double dutyCycle = 0.1;
    // Original: (t < dutyCycle) ? 1.0 : -1.0 => replace -1.0 with 0.0
    return (t < dutyCycle) ? 1.0 : 0.0;
}

double sawtoothWave(double t)
{
    // Original: 2.0 * t - 1.0 ranges [-1,1]
    // Shift to [0,1]: (2*t -1 +1)/2 = t
    return t;
}

double invertedSawtoothWave(double t)
{
    // Original: -2.0 * t + 1.0 ranges from 1 at t=0 to -1 at t=1
    // Shift to [0,1]: ((-2t+1)+1)/2 = (2 -2t)/2 = 1 - t
    return 1.0 - t;
}

double exponentialDecay(double t)
{
    // Original: exp(-5.0*t)*2.0 -1.0 in [-1,1]
    // Shift: (val +1)/2
    double val = exp(-5.0 * t) * 2.0 - 1.0;
    return (val + 1.0) / 2.0;
}

double randomNoise(double t)
{
    // Original: random(-1000,1001)/1000.0 ~ [-1,1]
    // Shift to [0,1]:
    double val = (double)random() / (double)RAND_MAX * 2.0 - 1.0;
    // If you previously used random(-1000, 1001), replace with a suitable function that returns an int in [-1000,1000].
    // Then val = (int_random / 1000.0) in [-1,1].
    // Shift:
    return (val + 1.0) / 2.0;
}

double cosineWave(double t)
{
    // cos(...) in [-1,1]
    return (cos(2.0 * PI * t) + 1.0) / 2.0;
}

double halfSineWave(double t)
{
    // sin(PI*t) runs from 0 to 1 and back to 0 over [0,1], never negative
    // No change needed
    return sin(PI * t);
}

double fullWaveRectifiedSine(double t)
{
    // fabs(sin(...)) is already [0,1], no negatives
    return fabs(sin(2.0 * PI * t));
}

double stepFunction(double t)
{
    // Original: (t<0.5)? -1.0 : 1.0
    // Now [0,1]: (t<0.5)?0.0:1.0
    return (t < 0.5) ? 0.0 : 1.0;
}

double parabolicWave(double t)
{
    // Original: -4.0*(t-0.5)^2 +1.0
    // This is already [0,1] (peak at 1, min at 0)
    return -4.0 * (t - 0.5) * (t - 0.5) + 1.0;
}

double gaussianPulse(double t)
{
    // exp(-((t-0.5)^2)*32.0) is always [0,1]
    // No change needed
    return exp(-((t - 0.5) * (t - 0.5)) * 32.0);
}

double sincFunction(double t)
{
    // sin(PI*x)/(PI*x) can be negative
    // Shift from [-1,1] to [0,1]:
    double x = (t - 0.5) * 8.0;
    double val = (x == 0.0) ? 1.0 : sin(PI * x) / (PI * x);
    return (val + 1.0) / 2.0;
}

double exponentialRise(double t)
{
    // Original: ((exp(5.0*t)-1)/(exp(5)-1))*2.0 -1.0 in [-1,1]
    // Shift to [0,1]:
    double val = ((exp(5.0 * t) - 1.0) / (exp(5.0) - 1.0)) * 2.0 - 1.0;
    return (val + 1.0) / 2.0;
}

double logarithmicCurve(double t)
{
    // Original: log10(t*9.0+1.0)*2.0 -0.0 = log10(t*9.0+1.0)*2.0
    // At t=0: log10(1)=0*2=0
    // At t=1: log10(10)=1*2=2
    // Range [0,2], to get [0,1], divide by 2
    return log10(t * 9.0 + 1.0); // now [0,1]
}

double pwmWave(double t)
{
    return (t < PowerSupply.funGenMem.dutyCycle) ? 1.0 : 0;
}

double f0 = 0.0;  // Start frequency
double f1 = 50.0; // End frequency
double linearChirp(double t)
{
    double beta = f1 - f0;
    double val = sin(2.0 * PI * (f0 * t + (beta / 2.0) * t * t));
    // `val` ranges from -1 to 1, so shift it to [0,1]
    return (val + 1.0) / 2.0;
}

double tablePoint(double t)
{
    return PowerSupply.funGenMem.table_points[int(t * 100) % 100][0];
}

double arbitraryBank0(double t)
{
    return PowerSupply.funGenArbitraryMem.arbitrary_points[int(t * CHART_POINTS) % CHART_POINTS][0] / 140.0;
}

double arbitraryBank1(double t)
{
    return PowerSupply.funGenArbitraryMem.arbitrary_points[int(t * CHART_POINTS) % CHART_POINTS][1] / 140.0;
}

// Array of waveform structs
Waveform waveforms[] = {
    {"Sine ", sineWave},
    {"Sawtooth ", sawtoothWave},
    {"Square ", squareWave},
    {"Triangular", triangularWave},
    {"PWM ", pwmWave},
    {"Exponential Decay", exponentialDecay},
    {"Exponential Rise", exponentialRise},
    {"Full Wave Rectified", fullWaveRectifiedSine},
    {"Gaussian Pulse", gaussianPulse},
    {"Inverted Sawtooth", invertedSawtoothWave},
    {"Linear Chirp", linearChirp},
    {"Logarithmic Curve", logarithmicCurve},
    {"Parabolic ", parabolicWave},
    {"Random Noise", randomNoise},
    {"Sinc Function", sincFunction},
    {"Table Points", tablePoint},
    {"Arbitrary Bank 0", arbitraryBank0},
    {"Arbitrary Bank 1", arbitraryBank1},
};

int numWaveforms = sizeof(waveforms) / sizeof(waveforms[0]);

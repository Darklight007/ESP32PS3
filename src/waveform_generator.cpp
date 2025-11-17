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

// Function to generate waveform based on parameters
bool functionGenerator()
{

    static unsigned long startTime = micros();
    unsigned long currentTime = micros();
    double elapsedTime = (currentTime - startTime) / 1'000'000.0;
    double t = fmod(elapsedTime * PowerSupply.funGenMem.frequency, 1.0);
    int selected_row = (int)Utility_objs.table_fun_gen_list->user_data;
    Waveform currentWaveform = waveforms[selected_row];
    double value = currentWaveform.function(t);
    // Serial.println(t);
    double outputValue = value * PowerSupply.funGenMem.amplitude + PowerSupply.funGenMem.offset;

    static double lastOutputValue = 0.0;
    if (outputValue != lastOutputValue)
    {
        PowerSupply.Voltage.SetUpdate(outputValue * PowerSupply.Voltage.adjFactor);
        // PowerSupply.Voltage.adjValue = outputValue;
        lastOutputValue = outputValue;
        // Serial.printf("\nSet output: %8.4f ", outputValue*2000.0);
    }

    // Track minimal change intervals
    //  Serial.printf("\nmonitor output: %1.3f ", monitorMinChanges(value, t) );
    // return (monitorMinChanges(value, t) < (.008));
    return true;
    Serial.print("Waveform: ");
    Serial.print(currentWaveform.name);
    Serial.print(" - Value: ");
    Serial.println(outputValue);
}

void functionGenerator_demo()
{
    static const unsigned long periodTotal = 100000000UL;    // 10 seconds in microseconds
    static const unsigned long periodWave = periodTotal / 3; // ~3.333 seconds per period
    static unsigned long startTime = micros();

    unsigned long currentTime = micros();
    unsigned long elapsedTime = currentTime - startTime;

    unsigned long totalDuration = numWaveforms * periodTotal;
    unsigned long timeInTotal = elapsedTime % totalDuration;

    int currentWaveformIndex = timeInTotal / periodTotal;
    unsigned long timeInWave = timeInTotal % periodTotal;
    unsigned long timeInPeriod = timeInWave % periodWave;
    double t = (double)timeInPeriod / (double)periodWave;

    double amplitude = 16.0;
    double offset = 16.0;

    // Get the current waveform struct
    Waveform currentWaveform = waveforms[currentWaveformIndex];
    double value = currentWaveform.function(t);
    double outputValue = value * amplitude + offset;

    // Get the current waveform name
    const char *currentWaveformName = currentWaveform.name;

    // Print the output value and waveform name
    if (!lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
        return;

    Serial.print("Waveform: ");
    Serial.print(currentWaveformName);
    Serial.print(" - Value: ");
    Serial.println(outputValue);

    // Reduce delay to improve smoothness
    delay(1);
}

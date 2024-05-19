#ifndef FFT_HANDLER_H
#define FFT_HANDLER_H

#include <arduinoFFT.h>
// #include <ArduinoFFT.h>  // Ensure this include is correct based on your project setup

class FFTHandler {
public:
    static const int SAMPLE_SIZE = 128; // Static constant for sample size

    ArduinoFFT<double> FFT;    // FFT object specialized for double
    double vReal[SAMPLE_SIZE]; // Real part array
    double vImag[SAMPLE_SIZE]; // Imaginary part array
    bool sampleReady = false;  // Flag to check if sample is ready
    double peak;

    FFTHandler();
    void shift();
    void computeFFT(double samplingFrequency);
    void push(double newSample);
};

#endif // FFT_HANDLER_H

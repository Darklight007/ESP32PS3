#include "FFTHandler.h"

FFTHandler::FFTHandler() {
    // Initialize vReal and vImag arrays or other initializations as needed
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        vReal[i] = 0.0;
        vImag[i] = 0.0;
    }
}

void FFTHandler::shift() {
    for (int i = 0; i < SAMPLE_SIZE - 1; i++) {
        vReal[i] = vReal[i + 1];
        vImag[i] = 0;
    }
    sampleReady = true;
}

void FFTHandler::computeFFT(double samplingFrequency) {
    FFT.windowing(vReal, SAMPLE_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(vReal, vImag, SAMPLE_SIZE, FFT_FORWARD);
    FFT.complexToMagnitude(vReal, vImag, SAMPLE_SIZE);
    peak = FFT.majorPeak(vReal, SAMPLE_SIZE,samplingFrequency); // samplingFrequency  0.911854103*
}

void FFTHandler::push(double newSample) {
    vReal[SAMPLE_SIZE - 1] = newSample;
    vImag[SAMPLE_SIZE - 1] = 0;
}

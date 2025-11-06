#ifndef DISPOBJECT_H
#define DISPOBJECT_H

#include <algorithm>
#include <vector>
#include <lvgl.h>
#include "myFonts.h"
#include "ESP32Encoder.h"
#include "buzzer.h"

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>

const int MAX_NO_OF_AVG = 128;
// extern  uint64_t MAX_NO_OF_AVG;

// #include "config.hpp"
// #include "gLobalVariables.h"

LV_IMG_DECLARE(p_l);
LV_IMG_DECLARE(p_u);

// extern TFT_eSPI tft;
// extern lv_style_t style_unit;
// extern lv_style_t style_set;

class Histogram
{
public:
    const int length_ = 350;
    lv_coord_t data[350] = {0};
    // double m = 50;
    // double b = 0;
    lv_obj_t *label_binMax;
    lv_obj_t *label_binMin;
    lv_obj_t *label_numberOfBin;
    lv_obj_t *label_binWidth;

    uint16_t binMax = -INFINITY;
    uint16_t binMin = INFINITY;
    uint16_t numberOfBin;
    // double binWidth;
    int maxHist;
    float histWinMin;
    float histWinMax;
    uint16_t cap = 164;

    Histogram() {};
    ~Histogram() {}

    void updateMinMax(int index)
    {
        if (index < binMin)
            binMin = index;
        if (index > binMax)
            binMax = index;
    }

    void rescaleHistogram()
    {
        maxHist = *std::max_element(data, data + length_);
        binMin = length_; // Reset binMin to maximum possible value
        binMax = -1;      // Reset binMax to minimum possible value

        for (int i = 0; i < length_; ++i)
        {
            // data[i] = static_cast<int>(data[i] * cap / maxHist);
            // data[i] = data[i] * cap / maxHist;
            // data[i] -= 10;
            //  if (data[i] < 0) data[i]=0;
            data[i] = std::max(0, data[i] - 3); // Decrement by 3 and clamp to 0 in one step

            if (data[i] != 0)
            {
                if (i < binMin)
                    binMin = i;
                if (i > binMax)
                    binMax = i;
            }
        }
    }

    void operator[](const double &value)
    {
        int Length_ = 328;
        const int index = 10 + static_cast<int>(std::round((value - histWinMin) / (histWinMax - histWinMin) * Length_));

        if (index < 0 || index >= length_)
            return;

        data[index]++;
        updateMinMax(index);

        if (data[index] > (cap + 3))
        {
            rescaleHistogram();
        }
    }
    void Reset()
    {
        for (int iter = 0; iter < length_; ++iter)
        {
            // data[iter] *= cap / maxHist;
            if (data[iter] == 0)
                continue;
            // auto &val = *iter;

            if (iter < binMin)
                binMin = iter;

            if (iter > binMax)
                binMax = iter;
        }

        for (auto &val : data)
            val = 0;
    }
    int RangeMin()
    {
        int iter = 0;
        for (iter = 0; iter < length_; ++iter)
        {
            // data[iter] *= cap / maxHist;
            if (data[iter] != 0)
                break;
            else
                continue;
        }

        return iter;
    }

    int RangeMax()
    {
        int iter = 0;
        for (iter = length_; iter >= 0; --iter)
        {
            // data[iter] *= cap / maxHist;
            if (data[iter] != 0)
                break;
            else
                continue;
        }

        return iter;
    }
};

//***********************************************************
// https://stackoverflow.com/questions/10990618/calculate-rolling-moving-average-in-c
// class MovingStats
// {
// public:
//     double value;
//     double mean;
//     double absMin = +INFINITY, absMax = -INFINITY;
//     bool enable = false;
//     uint16_t NofAvgs = MAX_NO_OF_AVG;
//     uint64_t windowSizeIndex_{0};

// public:
//     double samples_[MAX_NO_OF_AVG]; // Maximum number of average
//     double sum_{0};

//     MovingStats(int n)
//     {
//         SetWindowSize(n);
//     }
//     MovingStats()
//     {
//         SetWindowSize(64);
//     }

//     MovingStats &operator()(double sample)
//     {

//         value = sample;
//         // if (windowSizeIndex_ == 0)
//         //     sum_ = 0;

//         if (absMin > sample)
//         {
//             absMin = sample;
//             //  myTone(NOTE_A3, 1);
//         }

//         if (absMax < sample)
//         {
//             absMax = sample;
//             //  myTone(NOTE_A4, 1);
//         }

//         sum_ += sample;

//         if (windowSizeIndex_ < NofAvgs)

//             samples_[windowSizeIndex_++] = sample;

//         else
//         {
//             double &oldest = samples_[windowSizeIndex_++ % NofAvgs];
//             sum_ -= oldest;
//             oldest = sample;
//         }

//         // history.push_back(sample);
//         // if (history.size() > 100)
//         // history.erase(std::begin(history));

//         return *this;
//     }

//     ~MovingStats() {}

//     void SetWindowSize(uint16_t w)
//     {
//         NofAvgs = w;
//         sum_ = 0; // value;
//         windowSizeIndex_ = 0;
//         // samples_[windowSizeIndex_++] = 0;
//     }
//     operator double() const { return sum_ / std::max((const long long unsigned)1, std::min(windowSizeIndex_, uint64_t(NofAvgs))); }

//     double Mean() const
//     {
//         return sum_ / std::max((const long long unsigned)1, std::min(windowSizeIndex_, uint64_t(NofAvgs)));
//     }

//     double Sum() const { return sum_; }

//     double Rms() const
//     {
//         double total = 0;
//         size_t currentSize = std::min(windowSizeIndex_, uint64_t(NofAvgs));
//         for (int i = 0; i < currentSize; i++)
//             total += samples_[i] * samples_[i];

//         return sqrt(total / double(NofAvgs));
//     }

//     double Variance() const
//     {
//         double mu = Mean();
//         double total = 0;

//         size_t currentSize = std::min(windowSizeIndex_, uint64_t(NofAvgs));
//         for (int i = 0; i < currentSize; i++)
//             total += (samples_[i] - mu) * (samples_[i] - mu);

//         return (currentSize > 1) ? total / (currentSize - 1) : 0.0;
//     }

//     double StandardDeviation() const
//     {
//         return sqrt(Variance());
//     }

//     // Effective resolution:
//     // https://www.eetimes.com/understanding-noise-enob-and-effective-resolution-in-analog-to-digital-converters/
//     // https://training.ti.com/adc-noise-measurement-methods-parameters @12:30
//     double ER(double FSR)
//     {

//         return double(log2(FSR / (StandardDeviation())));
//     }

//     void ResetStats()
//     {
//         SetWindowSize(NofAvgs);
//         absMin = INFINITY;
//         absMax = -INFINITY;
//         sum_ = 0; // value;
//         windowSizeIndex_ = 0;
//     }
// };

class MovingStatistics
{
public:
    // Public member variables
    std::vector<double> samples_;
    double value;
    double absMin = +std::numeric_limits<double>::infinity();
    double absMax = -std::numeric_limits<double>::infinity();
    bool enable = false;
    uint16_t NofAvgs;
    uint64_t windowSizeIndex_{0};
    double sum_{0.0};
    double sum_sq_{0.0}; // Running sum of squares for RMS

    // Constructor with explicit keyword to prevent implicit conversions
    explicit MovingStatistics(uint16_t n = 64)
        : NofAvgs(n), samples_(n, 0.0)
    {
        if (NofAvgs == 0)
        {
            throw std::invalid_argument("Window size must be greater than 0.");
        }
        ResetStats();
    }

    // Overloaded function call operator to add a new sample
    MovingStatistics &operator()(double sample)
    {
        value = sample;

        // Update minimum and maximum
        absMin = std::min(absMin, sample);
        absMax = std::max(absMax, sample);

        sum_ += sample;
        if (windowSizeIndex_ < NofAvgs)
        {
            // Initial filling of the window
            size_t index = windowSizeIndex_++;
            samples_[index] = sample;
            sum_sq_ += sample * sample;
        }
        else
        {
            // Window is full; remove the oldest sample and add the new one
            size_t index = windowSizeIndex_ % NofAvgs;
            double oldest = samples_[index];
            samples_[index] = sample;
            sum_ += -oldest;
            sum_sq_ += sample * sample - oldest * oldest;
            windowSizeIndex_++;
        }

        return *this;
    }

    // Set a new window size
    void SetWindowSize(uint16_t w)
    {
        if (w == 0)
        {
            throw std::invalid_argument("Window size must be greater than 0.");
        }
        NofAvgs = w;
        samples_.resize(NofAvgs, 0.0); // Resize and initialize the vector
        std::fill(samples_.begin(), samples_.end(), 0.0);
        ResetStats();
    }

    // Calculate the mean of the samples
    double Mean() const
    {
        // size_t currentSize = std::min(windowSizeIndex_, static_cast<uint64_t>(NofAvgs));
        return sum_ / std::max(uint64_t(1), std::min(windowSizeIndex_, uint64_t(NofAvgs)));
    }

    // Get the sum of the samples
    double Sum() const
    {
        return sum_;
    }

    // Calculate the Root Mean Square (RMS) of the samples
    double Rms() const
    {
        uint64_t currentSize = std::min(windowSizeIndex_, static_cast<uint64_t>(NofAvgs));
        if (currentSize == 0)
            return 0.0;
        return std::sqrt(sum_sq_ / static_cast<double>(currentSize));
    }

    // Calculate the variance of the samples
    double Variance() const
    {
        size_t n = std::min(windowSizeIndex_, static_cast<uint64_t>(NofAvgs));
        if (n < 2)
            return 0.0;
        double mean = sum_ / static_cast<double>(n);
        double variance = (sum_sq_ - (sum_ * mean)) / static_cast<double>(n - 1);
        return (variance >= 0.0) ? variance : 0.0;
    }

    // Calculate the standard deviation of the samples
    double StandardDeviation() const
    {
        double var = Variance();
        return (var > 0.0) ? std::sqrt(var) : 0.0;
    }

    // Calculate ER (Effective Resolution) given Full Scale Range (FSR)
    double ER(double FSR) const
    {
        double stdDev = StandardDeviation();
        return (stdDev > 0.0) ? std::log2(FSR / stdDev) : 0.0;
    }

    // Reset all statistics and clear the window
    void ResetStats()
    {
        std::fill(samples_.begin(), samples_.end(), 0.0);
        sum_ = 0.0;
        sum_sq_ = 0.0;
        windowSizeIndex_ = 0;
        absMin = +std::numeric_limits<double>::infinity();
        absMax = -std::numeric_limits<double>::infinity();
    }
};

class Moving_Stats
{
public:
    std::vector<double> samples_;
    double value;
    double mean;
    double absMin = +INFINITY, absMax = -INFINITY;
    bool enable = false;
    uint16_t NofAvgs;
    uint64_t windowSizeIndex_{0};
    double sum_{0};

    double M2 = 0.0; // Sum of squares of differences from the current mean

    explicit Moving_Stats(uint16_t n = 64) : NofAvgs(n), samples_(n)
    {
        ResetStats();
    }

    Moving_Stats &operator()(double sample)
    {
        value = sample;

        // Update min and max statistics
        absMin = std::min(absMin, sample);
        absMax = std::max(absMax, sample);

        // Update the mean and variance using Welford's online algorithm adapted for a moving window
        // Update running total sum
        sum_ += sample;
        if (windowSizeIndex_ < NofAvgs)
        {
            // Still filling the initial window
            int index = windowSizeIndex_++;
            double delta = sample - mean;
            mean += delta / windowSizeIndex_;
            M2 += delta * (sample - mean);
            samples_[index] = sample;
        }
        else
        {
            // Window is full, roll over the oldest data point
            int index = windowSizeIndex_ % NofAvgs;
            double oldest = samples_[index];
            double delta = sample - oldest;
            double delta2 = sample - mean;
            sum_ -= oldest;
            mean += delta / NofAvgs;
            M2 += delta2 * (sample - mean) + delta * (oldest - mean);
            samples_[index] = sample;
            windowSizeIndex_++;
            // sum_ += sample;
        }

        // if (windowSizeIndex_ < NofAvgs)
        // {
        //     samples_[windowSizeIndex_++] = sample;
        // }
        // else
        // {
        //     double &oldest = samples_[windowSizeIndex_++ % NofAvgs];
        //     sum_ -= oldest;
        //     oldest = sample;
        // }

        return *this;
    }

    void SetWindowSize(uint16_t w)
    {
        NofAvgs = w;
        samples_.resize(NofAvgs, 0.0); // Resize and initialize the vector
        std::fill(samples_.begin(), samples_.end(), 0.0);
        sum_ = 0;
        windowSizeIndex_ = 0;
        absMin = +INFINITY;
        absMax = -INFINITY;
    }

    double Mean() const
    {
        //   return mean;
        // return value;
        return sum_ / std::max(uint64_t(1), std::min(windowSizeIndex_, uint64_t(NofAvgs)));
    }

    double Sum() const
    {
        return sum_;
    }

    double Rms() const
    {
        double total = 0;
        size_t currentSize = std::min(windowSizeIndex_, uint64_t(NofAvgs));
        for (size_t i = 0; i < currentSize; ++i)
        {
            total += samples_[i] * samples_[i];
        }
        return sqrt(total / currentSize);
    }

    double Variance() const
    {
        // double mu = Mean();
        // double total = 0;
        // size_t currentSize = std::min(windowSizeIndex_, uint64_t(NofAvgs));
        // for (size_t i = 0; i < currentSize; ++i)
        // {
        //     double diff = samples_[i] - mu;
        //     total += diff * diff;
        // }
        // return currentSize > 1 ? total / (currentSize - 1) : 0.0;
        return (windowSizeIndex_ >= NofAvgs) ? M2 / NofAvgs : (windowSizeIndex_ > 1 ? M2 / (windowSizeIndex_ - 1) : 0);
    }

    double StandardDeviation() const
    {
        return sqrt(Variance());
    }

    double ER(double FSR) const
    {
        return log2(FSR / StandardDeviation());
    }

    void ResetStats()
    {

        std::fill(samples_.begin(), samples_.end(), 0.0);
        // samples_.resize(NofAvgs, 0.0);  // Resize and initialize the vector
        sum_ = 0;
        windowSizeIndex_ = 0;
        absMin = +INFINITY;
        absMax = -INFINITY;
        mean = 0.0;
        M2 = 0.0;
    }
};

// Main Class ***********************************************************

struct _bar
{
    lv_obj_t *bar;
    lv_obj_t *bar_minMarker;
    lv_obj_t *bar_maxMarker;
    lv_obj_t *bar_maxValue;

    lv_obj_t *bar_adjValue;
    lv_obj_t *label_measureValue;
    bool changed = true;
    double oldValue;
    double scaleFactor;
    int32_t *curValuePtr; // = &((lv_bar_t *)PowerSupply.Voltage.Bar.bar)->cur_value;
};

struct _statisticsSmallLabels
{
    lv_obj_t *label_unit;
    lv_obj_t *label_setSmallFont;
    lv_obj_t *label_value;
    lv_obj_t *label_mean;
    lv_obj_t *label_std;
    lv_obj_t *label_max;
    lv_obj_t *label_min;
    lv_obj_t *label_fft;
    lv_obj_t *label_legend;
};
class DispObjects
{

private:
public:
    lv_obj_t *label_set;
    lv_obj_t *label_si_prefix;
    lv_obj_t *label_setValue;
    lv_obj_t *label_measureValue;
    lv_obj_t *label_unit;
    lv_obj_t *highlight_adjValue;
    _bar Bar;
    _statisticsSmallLabels statLabels;

    lv_obj_t *lock_img;
    lv_obj_t *unlock_img;

    MovingStatistics measured;
    MovingStatistics Statistics;
    MovingStatistics effectiveResolution;
    
    MovingStatistics rawValueStats;
    //  MovingStats measuredRaw;

    Histogram hist;

    int rawValue;
    double calib_m;
    double calib_1m;
    double calib_b;
    double internalResistance = (80e3 * 80e3) / (80e3 + 80e3); // 40kOhm

    double maxValue;
    double dac_maxValue;
    double adc_maxValue;
    double minValue;
    uint16_t adjValue;
    uint16_t adjOffset;
    double adjFactor;
    uint16_t adjValueOld;
    double adjValueChanged = {true};
    uint16_t adjEEPROMAddress = 0;

    double oldValue;
    unsigned long timeKeeper{0};
    bool changed;

    const char *restrict {"%+07.3f"};
    const lv_font_t *font_measure{&lv_font_montserrat_42}; // dseg_b_48
    const lv_font_t *font_set{&graph_R_16};                //&unscii_16b4 Tauri_R_20
    const lv_font_t *font_unit{&Tauri_R_62};

    lv_color_t measureColor;
    lv_color_t statColor;

    lv_style_t style_measure;
    lv_style_t style_unit;
    lv_style_t style_si_prefix;
    lv_style_t style_deviceColor;
    lv_style_t stat_style1;
    lv_style_t stat_style2;
    lv_style_t style_highlight_adjValue;
    lv_style_t style_set;

    // int tick = 1;
    long rotaryEncoderStep{1};

    bool lock = true;
    ESP32Encoder encoder;

    String adcRate = "20 SPS";
    void SetEncoderPins(int aPintNumber, int bPinNumber, enc_isr_cb_t enc_isr_cb);

    DispObjects();
    ~DispObjects();

    void measureUpdate(double value);
    void StatisticsUpdate(double value);
    void displayUpdate(void);
    void displayUpdate(bool update);
    void statUpdate(void);
    void barUpdate(void);

    void enableSetting(bool onOff);
    char *getValue();
    void SetRotaryStep(double val);
    void SetUpdate(int value);
    void Flush(void);
    void SetEncoderUpdate(void);
    void setLock(bool lck);
    bool getLock();
    void setMeasureColor(lv_color_t color);
    void setStatsColor(lv_color_t color);
    void setup(lv_obj_t *parent, const char *_text, int x, int y, const char *_unit, double maxValue_, double minValue_, double mTick,
               double offset, double factor, const char *si_prefix, const lv_font_t *font_measure = &dseg_b_48, const lv_font_t *font_unit = &Tauri_R_62);
    // const lv_font_t *font_measure, const lv_font_t *font_unit);
    void SetupStyles();
};

#endif
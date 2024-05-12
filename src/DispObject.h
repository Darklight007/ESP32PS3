#ifndef DISPOBJECT_H
#define DISPOBJECT_H

#include <algorithm>
#include <vector>
#include <lvgl.h>
#include "myFonts.h"
#include "ESP32Encoder.h"
#include "buzzer.h"

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
    const int length_ = 328;
    lv_coord_t data[328] = {0};
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
    float maxHist;
    float histWinMin;
    float histWinMax;
    uint16_t cap = 128;

    Histogram(){};
    ~Histogram() {}

    void operator[](double &value)
    {
        int index = round((value - histWinMin) / (histWinMax - histWinMin) * length_);

        // index = std::clamp(index, 0, length_);
        // Serial.printf("\n%i",index);

        if (index < 0 || index >= length_)
            return;

        data[index]++;

        if (index < binMin)
            binMin = index;

        if (index > binMax)
            binMax = index;

 
        if (data[index] > (cap + 2))
        {
            maxHist = *std::max_element(data, data + length_);
            binMax = -100;
            binMin = 1000;
            for (auto &val : data)
            {
                val *= cap / maxHist;
            }

            for (int iter = 0; iter < 328; ++iter)
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
        }
    }
    void Reset()
    {
        for (int iter = 0; iter < 328; ++iter)
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
        for ( iter = 0; iter < 328; ++iter)
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
        for ( iter = 328; iter >=0; --iter)
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
class MovingStats
{
public:
    double value;
    double mean;
    double absMin = +INFINITY, absMax = -INFINITY;
    bool enable = false;
    uint8_t NofAvgs = 128;
    uint64_t windowSizeIndex_{0};

public:
    double samples_[128]; // Maximum number of average
    double sum_{0};

    MovingStats &operator()(double sample)
    {

        value = sample;
        // if (windowSizeIndex_ == 0)
        //     sum_ = 0;

        if (absMin > sample)
        {
            absMin = sample;
            //  myTone(NOTE_A3, 1);
        }

        if (absMax < sample)
        {
            absMax = sample;
            //  myTone(NOTE_A4, 1);
        }

        sum_ += sample;

        if (windowSizeIndex_ < NofAvgs)

            samples_[windowSizeIndex_++] = sample;

        else
        {
            double &oldest = samples_[windowSizeIndex_++ % NofAvgs];
            sum_ -= oldest;
            oldest = sample;
        }

        // history.push_back(sample);
        // if (history.size() > 100)
        // history.erase(std::begin(history));

        return *this;
    }

    ~MovingStats() {}

    void SetWindowSize(uint8_t w)
    {
        NofAvgs = w;
        sum_ = 0; // value;
        windowSizeIndex_ = 0;
        // samples_[windowSizeIndex_++] = 0;
    }
    operator double() const { return sum_ / std::max((const long long unsigned)1,std::min(windowSizeIndex_, uint64_t(NofAvgs))); }
    double Mean() const { return sum_ / std::max((const long long unsigned)1,std::min(windowSizeIndex_, uint64_t(NofAvgs))); }
    double Sum() const { return sum_; }

    double Rms() const
    {
        double total = 0;
        size_t currentSize = std::min(windowSizeIndex_, uint64_t(NofAvgs));
        for (int i = 0; i < currentSize; i++)
            total += samples_[i] * samples_[i];

        return sqrt(total / double(NofAvgs));
    }

    double Variance() const
    {
        double mu = Mean();
        double total = 0;

        size_t currentSize = std::min(windowSizeIndex_, uint64_t(NofAvgs));
        for (int i = 0; i < currentSize; i++)
            total += (samples_[i] - mu) * (samples_[i] - mu);

        return (currentSize > 1) ? total / (currentSize - 1) : 0.0;
    }

    double StandardDeviation() const
    {
        return sqrt(Variance());
    }

    // Effective resolution:
    // https://www.eetimes.com/understanding-noise-enob-and-effective-resolution-in-analog-to-digital-converters/
    // https://training.ti.com/adc-noise-measurement-methods-parameters @12:30
    double ER(double FSR)
    {
        return double(
            log(FSR / (StandardDeviation())) / log(2));
    }

    void ResetStats()
    {
        SetWindowSize(NofAvgs);        
        absMin = INFINITY;
        absMax = -INFINITY;
        // for (int i = 0; i < 128; i++)
        //     samples_[i] = 0;
    }
};

//***********************************************************

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
};

class DispObjects
{

private:
public:
    lv_obj_t *label_set;
    lv_obj_t *label_setValue;
    lv_obj_t *label_measureValue;
    lv_obj_t *label_unit;
    lv_obj_t *label_setSmallFont;    
    lv_obj_t *label_value;
    lv_obj_t *label_mean;
    lv_obj_t *label_std;
    lv_obj_t *label_max;
    lv_obj_t *label_min;
    lv_obj_t *highlight_adjValue;
    _bar Bar;

    lv_obj_t *lock_img;
    lv_obj_t *unlock_img;

    MovingStats measured;
    //  MovingStats measuredRaw;

    Histogram hist;

    int rawValue;
    double calib_m;
    double calib_b;

    double maxValue;
    double minValue;
    double adjValue;
    double adjOffset;
    double adjFactor;
    double adjValueOld;
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
    lv_style_t style_deviceColor;
    lv_style_t stat_style1;
    lv_style_t stat_style2;
    lv_style_t style_highlight_adjValue;
    lv_style_t style_set;

    

    // int tick = 1;
    double rotaryEncoderStep{.0001};

    bool lock = true;
    ESP32Encoder encoder;

    String adcRate = "20 SPS";
    void SetEncoderPins(int aPintNumber, int bPinNumber, enc_isr_cb_t enc_isr_cb);

    DispObjects();
    ~DispObjects();

    void measureUpdate(double value);
    void displayUpdate(void);
    void statUpdate(void);
    void barUpdate(void);

    void enableSetting(bool onOff);
    char *getValue();
    void SetRotaryStep(double val);
    void SetUpdate(double value);
    void Flush(void);
    void SetEncoderUpdate(void);
    void setLock(bool lck);
    bool getLock();
    void setMeasureColor(lv_color_t color);
    void setStatsColor(lv_color_t color);
    void setup(lv_obj_t *parent, const char *_text, int x, int y, const char *_unit, double maxValue_, double mTick,
               double offset, double factor, const lv_font_t *font_measure = &dseg_b_48, const lv_font_t *font_unit = &Tauri_R_62);
    // const lv_font_t *font_measure, const lv_font_t *font_unit);
    void SetupStyles();
};

#endif
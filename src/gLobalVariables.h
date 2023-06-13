
enum class DEVICE
{
    OFF = 0x0000,
    ON = 0x0001,
    CV = 0x0002,
    CC = 0x0004,
    INFO = 0x0008,

    OVP = 0x0010,
    OCP = 0x0020,
    OTP = 0x0040,

    WARNING = 0x0080,
    ERROR = 0x0100,
    FAILURE = 0x0200,
};
DEVICE gDeviceState = DEVICE::OFF;

struct deviceColors
{
    lv_color_t measured;
    lv_color_t plotColor1;
    lv_color_t plotColor2;
    lv_color_t pageColor;
};

std::map<DEVICE, deviceColors> stateColor;

/*measured color, statistics and plot color 1,statistics and plot color 2*/

lv_obj_t *page1, *page2, *page3, *page4, *page5, *page6;
static lv_obj_t *chart;

/**
 * Display 1000 data points with zooming and scrolling.
 * See how the chart changes drawing mode (draw only vertical lines) when
 * the points get too crowded.
 */
static lv_chart_series_t *serV;
static lv_chart_series_t *serI;

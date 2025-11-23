#ifndef LTC2655_H
#define LTC2655_H
// #include <Wire.h>

// Page 19 Table 1 Command
#define WRITE_REGISTER 0b0000     // Write to Input Register n
#define UPDATE_REGISTER 0b0001    //  Update (Power-Up) DAC Register n
#define WRITE_X_UPDATE_ALL 0b0010 // Write to Input Register n, Update (Power-Up) All
#define WRITE_UPDATE 0b0011       // Write to and Update (Power-Up) n
#define POWER_DOWN 0b0100         // Power-Down n
#define POWER_DOWN_CHIP 0b0101    // Power-Down Chip (All DAC’s and Reference)
#define INTERNAL_REFERENCE 0b0110 // Select Internal Reference (Power-Up Reference)
#define EXTERNAL_REFERENCE 0b0111 // Select External Reference (Power-Down Reference)
// Page 19 Table 1 Address
#define DAC_A 0b0000   // DAC A
#define DAC_B 0b0001   // DAC B
#define DAC_C 0b0010   // DAC C
#define DAC_D 0b0011   // DAC D
#define ALL_DAC 0b1111 // All DAC's

typedef enum
{
    REFR_INTERNAL = INTERNAL_REFERENCE,
    REFR_EXTERNAL = EXTERNAL_REFERENCE,
} dacRef_t;

typedef enum
{
    CHANNEL_A = DAC_A,
    CHANNEL_B = DAC_B,
    CHANNEL_C = DAC_C,
    CHANNEL_D = DAC_D,
    ALL_CHANNEL = ALL_DAC,
} dacChannel_t;

class LTC2655
{
protected:

public:
    uint8_t address;
    
    // Constructor
    // DAC address is determined based on "Table 2. Slave Address Map" on page 20
    // of the datasheet
    // https://www.analog.com/media/en/technical-documentation/data-sheets/2655f.pdf
    LTC2655(uint8_t addr = 0x41);

    // Methods - return 0 on success, non-zero on I2C error
    void write(dacChannel_t channel, uint16_t data);
    void update(dacChannel_t channel, uint16_t data);
    uint8_t writeAndPowerAll(dacChannel_t channel, uint16_t data);
    void writeUpdate(dacChannel_t channel, uint16_t data);
    void writeUpdate(int channel, uint16_t data);
    void powerDown(dacChannel_t channel);
    void powerDownChip();
    void setVoltageReference(dacRef_t voltageReferencSource);

private:
    uint8_t sendCommand(int command, dacChannel_t channel, uint16_t data);
};

#endif
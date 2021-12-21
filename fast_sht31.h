//=========================================================================================================
// fast_sht31.h - Defines an efficient class for reading the SHT31 temperature/humidity sensor
//=========================================================================================================
#ifndef _FAST_SHT31_H_
#define _FAST_SHT31_H_
#include <stdint.h>


//=========================================================================================================
// sht31_rep_t - Defines a "repeatability".  Higher repeatability readings take longer
//=========================================================================================================
enum sht31_rep_t
{
    SHT31_LOW =  0x2C10,
    SHT31_MED =  0x2C0D,
    SHT31_HIGH = 0x2C06
};
//=========================================================================================================


//=========================================================================================================
// This class presumes that Wire.begin() has been called and the I2C bus-speed is set before read_x() .
// 
// To account for the possibility of encountering a CRC error while reading the device, the read_x()
// routines will make three attempts
//=========================================================================================================
class CFastSHT31
{
public:

    // Constructor
    CFastSHT31(uint8_t i2c_address, sht31_rep_t repeatability = SHT31_MED);

    // Call this to read the temperature.  Either pointer can be "nullptr" to skip the calculation
    bool     read_c(float* p_temp, int* p_rh = nullptr);
    bool     read_f(float* p_temp, int* p_rh = nullptr);

protected:

    // Reads both temperatures and relative humidity
    bool    read_raw(uint16_t* p_raw_temp, uint16_t* p_raw_rh);

    // This is the address of our device
    uint8_t m_i2c_address;

    // The command that gets written to the SHT31 to begin a measurement
    uint8_t m_command[2];
};

#endif


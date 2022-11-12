// I2Cdev library collection - BME280 I2C device class
// 2022-07-02 Ported to BME280 from BMP085 by DeeEmm <deeemm@deeemm.com>
// Based on I2Cdev library by Jeff Rowberg 
// Based on register information stored in the I2Cdevlib internal database
// 2012-06-28 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     2012-06-28 - initial release, dynamically built

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef _BME280_H_
#define _BME280_H_

#include "I2Cdev.h"
#include <math.h>


#define BME280_ADDRESS                        0x76
#define BME280_DEFAULT_ADDRESS                BME280_ADDRESS

#define BME280_CHIP_ID                        0xD0    

#define BME280_REG_CTRL_HUMID                 0xF2 // Status
#define BME280_REG_STATUS                     0xF3 // Status
#define BME280_REG_CTRL                       0xF4 // Control - (ctrl_meas) 
#define BME280_REG_CONFIG                     0xF5 // Config - rate / filter / interface

#define BME280_TEMP_PRESS_CALIB_DATA_ADDR     UINT8_C(0x88)
#define BME280_HUMIDITY_CALIB_DATA_ADDR       UINT8_C(0xE1)
#define BME280_DATA_ADDR                      UINT8_C(0xF7)

#define BME280_REG_T1_P9                      0x88 // Temp / Pressure cal data start
#define BME280_REG_H1                         0xA1 // Humidity cal data start
#define BME280_REG_H1_H6                      0xE1 // Humidity data

#define BME280_REG_DATA_START                 0xF7 // Contiguous start for data
#define BME280_REG_MSB_BARO                   0xF7 // Pressure MSB
#define BME280_REG_MSB_TEMP                   0xFA // Temp MSB
#define BME280_REG_MSB_HUMIDITY               0xFD // Humidity MSB



// 0xF4 ctrl_meas modes
#define BME280_OSRS_T                        0x00 // Normal
#define BME280_OSRS_P                        0x00 // Normal
#define BME280_MODE                          0x11 // Normal

#define BME280_REG_SOFTRESET                 0xE0

#define BME280_CONCAT_BYTES(msb, lsb)        (((uint16_t)msb << 8) | (uint16_t)lsb)

// Macros related to size 
#define BME280_TEMP_PRESS_CALIB_DATA_LEN     UINT8_C(26)
#define BME280_HUMIDITY_CALIB_DATA_LEN       UINT8_C(7)

#define BME280_B_T_H_DATA_LEN                UINT8_C(8)
#define BME280_B_T_DATA_LEN                  UINT8_C(6)
#define BME280_B_DATA_LEN                    UINT8_C(3)
#define BME280_T_DATA_LEN                    UINT8_C(3)
#define BME280_H_DATA_LEN                    UINT8_C(2)

// Sensor power modes 
#define BME280_SLEEP_MODE                    0b00
#define BME280_FORCED_MODE                   0b01
#define BME280_NORMAL_MODE                   0b10

// Sensor sampling rates
#define BME280_SAMPLE_RATE_0                 0b000 //Skipped - output set to 0x8000 / NaN
#define BME280_SAMPLE_RATE_X1                0b001
#define BME280_SAMPLE_RATE_X2                0b010
#define BME280_SAMPLE_RATE_X4                0b011
#define BME280_SAMPLE_RATE_X8                0b100
#define BME280_SAMPLE_RATE_X16               0b101

//Sensor standby time
#define BME_STANDBY_MS_0_5                   0b000
#define BME_STANDBY_MS_10                    0b110
#define BME_STANDBY_MS_20                    0b111
#define BME_STANDBY_MS_62_5                  0b001
#define BME_STANDBY_MS_125                   0b010
#define BME_STANDBY_MS_250                   0b011
#define BME_STANDBY_MS_500                   0b100
#define BME_STANDBY_MS_1000                  0b101

//sensor filter
#define BME_FILTER_OFF                       0b000
#define BME_FILTER_X2                        0b001
#define BME_FILTER_X4                        0b010
#define BME_FILTER_X8                        0b011
#define BME_FILTER_X16                      0b100
      

class BME280 {
    public:
        BME280();
        BME280(uint8_t address);

        void         initialise(uint8_t address);
        bool         testConnection();

        /* CONTROL register methods */
        uint8_t      getControl();
        void         setControl(uint8_t address, uint8_t value);
        bool         isMeasuring();
        bool         isReadingCalibration(void);

        /* MEASURE register methods */
        double       getTemperature();
        double       getBaro();
        double       getHumidity();

        // convenience methods
        void         getCalibrationCoefficients();
        uint8_t      getChipID();
        
        

   private:
        uint8_t      devAddr;
        uint8_t      dataBuffer[24];
        uint8_t      dataBuffer2[24];
        uint8_t      dataBufferCAL[24];       
        uint8_t      controlBuffer[8];


        int32_t      t_fine;
        int32_t      t_fine_adjust;

        bool         calibrationLoaded;
        bool         rawDataLoaded;
        bool         temperatureDataLoaded;

        
        uint8_t      measureMode;
        
        uint16_t     dig_T1, dig_P1;
        int16_t      dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2, dig_H4, dig_H5;
        uint8_t      dig_H1, dig_H3;
        int8_t       dig_H6;
         
        
        
};

#endif /* _BME280_H_ */

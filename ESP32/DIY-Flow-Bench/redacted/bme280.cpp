/* ============================================
I2Cdev library class for BME280 I2C device
Based on Bosch BME280 datasheet - https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
and also in part on BMP085 I2Cdev device class library by Jeff Rowberg https://github.com/jrowberg/i2cdevlib
Created for DIY Flow Bench project by Mick Percy aka DeeEmm <deeemm@deeemm.com>
DIYFB Project available at https://github.com/DeeEmm/DIY-Flow-Bench

This file to be used with I2Cdev library - https://github.com/jrowberg/i2cdevlib

Changelog:
    2012-06-28 - initial release, dynamically built by Jeff Rowberg <jeff@rowberg.net> 
    2022-07-02 - Ported to BME280 from BMP085 by Mick Percy aka DeeEmm <deeemm@deeemm.com> 
*/

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

#include "BME280.h"
#include <math.h>
#include "I2Cdev.h"
#include "Wire.h"
#include "messages.h"
#include "configuration.h"

/**
 * Default constructor, uses default I2C device address.
 * @see BME280_DEFAULT_ADDRESS
 */
BME280::BME280() {
    devAddr = BME280_DEFAULT_ADDRESS;
}

/**
 * Specific address constructor.
 * @param address Specific device address
 * @see BME280_DEFAULT_ADDRESS
 */
// BME280::BME280(uint8_t address) {
//     devAddr = address;
// }

/**
 * Prepare device for normal usage.
 */
void BME280::initialise(uint8_t address) {
    
    //TODO: getChipID and check for BME / BMP device - display error message if BMP
    
    devAddr = address;
    temperatureDataLoaded = false;
    
    // reset the device using soft-reset - this makes sure the IIR is off, etc.
    // I2Cdev::writeByte(devAddr, BME280_REG_SOFTRESET, 0xB6);
    
    delay(2000); //arbitrary delay for device to wake
    
    // Set device control status
   setControl(BME280_REG_CTRL, BME280_SLEEP_MODE); // making sure sensor is in sleep mode before setting configuration as it otherwise may be ignored
   setControl(BME280_REG_CTRL_HUMID, BME280_SAMPLE_RATE_X1); // Set Humidity sampling rate
   setControl(BME280_REG_CONFIG, BME_STANDBY_MS_1000 | BME_FILTER_X8); // Set rate, filter and SPI interface
   setControl(BME280_REG_CTRL, BME280_SAMPLE_RATE_X2 | BME280_SAMPLE_RATE_X16 | BME280_NORMAL_MODE); // Set temp and baro sampling rate + reset operating mode to normal
    
    delay(2000); //arbitrary delay for NVM

    // load sensor's calibration constants
    getCalibrationCoefficients();

    delay(1000); //arbitrary delay for calibration coefficients to load
    
    //TODO: WAIT FOR CALIBRATION TO BE READ
    // if chip is still reading calibration, delay
      // while (isReadingCalibration())
      //   delay(10);
      
    //from https://github.com/letscontrolit/ESPEasy/issues/1997
    // It takes at least 1.587 sec for valid measurements to complete.
      // The datasheet names this the "T63" moment.
      // 1 second = 63% of the time needed to perform a measurement.
      // if (!timeOutReached(sensor.last_measurement + 1587)) {
      //   return false;
      // }
      
   
}


/**
 * Get Chip ID.
 */
uint8_t BME280::getChipID() {
    //NOTE: Default chip ID for BME is 0x60 can be used to identify BMP280 devices (default 0x076)
    
    return I2Cdev::readByte(devAddr, BME280_CHIP_ID, controlBuffer);
}


bool BME280::isReadingCalibration(void) {
    Messages _message;
    _message.serialPrintf((char*)"Reading BME280 Calibration \n");
    uint8_t const rStatus = I2Cdev::readByte(devAddr, BME280_REG_STATUS, controlBuffer);

    return (rStatus & (1 << 0)) != 0;
}


/**
 * Verify the device is connected and available.
 */
bool BME280::testConnection() {
    return (I2Cdev::readByte(devAddr, BME280_CHIP_ID, controlBuffer)) == 1;
}



/* control register methods */

uint8_t BME280::getControl() {
    I2Cdev::readByte(devAddr, BME280_REG_CTRL, controlBuffer);
    return controlBuffer[0];
}


void BME280::setControl(uint8_t address, uint8_t value) {
    I2Cdev::writeByte(devAddr, address, value);
    measureMode = value;
}


bool BME280::isMeasuring() {
    I2Cdev::readByte(devAddr, BME280_REG_STATUS, controlBuffer);
    
    if (bitRead(controlBuffer[0], 2) == 1) {
        return false;
    } else {
        return true;    
    }
}






void BME280::getCalibrationCoefficients() {
    
    Messages _message;
    
    _message.serialPrintf((char*)"Loading BME280 Calibration Coefficients \n");
    
    uint8_t tpCalDataBuffer[BME280_TEMP_PRESS_CALIB_DATA_LEN];
    uint8_t hCalDataBuffer[BME280_HUMIDITY_CALIB_DATA_LEN];
    
    I2Cdev::readBytes(devAddr, BME280_TEMP_PRESS_CALIB_DATA_ADDR , BME280_TEMP_PRESS_CALIB_DATA_LEN, tpCalDataBuffer);
    
    //struct bme280_calib_data *calib_data = &dev->calib_data;
    
    dig_T1 = (uint16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[1], tpCalDataBuffer[0]);
    dig_T2 = (uint16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[3], tpCalDataBuffer[2]);
    dig_T3 = (uint16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[5], tpCalDataBuffer[4]);
    
    dig_P1 = BME280_CONCAT_BYTES(tpCalDataBuffer[7], tpCalDataBuffer[6]);
    dig_P2 = (int16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[9], tpCalDataBuffer[8]);
    dig_P3 = (int16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[11], tpCalDataBuffer[10]);
    dig_P4 = (int16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[13], tpCalDataBuffer[12]);
    dig_P5 = (int16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[15], tpCalDataBuffer[14]);
    dig_P6 = (int16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[17], tpCalDataBuffer[16]);
    dig_P7 = (int16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[19], tpCalDataBuffer[18]);
    dig_P8 = (int16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[21], tpCalDataBuffer[20]);
    dig_P9 = (int16_t)BME280_CONCAT_BYTES(tpCalDataBuffer[23], tpCalDataBuffer[22]);
    
    dig_H1 = tpCalDataBuffer[25];
    
    
      
//     // temp coefficients - little endian
//     dig_T1 = (uint16_t)((tpCalDataBuffer[0]) >> 8 | ((tpCalDataBuffer[1]) << 8)); 
//     dig_T2 = (int16_t)((tpCalDataBuffer[2] >> 8) | (tpCalDataBuffer[3] << 8));
//     dig_T3 = (int16_t)((tpCalDataBuffer[4] >> 8) | (tpCalDataBuffer[5] << 8));
//      
//     // baro coefficients - little endian
//     dig_P1 = (uint16_t)((tpCalDataBuffer[6]) >> 8 | ((tpCalDataBuffer[7]) << 8));
//     dig_P2 = (int16_t)((tpCalDataBuffer[8] >> 8) | (tpCalDataBuffer[9] << 8));
//     dig_P3 = (int16_t)((tpCalDataBuffer[10] >> 8) | (tpCalDataBuffer[11] << 8));
//     dig_P4 = (int16_t)((tpCalDataBuffer[12] >> 8) | (tpCalDataBuffer[13] << 8));
//     dig_P5 = (int16_t)((tpCalDataBuffer[14] >> 8) | (tpCalDataBuffer[15] << 8));
//     dig_P6 = (int16_t)((tpCalDataBuffer[16] >> 8) | (tpCalDataBuffer[17] << 8));
//     dig_P7 = (int16_t)((tpCalDataBuffer[18] >> 8) | (tpCalDataBuffer[19] << 8));
//     dig_P8 = (int16_t)((tpCalDataBuffer[20] >> 8) | (tpCalDataBuffer[21] << 8));
//     dig_P9 = (int16_t)((tpCalDataBuffer[22] >> 8) | (tpCalDataBuffer[23] << 8));
// 
//     // humidity coefficients - doubles are little endian
//     dig_H1 = (uint8_t)tpCalDataBuffer[24];

    I2Cdev::readBytes(devAddr, BME280_HUMIDITY_CALIB_DATA_ADDR, BME280_HUMIDITY_CALIB_DATA_LEN, hCalDataBuffer);
    
    
    //struct bme280_calib_data *calib_data = &dev->calib_data;
    int16_t dig_H4_lsb;
    int16_t dig_H4_msb;
    int16_t dig_H5_lsb;
    int16_t dig_H5_msb;
    
    dig_H2 = (int16_t)BME280_CONCAT_BYTES(hCalDataBuffer[1], hCalDataBuffer[0]);
    dig_H3 = hCalDataBuffer[2];
    dig_H4_msb = (int16_t)(int8_t)hCalDataBuffer[3] * 16;
    dig_H4_lsb = (int16_t)(hCalDataBuffer[4] & 0x0F);
    dig_H4 = dig_H4_msb | dig_H4_lsb;
    dig_H5_msb = (int16_t)(int8_t)hCalDataBuffer[5] * 16;
    dig_H5_lsb = (int16_t)(hCalDataBuffer[4] >> 4);
    dig_H5 = dig_H5_msb | dig_H5_lsb;
    dig_H6 = (int8_t)hCalDataBuffer[6];
    
    
    // dig_H2 = (int16_t)((hCalDataBuffer[0] >> 8) | (hCalDataBuffer[1] << 8));
    // dig_H3 = (uint8_t)hCalDataBuffer[2];
    // dig_H4 = (int16_t)(((int16_t)hCalDataBuffer[3] << 4) | (int16_t)(hCalDataBuffer[4] & 0xF));
    // dig_H5 = (int16_t)((int16_t)(hCalDataBuffer[5] << 4) | (int16_t)(hCalDataBuffer[4] >> 4));
    // dig_H6 = (int8_t)hCalDataBuffer[6];

    calibrationLoaded = true;
    
    _message.serialPrintf((char*)"BME280 Calibration Coefficients Loaded \n");

}





double BME280::getTemperature() {
    
    Messages _message;
    
    int32_t uncomp_data;
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;
    
    //int32_t var1, var2;
    uint8_t dataBufferT[BME280_T_DATA_LEN];

    // double T;
    
    float T;
    
    //uint8_t error = Wire.endTransmission(true);
    //Wire.requestFrom(BME280_I2C_ADDR, BME280_T_DATA_LEN);
    //Wire.readBytes(dataBufferT, error);
    
    uint8_t bytesReceived = Wire.requestFrom(BME280_I2C_ADDR, BME280_T_DATA_LEN);
    
    if((bool)bytesReceived){ //If received more than zero bytes
       Wire.readBytes(dataBufferT, BME280_T_DATA_LEN);
    } else {
       //TODO: read has failed - handle read error
    }
    
    //I2Cdev::readBytes(devAddr, BME280_REG_MSB_BARO, BME280_T_DATA_LEN, dataBufferT);    
    
    /* Store the parsed register values for temperature data */
    data_msb = (uint32_t)dataBufferT[0] << 12;
    data_lsb = (uint32_t)dataBufferT[1] << 4;
    data_xlsb = ((uint32_t)dataBufferT[2] >> 4) & 0x0F;
    uncomp_data = data_msb | data_lsb | data_xlsb;
    
    int64_t var1, var2;
    //uncomp_data = ((uint32_t)dataBufferT[0] << 12) | ((uint32_t)dataBufferT[1] << 4) | ((dataBufferT[2] >> 4) & 0x0F);
    
    var1 = ((double)uncomp_data) / 16384.0 - ((double)dig_T1) / 1024.0;
    var1 = var1 * ((double)dig_T2);
    var2 = (((double)uncomp_data) / 131072.0 - ((double)dig_T1) / 8192.0);
    var2 = (var2 * var2) * ((double)dig_T3);
    t_fine = (int32_t)(var1 + var2);
    T = (var1 + var2) / 5120.0;


    // var1 = ((((uncomp_data>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
    // var2 = (((((uncomp_data>>4) - ((int32_t)dig_T1)) * ((uncomp_data>>4) - ((int32_t)dig_T1))) >> 12) *
    // ((int32_t)dig_T3)) >> 14;
    // t_fine = var1 + var2;
    // T = (t_fine * 5 + 128) >> 8;
    // T = T / 100;


    temperatureDataLoaded = true;
    
    
//REMOVE:
_message.debugPrintf((char*)"%s - %s - $f", uncomp_data, t_fine, T);
    
    return T;
    
    
    
    
}


double BME280::getBaro() {
    
    Messages _message;
    
    uint32_t uncomp_data;
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;
    
    double pressure_min = 30000.0;
    double pressure_max = 110000.0;
    
    int32_t var1, var2, var3;
    uint8_t dataBufferB[BME280_B_DATA_LEN];

    double B = 0.0;

    if (!temperatureDataLoaded) getTemperature();
    
    I2Cdev::readBytes(devAddr, BME280_REG_MSB_BARO, BME280_B_DATA_LEN, dataBufferB);
 
    /* Store the parsed register values for pressure data */
    data_msb = (uint32_t)dataBufferB[0] << 12;
    data_lsb = (uint32_t)dataBufferB[1] << 4;
    data_xlsb = (uint32_t)dataBufferB[2] >> 4;
    uncomp_data = data_msb | data_lsb | data_xlsb;
 
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var3 = ((double)dig_P3) * var1 * var1 / 524288.0;
    var1 = (var3 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
    
    /* avoid exception caused by division by zero */
    if (var1 > (0.0))  {
         B = 1048576.0 - (double) uncomp_data;
         B = (B - (var2 / 4096.0)) * 6250.0 / var1;
         var1 = ((double)dig_P9) * B * B / 2147483648.0;
         var2 = B * ((double)dig_P8) / 32768.0;
         B = B + (var1 + var2 + ((double)dig_P7)) / 16.0;
    
         if (B < pressure_min) {
             //B = pressure_min;
         } else if (B > pressure_max) {
             //B = pressure_max;
         }

    
    }
    
    return B;
 
 
 
    // // Compensation code From Bosch official API - https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
    // // Returns baro in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
    // var1 = ((double)t_fine/2.0) - 64000.0;
    // var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    // var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    // var2 = (var2/4.0)+(((double)dig_P4) * 65536.0);
    // var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    // var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);
    // if (var1 == 0.0)   {
    //     return 0; // avoid exception caused by division by zero
    // }
    // B = 1048576.0 - (double)rawB;
    // B = (B - (var2 / 4096.0)) * 6250.0 / var1;
    // var1 = ((double)dig_P9) * B * B / 2147483648.0;
    // var2 = B * ((double)dig_P8) / 32768.0;
    // B = B + (var1 + var2 + ((double)dig_P7)) / 16.0;
    // return B; // convert from hPa to kPa;
    
}


double BME280::getHumidity() {
    
    Messages _message;
    
    // double humidity_min = 0.0;
    // double humidity_max = 100.0;
    uint32_t uncomp_data;
    uint32_t data_lsb;
    uint32_t data_msb;
    double var1, var2, var3, var4, var5, var6;
    uint8_t dataBufferH[BME280_H_DATA_LEN];
    double H;
    
    if (!temperatureDataLoaded) getTemperature();
    
    I2Cdev::readBytes(devAddr, BME280_REG_MSB_HUMIDITY, BME280_H_DATA_LEN, dataBufferH);

    /* Store the parsed register values for humidity data */
    data_msb = (uint32_t)dataBufferH[0] << 8;
    data_lsb = (uint32_t)dataBufferH[1];
    uncomp_data = data_msb | data_lsb;
   
   
    var1 = ((double)t_fine) - 76800.0;
    var2 = (((double)dig_H4) * 64.0 + (((double)dig_H5) / 16384.0) * var1);
    var3 = uncomp_data - var2;
    var4 = ((double)dig_H2) / 65536.0;
    var5 = (1.0 + (((double)dig_H3) / 67108864.0) * var1);
    var6 = 1.0 + (((double)dig_H6) / 67108864.0) * var1 * var5;
    var6 = var3 * var4 * (var5 * var6);
    H = (double)(var6 * (1.0 - ((double)dig_H1) * var6 / 524288.0));

    // if (H > humidity_max)  {
    //     H = humidity_max;
    // } else if (H < humidity_min)  {
    //     H = humidity_min;
    // }
    
    return H;


    
}
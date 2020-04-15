#ifndef i2c_SI7021_h
#define i2c_SI7021_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the SI7021-Sensor

CONSUMPTION: sleep 0.6µA, measure 120-180µA, 3.1mA Heater
MEASUREMENT: 12ms for 12bit RH, 11ms for 14bit Temp --> measurement time is combination of these two
ACCURACY     in Main Range 0.4°C, 3% RH

With every humiditymeasurement there is an automatical temperaturmeasurement (just read it afterwards)

You have to trigger the Measurement yourself --> requestHumidity,

########################################################################  */

class SI7021 : public i2cSensor, public manualSensor
{

private:

    /** ######### Register-Map ################################################################# */

    static const uint8_t    I2C_ADDRESS 		                =(0x40);

    static const uint8_t    CMD_MEASURE_HUMIDITY_HOLD           =(0xE5);    // Hold:    clock stretching
    static const uint8_t    CMD_MEASURE_HUMIDITY_NO_HOLD        =(0xF5);    // No Hold: Not Acknowledging read requests
    static const uint8_t    CMD_MEASURE_TEMPERATURE_HOLD        =(0xE3);    // performs an extra measurement
    static const uint8_t    CMD_MEASURE_TEMPERATURE_NO_HOLD     =(0xF3);

    static const uint8_t    CMD_READ_PREVIOUS_TEMPERATURE       =(0xE0);    // performs NO extra measurement,
    static const uint8_t    CMD_RESET                           =(0xFE);
    static const uint8_t    CMD_WRITE_REG1                      =(0xE6);
    static const uint8_t    CMD_READ_REG1                       =(0xE7);
    static const uint8_t        MASK_RESOLUTION                 =(B10000001);
    static const uint8_t            VAL_RESOLUTION_H12_T14      =(0x00);    // takes 10 -12 ms
    static const uint8_t            VAL_RESOLUTION_H11_T11      =(0x01);    // takes 5.8- 7 ms
    static const uint8_t            VAL_RESOLUTION_H10_T13      =(0x80);    // takes 3.7-4.5 ms
    static const uint8_t            VAL_RESOLUTION_H08_T12      =(0x81);    // takes 2.6-3.1 ms
    static const uint8_t        MASK_LOW_VOLTAGE                =(0x40);    // just for reading/checking
    static const uint8_t        MASK_ENABLE_HEATER              =(0x04);    // for dew-point-measurement


    /** ######### function definition ################################################################# */

protected: // TODO: why protected and not private?

    /**< helperfunction */
    uint16_t readValue(const uint8_t registerAdd)
    {
        uint16_t _ret;
        Wire.beginTransmission(I2C_ADDRESS);
        Wire.write(registerAdd);
        Wire.endTransmission();                   // difference to normal read
        Wire.beginTransmission(I2C_ADDRESS);   // difference to normal read
        Wire.requestFrom(I2C_ADDRESS, (uint8_t) 2);
        _ret      = (Wire.read() << 8 );
        _ret      |= Wire.read();
        Wire.endTransmission();
        return _ret;
    };

public:

    SI7021(void)
    {
    };

    /**< resolutions for temp and humidity is combined, influences the duration */
    inline void setResolution(const uint8_t stepH=4)
    {
        uint8_t _value;
        if      (stepH == 1)    _value = VAL_RESOLUTION_H08_T12;
        else if (stepH == 2)    _value = VAL_RESOLUTION_H10_T13;
        else if (stepH == 3)    _value = VAL_RESOLUTION_H11_T11;
        else                    _value = VAL_RESOLUTION_H12_T14; // takes about 12ms
        i2c.setRegister(I2C_ADDRESS, CMD_WRITE_REG1, MASK_RESOLUTION, _value);
    };

    /**< for dew-point measurements, takes 3.1mA */
    inline void setHeater(const uint8_t enable=1)
    {
        uint8_t _value;
        if (enable)  _value = 255;
        else         _value = 0;
        i2c.setRegister(I2C_ADDRESS, CMD_WRITE_REG1, MASK_ENABLE_HEATER, _value);
    };

    /**< no function */
    inline void setEnabled(const uint8_t enable=1)
    {

    };

    /**< softwarereset */
    inline void reset(void)
    {
        i2c.writeCMD(I2C_ADDRESS, CMD_RESET);
    };

    /**< set standardvalues */
    inline uint8_t initialize()
    {
        if (i2c.probe(I2C_ADDRESS)==0) return 0;

        setHeater(0);
        setResolution(4);
        triggerMeasurement();   // Manual Mode: Request and Read
        return 1;
    };

    /**< there is also a temperaturemeasurement taken */
    void triggerMeasurement()
    {
        i2c.writeCMD(I2C_ADDRESS, CMD_MEASURE_HUMIDITY_HOLD);
    };

    /**< check for new data, return 1 when Measurement is ready */
    inline uint8_t checkMeasurement(void)
    {
        /**< TODO: Implement */
        return 1; // Measurement finished
    };

    /**<  wait for new data*/
    inline uint8_t awaitMeasurement(void)
    {
        /**< TODO: Implement */
        return 1; // Measurement finished
    };

    /**< inefficient way to get a value */
    float readHumidity(void)
    {
        float     _rh;
        int32_t   _rawHumi;
        _rawHumi  = readValue(CMD_MEASURE_HUMIDITY_HOLD);
        _rh       = (_rawHumi*125.0/65536) - 6;
        return    _rh;
    };

    inline void getHumidity(float& rh) // pass a reference
    {
        int32_t   _rawHumi;
        _rawHumi  = readValue(CMD_MEASURE_HUMIDITY_HOLD);
        rh        = (_rawHumi*125.0/65536) - 6;
    };

    /**< standardcall */
    inline void getMeasurement(float& rh)
    {
        getHumidity(rh);
    }

    /**< inefficient way to get a value */
    float readTemperature(void)
    {
        float     _celsius;
        int32_t   _rawTemp;
        _rawTemp  = readValue(CMD_READ_PREVIOUS_TEMPERATURE);
        _celsius  = (_rawTemp*175.72/65536) - 46.85;
        return    _celsius;
    };

    /**< there is an extra measurement triggered */
    void getTemperature(float& celsius) // pass a reference
    {
        int32_t   _rawTemp;
        _rawTemp  = readValue(CMD_READ_PREVIOUS_TEMPERATURE);
        celsius   = (_rawTemp*175.72/65536) - 46.85;
    };

    /**< not very usefull */
    void requestTemperature()
    {
        i2c.writeCMD(I2C_ADDRESS, CMD_MEASURE_TEMPERATURE_HOLD);
    };

    /**< inefficient way to get a value */
    float readTemperatureReq(void)
    {
        float     _celsius;
        int32_t   _rawTemp;
        _rawTemp  = readValue(CMD_MEASURE_TEMPERATURE_HOLD);
        _celsius  = (_rawTemp*175.72/65536) - 46.85;
        return    _celsius;
    };

    /**< there is an extra measurement triggered */
    void getTemperatureReq(float& celsius) // pass a reference
    {
        int32_t   _rawTemp;
        _rawTemp  = readValue(CMD_MEASURE_TEMPERATURE_HOLD);
        celsius   = (_rawTemp*175.72/65536) - 46.85;
    };


};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PRESET preset = PRESET();

#endif





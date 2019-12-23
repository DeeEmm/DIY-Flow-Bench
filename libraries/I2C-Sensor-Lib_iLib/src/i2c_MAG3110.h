#ifndef i2c_h
#define i2c_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the MAG3310-Sensor

 CONSUMPTION: standby 2 µA, measure 9-900 µA

 Details: up to 80Hz, ±1000 µT, Sensitivity of 0.10 µT


########################################################################  */

class MAG3110 : public i2cSensor, public manualSensor
{

private:

    /** ######### Register-Map ################################################################# */
    static const uint8_t  I2C_ADDRESS               =(0x0E);

    static const uint8_t  REG_DR_STATUS	            =(0x00);
    static const uint8_t  REG_OUT_X_MSB	            =(0x01);
    static const uint8_t  REG_OUT_X_LSB	            =(0x02);
    static const uint8_t  REG_OUT_Y_MSB	            =(0x03);
    static const uint8_t  REG_OUT_Y_LSB	            =(0x04);
    static const uint8_t  REG_OUT_Z_MSB	            =(0x05);
    static const uint8_t  REG_OUT_Z_LSB	            =(0x06);

    static const uint8_t  REG_WHO_AM_I	            =(0x07);
    static const uint8_t      MAG_VAL_WHO_AM_I      =(0xC4);
    static const uint8_t  REG_SYSMOD		        =(0x08);

    static const uint8_t  REG_OFF_X_MSB	            =(0x09);
    static const uint8_t  REG_OFF_X_LSB	            =(0x0A);
    static const uint8_t  REG_OFF_Y_MSB	            =(0x0B);
    static const uint8_t  REG_OFF_Y_LSB	            =(0x0C);
    static const uint8_t  REG_OFF_Z_MSB	            =(0x0D);
    static const uint8_t  REG_OFF_Z_LSB	            =(0x0E);

    static const uint8_t  REG_DIE_TEMP	            =(0x0F);
    static const uint8_t  REG_CTRL_REG1	            =(0x10);
    static const uint8_t      MASK_ACTIVE		    =(0x01);        // standby --> active
    static const uint8_t      MASK_TM		        =(0x02);        // Trigger Measurement
    static const uint8_t      MASK_FR               =(0x04);        // Fast read: skip low-registers
    static const uint8_t      MASK_OSR              =(B00011000);   // Oversampling
    static const uint8_t          VAL_OSR_16	    =(B00000000);
    static const uint8_t          VAL_OSR_32	    =(B00001000);
    static const uint8_t          VAL_OSR_64		=(B00010000);
    static const uint8_t          VAL_OSR_128		=(B00011000);
    static const uint8_t      MASK_DR               =(B11100000);   // datarate
    static const uint8_t          VAL_DR_0	        =(B00000000);
    static const uint8_t          VAL_DR_1	        =(B00100000);
    static const uint8_t          VAL_DR_2	        =(B01000000);
    static const uint8_t          VAL_DR_3	        =(B01100000);
    static const uint8_t          VAL_DR_4	        =(B10000000);
    static const uint8_t          VAL_DR_5	        =(B10100000);
    static const uint8_t          VAL_DR_6	        =(B11000000);
    static const uint8_t          VAL_DR_7	        =(B11100000);

    static const uint8_t  REG_CTRL_REG2	            =(0x11);
    static const uint8_t      MASK_AUTO_MRST	    =(B10000000);   // Magnetic Sensor reset
    static const uint8_t      MASK_RAW			    =(B00100000);   // read uncorrected RAW values
    static const uint8_t      MASK_RESET		    =(B00010000);   // Trigger a Reset

    static const uint8_t  SYSMOD_STANDBY		    =(0x00);
    static const uint8_t  SYSMOD_ACTIVE_RAW	        =(0x01);
    static const uint8_t  SYSMOD_ACTIVE		        =(0x02);

    /** ######### function definition ################################################################# */

public:
    /**< TODO: do i need a constructor? */
    MAG3110(void)
    {
    };

    /**<  Trigger a software-reset*/
    inline void reset(void)
    {
        i2c.setRegister(I2C_ADDRESS,REG_CTRL_REG2,MASK_RESET, 255);
    };

    /**<  recommended: the resets occur automatically before each data acquisition*/
    inline void setSensorAutoReset(const uint8_t enable=1)
    {
        uint8_t _value;
        if (enable) _value = MASK_AUTO_MRST;
        else        _value = 0;
        i2c.setRegister(I2C_ADDRESS,REG_CTRL_REG2,MASK_AUTO_MRST, _value);
    };

    /**< data values are not corrected by the user offset register values. */
    inline void setRawMode(const uint8_t enable=1)
    {
        uint8_t _value;
        if (enable) _value = MASK_RAW;
        else        _value = 0;
        i2c.setRegister(I2C_ADDRESS,REG_CTRL_REG2,MASK_RAW, _value);
    };

    /**< switch between continious and trigger/manual mode */
    inline void setEnabled(const uint8_t enable=1)
    {
        uint8_t _value;
        if (enable) _value = MASK_ACTIVE;
        else        _value = 0;
        i2c.setRegister(I2C_ADDRESS,REG_CTRL_REG1,MASK_ACTIVE, _value);
    };

    inline void setDataRate(const uint8_t hz=10)
    {
        uint8_t _value;
        // use modes with 1280 Hz ADC Rate
        if 		(hz > 40)   _value =  0;  // 80 Hz with 16x OS
        else if (hz > 20) 	_value =  1;  // 40 Hz with 32x OS
        else if (hz > 10) 	_value =  2;  // 20 Hz with 64x OS
        else if (hz >  5)   _value =  3;  // 10 Hz with 128x OS
        else if (hz >= 3)   _value =  7;  //  5 Hz with 128x OS, 640 Hz ADC
        else                _value = 11;  // 2.5Hz with 128x OS, 320 Hz ADC
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG1, (MASK_DR | MASK_OSR), _value<<3); // set combination of data rate and oversampling
    }

    inline uint8_t initialize(void)
    {
        return initialize(100);
    };

    inline uint8_t initialize(const uint8_t hzFreq)
    {
        if (i2c.probe(I2C_ADDRESS)==0) return 0;

        setEnabled(0);

        setSensorAutoReset(1);
        setRawMode(1);
        setDataRate(hzFreq);

        setEnabled(1);

        return 1;
    };

    /**< only used when in manual/standby mode */
    inline void triggerMeasurement(void)
    {
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG1, MASK_TM, MASK_TM);
    };

    /**< check for new data, return 1 when Measurement is ready */
    inline uint8_t checkMeasurement(void)
    {
        uint8_t _val;
        i2c.read(I2C_ADDRESS, REG_DR_STATUS, &_val, 1);
        if (_val & B00001000) return 1; // Measurement finished
        else                  return 0;
    };

    /**<  wait for new data*/
    inline uint8_t awaitMeasurement(void)
    {
        uint8_t _counter = 0;
        while (checkMeasurement()==0)
        {
            if (++_counter > 250) return 0; // check took longer than 250ms
            delay(1);
        };
        return 1; // Measurement finished
    };

    /**< get RAW values */
    void getMeasurement(int16_t xyz_raw[])
    {
        uint8_t buf[6];
        i2c.read(I2C_ADDRESS, REG_OUT_X_MSB, buf, 6);
        xyz_raw[0] = (buf[0]<<8) | buf[1];
        xyz_raw[1] = (buf[2]<<8) | buf[3];
        xyz_raw[2] = (buf[4]<<8) | buf[5];
    };

    /**< values scaled to uT */
    void getMeasurement(float xyz_uT[])
    {
        uint8_t buf[6];
        i2c.read(I2C_ADDRESS, REG_OUT_X_MSB, buf, 6);
        xyz_uT[0] = int16_t((buf[0]<<8) | buf[1])*0.2471f;
        xyz_uT[1] = int16_t((buf[2]<<8) | buf[3])*0.2479f;
        xyz_uT[2] = int16_t((buf[4]<<8) | buf[5])*0.2480f;
    };


};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PRESET preset = PRESET();

#endif





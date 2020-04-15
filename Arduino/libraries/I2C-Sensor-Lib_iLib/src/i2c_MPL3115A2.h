#ifndef i2c_mpl3115a2_h
#define i2c_mpl3115a2_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the MPL3115A2-Sensor

 CONSUMPTION: standby 2µA, measure 260-2000µA

 ONE-TIME-MEASURE: disable sensor, [start measurement, wait, read ] ...
 AUTO-Measure: enable sensor, start measurement, [read, read ] ...


########################################################################  */

class MPL3115A2 : public i2cSensor, public manualSensor
{
private:
    /** ######### Register-Map ################################################################# */

    static const uint8_t  I2C_ADDRESS 	            =(0x60);

    static const uint8_t  REG_STATUS 	            =(0x00); // data ready, previous data unread and overwritten
    static const uint8_t  REG_OUT_P_MSB 	        =(0x01);
    static const uint8_t  REG_OUT_P_CSB 	        =(0x02);
    static const uint8_t  REG_OUT_P_LSB 	        =(0x03);
    static const uint8_t  REG_OUT_T_MSB 	        =(0x04);
    static const uint8_t  REG_OUT_T_LSB 	        =(0x05);
    static const uint8_t  REG_DR_STATUS 	        =(0x06);
    static const uint8_t  REG_OUT_P_DELTA_MSB 	    =(0x07);
    static const uint8_t  REG_OUT_P_DELTA_CSB 	    =(0x08);
    static const uint8_t  REG_OUT_P_DELTA_LSB 	    =(0x09);
    static const uint8_t  REG_OUT_T_DELTA_MSB       =(0x0A);
    static const uint8_t  REG_OUT_T_DELTA_LSB 	    =(0x0B);
    static const uint8_t  REG_WHO_AM_I   	        =(0x0C);
    static const uint8_t    VAL_WHO_AM_I            =(0xC4);

    static const uint8_t  REG_FIFO_STATUS 	        =(0x0D);
    static const uint8_t  REG_FIFO_DATA 	        =(0x0E);
    static const uint8_t  REG_FIFO_SETUP 	        =(0x0F);
    static const uint8_t  REG_TIME_DLY 	            =(0x10);
    static const uint8_t  REG_SYSMOD 	            =(0x11);
    static const uint8_t    VAL_SYS_ACTIVE          =(0x01);
    static const uint8_t    VAL_SYS_STANDBY         =(0x00);

    static const uint8_t  REG_INT_SOURCE 	        =(0x12);
    static const uint8_t  REG_PT_DATA_CFG 	        =(0x13);
    static const uint8_t    MSK_DATA_READY          =(0x04);
    static const uint8_t    MSK_PRES_READY          =(0x02);
    static const uint8_t    MSK_TEMP_READY          =(0x01);

    static const uint8_t  REG_BAR_IN_MSB 	        =(0x14);
    static const uint8_t  REG_BAR_IN_LSB 	        =(0x15);
    static const uint8_t  REG_P_TGT_MSB 	        =(0x16);  // target int16 in meters OR uint16 in 2Pa
    static const uint8_t  REG_P_TGT_LSB 	        =(0x17);
    static const uint8_t  REG_T_TGT 	            =(0x18);
    static const uint8_t  REG_P_WND_MSB 	        =(0x19);
    static const uint8_t  REG_P_WND_LSB 	        =(0x1A);
    static const uint8_t  REG_T_WND 	            =(0x1B);
    static const uint8_t  REG_P_MIN_MSB 	        =(0x1C);
    static const uint8_t  REG_P_MIN_CSB 	        =(0x1D);
    static const uint8_t  REG_P_MIN_LSB 	        =(0x1E);
    static const uint8_t  REG_T_MIN_MSB 	        =(0x1F);
    static const uint8_t  REG_T_MIN_LSB 	        =(0x20);
    static const uint8_t  REG_P_MAX_MSB 	        =(0x21);
    static const uint8_t  REG_P_MAX_CSB 	        =(0x22);
    static const uint8_t  REG_P_MAX_LSB 	        =(0x23);
    static const uint8_t  REG_T_MAX_MSB 	        =(0x24);
    static const uint8_t  REG_T_MAX_LSB 	        =(0x25);

    static const uint8_t  REG_CTRL1 	            =(0x26);  //
    static const uint8_t    MSK_SBYB                =(0x01);
    static const uint8_t    MSK_OST                 =(0x02);  // triggers measurement even when standby (onetime)!!!
    static const uint8_t    MSK_RST                 =(0x04);
    static const uint8_t    MSK_OS                  =(B00111000);
    static const uint8_t    MSK_RAW                 =(B01000000);
    static const uint8_t    MSK_ALT                 =(B10000000);

    static const uint8_t  REG_CTRL2 	            =(0x27);  // can only be modified in standby
    static const uint8_t  REG_CTRL3 	            =(0x28);  // can only be modified in standby
    static const uint8_t  REG_CTRL4 	            =(0x29);  // can only be modified in standby
    static const uint8_t  REG_CTRL5 	            =(0x2A);  // can only be modified in standby

    static const uint8_t  REG_OFF_P 	            =(0x2B);  // registers not preserved in standby
    static const uint8_t  REG_OFF_T 	            =(0x2C);  // registers not preserved in standby
    static const uint8_t  REG_OFF_H 	            =(0x2D);  // registers not preserved in standby

    /** ######### function definition ################################################################# */

public:

    MPL3115A2(void)
    {
    };

    /**< Enable Altimeter / Barometer MODE */
    inline void setAltimeter(const uint8_t enable = 1)
    {
        uint8_t _value;
        if (enable) _value=MSK_ALT;
        else        _value=0;
        i2c.setRegister(I2C_ADDRESS,REG_CTRL1, MSK_ALT, _value);
    };

    /**< Enable / Disable the Sensor */
    inline void    setEnabled(const uint8_t enable = 1)
    {
        uint8_t _value;
        if (enable) _value=1;
        else        _value=0;
        i2c.setRegister(I2C_ADDRESS,REG_CTRL1, MSK_SBYB, _value);
    };

    /**< read Enable / Disable - Status */
    inline uint8_t getEnabled()
    {
        return (1 & i2c.readByte(I2C_ADDRESS,REG_SYSMOD));
    };

    /**< do a software reset */
    inline void    reset()
    {
        i2c.writeByte(I2C_ADDRESS,REG_CTRL1, MSK_RST);
    };

    /**<  */
    inline uint8_t setOversampleRatio(const uint8_t sampleRatio = 128)
    {
        uint8_t ratio, sample;
        if      (sampleRatio > 127)
        {
            sample = 7;    // takes 512ms
            ratio = 128;
        }
        else if (sampleRatio > 63)
        {
            sample = 6;    // takes 258ms
            ratio = 64;
        }
        else if (sampleRatio > 31)
        {
            sample = 5;    // takes 130ms
            ratio = 32;
        }
        else if (sampleRatio > 15)
        {
            sample = 4;    // takes  66ms
            ratio = 16;
        }
        else if (sampleRatio > 7)
        {
            sample = 3;    // takes  34ms
            ratio = 8;
        }
        else if (sampleRatio > 3)
        {
            sample = 2;    // takes  18ms
            ratio = 4;
        }
        else if (sampleRatio > 1)
        {
            sample = 1;    // takes  10ms
            ratio = 2;
        }
        else
        {
            sample = 0;    // takes   6ms
            ratio = 1;
        }
        sample <<= 3; //Align it for the CTRL_REG1 register
        i2c.setRegister(I2C_ADDRESS,REG_CTRL1, MSK_OS, sample); //Read current settings
        return ratio;
    };

    /**< Enables the measurement event flags */
    /**< This is recommended in datasheet during setup. */
    inline void setEventFlags(const uint8_t flags = (MSK_DATA_READY | MSK_PRES_READY | MSK_TEMP_READY))
    {
        i2c.writeByte(I2C_ADDRESS,REG_PT_DATA_CFG, (flags & 0x07)); // Enable all three pressure and temp event flags
    };


    /**< initialize */
    inline uint8_t initialize(void)
    {
        if (i2c.probe(I2C_ADDRESS)==0) return 0;

        reset();
        delay(2);
        setEnabled(0);
        setEventFlags();
        setOversampleRatio(128);
        setAltimeter(1);
        setEnabled(1);

        return 1;
    };



    /**< Clears and sets the OST bit --> immediately take another reading */
    /**< Needed to sample faster than 1Hz */
    void triggerMeasurement(void)
    {
        uint8_t setting = i2c.readByte(I2C_ADDRESS,REG_CTRL1); //Read current settings
        if (setting&2)
        {
            setting &= ~(1<<1); //Clear OST bit
            i2c.writeByte(I2C_ADDRESS,REG_CTRL1, setting);
            setting = i2c.readByte(I2C_ADDRESS,REG_CTRL1); //Read current settings to be safe
        }
        setting |= (1<<1); //Set OST bit
        i2c.writeByte(I2C_ADDRESS,REG_CTRL1, setting);
    };

    uint8_t checkMeasurement(void)
    {
        uint8_t _val;
        //Wait for PDR bit, indicates we have new pressure data
        i2c.read(I2C_ADDRESS, REG_STATUS, &_val, 1);
        if (_val & B00000010) return 1; // Measurement finished
        else                  return 0;
    };

    /**<  if you started a measurement and want to actively wait for it to finish */
    uint8_t awaitMeasurement(void)
    {
        uint8_t _counter = 0;
        while(checkMeasurement()==0)
        {
            if(++_counter > 250) return 0; //Error out after max of 500ms for a read
            delay(2);
        }
        return 1; // Measurement finished
    };



    /**<  gives the number of meters above sea level */
    inline void getAltitude(float& meter)
    {
        uint8_t value[3];
        i2c.read(I2C_ADDRESS, REG_OUT_P_MSB, value, 3);  // meter in Q16.4 signed in 3x8bit left
        float tempcsb = (float(value[2]>>4))/16.0;
        meter = (float)( int16_t(value[0] << 8) | int16_t(value[1])) + tempcsb;
    };


    /**<  gives airpressure in Pascal */
    inline void getPressure(float& pascal)
    {
        // Read pressure registers
        uint8_t value[3];
        i2c.read(I2C_ADDRESS, REG_OUT_P_MSB, value, 3);  // pascal in Q18.2 unsigned in 3x8bit left

        float tempcsb = (float(value[2]>>4))/4.0;
        pascal = (float( uint16_t(value[0] << 8) | value[1]))*4 + tempcsb;
    };

    /**< not so much precision (2bit), mut much faster (26 instructions) */
    inline void getPressure(uint32_t& pascal)
    {
        // Read pressure registers
        uint8_t value[3];
        i2c.read(I2C_ADDRESS, REG_OUT_P_MSB, value, 3);  // pascal in Q18.2 unsigned in 3x8bit left
        pascal = uint32_t(( (uint32_t(value[0]<<8) | uint32_t(value[1]))<<2) + (value[2]>>6) );
    };

    /**<  gives pressure-values */
    inline void getMeasurement(float& pascal)
    {
        getPressure(pascal);
    };

    /**<  gives temperature in degree celsius */
    inline void getTemperature(float& celsius)
    {
        // Read temperature registers
        byte value[2];
        i2c.read(I2C_ADDRESS, REG_OUT_T_MSB, value, 2); // °C in Q8.4 signed in 2x

        uint16_t foo;
        bool negSign = false;
        if(value[0] > 0x7F) //Check for 2s compliment
        {
            foo = ~((value[0] << 8) + value[1]) + 1;
            negSign = true;
        }
        else
        {
            foo = ((value[0] << 8) + value[1]);
        }
        celsius = ((float)(foo))/256.0;
        if (negSign) celsius = 0 - celsius;

    };

};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//MPL3115 mpl3115 = MPL3115();

#endif




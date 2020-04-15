#ifndef i2c_TCS3772_h
#define i2c_TCS3772_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the TCS3772-Sensor

CONSUMPTION: sleep 4.5µA, wait 65µA, measure 230-330µA
possible gains: 4 (3.8 .. 4.2), 16 (15 .. 16.8), 60 (58 .. 63)


########################################################################  */

class TCS3772 : private i2cSensor
{

private:

    /** ######### Register-Map ################################################################# */

    static const uint8_t    I2C_ADDRESS 				=(0x29);

// Keept Out: Proximity Feature, Interrupts,
    static const uint8_t    CMD_REPEAT                  =(B10000000);
    static const uint8_t        CMD_INCREMENT           =(B10100000);
    static const uint8_t        CMD_SPECIAL             =(B11100000);

    static const uint8_t    REG_ENABLE				    =(0x00);
    static const uint8_t        MASK_PON 			  	=(bit(0));   // POWER ON
    static const uint8_t            VAL_PWR_ON			=(0x01);
    static const uint8_t            VAL_PWR_OFF			=(0x00);
    static const uint8_t        MASK_AEN                =(bit(1));   // RGBC-Sensor Enable
    static const uint8_t        MASK_WEN                =(bit(3));   // WAIT Enable

    static const uint8_t    REG_ATIME				    =(0x01);       // Integration time in 2.4ms Steps
    static const uint8_t        VAL_MIN                 =(0xFF);       // 2.4ms
    static const uint8_t        VAL_MAX                 =(0x00);       // 614ms
    static const uint8_t    REG_WTIME                   =(0x03);       // WLONG=0 2.4ms Steps, WLONG=1 28.8ms Steps

//// INTERRUPT THRESHOLD, ....

    static const uint8_t    REG_CONFIG                  =(0x0D);
    static const uint8_t        MASK_WLONG              =(bit(1));    // Factor 12x for WTIME
    static const uint8_t    REG_CONTROL				    =(0x0F);
    static const uint8_t        MASK_AGAIN				=(0x03);
    static const uint8_t            VAL_AGAIN_01 		=(0x00);
    static const uint8_t            VAL_AGAIN_04 		=(0x01);
    static const uint8_t            VAL_AGAIN_16 		=(0x02);
    static const uint8_t            VAL_AGAIN_60 		=(0x03);

    static const uint8_t    REG_ID   				    =(0x12);
    static const uint8_t        VAL_DEVICE_ID_A 		=(0x48);        // TCS37721 & TCS37725
    static const uint8_t        VAL_DEVICE_ID_B 		=(0x49);	     // TCS37723 & TCS37727

    static const uint8_t    REG_STATUS   				=(0x13);
    static const uint8_t        MASK_AVALID             =(bit(0));     // cylce completed since AEN=1

    static const uint8_t    REG_CLEAR_DATAL 			=(0x14);
    static const uint8_t    REG_CLEAR_DATAH 			=(0x15);
    static const uint8_t    REG_RED_DATAL 				=(0x16);
    static const uint8_t    REG_RED_DATAH 				=(0x17);
    static const uint8_t    REG_GREEN_DATAL		  	    =(0x18);
    static const uint8_t    REG_GREEN_DATAH			    =(0x19);
    static const uint8_t    REG_BLUE_DATAL 				=(0x1A);
    static const uint8_t    REG_BLUE_DATAH 				=(0x1B);

    /** ######### function definition ################################################################# */

    inline uint16_t static const   ATIME_TO_MSEC(const uint8_t atime)
    {
        return (2.4*(256-atime));
    }; // TODO: WB
    inline uint8_t static const   MSEC_TO_ATIME(const uint16_t msec)
    {
        return (256 - (msec/2.4));
    };
    inline uint8_t static const   FACT_TO_ATIME(const uint8_t factor)
    {
        return (256 - factor);
    }; // TODO: WB
    inline uint8_t    MSEC_TO_ATIMELONG(const uint16_t msec)
    {
        return (256 - (msec/28.8));
    }; // TODO: WB

public:

    TCS3772(void) // TODO: WB
    {
    };

    inline void setEnabled(const uint8_t enable = 1)
    {
        uint8_t _value;
        if (enable) _value = VAL_PWR_ON | MASK_AEN;
        else        _value = 0;
        i2c.setRegister(I2C_ADDRESS, REG_ENABLE | CMD_INCREMENT, MASK_AEN | MASK_PON, _value);
    };

    inline void reset() {};

    inline void setWaitTime(const uint16_t wait = 0)
    {
        uint8_t _valueA, _valueB, _valueC;
        if (wait > 7372)
        {
            _valueA = 255;
            _valueB = 255;
            _valueC = VAL_MAX;
        }
        else if (wait > 614)
        {
            _valueA =  255;
            _valueB =  255;
            _valueC =  MSEC_TO_ATIMELONG(wait);
        }
        else if (wait < 4)
        {
            _valueA =  0;
            _valueB =  0;
            _valueC =  VAL_MIN;
        }
        else
        {
            _valueA = 255;
            _valueB = 0;
            _valueC = MSEC_TO_ATIME(wait);
        }
        i2c.setRegister(I2C_ADDRESS, REG_ENABLE | CMD_INCREMENT, MASK_WEN,   _valueA);
        i2c.setRegister(I2C_ADDRESS, REG_CONFIG | CMD_INCREMENT, MASK_WLONG, _valueB);
        i2c.writeByte(  I2C_ADDRESS, REG_WTIME  | CMD_INCREMENT, _valueC);
    };

    inline void setATime(const uint16_t integrationtime_ms)
    {
        uint8_t _value;
        if (integrationtime_ms > 614)      _value = VAL_MAX;
        else if (integrationtime_ms < 4)   _value = VAL_MIN;
        else                               _value = (uint8_t) MSEC_TO_ATIME(integrationtime_ms);
        i2c.writeByte( I2C_ADDRESS, REG_ATIME  | CMD_INCREMENT, _value);
    };

    uint8_t setAGain(const uint8_t gain)
    {
        uint8_t _valueA, _valueB;
        /**< TODO: rewrite with local Vars and only 1 writeCall at the end */
        if      (gain <  4)
        {
            _valueA = VAL_AGAIN_01;
            _valueB = 1;
        }
        else if (gain < 16)
        {
            _valueA = VAL_AGAIN_04;
            _valueB = 4;
        }
        else if (gain < 60)
        {
            _valueA = VAL_AGAIN_16;
            _valueB = 16;
        }
        else
        {
            _valueA = VAL_AGAIN_60;
            _valueB = 60;
        }
        i2c.setRegister(I2C_ADDRESS, REG_CONTROL | CMD_INCREMENT, MASK_AGAIN, _valueA);
        return _valueB;
    };

    /**< gives back the total gain when values are in good range, otherwise return 0!  */
    uint8_t autoGain(const uint16_t val_clear)
    {
        /**< TODO: something is wrong here! switching integrationtime shows no faster measurement?!? */

        static uint16_t val_last;

        // val_clear between 0 .. 65k
        static const uint16_t  MARGIN_LOW   =(5000);
        static const uint16_t  MARGIN_HIGH  =(0xFFFF - MARGIN_LOW);

        // val_gain: 0=G1,  1=G4,   2=G16, 3=G60
        // val_time: 0=i64, 1=i128, 2=i256
        if ((val_clear != val_last) || (val_clear == 0xFFFF))
        {
            static uint8_t val_gain, val_time, gain;

            val_last = val_clear;

            if      (val_clear < MARGIN_LOW)
            {

                if (val_gain<3) // first try to raise gain, before raising integrationtime
                {
                    val_gain++;
                    gain = 1<<(2*val_gain);
                    gain = setAGain(gain);
                    return 0;
                }
                else if (val_time<2)
                {
                    val_time++;
                    uint16_t time;
                    time = 1<<(val_time);
                    setATime(2.4*64*time);
                    return 0;
                }

            }
            else if (val_clear > MARGIN_HIGH)
            {
                if (val_time>0)
                {
                    val_time--;
                    uint16_t time;
                    time = 1<<(val_time);
                    setATime(2.4*64*time);
                    return 0;
                }
                else if (val_gain>0)
                {
                    val_gain--;
                    gain = 1<<(2*val_gain);
                    gain = setAGain(gain);
                    return 0;
                };
            }
            return (gain*(1<<val_time));
        }

        return 0;

        /* OLD CODE - doesnt wait for good measurement-range
          if      (val_clear < MARGIN_LOW)  { if (val_gain<3) { val_gain++; } else if ( val_time < 2) { val_time++; }}
          else if (val_clear > MARGIN_HIGH) { if (val_time>0) { val_time--; } else if ( val_gain > 0) { val_gain--; }}
        }
        uint16_t time;
        time = 1<<(6+val_time);
        tcs_setATime(2.4*time);
        gain = 1<<(2*val_gain);
        gain = tcs_setAGain(gain);
        return (gain*(1<<val_time));
        */

    };


    inline uint8_t initialize()
    {
        if (i2c.probe(I2C_ADDRESS)==0) return 0;

        byte _sensor_id;
        _sensor_id = i2c.readByte(I2C_ADDRESS, REG_ID | CMD_INCREMENT);

        setEnabled(1);
        setATime(2.4*64);
        setAGain(1);
        setWaitTime(0);

        if      (_sensor_id == VAL_DEVICE_ID_A) return 1;
        else if (_sensor_id == VAL_DEVICE_ID_B) return 1;
        else                                    return 0;
    }

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

    void getMeasurement(uint16_t value_crgb[])
    {
        uint8_t _content[8];
        i2c.read(I2C_ADDRESS, REG_CLEAR_DATAL | CMD_INCREMENT, _content, 8);
        value_crgb[0] = (_content[1]<<8) + _content[0];
        value_crgb[1] = (_content[3]<<8) + _content[2];
        value_crgb[2] = (_content[5]<<8) + _content[4];
        value_crgb[3] = (_content[7]<<8) + _content[6];
    }

};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PRESET preset = PRESET();

#endif





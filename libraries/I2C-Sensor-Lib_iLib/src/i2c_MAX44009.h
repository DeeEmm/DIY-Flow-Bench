#ifndef i2c_MAX44009_h
#define i2c_MAX44009_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the MAX44009-Sensor

CONSUMPTION: sleep ???, measure 0.65 - 1.6µA
MEAS_RANGE: 45 mLux - 180k Lux


########################################################################  */

class MAX44009 : public i2cSensor
{

private:
    /** ######### Register-Map ################################################################# */

    static const uint8_t    I2C_ADDRESS 				    =(0x4B);

    static const uint8_t    INT_STATUS_REG				    =(0x00);
    static const uint8_t    INT_ENABLE_REG                  =(0x01);
    static const uint8_t    CONFIGURATION_REG               =(0x02);
    static const uint8_t        CONFIG_CONT_MASK            =(bit(7));  // CONTINOUS MODE
    static const uint8_t            CONFIG_CONT_ON          =(1<<7);
    static const uint8_t            CONFIG_CONT_OFF         =(0);
    static const uint8_t        CONFIG_MANUAL_MASK          =(bit(6));  // MANUAL Set CDR and TIME
    static const uint8_t            CONFIG_MANUAL_ON        =(1<<6);
    static const uint8_t            CONFIG_MANUAL_OFF       =(0);
    static const uint8_t        CONFIG_CDR_MASK             =(bit(3));  // Current DIVISION RATIO (When HIGH Brightness --> Current/8
    static const uint8_t            CONFIG_CDR_1div1        =(0);
    static const uint8_t            CONFIG_CDR_1div8        =(1<<3);
    static const uint8_t        CONFIG_TIM_MASK             =(bit(2)|bit(1)|bit(0));
    static const uint8_t            CONFIG_TIM_800MS        =(0);
    static const uint8_t            CONFIG_TIM_400MS        =(1);
    static const uint8_t            CONFIG_TIM_200MS        =(2);
    static const uint8_t            CONFIG_TIM_100MS        =(3);
    static const uint8_t            CONFIG_TIM_50MS         =(4);
    static const uint8_t            CONFIG_TIM_25MS         =(5);
    static const uint8_t            CONFIG_TIM_12MS         =(6);
    static const uint8_t            CONFIG_TIM_6MS          =(7);

    static const uint8_t    LUX_HIGH_REG                    =(0x03);
    static const uint8_t    LUX_LOW_REG                     =(0x04);
    static const uint8_t    THRESHOLD_UPPER_REG             =(0x05);
    static const uint8_t    THRESHOLD_LOWER_REG             =(0x06);
    static const uint8_t    THRESHOLD_TIMER_REG             =(0x07);

    /** ######### function definition ################################################################# */

public:

    MAX44009(void)
    {
    };


    inline void setEnabled(const uint8_t enable=1)
    {
        uint8_t _value;
        if (enable)
        {
            _value = CONFIG_CONT_ON;
            i2c.setRegister(I2C_ADDRESS, INT_ENABLE_REG, 1, 0);
            i2c.setRegister(I2C_ADDRESS, CONFIGURATION_REG, CONFIG_MANUAL_MASK, CONFIG_MANUAL_OFF);
        }
        else
        {
            _value = CONFIG_CONT_OFF;
        }
        i2c.setRegister(I2C_ADDRESS, CONFIGURATION_REG, CONFIG_CONT_MASK, _value);
    };

    inline void reset()
    {
        // nothing to do
    };

    inline uint8_t initialize()
    {
        if (i2c.probe(I2C_ADDRESS)==0) return 0;

        setEnabled(1);
        return 1;
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

    /**
     READING only high-register:
     Lux = 2(exponent) x mantissa x 0.72
     Exponent = 8xE3 + 4xE2 + 2xE1 + E0
     Mantissa = 8xM7 + 4xM6 + 2xM5 + M4

     READING combined Registers:
     E3–E0: Exponent bits of lux reading
     M7–M0: Mantissa byte of lux reading
     Lux = 2(exponent) x mantissa x 0.045
     */
    inline void getMeasurement(uint32_t& mLux_value)
    {
        uint8_t lux[2], lux_exponent;

        i2c.read(I2C_ADDRESS, LUX_HIGH_REG, lux, 2);

        lux_exponent    = ((lux[0] >> 4) & 0x0F);
        lux[0]          = ((lux[0] << 4) & 0xF0);
        lux[1]         &= 0x0F;

        //lux_value    = 0.045 * ( lux_high + lux_low ) * (1<< lux_exponent);
        mLux_value    = 45L * ( lux[0] | lux[1] ) * (1<< lux_exponent);
    };




};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PRESET preset = PRESET();

#endif





#ifndef i2c_MMA8451_h
#define i2c_MMA8451_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the MMA8451-Sensor

 CONSUMPTION: standby X µA, measure X µA

 Details:


########################################################################  */

class MMA8451 : public i2cSensor
{

private:

    /** ######### Register-Map ################################################################# */
    static const uint8_t  I2C_ADDRESS 	            =(0x1C);
//static const uint8_t  I2C_ADDRESS 	        =(0x1D); // another common value

    static const uint8_t  REG_STATUS 				=(0x00); //(R) Real time status
    static const uint8_t  REG_OUT_X_MSB 			=(0x01); //(R) [7:0] are 8 MSBs of 10-bit sample
    static const uint8_t  REG_OUT_X_LSB 			=(0x02); //(R) [7:6] are 2 LSBs of 10-bit sample
    static const uint8_t  REG_OUT_Y_MSB 			=(0x03); //(R) [7:0] are 8 MSBs of 10-bit sample
    static const uint8_t  REG_OUT_Y_LSB 			=(0x04); //(R) [7:6] are 2 LSBs of 10-bit sample
    static const uint8_t  REG_OUT_Z_MSB 			=(0x05); //(R) [7:0] are 8 MSBs of 10-bit sample
    static const uint8_t  REG_OUT_Z_LSB 			=(0x06); //(R) [7:6] are 2 LSBs of 10-bit sample
    static const uint8_t  REG_F_SETUP 			    =(0x09); // FIFO SETUP
    static const uint8_t  REG_SYSMOD 				=(0x0b); //(R) Current system mode
    static const uint8_t  REG_INT_SOURCE 			=(0x0c); //(R) Interrupt status
    static const uint8_t  REG_WHO_AM_I 			    =(0x0d); //(R) Device ID (0x3A)
    static const uint8_t      VAL_WHO_AM_I_A        =(0x3A); // MMA8453
    static const uint8_t      VAL_WHO_AM_I_B        =(0x1A); // MMA8451
    static const uint8_t  REG_XYZ_DATA_CFG 		    =(0x0e); //(R/W) Dynamic range settings
    static const uint8_t  REG_HP_FILTER_CUTOFF 	    =(0x0f); //(R/W) cut-off frequency is set to 16Hz @ 800Hz
    static const uint8_t  REG_PL_STATUS 			=(0x10); //(R) Landscape/Portrait orientation status
    static const uint8_t  REG_PL_CFG 				=(0x11); //(R/W) Landscape/Portrait configuration
    static const uint8_t  REG_PL_COUNT 			    =(0x12); //(R) Landscape/Portrait debounce counter
    static const uint8_t  REG_PL_BF_ZCOMP 		    =(0x13); //(R) Back-Front, Z-Lock trip threshold
    static const uint8_t  REG_P_L_THS_REG 		    =(0x14); //(R/W) Portrait to Landscape trip angle is 29 degree
    static const uint8_t  REG_FF_MT_CFG 			=(0x15); //(R/W) Freefall/motion functional block configuration
    static const uint8_t  REG_FF_MT_SRC 			=(0x16); //(R) Freefall/motion event source register
    static const uint8_t  REG_FF_MT_THS 			=(0x17); //(R/W) Freefall/motion threshold register
    static const uint8_t  REG_FF_MT_COUNT 		    =(0x18); //(R/W) Freefall/motion debounce counter
    static const uint8_t  REG_TRANSIENT_CFG 		=(0x1d); //(R/W) Transient functional block configuration
    static const uint8_t  REG_TRANSIENT_SRC 		=(0x1e); //(R) Transient event status register
    static const uint8_t  REG_TRANSIENT_THS 		=(0x1f); //(R/W) Transient event threshold
    static const uint8_t  REG_TRANSIENT_COUNT 	    =(0x20); //(R/W) Transient debounce counter
    static const uint8_t  REG_PULSE_CFG 			=(0x21); //(R/W) ELE, Double_XYZ or Single_XYZ
    static const uint8_t  REG_PULSE_SRC 			=(0x22); //(R) EA, Double_XYZ or Single_XYZ
    static const uint8_t  REG_PULSE_THSX 			=(0x23); //(R/W) X pulse threshold
    static const uint8_t  REG_PULSE_THSY 			=(0x24); //(R/W) Y pulse threshold
    static const uint8_t  REG_PULSE_THSZ 			=(0x25); //(R/W) Z pulse threshold
    static const uint8_t  REG_PULSE_TMLT 			=(0x26); //(R/W) Time limit for pulse
    static const uint8_t  REG_PULSE_LTCY 			=(0x27); //(R/W) Latency time for 2nd pulse
    static const uint8_t  REG_PULSE_WIND 			=(0x28); //(R/W) Window time for 2nd pulse
    static const uint8_t  REG_ASLP_COUNT 			=(0x29); //(R/W) Counter setting for auto-sleep
    static const uint8_t  REG_CTRL_REG1 			=(0x2a); //(R/W) ODR = 800 Hz, STANDBY mode
    static const uint8_t  REG_CTRL_REG2 			=(0x2b); //(R/W) Sleep enable, OS Modes, RST, ST
    static const uint8_t  REG_CTRL_REG3 			=(0x2c); //(R/W) Wake from sleep, IPOL, PP_OD
    static const uint8_t  REG_CTRL_REG4 			=(0x2d); //(R/W) Interrupt enable register
    static const uint8_t  REG_CTRL_REG5 			=(0x2e); //(R/W) Interrupt pin (INT1/INT2) map
    static const uint8_t  REG_OFF_X 				=(0x2f); //(R/W) X-axis offset adjust
    static const uint8_t  REG_OFF_Y 				=(0x30); //(R/W) Y-axis offset adjust
    static const uint8_t  REG_OFF_Z 				=(0x31); //(R/W) Z-axis offset adjust

    static const uint8_t  FULL_SCALE_RANGE_2g 	    =(0x00);
    static const uint8_t  FULL_SCALE_RANGE_4g 	    =(0x01);
    static const uint8_t  FULL_SCALE_RANGE_8g 	    =(0x02);

    int16_t  sensitivity;

    /** ######### function definition ################################################################# */

public:
    /**< TODO: do i need a constructor? */
    MMA8451(void) : sensitivity(4096)
    {
    };

    inline void setEnabled(const uint8_t enable)
    {
        uint8_t _value;
        if (enable) _value = 255;
        else        _value = 0;
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG1, 0x01, _value); // deactivate the device
    };

    inline void setSensibility(const uint8_t gScaleRange=2)
    {
        uint8_t _value;
        // figure out the Range
        if 	   ( gScaleRange <= 3)
        {
            _value = FULL_SCALE_RANGE_2g;    //0-3 = 2g
            sensitivity = 4096;
        }
        else if( gScaleRange <= 5)
        {
            _value = FULL_SCALE_RANGE_4g;    //4-5 = 4g
            sensitivity = 2048;
        }
        else
        {
            _value = FULL_SCALE_RANGE_8g;    // 6-8 = 8g till boundary
            sensitivity = 1024;
        }
        i2c.setRegister(I2C_ADDRESS,REG_XYZ_DATA_CFG, 0x03, _value); 	// set Range
        if (sensitivity >= 2048)    _value = 255; // Reduced Noise <=4g
        else                        _value = 0;
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG1, B00000100, _value);
    };

    inline void setDatarate(const uint16_t hzFreq=100)
    {
        uint8_t _value;
        // figure out Frequency
        if 	   ( hzFreq <= 2)	_value = 7; // 1.56 Hz
        else if( hzFreq <= 6)	_value = 6; // 6.25 Hz
        else if( hzFreq <= 13)	_value = 5; // 12.5 Hz
        else if( hzFreq <= 50) 	_value = 4; // 50 Hz
        else if( hzFreq <= 100) _value = 3; // 100 Hz
        else if( hzFreq <= 200)	_value = 2; // 200 Hz
        else if( hzFreq <= 400)	_value = 1; // 400 Hz
        else    				_value = 0; // 800 Hz
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG1, B00111000, _value<<3 );  	// set OutputDataRate
    };

    inline void reset(void)
    {
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG2,    B01000000, 255);
    };

    inline uint8_t initialize()
    {
        return initialize(100, 8);
    };

    inline uint8_t initialize(const uint16_t hzFreq, const uint8_t gScaleRange)
    {
        if (i2c.probe(I2C_ADDRESS)==0) return 0;

        setEnabled(0);

        setSensibility(gScaleRange);
        setDatarate(hzFreq);
        /**< TODO: needs some tuning */
        i2c.setRegister(I2C_ADDRESS, REG_F_SETUP,      B11000000,  0 );  	// disable FIFO
        i2c.setRegister(I2C_ADDRESS, REG_XYZ_DATA_CFG, B00010000,  0 ); 	// deactivate HighPassFilter
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG2,    0x3,        2 );  	// HighResolution while awake
        //const uint8_t  BITRES_14_TO_10
#ifdef  BITRES_14_TO_10
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG1,    0x02,       0x02 );  // enable Fast Read Mode
#else
        i2c.setRegister(I2C_ADDRESS, REG_CTRL_REG1,    0x02,       0 );  	// disable Fast Read Mode (FullResolution)
#endif
        setEnabled(1);

        return 1;
    };

    /**< check for new data, return 1 when Measurement is ready */
    uint8_t checkMeasurement(void)
    {
        uint8_t _value;
        i2c.read(I2C_ADDRESS, REG_STATUS, &_value, 1);
        if (_value & B00001000) return 1; // Measurement finished
        else                    return 0;
    };

    /**<  wait for new data*/
    uint8_t awaitMeasurement(void)
    {
        uint8_t _counter = 0;
        while (checkMeasurement()==0)
        {
            if (++_counter > 250) return 0; // check took longer than 250ms
            delay(1);
        };
        return 1; // Measurement finished
    };

    /*************************************************************
    *
    * xyz
    *
    * Get accelerometer readings (x, y, z)
    * by default, standard 10 bits mode is used.
    *
    * This function also convers 2's complement number to
    * signed integer result.
    *
    * If accelerometer is initialized to use low res mode,
    * isHighRes must be passed in as false.
    *
    *************************************************************/
    void inline getMeasurement(int16_t raw_xyz[])
    {
        uint8_t _buf[6];
        i2c.read(I2C_ADDRESS, REG_OUT_X_MSB, _buf, 6);
#ifdef  BITRES_14_TO_10
        raw_xyz[0] = (_buf[0] << 2) | ((_buf[1] >> 6) & 0x3); // Only 10 bit of 14 used
        raw_xyz[1] = (_buf[2] << 2) | ((_buf[3] >> 6) & 0x3);
        raw_xyz[2] = (_buf[4] << 2) | ((_buf[5] >> 6) & 0x3);
        if (raw_xyz[0] > 511) raw_xyz[0] -= 1024;
        if (raw_xyz[1] > 511) raw_xyz[1] -= 1024;
        if (raw_xyz[2] > 511) raw_xyz[2] -= 1024;
#else
        raw_xyz[0] = (_buf[0] << 6) | ((_buf[1] >> 2) & 0x63); // use all 14 bits
        raw_xyz[1] = (_buf[2] << 6) | ((_buf[3] >> 2) & 0x63);
        raw_xyz[2] = (_buf[4] << 6) | ((_buf[5] >> 2) & 0x63);
        if (raw_xyz[0] > 8191) raw_xyz[0] -= 16384;
        if (raw_xyz[1] > 8191) raw_xyz[1] -= 16384;
        if (raw_xyz[2] > 8191) raw_xyz[2] -= 16384;
#endif
    };

    void inline getMeasurement(float acc_xyz[])
    {
        int16_t _raw_xyz[3];
        getMeasurement(_raw_xyz);
        acc_xyz[0] = float(_raw_xyz[0]) / float(sensitivity);
        acc_xyz[1] = float(_raw_xyz[1]) / float(sensitivity);
        acc_xyz[2] = float(_raw_xyz[2]) / float(sensitivity);
    };

};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PRESET preset = PRESET();

#endif





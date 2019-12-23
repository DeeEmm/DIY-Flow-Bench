#ifndef i2c_L3G_h
#define i2c_L3G_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the L3G-Sensor

 CONSUMPTION: standby X µA, measure X µA

 Details:

///
########################################################################  */

class L3G : public i2cSensor
{

private:

    /** ######### Register-Map ################################################################# */
    static const uint8_t    L3G4200D_ADDRESS_SA0_LOW    =(0xD0 >> 1);
    static const uint8_t    L3G4200D_ADDRESS_SA0_HIGH   =(0xD2 >> 1);
    static const uint8_t    L3GD20_ADDRESS_SA0_LOW      =(0xD4 >> 1);
    static const uint8_t    L3GD20_ADDRESS_SA0_HIGH     =(0xD6 >> 1);

    static const uint8_t    I2C_ADDRESS				    =(L3GD20_ADDRESS_SA0_HIGH);

    static const uint8_t    WHO_AM_I                    =(0x0F);

    static const uint8_t    CTRL_REG1                   =(0x20);

    static const uint8_t		DATA_RATE_MASK	        =(B11000000);
    static const uint8_t		DATA_RATE_100HZ	        =(B00000000);
    static const uint8_t		DATA_RATE_200HZ         =(B01000000);
    static const uint8_t		DATA_RATE_400HZ         =(B10000000);
    static const uint8_t		DATA_RATE_800HZ         =(B11000000);

    static const uint8_t		BANDWIDTH_MASK	        =(B00110000);
    static const uint8_t		BANDWIDTH_MIN           =(B00000000);
    static const uint8_t		BANDWIDTH_LOW           =(B00010000);
    static const uint8_t		BANDWIDTH_MED           =(B00100000);
    static const uint8_t		BANDWIDTH_HIG           =(B00110000);

    static const uint8_t		PWRDWN_DIS_MASK         =(B00001000);
    static const uint8_t		Z_AXIS_EN_MASK	        =(B00000100);
    static const uint8_t		Y_AXIS_EN_MASK	        =(B00000010);
    static const uint8_t		X_AXIS_EN_MASK	        =(B00000001);

    static const uint8_t    CTRL_REG2                   =(0x21);

    static const uint8_t		HIGH_PASS_MODE_MASK	    =(B00110000);
    static const uint8_t		HIGH_PASS_MODE_NORR	    =(B00000000);
    static const uint8_t		HIGH_PASS_MODE_REFE	    =(B00010000);
    static const uint8_t		HIGH_PASS_MODE_NORM	    =(B00100000);
    static const uint8_t		HIGH_PASS_MODE_AUTO	    =(B00110000);

    static const uint8_t		HIGH_PASS_FREQ_MASK     =(B00001111);
    static const uint8_t		HIGH_PASS_FREQ_STP9     =(B00000000);
    static const uint8_t		HIGH_PASS_FREQ_STP8     =(B00000001);
    static const uint8_t		HIGH_PASS_FREQ_STP7     =(B00000010);
    static const uint8_t		HIGH_PASS_FREQ_STP6     =(B00000011);
    static const uint8_t		HIGH_PASS_FREQ_STP5     =(B00000100);
    static const uint8_t		HIGH_PASS_FREQ_STP4     =(B00000101);
    static const uint8_t		HIGH_PASS_FREQ_STP3     =(B00000110);
    static const uint8_t		HIGH_PASS_FREQ_STP2     =(B00000111);
    static const uint8_t		HIGH_PASS_FREQ_STP1     =(B00001000);
    static const uint8_t		HIGH_PASS_FREQ_STP0     =(B00001001);

    static const uint8_t    CTRL_REG3                   =(0x22);

    static const uint8_t		INT1_EN_MASK		    =(B10000000);
    static const uint8_t		INT1_BOOTSTAT_MASK	    =(B01000000);
    static const uint8_t		INT1_LOW_ACT_MASK	    =(B00100000);
    static const uint8_t		INT_OPNDRAIN_MASK	    =(B00010000);
    static const uint8_t		DRDY_DRDY_MASK		    =(B00001000);
    static const uint8_t		DRDY_FIFO_WTM_MASK	    =(B00000100);
    static const uint8_t		DRDY_FIFO_ORUN_MASK	    =(B00000010);
    static const uint8_t		DRDY_FIFO_EMTY_MASK	    =(B00000001);

    static const uint8_t    CTRL_REG4                   =(0x23);

    static const uint8_t		BLOCK_D_UPDATE_MASK	    =(B10000000);
    static const uint8_t		MSB_ON_LOWADD_MASK	    =(B01000000);

    static const uint8_t		SCALE_MASK			    =(B00110000);
    static const uint8_t		SCALE_0250DPS		    =(B00000000);
    static const uint8_t		SCALE_0500DPS		    =(B00010000);
    static const uint8_t		SCALE_1000DPS		    =(B00100000);
    static const uint8_t		SCALE_2000DPS		    =(B00110000);

    static const uint8_t		SELFTEST_EN_MASK	    =(B00000110);
    static const uint8_t		SELFTEST_DIS		    =(B00000000);
    static const uint8_t		SELFTEST_EN0		    =(B00000010);
    static const uint8_t		SELFTEST_EN1		    =(B00000110);

    static const uint8_t		SPI_TO_3WIRE_MASK	    =(B00000001);
    static const uint8_t		SPI_TO_3WIRE_DIS	    =(B00000000);
    static const uint8_t		SPI_TO_3WIRE_EN		    =(B00000001);

    static const uint8_t    CTRL_REG5                   =(0x24);

    static const uint8_t		REBOOT_MEM_MASK		    =(B10000000);
    static const uint8_t		FIFO_EN_MASK		    =(B01000000);
    static const uint8_t		HIGH_PASS_EN_MASK	    =(B00010000);
    static const uint8_t		INTSEL_LPF_EN_MASK	    =(B00001000);
    static const uint8_t		INTSEL_HPF_EN_MASK	    =(B00000100);
    static const uint8_t		DATA_LPF_EN_MASK	    =(B00000010);
    static const uint8_t		DATA_HPF_EN_MASK	    =(B00000001);

    static const uint8_t    REFERENCE                   =(0x25);
    static const uint8_t    OUT_TEMP                    =(0x26);
    static const uint8_t    STATUS_REG                  =(0x27);

    static const uint8_t    OUT_X_L                     =(0x28);
    static const uint8_t    OUT_X_H                     =(0x29);
    static const uint8_t    OUT_Y_L                     =(0x2A);
    static const uint8_t    OUT_Y_H                     =(0x2B);
    static const uint8_t    OUT_Z_L                     =(0x2C);
    static const uint8_t    OUT_Z_H                     =(0x2D);

    static const uint8_t    FIFO_CTRL_REG               =(0x2E);
    static const uint8_t    FIFO_SRC_REG                =(0x2F);

    static const uint8_t    INT1_CFG                    =(0x30);
    static const uint8_t    INT1_SRC                    =(0x31);
    static const uint8_t    INT1_THS_XH                 =(0x32);
    static const uint8_t    INT1_THS_XL                 =(0x33);
    static const uint8_t    INT1_THS_YH                 =(0x34);
    static const uint8_t    INT1_THS_YL                 =(0x35);
    static const uint8_t    INT1_THS_ZH                 =(0x36);
    static const uint8_t    INT1_THS_ZL                 =(0x37);
    static const uint8_t    INT1_DURATION               =(0x38);

    float sens_factor;

    /** ######### function definition ################################################################# */

public:

    L3G(void) : sens_factor(1)
    {
    };

    inline void reset(void)
    {
        i2c.setRegister(I2C_ADDRESS, CTRL_REG5, REBOOT_MEM_MASK,	255);
        delay(3);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG5, REBOOT_MEM_MASK,	0);
    };

    inline void setEnabled(const uint8_t enable=1)
    {
        uint8_t _value;
        if (enable) _value = 255;
        else        _value = 0;
        i2c.setRegister(I2C_ADDRESS, CTRL_REG1, PWRDWN_DIS_MASK, _value);
    };

    inline void setDatarate(const uint16_t hzFreq = 100)
    {
        uint8_t _value;
        if 		(hzFreq <= 100) _value = DATA_RATE_100HZ;  //  95Hz
        else if (hzFreq <= 200) _value = DATA_RATE_200HZ;  // 190Hz
        else if (hzFreq <= 400) _value = DATA_RATE_400HZ;  // 380Hz
        else  				    _value = DATA_RATE_800HZ;  // 760Hz
        i2c.setRegister(I2C_ADDRESS, CTRL_REG1, DATA_RATE_MASK,	_value);	// 100HZ
    };

    inline void setSensibility(const uint16_t degScaleRange)
    {
        uint8_t _value;
        if 		(degScaleRange <= 250)
        {
            _value      = SCALE_0250DPS;
            sens_factor = 0.00875;
        }
        else if (degScaleRange <= 500)
        {
            _value      = SCALE_0500DPS;
            sens_factor = 0.01750;
        }
        //else if (degScaleRange <= 400)	{ value = SCALE_1000DPS; }
        else
        {
            _value      = SCALE_2000DPS;
            sens_factor = 1/13.98f; // 0.07000;
        }
        i2c.setRegister(I2C_ADDRESS, CTRL_REG4, SCALE_MASK,	_value);
    };

    inline uint8_t initialize(void)
    {
        return initialize(100, 250);
    }

    inline uint8_t initialize(const uint16_t hzFreq, const uint16_t degScaleRange)
    {
        if (i2c.probe(I2C_ADDRESS)==0) return 0;

        reset();
        setEnabled(0);
        setDatarate(hzFreq);

        i2c.setRegister(I2C_ADDRESS, CTRL_REG1, BANDWIDTH_MASK, BANDWIDTH_HIG);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG1, Z_AXIS_EN_MASK, Z_AXIS_EN_MASK);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG1, Y_AXIS_EN_MASK, Y_AXIS_EN_MASK);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG1, X_AXIS_EN_MASK, X_AXIS_EN_MASK);

        i2c.setRegister(I2C_ADDRESS, CTRL_REG2, B11000000, 0); // Just for secure operation
        i2c.setRegister(I2C_ADDRESS, CTRL_REG2, HIGH_PASS_MODE_MASK, HIGH_PASS_MODE_NORM);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG2, HIGH_PASS_FREQ_MASK, HIGH_PASS_FREQ_STP0);

        i2c.setRegister(I2C_ADDRESS, CTRL_REG3, INT1_EN_MASK,			0);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG3, INT1_BOOTSTAT_MASK,		0);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG3, INT1_LOW_ACT_MASK,		0); // 0=HIGH active
        i2c.setRegister(I2C_ADDRESS, CTRL_REG3, INT_OPNDRAIN_MASK,		0); // 0=PushPull
        i2c.setRegister(I2C_ADDRESS, CTRL_REG3, DRDY_DRDY_MASK,			255);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG3, DRDY_FIFO_WTM_MASK,		0);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG3, DRDY_FIFO_ORUN_MASK,	0);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG3, DRDY_FIFO_EMTY_MASK,	0);

        i2c.setRegister(I2C_ADDRESS, CTRL_REG4, BLOCK_D_UPDATE_MASK,	0); // 0=continuos update
        i2c.setRegister(I2C_ADDRESS, CTRL_REG4, MSB_ON_LOWADD_MASK,		0);

        setSensibility(degScaleRange);

        //i2c.setRegister(I2C_ADDRESS, CTRL_REG4, SELFTEST_EN_MASK,	SELFTEST_DIS); // should not be changed in gd20
        i2c.setRegister(I2C_ADDRESS, CTRL_REG4, SPI_TO_3WIRE_MASK,	SPI_TO_3WIRE_DIS);

        i2c.setRegister(I2C_ADDRESS, CTRL_REG5, FIFO_EN_MASK,		0);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG5, HIGH_PASS_EN_MASK,	0); 	// Combination - not helpfull, or?
        i2c.setRegister(I2C_ADDRESS, CTRL_REG5, INTSEL_LPF_EN_MASK,	0);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG5, INTSEL_HPF_EN_MASK,	0);
        i2c.setRegister(I2C_ADDRESS, CTRL_REG5, DATA_LPF_EN_MASK,	0);	// Combination
        i2c.setRegister(I2C_ADDRESS, CTRL_REG5, DATA_HPF_EN_MASK,	0); 	// Combination

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

    /**< Reads the 3 gyro channels RAW!!!

        Sensitivity for the L3GD20:
            250dps --> 8.75 mdps/digit
            500dps --> 17.5 mdps/digit
            2000dps --> 70.0 mdps/digit
     */
    void getMeasurement(int16_t xyz_raw[])
    {
        byte _byte[6];

        i2c.read(I2C_ADDRESS, (OUT_X_L | (1 << 7)), _byte, 6);

        // combine high and low bytes
        xyz_raw[0]  = (int16_t)(_byte[1] << 8 | _byte[0]); // X
        xyz_raw[1]  = (int16_t)(_byte[3] << 8 | _byte[2]); // Y
        xyz_raw[2]  = (int16_t)(_byte[5] << 8 | _byte[4]); // Z
    };

    /**< with sensitivity in dps */
    void getMeasurement(float xyz_dps[])
    {
        byte _byte[6];

        i2c.read(I2C_ADDRESS, (OUT_X_L | (1 << 7)), _byte, 6);

        // combine high and low bytes
        xyz_dps[0]  = ((float)(_byte[1] << 8 | _byte[0]))*sens_factor; // X
        xyz_dps[1]  = ((float)(_byte[3] << 8 | _byte[2]))*sens_factor; // Y
        xyz_dps[2]  = ((float)(_byte[5] << 8 | _byte[4]))*sens_factor; // Z
    };

};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PRESET preset = PRESET();

#endif





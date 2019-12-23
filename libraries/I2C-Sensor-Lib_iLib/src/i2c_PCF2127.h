#ifndef i2c_pcf2127_h
#define i2c_pcf2127_h

#include "i2c.h"



/** ######################################################################

Driver for the PCF2127T RTC

 CONSUMPTION: standby X µA, measure X µA

 Details:


########################################################################  */

class PCF2127
{

private:

    /** ######### Register-Map ################################################################# */
    static const uint8_t    I2C_ADDRESS 	            =(0x51);

    static const uint8_t    REG_CONTROL1                =(0x00);
    static const uint8_t        MSK_CONTROL1_EXTTST     =(B10000000); // test mode? NO
    static const uint8_t        MSK_CONTROL1_T          =(B01000000); // unused
    static const uint8_t        MSK_CONTROL1_STOP       =(B00100000);
    static const uint8_t        MSK_CONTROL1_TSF1       =(B00010000);
    static const uint8_t        MSK_CONTROL1_POROVRD    =(B00001000);
    static const uint8_t        MSK_CONTROL1_12_24      =(B00000100);
    static const uint8_t        MSK_CONTROL1_MI         =(B00000010);
    static const uint8_t        MSK_CONTROL1_SI         =(B00000001);

    static const uint8_t    REG_CONTROL2                =(0x01);
    static const uint8_t        MSK_CONTROL2_MSF        =(B10000000);
    static const uint8_t        MSK_CONTROL2_WDTF       =(B01000000);
    static const uint8_t        MSK_CONTROL2_TSF2       =(B00100000);
    static const uint8_t        MSK_CONTROL2_AF         =(B00010000);
    static const uint8_t        MSK_CONTROL2_CDTF       =(B00001000);
    static const uint8_t        MSK_CONTROL2_TSIE       =(B00000100);
    static const uint8_t        MSK_CONTROL2_AIE        =(B00000010);
    static const uint8_t        MSK_CONTROL2_CDTIE      =(B00000001);
    static const uint8_t    REG_CONTROL3                =(0x02);
    static const uint8_t        MSK_CONTROL3_PWRMNG     =(B11100000);
    static const uint8_t        MSK_CONTROL3_BTSE       =(B00010000);
    static const uint8_t        MSK_CONTROL3_BF         =(B00001000);
    static const uint8_t        MSK_CONTROL3_BLF        =(B00000100);
    static const uint8_t        MSK_CONTROL3_BIE        =(B00000010);
    static const uint8_t        MSK_CONTROL3_BLIE       =(B00000001);

    static const uint8_t    REG_SECONDS                 =(0x03);    // 7bit
    static const uint8_t    REG_MINUTES                 =(0x04);    // 7bit
    static const uint8_t    REG_HOURS                   =(0x05);    // 6bit
    static const uint8_t    REG_DAYS                    =(0x06);    // 6bit
    static const uint8_t    REG_WEEKDAYS                =(0x07);    // 3bit
    static const uint8_t    REG_MONTH                   =(0x08);    // 5bit
    static const uint8_t    REG_YEARS                   =(0x09);    // 8bit
    static const uint16_t       VAL_YEAR_OFFSET         =(2000);

    static const uint8_t    REG_SECOND_ALARM            =(0x0A);
    static const uint8_t    REG_MINUTE_ALARM            =(0x0B);
    static const uint8_t    REG_HOUR_ALARM              =(0x0C);
    static const uint8_t    REG_DAY_ALARM               =(0x0D);
    static const uint8_t    REG_WEEKDAY_ALARM           =(0x0E);
    static const uint8_t        VAL_ENABLE_ALARM        =(B10000000); // highest bit always for Enable!

    static const uint8_t    REG_CLOCKOUT_CTL            =(0x0F);
    static const uint8_t        MSK_CLOCKOUT_TCR        =(B11000000);
    static const uint8_t        MSK_CLOCKOUT_COF        =(B00000111);

    static const uint8_t    REG_WATCHDOG_TIM_CTL        =(0x10);
    static const uint8_t        MSK_WATCHDOG_WDCD       =(B11000000);
    static const uint8_t        MSK_WATCHDOG_TITP       =(B00100000);
    static const uint8_t        MSK_WATCHDOG_TF         =(B00000011);
    static const uint8_t    REG_WATCHDOG_TIM_VAL        =(0x11);

    static const uint8_t    REG_TIMESTP_CTL             =(0x12);
    static const uint8_t        MSK_TIMESTP_TSM         =(B10000000);
    static const uint8_t        MSK_TIMESTP_TSOFF       =(B01000000);
    static const uint8_t        MSK_TIMESTP_1O16        =(B00011111);
    static const uint8_t    REG_SEC_TIMESTP             =(0x13);
    static const uint8_t    REG_MIN_TIMESTP             =(0x14);
    static const uint8_t    REG_HOUR_TIMESTP            =(0x15);
    static const uint8_t    REG_DAY_TIMESTP             =(0x16);
    static const uint8_t    REG_MON_TIMESTP             =(0x17);
    static const uint8_t    REG_YEAR_TIMESTP            =(0x18);

    static const uint8_t    REG_AGING_OFFSET            =(0x19);    // 4bit

    static const uint8_t    REG_RAM_ADDR_MSB            =(0x1A);    // 1bit / last
    static const uint8_t    REG_RAM_ADDR_LSB            =(0x1B);
    static const uint8_t    REG_RAM_WRT_CMD             =(0x1C);
    static const uint8_t    REG_RAM_RD_CMD              =(0x1D);

    uint8_t datetime[7];

    /** ######### function definition ################################################################# */

public:
    /**< TODO: do i need a constructor? */
    PCF2127(void) : datetime({0,0,0,0,0,0})
    {

    };

    inline uint8_t initialize()
    {
        if (i2c.probe(I2C_ADDRESS)==0) return 0;

        uint8_t regPCF;

        regPCF  = 0;
        regPCF &= !MSK_CONTROL1_EXTTST;
        regPCF &= !MSK_CONTROL1_STOP;
        regPCF &= !MSK_CONTROL1_TSF1;
        regPCF &= !MSK_CONTROL1_POROVRD;
        regPCF &= !MSK_CONTROL1_12_24;
        regPCF &= !MSK_CONTROL1_MI;
        regPCF &= !MSK_CONTROL1_SI;
        i2c.writeByte(I2C_ADDRESS, REG_CONTROL1, regPCF);

        regPCF  = 0;
        regPCF &= !MSK_CONTROL2_TSIE;
        regPCF &= !MSK_CONTROL2_AIE;
        regPCF &= !MSK_CONTROL2_CDTIE;
        i2c.writeByte(I2C_ADDRESS, REG_CONTROL2, regPCF);

        regPCF  = 0;
        regPCF |= MSK_CONTROL3_PWRMNG & B10100000; // direct switching and no low power detection
        regPCF &= !MSK_CONTROL3_BTSE;
        regPCF &= !MSK_CONTROL3_BIE;
        regPCF &= !MSK_CONTROL3_BLIE;
        i2c.writeByte(I2C_ADDRESS, REG_CONTROL3, regPCF);


        setTemperaturePeriod(240);

        return 1;
    };

    inline void clearInterrupt()
    {
        i2c.setRegister(I2C_ADDRESS, REG_CONTROL1, MSK_CONTROL1_TSF1, 0);
    };

    inline void setTemperaturePeriod(const uint8_t seconds=240)
    {
        uint8_t _value;
        if      (seconds > 160)  _value = B00000000;    // 240s
        else if (seconds > 90)   _value = B01000000;    // 120s
        else if (seconds > 45)   _value = B10000000;    // 60s
        else                     _value = B11000000;    // 30s
        i2c.setRegister(I2C_ADDRESS, REG_CLOCKOUT_CTL, MSK_CLOCKOUT_TCR, _value);
    };

    void setClockOut(const uint16_t clock = 1)
    {
        uint8_t _value;
        if      (clock > 30000) _value = 000;    // 32768
        else if (clock > 16000) _value = 001;    // 16384
        else if (clock > 8000)  _value = 010;    // 8192
        else if (clock > 4000)  _value = 011;    // 4096
        else if (clock > 2000)  _value = 100;    // 2048
        else if (clock > 1000)  _value = 101;    // 1024
        else if (clock > 0)     _value = 110;    // PPS
        else                    _value = 111;    // disable
        i2c.setRegister(I2C_ADDRESS, REG_CLOCKOUT_CTL, MSK_CLOCKOUT_COF, _value);

    };

    inline void setTime(const uint16_t YYYY, const uint8_t MM, const uint8_t W, const uint8_t DD, const uint8_t hh, const uint8_t mm, const uint8_t ss)
    {
        uint8_t decimal = 0, _value;

        /// YEAR
        uint16_t _ivalue = YYYY;
        if (YYYY >= VAL_YEAR_OFFSET) _ivalue -= VAL_YEAR_OFFSET;
        if (_ivalue > 99) return;
        decimal = 0;
        while (_ivalue > 9)
        {
            _ivalue -= 10;
            decimal += 1;
        }
        datetime[6] = _ivalue | ((decimal)<<4);

        /// MONTH
        if (MM > 12) return;
        _value = MM;
        decimal = 0;
        while (_value > 9)
        {
            _value -= 10;
            decimal++;
        }
        datetime[5] = _value | (decimal)<<4;

        /// WEEKDAYS
        if (W > 6) return;
        datetime[4] = W;

        /// DAY
        if (DD > 32) return;
        _value  = DD;
        decimal = 0;
        while (_value > 9)
        {
            _value -= 10;
            decimal++;
        }
        datetime[3] = _value | (decimal)<<4;


        /// HOURS
        if (hh > 24) return;
        _value  = hh;
        decimal = 0;
        while (_value > 9)
        {
            _value -= 10;
            decimal++;
        }
        datetime[2] = _value | (decimal)<<4;

        /// MINUTES
        if (mm > 60) return;
        _value  = mm;
        decimal = 0;
        while (_value > 9)
        {
            _value -= 10;
            decimal++;
        }
        datetime[1] = _value | (decimal)<<4;


        /// SECONDS
        if (ss > 60) return;
        _value  = ss;
        decimal = 0;
        while (_value > 9)
        {
            _value -= 10;
            decimal++;
        }
        datetime[0] = _value | (decimal)<<4;

        i2c.setRegister(I2C_ADDRESS, REG_CONTROL1, MSK_CONTROL1_STOP, MSK_CONTROL1_STOP);
        i2c.write(I2C_ADDRESS, REG_SECONDS, datetime, 7);
        i2c.setRegister(I2C_ADDRESS, REG_CONTROL1, MSK_CONTROL1_STOP, 0);

    };

    inline void readTime()
    {
        i2c.read(I2C_ADDRESS, REG_SECONDS, datetime, 7);

        //uint8_t counter;
        //counter = 0;
        // while (counter<7)
        for (uint8_t counter = 0; counter<7; counter++)
        {
            datetime[counter]  = uint8_t(((datetime[counter]>>4) * 10) + (datetime[counter] & B00001111));
            //counter++;
        }
    };


    inline void setSeconds(const uint8_t unit = 0)
    {
        // TODO:
    };

    inline void getSeconds(uint8_t& unit)
    {
        unit = datetime[0];
    };

    inline void setMinutes(const uint8_t unit = 0)
    {
        // TODO:
    };

    inline void getMinutes(uint8_t& unit)
    {
        unit = datetime[1];
    };

    inline void setHours(const uint8_t unit = 0)
    {
        // TODO
    };

    inline void getHours(uint8_t& unit)
    {
        unit = datetime[2];
    };

    inline void setDays(const uint8_t unit = 0)
    {
        // TODO
    };

    inline void getDays(uint8_t& unit)
    {
        unit = datetime[3];
    }

    inline void setWeekdays(const uint8_t unit = 0)
    {
    };

    inline void getWeekdays(uint8_t& unit)
    {
        unit = datetime[4];
    };

    inline void setMonth(const uint8_t unit = 0)
    {
        // TODO:
    };

    inline void getMonth(uint8_t& unit)
    {
        unit = datetime[5];
    };

    inline void setYears(const uint8_t unit = 0)
    {
        // TODO
    };

    inline void getYears(uint16_t& unit)
    {
        unit = VAL_YEAR_OFFSET + uint16_t(datetime[6]);
    };
    /*
    TODO:

    static const uint8_t REG_SECONDS             0x03    // 7bit
    static const uint8_t REG_MINUTES             0x04    // 7bit
    static const uint8_t REG_HOURS               0x05    // 6bit
    static const uint8_t REG_DAYS                0x06    // 6bit
    static const uint8_t REG_WEEKDAYS            0x07    // 3bit
    static const uint8_t REG_MONTH               0x08    // 5bit
    static const uint8_t REG_YEARS               0x09    // 8bit

    static const uint8_t REG_SECOND_ALARM        0x0A
    static const uint8_t REG_MINUTE_ALARM        0x0B
    static const uint8_t REG_HOUR_ALARM          0x0C
    static const uint8_t REG_DAY_ALARM           0x0D
    static const uint8_t REG_WEEKDAY_ALARM       0x0E
    static const uint8_t     VAL_ENABLE_ALARM    (B10000000) // highest bit always for Enable!



    static const uint8_t REG_WATCHDOG_TIM_CTL    0x10
    static const uint8_t     MSK_WATCHDOG_WDCD   (B11000000)
    static const uint8_t     MSK_WATCHDOG_TITP   (B00100000)
    static const uint8_t     MSK_WATCHDOG_TF     (B00000011)
    static const uint8_t REG_WATCHDOG_TIM_VAL    0x11

    static const uint8_t REG_AGING_OFFSET        0x19    // 4bit

    static const uint8_t REG_RAM_ADDR_MSB        0x1A    // 1bit / last
    static const uint8_t REG_RAM_ADDR_LSB        0x1B
    static const uint8_t REG_RAM_WRT_CMD         0x1C
    static const uint8_t REG_RAM_RD_CMD          0x1D
    */
};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PCF2127 pcf2127 = PCF2127();

#endif

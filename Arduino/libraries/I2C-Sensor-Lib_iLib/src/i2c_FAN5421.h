#ifndef i2c_FAN5421_h
#define i2c_FAN5421_h

#include "i2c.h"

///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  FAN5421  ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

class FAN5421
{

    /** ######### Register-Map ################################################################# */
public:

    static const uint8_t I2C_ADDRESS                	   =(0x6A);

private:

    static const uint8_t FAN5421_REG_CONTROL0              =(0x00);
    static const uint8_t   FAN5421_MSK_TMR_RST             =(1<<7);   // has to be written every 30s to enable charging
    static const uint8_t   FAN5421_MSK_EN_STAT             =(1<<6);
    static const uint8_t   FAN5421_MSK_STAT                =(3<<4);
    static const uint8_t     FAN5421_VAL_STAT_READY        =(0<<4);
    static const uint8_t     FAN5421_VAL_STAT_CHARGE       =(1<<4);
    static const uint8_t     FAN5421_VAL_STAT_FULL         =(2<<4);
    static const uint8_t     FAN5421_VAL_STAT_FAULT        =(3<<4);
    static const uint8_t   FAN5421_MSK_FAULT               =(7<<0);
    static const uint8_t     FAN5421_VAL_FAULT_NORMAL      =(0<<0);
    static const uint8_t     FAN5421_VAL_FAULT_VBUSOVP     =(1<<0);
    static const uint8_t     FAN5421_VAL_FAULT_SLEEP       =(2<<0);
    static const uint8_t     FAN5421_VAL_FAULT_POORSRC     =(3<<0);
    static const uint8_t     FAN5421_VAL_FAULT_BATOVP      =(4<<0);
    static const uint8_t     FAN5421_VAL_FAULT_THSHTDWN    =(5<<0);
    static const uint8_t     FAN5421_VAL_FAULT_TMR         =(6<<0);
    static const uint8_t     FAN5421_VAL_FAULT_NOBAT       =(7<<0);
    static const uint8_t FAN5421_REG_CONTROL1              =(0x01);
    static const uint8_t   FAN5421_MSK_V_LOW_V             =(3<<4);
    static const uint8_t     FAN5421_VAL_V_LOW_3V4         =(0<<4);
    static const uint8_t     FAN5421_VAL_V_LOW_3V5         =(1<<4);
    static const uint8_t     FAN5421_VAL_V_LOW_3V6         =(2<<4);
    static const uint8_t     FAN5421_VAL_V_LOW_3V7         =(3<<4);
    static const uint8_t   FAN5421_MSK_TE                  =(1<<3);  // Current Termination
    static const uint8_t   FAN5421_MSK_CE_N                =(1<<2); // Charge Enabled NOT
    static const uint8_t   FAN5421_MSK_HZ_MODE             =(1<<1); // High impedance mode
    static const uint8_t FAN5421_REG_OREG                  =(0x02);
    static const uint8_t   FAN5421_MSK_OREG                =(0b11111100);
    static const uint8_t     FAN5421_VAL_OREG_3V80         =(15<<2);
    static const uint8_t     FAN5421_VAL_OREG_3V90         =(20<<2);
    static const uint8_t     FAN5421_VAL_OREG_4V00         =(25<<2);
    static const uint8_t     FAN5421_VAL_OREG_4V10         =(30<<2);
    static const uint8_t     FAN5421_VAL_OREG_4V20         =(35<<2); // =(4200-3500)/20
    static const uint8_t     FAN5421_VAL_OREG_4V30         =(40<<2);
    static const uint8_t     FAN5421_VAL_OREG_4V40         =(45<<2);
    static const uint8_t     FAN5421_VAL_OREG_4V44         =(62<<2);
    static const uint8_t FAN5421_REG_IC_INFO               =(0x03);
    static const uint8_t   FAN5421_MSK_VENDOR              =(7<<5);
    static const uint8_t     FAN5421_VAL_FAIRCHILD         =(0b10000000);
    static const uint8_t   FAN5421_MSK_PN                  =(3<<3);
    static const uint8_t     FAN5421_VAL_PN                =(0b00000000);
    static const uint8_t   FAN5421_MSK_REV                 =(7);
    static const uint8_t     FAN5421_VAL_REV               =(1);
    static const uint8_t FAN5421_REG_IBAT                  =(0x04);
    static const uint8_t   FAN5421_MSK_RESET               =(1<<7);
    static const uint8_t   FAN5421_MSK_IOCHARGE            =(15<<3);
    static const uint8_t     FAN5421_VAL_IO_1A55           =(10<<3);
    static const uint8_t     FAN5421_VAL_IO_1A45           =(9<<3);
    static const uint8_t     FAN5421_VAL_IO_1A35           =(8<<3);
    static const uint8_t     FAN5421_VAL_IO_1A25           =(7<<3);
    static const uint8_t     FAN5421_VAL_IO_1A15           =(6<<3);
    static const uint8_t     FAN5421_VAL_IO_1A05           =(5<<3);
    static const uint8_t     FAN5421_VAL_IO_0A95           =(4<<3);
    static const uint8_t     FAN5421_VAL_IO_0A85           =(3<<3);
    static const uint8_t     FAN5421_VAL_IO_0A75           =(2<<3);
    static const uint8_t     FAN5421_VAL_IO_0A65           =(1<<3);
    static const uint8_t     FAN5421_VAL_IO_0A55           =(0<<3);
    static const uint8_t   FAN5421_MSK_ITERM               =(7);
    static const uint8_t     FAN5421_VAL_ITERM_050MA       =(0);
    static const uint8_t     FAN5421_VAL_ITERM_100MA       =(1);
    static const uint8_t     FAN5421_VAL_ITERM_150MA       =(2);
    static const uint8_t     FAN5421_VAL_ITERM_200MA       =(3);

    static const uint8_t FAN5421_REG_SP_CHARGER            =(0x05);
    static const uint8_t   FAN5421_MSK_IO_LEVEL            =(1<<5);
    static const uint8_t   FAN5421_MSK_SP                  =(1<<4); // Special Charger
    static const uint8_t   FAN5421_MSK_EN_LEVEL            =(1<<3); // ReadOnly
    static const uint8_t   FAN5421_MSK_VSP                 =(7<<0);
    static const uint8_t     FAN5421_VAL_VSP_4V20          =(0<<0);
    static const uint8_t     FAN5421_VAL_VSP_4V44          =(3<<0);
    static const uint8_t     FAN5421_VAL_VSP_4V60          =(5<<0);
    static const uint8_t     FAN5421_VAL_VSP_4V76          =(7<<0);
    static const uint8_t FAN5421_REG_SAFETY                =(0x06);  // can only be written before any other register is written =(after vbat > vshort)
    static const uint8_t   FAN5421_MSK_ISAFE               =(15<<4);
    static const uint8_t     FAN5421_VAL_ISAFE_1A0         =(5<<4);
    static const uint8_t     FAN5421_VAL_ISAFE_1A5         =(10<<4);
    static const uint8_t     FAN5421_VAL_ISAFE_2A0         =(15<<4);
    static const uint8_t   FAN5421_MSK_VSAFE               =(15);
    static const uint8_t     FAN5421_VAL_VSAFE_4V20        =(0);
    static const uint8_t     FAN5421_VAL_VSAFE_4V30        =(5);
    static const uint8_t     FAN5421_VAL_VSAFE_4V40        =(10);
    static const uint8_t     FAN5421_VAL_VSAFE_4V44        =(15);

    FAN5421(const FAN5421&);            // declaration only for copy constructor
    FAN5421& operator=(const FAN5421&);  // declaration only for copy assignment --> make it uncopyable

public:

    FAN5421(void)
    {

    };

    // reset IC --> dont forget to set safety-registers first after reset
    void reset()
    {
        i2c.setRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_RESET, 255);
        i2c.setRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_RESET, 0);
    };

    // has to be written before any other register is written (locked after)
    uint8_t set_safety()
    {
        uint8_t error = 0;
        i2c.writeByte(    I2C_ADDRESS, FAN5421_REG_SAFETY, FAN5421_VAL_ISAFE_1A5 | FAN5421_VAL_VSAFE_4V20);

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_SAFETY, FAN5421_MSK_ISAFE) != FAN5421_VAL_ISAFE_1A5)  error++;
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_SAFETY, FAN5421_MSK_VSAFE) != FAN5421_VAL_VSAFE_4V20) error++;

        return error;
    };

    // has to be polled at least every 32s during charging (HOST alive)
    void poll_timer()
    {
        i2c.setRegister(    I2C_ADDRESS, FAN5421_REG_CONTROL0, FAN5421_MSK_TMR_RST,    255);
    };

    // feedback for debugging / status
    void print_status()
    {
        Serial.print("FAN5421 is ");

        uint8_t status = i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL0,FAN5421_MSK_STAT);

        if (status == FAN5421_VAL_STAT_READY)    Serial.print("READY");
        if (status == FAN5421_VAL_STAT_CHARGE)   Serial.print("CHARGING");
        if (status == FAN5421_VAL_STAT_FULL)     Serial.print("FULL");
        if (status == FAN5421_VAL_STAT_FAULT)    Serial.print("FAULTY");

        Serial.print(" with ");

        uint8_t fault = i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL0,FAN5421_MSK_FAULT);

        if (fault == FAN5421_VAL_FAULT_NORMAL)   Serial.print("no fault");
        if (fault == FAN5421_VAL_FAULT_VBUSOVP)  Serial.print("Vbus OVP");
        if (fault == FAN5421_VAL_FAULT_SLEEP)    Serial.print("sleep enabled");
        if (fault == FAN5421_VAL_FAULT_POORSRC)  Serial.print("poor source");
        if (fault == FAN5421_VAL_FAULT_BATOVP)   Serial.print("battery OVP");
        if (fault == FAN5421_VAL_FAULT_THSHTDWN) Serial.print("thermal shutdown");
        if (fault == FAN5421_VAL_FAULT_TMR)      Serial.print("timer error");
        if (fault == FAN5421_VAL_FAULT_NOBAT)    Serial.print("no battery");

        Serial.print(" (");
        Serial.print(3400 + 100*(i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL1, FAN5421_MSK_V_LOW_V)>>4));
        Serial.print("mV,");
        Serial.print(550 + 100*(i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_IOCHARGE)>>3));
        Serial.print("mA,");
        Serial.print(3500.0+20.0*(i2c.getRegister(I2C_ADDRESS, FAN5421_REG_OREG, FAN5421_MSK_OREG)>>2),0);
        Serial.print("mV)");
    };


    // only for debug purpose
    void print_register()
    {
        Serial.print("DEBUG \t ");
        for (uint8_t loopvar=0; loopvar < 7; loopvar++)
        {
            Serial.print(" R");
            Serial.print(loopvar);
            Serial.print(": ");
            Serial.print(i2c.getRegister(I2C_ADDRESS, loopvar, 255), BIN);
            Serial.print(" \t ");
        }
    };

    // give a basic and safe set for enable charging
    uint8_t set_config(const uint16_t charge_current_mA = 750)
    {
        uint8_t error = 0;

        // TODO: probe IC

        //// FAN5421_REG_CONTROL0
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_CONTROL0, FAN5421_MSK_TMR_RST | FAN5421_MSK_EN_STAT);

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL0, FAN5421_MSK_EN_STAT)  != FAN5421_MSK_EN_STAT)    error++;

        //// FAN5421_REG_CONTROL1
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_CONTROL1, FAN5421_VAL_V_LOW_3V6 | FAN5421_MSK_TE | 0 | 0);

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL1, FAN5421_MSK_V_LOW_V) != FAN5421_VAL_V_LOW_3V6)   error++;
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL1, FAN5421_MSK_TE)      != FAN5421_MSK_TE)          error++;
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL1, FAN5421_MSK_CE_N)    != 0)                       error++;
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL1, FAN5421_MSK_HZ_MODE) != 0)                       error++;

        //// FAN5421_REG_OREG
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_OREG, FAN5421_VAL_OREG_4V20);

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_OREG, FAN5421_MSK_OREG) != FAN5421_VAL_OREG_4V20)          error++;

        //// FAN5421_REG_IC_INFO
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IC_INFO, FAN5421_MSK_VENDOR) != FAN5421_VAL_FAIRCHILD)     error++;
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IC_INFO, FAN5421_MSK_PN)     != FAN5421_VAL_PN)            error++;
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IC_INFO, FAN5421_MSK_REV)    != FAN5421_VAL_REV)           error++;

        //// FAN5421_REG_IBAT
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_VAL_IO_0A75 | FAN5421_VAL_ITERM_100MA);

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_IOCHARGE)!= FAN5421_VAL_IO_0A75)         error++;
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_ITERM)   != FAN5421_VAL_ITERM_100MA)     error++;

        error += set_current(charge_current_mA);

        //// FAN5421_REG_SP_CHARGER
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_SP_CHARGER, 0 | FAN5421_VAL_VSP_4V20);

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_SP_CHARGER, FAN5421_MSK_IO_LEVEL) != 0)                    error++;
        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_SP_CHARGER, FAN5421_MSK_VSP) != FAN5421_VAL_VSP_4V20)      error++;

        return error;
    };

    // charging current routine
    uint8_t set_current(uint16_t charge_current_mA)
    {
        uint8_t error = 0, value = 0;

        while (charge_current_mA > 550)
        {
            charge_current_mA -= 100;
            value             += (1<<3);
        }

        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_IBAT, (value) | i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_ITERM));

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_IOCHARGE)!= value)       error++;

        return error;
    };

    // increase charging-current step by step
    uint8_t increase_current()
    {
        uint8_t error = 0;

        uint8_t value = i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_IOCHARGE);

        if (value == (15<<3))   return 1; // error - maximum reached

        value +=  (1<<3);

        // watch out - dont trigger reset
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_IBAT, (value) | i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_ITERM));

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_IOCHARGE)!= value)     error++;

        return error;
    };

    // decrease charging-current step by step
    uint8_t decrease_current()
    {
        uint8_t error = 0;

        uint8_t value = i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_IOCHARGE);

        if (value == 0)   return 1; // error - minimum reached

        value -=  (1<<3);

        // watch out - dont trigger reset
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_IBAT, (value) | i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_ITERM));

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_IOCHARGE)!= value)     error++;

        return error;
    };

    // mostly for debugging
    void print_info()
    {
        Serial.print(" SRST: \t ");
        Serial.println(i2c.getRegister(    I2C_ADDRESS, FAN5421_REG_CONTROL0, FAN5421_MSK_TMR_RST)>>7);

        Serial.print(" SP: \t");
        Serial.println(i2c.getRegister(    I2C_ADDRESS, FAN5421_REG_SP_CHARGER, FAN5421_MSK_IO_LEVEL)>>4);
        Serial.print(" DIS: \t");
        Serial.println(i2c.getRegister(    I2C_ADDRESS, FAN5421_REG_SP_CHARGER, FAN5421_MSK_EN_LEVEL)>>3);

        Serial.print(" VE: \t");
        Serial.println(i2c.getRegister(    I2C_ADDRESS, FAN5421_REG_IC_INFO, FAN5421_MSK_VENDOR)>>5);
        Serial.print(" PN: \t");
        Serial.println(i2c.getRegister(    I2C_ADDRESS, FAN5421_REG_IC_INFO, FAN5421_MSK_PN)>>3);

        Serial.print(" REV \t 1.");
        Serial.println(i2c.getRegister(    I2C_ADDRESS, FAN5421_REG_IC_INFO, FAN5421_MSK_REV));
    };

    // gives feedback if it charges or not
    uint8_t get_chargingstatus()
    {
        uint8_t status = i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL0,FAN5421_MSK_STAT);

        if (status == FAN5421_VAL_STAT_CHARGE)   return 1;
        else                                     return 0;
    };

    // workaround for: chip doesnt charge when already charged and external power still present
    // disable charging, wait ~30ms, enable charging --> not working as expected, try chip.reset
    uint8_t charging_enable(const uint8_t enable = 1)
    {
        i2c.setRegister(I2C_ADDRESS, FAN5421_REG_CONTROL1, FAN5421_MSK_CE_N, ~enable);

        if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_CONTROL1, FAN5421_MSK_CE_N) == (FAN5421_MSK_CE_N&(~enable)))    return 1; // charging-changed
        else 																				                         return 0; // error
    };


    /// by default the system current is limited to 325mA. increase with following sequence:
    // program safety register
    // set oreg to the desired value (4...18)
    // set iocharge, then reset the iolevel-bit
    uint8_t power_without_battery()
    {
        uint8_t error = 0;

        //// FAN5421_REG_OREG
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_OREG, FAN5421_VAL_OREG_4V20);

        //if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_OREG, FAN5421_MSK_OREG) != FAN5421_VAL_OREG_4V20)          error++;

        //// FAN5421_REG_IBAT
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_VAL_IO_1A45 | FAN5421_VAL_ITERM_100MA);

        //if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_IOCHARGE)!= FAN5421_VAL_IO_0A75)         error++;
        //if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_IBAT, FAN5421_MSK_ITERM)   != FAN5421_VAL_ITERM_100MA)     error++;

        //// FAN5421_REG_SP_CHARGER
        //i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_SP_CHARGER, FAN5421_MSK_IO_LEVEL);
        i2c.writeByte(      I2C_ADDRESS, FAN5421_REG_SP_CHARGER, 0 | FAN5421_VAL_VSP_4V20);

        //if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_SP_CHARGER, FAN5421_MSK_IO_LEVEL) != 0)                    error++;
        //if (i2c.getRegister(I2C_ADDRESS, FAN5421_REG_SP_CHARGER, FAN5421_MSK_VSP) != FAN5421_VAL_VSP_4V20)      error++;

        return error;
    };

};

#endif

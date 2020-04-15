#ifndef i2c_MAX17047_h
#define i2c_MAX17047_h

#include "i2c.h"

///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  MAX17047 ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

class MAX17047
{

    /** ######### Register-Map ################################################################# */
public:

    static const uint8_t I2C_ADDRESS                    	=(0x36);

private:

    static constexpr float   MAX17047_SENSE                     =(0.01);

    static const uint8_t MAX17047_STATUS 					=(0x00);
    static const uint8_t MAX17047_V_ALRT_THRESHOLD 	        =(0x01);
    static const uint8_t MAX17047_T_ALRT_THRESHOLD 	        =(0x02);
    static const uint8_t MAX17047_SOC_ALRT_THRESHOLD	    =(0x03);
    static const uint8_t MAX17047_AT_RATE					=(0x04);
    static const uint8_t MAX17047_REM_CAP_REP			    =(0x05);  // Capacity in uVh / filtered -AV
    static const uint8_t MAX17047_SOC_REP				    =(0x06);  // State of Charge in % (Highbyte) Filtered -AV
    static const uint8_t MAX17047_AGE						=(0x07);
    static const uint8_t MAX17047_TEMPERATURE			    =(0x08);
    static const uint8_t MAX17047_V_CELL					=(0x09);
    static const uint8_t MAX17047_CURRENT				    =(0x0A);
    static const uint8_t MAX17047_AVERAGE_CURRENT		    =(0x0B);
    static const uint8_t MAX17047_SOC_MIX			        =(0x0D);  // State of Charge in % (Highbyte)
    static const uint8_t MAX17047_SOC_AV					=(0x0E);    // State of Charge in % (Highbyte) considering all information
    static const uint8_t MAX17047_REM_CAP_MIX			    =(0x0F);    // Capacity in uVh (div Senseresistor for mAh)
    static const uint8_t MAX17047_FULL_CAP				    =(0x10);    // best case full capacity in uVh
    static const uint8_t MAX17047_TTE						=(0x11);    // Time to Empty 5.625s/LSB
    static const uint8_t MAX17047_Q_RESIDUAL_00			    =(0x12);
    static const uint8_t MAX17047_FULL_SOC_THR			    =(0x13);
    static const uint8_t MAX17047_AVERAGE_TEMP			    =(0x16);
    static const uint8_t MAX17047_CYCLES					=(0x17);    // accumulate total percent in Change in %
    static const uint8_t MAX17047_DESIGN_CAP				=(0x18);
    static const uint8_t MAX17047_AVERAGE_V_CELL		    =(0x19);
    static const uint8_t MAX17047_MAX_MIN_TEMP			    =(0x1A);
    static const uint8_t MAX17047_MAX_MIN_VOLTAGE		    =(0x1B);
    static const uint8_t MAX17047_MAX_MIN_CURRENT		    =(0x1C);
    static const uint8_t MAX17047_CONFIG					=(0x1D);
    static const uint8_t MAX17047_I_CHG_TERM				=(0x1E);
    static const uint8_t MAX17047_REM_CAP_AV				=(0x1F);
    static const uint8_t MAX17047_VERSION					=(0x21);
    static const uint8_t MAX17047_Q_RESIDUAL_10			    =(0x22);
    static const uint8_t MAX17047_FULL_CAP_NOM			    =(0x23);
    static const uint8_t MAX17047_TEMP_NOM				    =(0x24);
    static const uint8_t MAX17047_TEMP_LIM				    =(0x25);
    static const uint8_t MAX17047_AIN						=(0x27);
    static const uint8_t MAX17047_LEARN_CFG				    =(0x28);
    static const uint8_t MAX17047_FILTER_CFG				=(0x29);
    static const uint8_t MAX17047_RELAX_CFG				    =(0x2A);
    static const uint8_t MAX17047_MISC_CFG				    =(0x2B);
    static const uint8_t MAX17047_T_GAIN					=(0x2C);
    static const uint8_t MAX17047_T_OFF					    =(0x2D);
    static const uint8_t MAX17047_C_GAIN					=(0x2E);
    static const uint8_t MAX17047_C_OFF					    =(0x2F);
    static const uint8_t MAX17047_Q_RESIDUAL_20			    =(0x32);
    static const uint8_t MAX17047_I_AVG_EMPTY			    =(0x36);
    static const uint8_t MAX17047_F_CTC					    =(0x37);
    static const uint8_t MAX17047_RCOMP_0				    =(0x38);
    static const uint8_t MAX17047_TEMP_CO				    =(0x39);
    static const uint8_t MAX17047_V_EMPTY					=(0x3A);    // Empty Voltage
    static const uint8_t MAX17047_F_STAT					=(0x3D);
    static const uint8_t MAX17047_TIMER					    =(0x3E);
    static const uint8_t MAX17047_SHDN_TIMER				=(0x3F);
    static const uint8_t MAX17047_Q_RESIDUAL_30			    =(0x42);
    static const uint8_t MAX17047_D_QACC					=(0x45);
    static const uint8_t MAX17047_D_PACC					=(0x46);
    static const uint8_t MAX17047_QH						=(0x4D);
    static const uint8_t MAX17047_V_FOCV					=(0xFB);
    static const uint8_t MAX17047_SOC_VF					=(0xFF);    // State of Charge according to voltage fuel gauge

    MAX17047(const MAX17047&);            // declaration only for copy constructor
    MAX17047& operator=(const MAX17047&);  // declaration only for copy assignment --> make it uncopyable

public:

    MAX17047(void)
    {

    };

    // application specific (only set on POR)
    void set_capacity_design(const uint16_t capacity_mA)
    {
        uint16_t uvalue = capacity_mA * (MAX17047_SENSE * 200); // mAh * R / uVh
        uint8_t  value[2];
        value[0] = uvalue & 0xFF;
        value[1] = (uvalue>>8) & 0xFF;
        i2c.write(I2C_ADDRESS, MAX17047_DESIGN_CAP, value, 2);
    };

    // application specific (only set on POR)
    void set_full_threshold(const uint8_t percent_soc)
    {
        uint8_t  value[2];
        value[0] = 0;
        value[1] = (percent_soc) & 0xFF;
        i2c.write(I2C_ADDRESS, MAX17047_FULL_SOC_THR, value, 2);
    };

    // application specific (only set on POR)
    void set_termination_charge_current(const uint16_t current_mA)
    {
        uint8_t  value[2];
        uint16_t uvalue = current_mA * (MAX17047_SENSE * 640);
        value[0] = uvalue & 0xFF;
        value[1] = (uvalue>>8) & 0xFF;
        i2c.write(I2C_ADDRESS, MAX17047_I_CHG_TERM, value, 2);
    };

    // application specific (only set on POR)
    void set_empty_voltage(const uint16_t empty_mV, const uint16_t recovery_mV)
    {
        uint8_t  value[2];
        value[0] = (recovery_mV / 40) & 0b01111111;
        value[1] = (empty_mV / 20) & 0xFF;
        i2c.write(I2C_ADDRESS, MAX17047_V_EMPTY, value, 2);
    };

    // alarm turns the alarm-led on
    void set_alarm_voltage(const uint16_t lowVolt_mV, const uint16_t highVolt_mV)
    {
        uint8_t  value[2];
        value[0] = (lowVolt_mV / 20) & 0xFF;       // low Voltage
        value[1] = (highVolt_mV / 20) & 0xFF; // high Voltage
        i2c.write(I2C_ADDRESS, MAX17047_V_ALRT_THRESHOLD, value, 2);
    };

    // only set once on POR!!!
    uint8_t initialize()
    {
        // TODO: Probe IC

        // Check POR-Status
        uint8_t value[2];
        i2c.read(I2C_ADDRESS, MAX17047_STATUS, value, 2);
        if (value[0] & 2)   Serial.println("POR detected - will initialize the IC");
        else                return 0;

        Serial.println("Set ");
        Serial.println(CELL_CAPACITY_mA);
        Serial.println("mAh Capacity");
        set_capacity_design(CELL_CAPACITY_mA); // mA

        Serial.println("Set Full Cap at 95%.");
        set_full_threshold(95); // %

        Serial.println("Set Charge Termination Current to 100mA");
        set_termination_charge_current(100); //mA

        Serial.println("Set Voltage Thresholds for Empty-Detection (3400,3500mV)");
        set_empty_voltage(3400, 3500); // mV

        // play back backup if it exists
        uint8_t backup[20];
        if (eeprom_read(backup)) restore_data(backup);

        // clear POR-bit
        Serial.println("POR-Status is being reset");
        value[0] &= 0b11111101; // POR
        i2c.write(I2C_ADDRESS, MAX17047_STATUS, value, 2);

        return 1;
    };

    // do this at end-of-charge or end-of-discharge, gives back 20 byte, INFO about cells
    void backup_data(uint8_t registers[])
    {
        i2c.read(I2C_ADDRESS, MAX17047_FULL_CAP,       &registers[0], 2);
        i2c.read(I2C_ADDRESS, MAX17047_CYCLES,         &registers[2], 2);
        i2c.read(I2C_ADDRESS, MAX17047_RCOMP_0,        &registers[4], 2);
        i2c.read(I2C_ADDRESS, MAX17047_TEMP_CO,        &registers[6], 2);
        i2c.read(I2C_ADDRESS, MAX17047_Q_RESIDUAL_00,  &registers[8], 2);
        i2c.read(I2C_ADDRESS, MAX17047_Q_RESIDUAL_10,  &registers[10], 2);
        i2c.read(I2C_ADDRESS, MAX17047_Q_RESIDUAL_20,  &registers[12], 2);
        i2c.read(I2C_ADDRESS, MAX17047_Q_RESIDUAL_30,  &registers[14], 2);
        i2c.read(I2C_ADDRESS, MAX17047_D_QACC,         &registers[16], 2);
        i2c.read(I2C_ADDRESS, MAX17047_D_PACC,         &registers[18], 2);
    };

    // after POR you have can restore registers and don't have to relearn the cell
    uint8_t restore_data(uint8_t registers[])
    {
        uint8_t value[2];
        // read status and wait for POR != 0
        i2c.read(I2C_ADDRESS, MAX17047_STATUS, value, 2);
        if (value[0] & 2)   Serial.println("POR detected - will restore Data-Backup");
        else                return 0;

        // restore app-specific-register
        // restore gauge-learned register
        i2c.write(I2C_ADDRESS, MAX17047_FULL_CAP,      &registers[0], 2);
        i2c.write(I2C_ADDRESS, MAX17047_CYCLES,        &registers[2], 2);
        i2c.write(I2C_ADDRESS, MAX17047_RCOMP_0,       &registers[4], 2);
        i2c.write(I2C_ADDRESS, MAX17047_TEMP_CO,       &registers[6], 2);
        i2c.write(I2C_ADDRESS, MAX17047_Q_RESIDUAL_00, &registers[8], 2);
        i2c.write(I2C_ADDRESS, MAX17047_Q_RESIDUAL_10, &registers[10], 2);
        i2c.write(I2C_ADDRESS, MAX17047_Q_RESIDUAL_20, &registers[12], 2);
        i2c.write(I2C_ADDRESS, MAX17047_Q_RESIDUAL_30, &registers[14], 2);
        i2c.write(I2C_ADDRESS, MAX17047_D_QACC,        &registers[16], 2);
        i2c.write(I2C_ADDRESS, MAX17047_D_PACC,        &registers[18], 2);

        // clear POR-bit
        Serial.println("POR-Status is being reset");
        value[0] &= 0b11111101; // POR
        i2c.write(I2C_ADDRESS, MAX17047_STATUS, value, 2);

        return 1;
    };

    // basic initialization
    void set_config()
    {
        uint8_t  uvalue[2];
        Serial.println("Programm Alert to SOC_AV");
        i2c.read(I2C_ADDRESS, MAX17047_MISC_CFG, uvalue, 2);
        uvalue[0] &= 0b11111100;
        uvalue[0] |= 0b00000001;
        i2c.write(I2C_ADDRESS, MAX17047_MISC_CFG, uvalue, 2);

        // MAX17047_CONFIG
        i2c.read(I2C_ADDRESS, MAX17047_CONFIG, uvalue, 2);
        Serial.println("Enable Alarms");
        uvalue[0] |= 0b00000100; // Aen
        uvalue[1] &= 0b11110111; // ALRTp
        // disable Thermistor
        Serial.println("Disable Thermistor-Input");
        uvalue[0] &= 0b11101111; // ETHRM
        i2c.write(I2C_ADDRESS, MAX17047_CONFIG, uvalue, 2);
    };

    // feedback and status
    void print_status()
    {
        uint8_t value[2];
        float   fvalue;

        // empty-Status
        i2c.read(I2C_ADDRESS, MAX17047_F_STAT, value, 2);
        if (value[1] & 1)  Serial.print("E");
        else               Serial.print("N");

        i2c.read(I2C_ADDRESS, MAX17047_STATUS, value, 2);
        // Voltage Status
        if      (value[1] & 0b00000001)  Serial.print("|E"); // EMPTY
        else if (value[1] & 0b00010000)  Serial.print("|M"); // FULL exceed maximum
        else                             Serial.print("|N"); // normal
        // POR-Status
        if (value[0] & 2)  Serial.print("|R "); // POR SET
        else               Serial.print("|N ");


        // Voltage of Cells
        i2c.read(I2C_ADDRESS, MAX17047_V_CELL, value, 2);
        fvalue = float(uint16_t(value[1]<<5) + (value[0]>>3)) / 1.6;
        Serial.print(fvalue,0);
        Serial.print(" mV :: ");

        // State of Charge
        i2c.read(I2C_ADDRESS, MAX17047_SOC_AV, value, 2);
        fvalue = float(uint16_t(value[1]<<8) + (value[0])) / 256;
        Serial.print(fvalue,2);
        Serial.print(" %SOC :: ");

        // Remaining capacity
        i2c.read(I2C_ADDRESS, MAX17047_REM_CAP_AV, value, 2);
        fvalue = float(uint16_t(value[1]<<8) + (value[0])) * (0.000005 / MAX17047_SENSE);
        Serial.print(fvalue,3);
        Serial.print(" of ");

        // full capacity
        i2c.read(I2C_ADDRESS, MAX17047_FULL_CAP, value, 2);
        fvalue = float(uint16_t(value[1]<<8) + (value[0])) * (0.000005 / MAX17047_SENSE);
        Serial.print(fvalue,3);
        Serial.print(" Ah :: ");

        // time to empty
        i2c.read(I2C_ADDRESS, MAX17047_TTE, value, 2);
        fvalue = float(uint16_t(value[1]<<8) + (value[0])) * (5.625 / 60);
        Serial.print(fvalue,0);
        Serial.print(" min left at ");

        // MAX17047_CURRENT
        i2c.read(I2C_ADDRESS, MAX17047_CURRENT, value, 2);
        fvalue = float((value[1]<<8) + (value[0])) * (0.001 * (1.5625 / MAX17047_SENSE));
        Serial.print(fvalue,2);
        Serial.print("|");
        i2c.read(I2C_ADDRESS, MAX17047_AVERAGE_CURRENT, value, 2);
        fvalue = float((value[1]<<8) + (value[0])) * (0.001 * (1.5625 / MAX17047_SENSE));
        Serial.print(fvalue,2);
        Serial.print(" mA :: ");

        // MAX17047_CYCLES
        i2c.read(I2C_ADDRESS, MAX17047_CYCLES, value, 2);
        fvalue = float(uint16_t(value[1]<<8) + (value[0]));
        Serial.print(fvalue,0);
        Serial.print(" %cycled :: ");

        // MAX17047_AGE
        i2c.read(I2C_ADDRESS, MAX17047_AGE, value, 2);
        fvalue = float(uint16_t(value[1]<<8) + (value[0])) * 0.0039;
        Serial.print(fvalue,2);
        Serial.print(" %age :: ");
    };

    // return measured cell voltage in mV
    uint16_t get_cell_voltage()
    {
        uint8_t value[2];
        i2c.read(I2C_ADDRESS, MAX17047_V_CELL, value, 2);
        return uint16_t((uint16_t(value[1]<<5) + (value[0]>>3)) / 1.6); // mV
    };

    // return measured cell coltage in mA
    int16_t get_cell_current()
    {
        uint8_t value[2];
        i2c.read(I2C_ADDRESS, MAX17047_CURRENT, value, 2);
        return (((value[1]<<8) + (value[0])) * (0.001 * (1.5625 / MAX17047_SENSE)));
    };

};

#endif

/****************************************
 * CONFIGURE PINS 
 *
 * Board type defined in configuration.h
 ***/


/****************************************
 * ARDUINO MEGA 2560
 ***/
#ifdef ARDUINO_MEGA_2560 
    // Pins 0+1 reserved for serial comms (RX/TX)
    // Pins 20+21 are reseverd for I2C (Interrupts) 
    // Available interrupt pins - 2/3/6/7/43/44

    // Define Physical Pins
    #define VOLTAGE_PIN A0
    #define MAF_PIN A1
    #define REF_PRESSURE_PIN A2
    #define REF_VAC_PIN A3
    #define PITOT_PIN A4
    #define TEMPERATURE_PIN A5
    #define REF_BARO_PIN A6
    #define HUMIDITY_PIN A7
    #define SCA_PIN 20 // Dedicated SCA pin on Mega2560
    #define SCL_PIN 21 // Dedicated SCL pin on Mega2560

    // NVM Addresses (NOTE: 0-99 reserved for menu)
    #define NVM_HIGH_FLOW_CAL_ADDR 100    //8 bytes for float
    #define NVM_LOW_FLOW_CAL_ADDR 107     //8 bytes for float
    #define NVM_LEAK_CAL_ADDR 115         //8 bytes for float
    #define NVM_CD_CAL_OFFSET_ADDR 116    //8 bytes for float
    #define NVM_REF_PRESS_CAL_ADDR 123    //8 bytes for float

#endif


/****************************************
 * ARDUINO UNO
 ***/
#ifdef ARDUINO_UNO
    // Pins 2+3 reserved for serial comms (RX/TX)
    // Pins 20+21 are reseverd for I2C (Interrupts)

    // Define Physical Pins
    #define VOLTAGE_PIN A0                //IMPORTANT!! - tie A0 to the +5v rail 
    #define MAF_PIN A1
    #define REF_PRESSURE_PIN A2
    #define REF_VAC_PIN A3
    #define PITOT_PIN A4
    #define TEMPERATURE_PIN A5
    #define REF_BARO_PIN A6               //NOTE this cannot be same as REF_PRESSURE_PIN
    #define HUMIDITY_PIN A7
    #define SCA_PIN 20
    #define SCL_PIN 21


    // NVM Addresses (note 0-99 reserved for menu)
    #define NVM_HIGH_FLOW_CAL_ADDR 100    //8 bytes for float
    #define NVM_LOW_FLOW_CAL_ADDR 107     //8 bytes for float
    #define NVM_REF_PRESS_CAL_ADDR 123    //8 bytes for float
    #define NVM_LEAK_CAL_ADDR 131         //8 bytes for float
    #define NVM_CD_CAL_OFFSET_ADDR 116    //8 bytes for float
    #define NVM_REF_PRESS_CAL_ADDR 123    //8 bytes for float

#endif
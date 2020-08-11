/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 *
 * boards.h - define boards + pins
 *
 ***/


/****************************************
 * DIYFB Shield
 *
 * DESIGN NOTES: 
 * A0 & D4-D10 are reserved for D1 Robot LCD Keypad Shield 
 * 
 * SDA / SCL 20/21
 * Pins 0+1 reserved for serial comms (RX/TX)
 * Pins 20+21 are reseverd for I2C (Interrupts) 
 * Available interrupt pins - 2/3/6/7/43/44
 ***/
#ifdef DIYFB_SHIELD 

    // Define Physical Pins
    // RESERVED BY ROBOT D1 LCD KEYPAD DISPLAY (default display)
    #define D1ROBOT_A0          A0
    #define D1ROBOT_D4          4
    #define D1ROBOT_D5          5
    #define D1ROBOT_D6          6
    #define D1ROBOT_D7          7
    #define D1ROBOT_D8          8
    #define D1ROBOT_D9          9
    #define D1ROBOT_D10         10

    // VAC CONTROL
    #define VAC_CONTROL_PIN     1   // vac motor(s) on/off
    #define VAC_SPEED_PIN       A1  // speed reference for VFD
    #define VAC_BLEED_VALVE_PIN A2  // bleed valve control

    // SENSORS
    #define SPEED_SENSOR_PIN    2   // turbine speed for turbo flow bench
    #define VOLTAGE_PIN         A8
    #define MAF_PIN             A9
    #define REF_PRESSURE_PIN    A10
    #define REF_VAC_PIN         A11
    #define PITOT_PIN           A12
    #define TEMPERATURE_PIN     A13
    #define REF_BARO_PIN        A14
    #define HUMIDITY_PIN        A15

    // COMMS
    #define SERIAL1_TX          18
    #define SERIAL1_RX          19
    #define SCA_PIN             20 // Dedicated SCA pin on Mega2560
    #define SCL_PIN             21 // Dedicated SCL pin on Mega2560

    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL            13
    #define AVO_STEP            12
    #define AVO_DIR             11


    // NVM Addresses (NOTE: 0-99 reserved for menu)
    #define NVM_HIGH_FLOW_CAL_ADDR 100    //8 bytes for float
    #define NVM_LOW_FLOW_CAL_ADDR 107     //8 bytes for float
    #define NVM_LEAK_CAL_ADDR 115         //8 bytes for float
    #define NVM_CD_CAL_OFFSET_ADDR 116    //8 bytes for float
    #define NVM_REF_PRESS_CAL_ADDR 123    //8 bytes for float

#endif




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
/****************************************
 * configurations.h - define variables and configure hardware
 * This file is part of the DIY Flow Bench project. For more information including usage and licensing please refer to: https://github.com/DeeEmm/DIY-Flow-Bench
 * 
 * NOTE REGARDING CHANGING BOARDS, DISPLAYS AND INPUT DEVICES
 * Pin defs for displays and input devices are auto-generated in DIY-Flow-Bench_menu.h by the tcMenu app
 * DIY-Flow-Bench_menu.h should not be manually edited
 * 
 * To change board type, display or input device pins for your project
 * Download and run the tcMenu application and regenerate the tcMenu code files.
 * INSTRUCTIONS: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/Customisation
 * 
 * The standard project board is the Arduino Mega 2560 
 * Mega 2560 pins for display and encoder click wheel are listed below for reference only
 * ENCODER_PIN_A = 2;
 * ENCODER_PIN_B = 3;
 * BUTTON = 4;
 * LCD_WIDTH = 20;
 * LCD_HEIGHT = 4;
 * I2C_ADDRESS = "0x20";
 * I2C CLK Pin = 20; (Standard I2C CLK pin for Mega 2560)
 * I2C DIO Pim = 21; (Standard I2C DIO pin for Mega 2560)
 ***/

#pragma once


/****************************************
 * SELECT BOARD TYPE 
 ***/
#define ARDUINO_MEGA_2560 //default
//#define ARDUINO_NANO
//#define ARDUINO_UNO



/****************************************
 * CONFIGURE PINS 
  ***/


// ARDUINO MEGA 2560
#ifdef ARDUINO_MEGA_2560 
    // Pins 2+3 reserved for serial comms (RX/TX)
    // Pins 20+21 are reseverd for I2C (Interrupts) 

    // Define Physical Pins
    #define MAF_PIN A0
    #define REF_PRESSURE_PIN A1
    #define REF_VAC_PIN A2
    #define PITOT_PIN A3
    #define TEMPERATURE_PIN A4
    #define BAROMETRIC_PIN A5
    #define HUMIDITY_PIN A6

    // NVM Addresses (NOTE: 0-99 reserved for menu)
    #define NVM_FLOW_CAL_ADDR 100 //8 bytes for float
    #define NVM_LEAK_CAL_ADDR 107 //8 bytes for float

#endif


// ARDUINO NANO
#ifdef ARDUINO_NANO

    // Define Physical Pins
    #define MAF_PIN A0
    #define REF_PRESSURE_PIN A1
    #define REF_VAC_PIN A2
    #define PITOT_PIN A3
    #define TEMPERATURE_PIN A4
    #define BAROMETRIC_PIN A5
    #define HUMIDITY_PIN A6

    // NVM Addresses (note 0-99 reserved for menu)
    #define NVM_FLOW_CAL_ADDR 100 //8 bytes for float
    #define NVM_LEAK_CAL_ADDR 107 //8 bytes for float

#endif


// ARDUINO UNO
#ifdef ARDUINO_UNO
    // Pins 2+3 reserved for serial comms (RX/TX)
    // Pins 20+21 are reseverd for I2C (Interrupts)

    // Define Physical Pins
    #define MAF_PIN A0
    #define REF_PRESSURE_PIN A1
    #define REF_VAC_PIN A2
    #define PITOT_PIN A3
    #define TEMPERATURE_PIN A4
    #define BAROMETRIC_PIN A5
    #define HUMIDITY_PIN A6

    // NVM Addresses (note 0-99 reserved for menu)
    #define NVM_FLOW_CAL_ADDR 100 //8 bytes for float
    #define NVM_LEAK_CAL_ADDR 107 //8 bytes for float

#endif



/****************************************
 * CONFIGURE MAF
 * Uncomment active MAF
 ***/

#define SIEMENS__5WK9605 //default
//#define SOME_OTHER_SENSOR 
//#define ADD_YOUR_OWN_SENSOR




/****************************************
 * CONFIGURE PRESSURE SENSORS
 ***/





/****************************************
 * CONFIGURE TEMPERATURE / BARO SENSORS
 ***/






/****************************************
 * CONFIGURE DISPLAYS
 ***/

//Optional additional displays connected to I2C
#define CFM_4X7SEG
#define PITOT_4X7SEG




/****************************************
 * CALIBRATION SETTINGS
 ***/

#define calibrationToleranceCFM 0 // accuracy of bench in cfm
#define calibrationPlateHighCFM 100 // flow rate for large calibration orifice
#define calibrationPlateLowCFM 10 // flow rate for small calibration orifice

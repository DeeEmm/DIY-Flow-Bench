/****************************************
 * configurations.h - define variables and configure hardware
 * This file is part of the DIY Flow Bench project. For more information including usage and licensing please refer to: https://github.com/DeeEmm/DIY-Flow-Bench
 ***/

#pragma once

/****************************************
 * CONFIGURE PINS 
 ***/

#define ARDUINO_MEGA_2560 //default
//#define ARDUINO_NANO
//#define ARDUINO_UNO


// ARDUINO MEGA 2560
// Pins 2+3 reserved for serial comms
// Pins 20+21 are reseverd for I2C 
#ifdef ARDUINO_MEGA_2560
    #define MAF_PIN A0
    #define REF_PRESSURE_PIN A1
    #define REF_VAC_PIN A2
    #define PITOT_PIN A3
    #define TEMPERATURE_PIN A4
    #define BAROMETRIC_PIN A5
    #define HUMIDITY_PIN A6
    //TODO Add NVM addresses for calibration data
    //TODO Add comments showing display connection pins

    //ENCODER_PIN_A = 2;
    //ENCODER_PIN_A = 3;
    //ENCODER_PIN_A = 4;
    //LCD_WIDTH = 20;
    //LCD_HEIGHT = 4;
    //I2C_ADDRESS = "0x20";

#endif


// ARDUINO NANO
#ifdef ARDUINO_NANO
    #define MAF_PIN A0
    #define REF_PRESSURE_PIN A1
    #define REF_VAC_PIN A2
    #define PITOT_PIN A3
    #define TEMPERATURE_PIN A4
    #define BAROMETRIC_PIN A5
    #define HUMIDITY_PIN A6
    //TODO Add NVM addresses for calibration data
    //TODO Add comments showing display connection pins
#endif


// ARDUINO UNO
#ifdef ARDUINO_UNO
    #define MAF_PIN A0
    #define REF_PRESSURE_PIN A1
    #define REF_VAC_PIN A2
    #define PITOT_PIN A3
    #define TEMPERATURE_PIN A4
    #define BAROMETRIC_PIN A5
    #define HUMIDITY_PIN A6
    //TODO Add NVM addresses for calibration data
    //TODO Add comments showing display connection pins
#endif


/****************************************
 * CONFIGURE MAF
 * Uncomment active MAF
 ***/

#define SIEMENS__5WK9605 //default
//#define SOME_OTHER_SENSOR 
//#define ADD_YOUR_OWN_SENSOR



/****************************************
 * CONFIGURE DISPLAYS
 ***/

#define CFM_4X7SEG
#define PITOT_4X7SEG


/****************************************
 * CALIBRATION SETTINGS
 ***/

#define calibrationToleranceCFM 0 //accuracy of bench in cfm

/****************************************
 * configurations.h - define variables and configure hardware
 * This file is part of the DIY Flow Bench project. For more information including usage and licensing please refer to: https://github.com/DeeEmm/DIY-Flow-Bench
 * 
 * NOTE REGARDING CHANGING BOARDS, DISPLAYS AND INPUT DEVICES
 * Pin defs for displays and input devices are auto-generated in DIY-Flow-Bench_menu.h by the tcMenu app
 * DIY-Flow-Bench_menu.h & DIY-Flow-Bench_menu.cpp should not be manually edited
 * 
 * To change board type, display or input device pins for your project
 * Download and run the tcMenu application and regenerate the tcMenu code files.
 * INSTRUCTIONS: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/7.-Customisation
 * 
 * The standard project board is the Arduino Mega 2560 
 * D1 Robot 16x2 Robot LCD keypad is the default input/display combination:
 * 
 * D1 Robot 16x2 LCD Keyshield info.
 * https://wiki.dfrobot.com/Arduino_LCD_KeyPad_Shield__SKU__DFR0009_
 * D1 Robot reserved pins - A0/4/5/6/7/8/9 - do not assign these other other I/O if using this display                                              
 *
 * I2c display and encoder click wheel standard pins
 * ENCODER_PIN_A = 2; // Interrupt must be used
 * ENCODER_PIN_B = 3; // Interrupt must be used
 * BUTTON = 4;
 * LCD_WIDTH = 20;
 * LCD_HEIGHT = 4;
 * I2C_ADDRESS = "0x20";
 * I2C CLK Pin = 20; (Standard I2C CLK pin for Mega 2560)
 * I2C DIO Pim = 21; (Standard I2C DIO pin for Mega 2560)
 *
 * Default temp / baro / RelH uses BME280 device (Sparkfun / clone)
 * I2C address for the BME280 is 0x77
 *
 * Default MAF unit recommended is the GM LS2 MAF (ACDELCO_ 92281162.h)
 * This will measure up to approx 277cfm
 * 
 * DEPENDENCIES
 * The program has a number of dependencies that must be available for it to work.
 * These libraries are provided in the libraries folder 
 *
 * /libraries/tcMenu
 * /libraries/IoAbstraction
 * /libraries/liquidCrystalIO
 * /libraries/SimpleDHT
 * /libraries/SparkfunBME280
 * /libraries/
 *
 * The following libraries are also needed and available within the Arduino IDE
 *
 * arduino-NVM
 *
 * NOTE: you may need to include your library in DIY-Flow-Bench.ino 
 *
 ***/

#pragma once


/****************************************
 * LANGUAGE SETTINGS
 ***/

char LANGUAGE_FILE = "EN_Language.h";



/****************************************
 * DEVELOPER SETTINGS
 ***/

//#define DEBUG_MODE             



/****************************************
 * GENERAL SETTINGS
 ***/

#define MIN_BENCH_PRESSURE 0              // Minimum pressure that we consider the bench is 'operational' / 'running' / vac source is on
#define MIN_FLOW_RATE 3                   // Flow rate in cfm below which bench is considered off
#define CYCLIC_AVERAGE_BUFFER 5           // Number of scans over which to average output (helps stabilise results)
#define MIN_MAF_MILLIVOLTS 100
#define minTestPressurePercentage 80      // Lowest test pressure bench will generate accurate results. Please see note in wiki



/****************************************
 * SELECT BOARD TYPE 
 *
 * Default ARDUINO_MEGA_2560 
 ***/

#define ARDUINO_MEGA_2560
//#define ARDUINO_UNO



/****************************************
 * CONFIGURE MAF
 * Uncomment active MAF
 * If you want to modify the code to include additional MAF sensors
 * You will need to create your own MAF data file. Use exampleSensor.h as an example
 ***/


//#include "mafData/TEST.h"                 // Test Data
#include "mafData/ACDELCO_ 92281162.h"    // GM LS2              
//#include "mafData/exampleKeyValueData.h"  // Example key > value data file (duplicate this as required)
//#include "mafData/exampleAnalogData.h"    // Example Analog point data file (duplicate this as required)
//#include "mafData/SIEMENS_5WK9605.h"      // Data from Tonys tests
//#include "mafData/DELPHI_AF10118.h"       // kg/hr - Data from efidynotuning.com/maf.htm (Stock - Ford '98 Explorer 5.0L)



/****************************************
 * CONFIGURE REFERENCE PRESSURE SENSOR
 * If you want to modify the code to include additional reference pressure sensors
 * You will need to add your volts to kPa algorythm in the function getRefPressure()
 ***/
#define DEFAULT_REF_PRESS 1

// Default: no selection
//#define REF_MPXV7007                      // -7 / +7 psi
//#define REF_MPX4250                       // 0-36psi (absolute)



/****************************************
 * CONFIGURE PITOT PRESSURE SENSOR
 * If you want to modify the code to include additional pitot pressure sensors
 * You will need to add your volts to kPa algorythm in the function getPitotPressure()
 * Note Pitot sensors need to be a differential pressure sensor (DP)
 ***/

// Default: no selection

// Uncomment ONE of the following
//#define PITOT_MPXV7007DP                  



/****************************************
 * CONFIGURE BARO SENSOR
 *
 * Default Baro 101.3529kpa - standard sealevel baro pressure (14.7 psi) 
 ***/
#define DEFAULT_BARO 101.3529

// Uncomment ONE of the following
//#define USE_REF_PRESS_AS_BARO
//#define BARO_SPARKFUN_BME280 
//#define BARO_MPX4115 
//#define BARO_ADAFRUIT_BME280

#define startupBaroScalingFactor 1        // scaling factor when using reference pressure sensor for baro correction
#define startupBaroScalingOffset 100      // scaling offset when using reference pressure sensor for baro correction



/****************************************
 * CONFIGURE TEMPERATURE SENSOR
 *
 * Default 21 Degrees Celcius
 ***/
#define DEFAULT_TEMP 21

// Uncomment ONE of the following
//#define TEMP_SPARKFUN_BME280
//#define TEMP_ADAFRUIT_BME280
//#define SIMPLE_TEMP_DHT11 



/****************************************
 * CONFIGURE HUMIDITY SENSOR
 *
 * Default 36% Rel H
 ***/
#define DEFAULT_RELH 36

// Uncomment ONE of the following
//#define RELH_SPARKFUN_BME280
//#define SIMPLE_RELH_DHT11 
//#define RELH_ADAFRUIT_BME280



/****************************************
 * CONFIGURE DISPLAYS
 ***/

// Default: no selection
// Optional additional displays connected to I2C
//#define CFM_4X7SEG
//#define PITOT_4X7SEG



/****************************************
 * CONFIGURE COMMUNICATIONS
 ***/
 

#define SERIAL_BAUD_RATE 9600           // default 9600
#define showRemoteDialogs true          // show menu dialogs on remote displays
#define BME280_I2C_ADDR 0x77            // (default 0x77) / Alternate 0x76
  

 
/****************************************
 * CONFIGURE ALARMS
 ***/
#define showAlarms true 

//TODO - more granular alarm control - need to be able to disable individual alarms


 /****************************************
 * LEAK TEST SETTINGS
 ***/
#define leakTestTolerance 2               // Tolerance in cfm



 /****************************************
 * CALIBRATION SETTINGS
 ***/
 
#define calibrationRefPressure 10         // Reference pressure orifices were measured at (leave at 10" if calibrating with CD)
#define calibrationFlowRate 14.4          // Standard flow rate for CD @ 10"/wg



 /****************************************
 * ADVANCED CALIBRATION SETTINGS
 *
 * For a more detailed explaination of these settings
 * Please refer to the WIKI - https://github.com/DeeEmm/DIY-Flow-Bench/wiki/4.-Configuration
 *
 ***/

// NOTE: This method is currently unused (also removed from menu)
// It is not clear if this method would produce usable results for all sensors.
// Suggest that once project is stable, method is implimented and results compared against known good data to validate 

// generate MAF data dable using three point method
#define calibrationPlateHighCFM 100       // Flow rate for large calibration orifice
#define calibrationPlateMidCFM 50         // Flow rate for med calibration orifice
#define calibrationPlateLowCFM 10         // Flow rate for small calibration orifice



//Test env /DM
#if defined DM
    #define BARO_SPARKFUN_BME280
    #define TEMP_SPARKFUN_BME280
    #define RELH_SPARKFUN_BME280
#endif
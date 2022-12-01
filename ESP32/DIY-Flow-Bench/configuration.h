/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file configuration.h
 * 
 * @brief define variables and configure hardware
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/
#pragma once

#include "constants.h"


// Don't forget to update the changelog & README Versions!!
// TODO: Automate build numbering with git tasks

#define MAJOR_VERSION "V2"
#define MINOR_VERSION "0"
#define BUILD_NUMBER "22120103"
#define RELEASE "V.2.0-RC.3-WIP"
#define DEV_BRANCH "https://github.com/DeeEmm/DIY-Flow-Bench/tree/ESP32"



/***********************************************************
* SELECT BENCH TYPE
* NOTE: Only MAF style bench working at this stage
***/

#define MAF_STYLE_BENCH
//#define ORIFICE_STYLE_BENCH
//#define PITOT_STYLE_BENCH
//#define VENTURI_STYLE_BENCH



/***********************************************************
* SELECT BOARD TYPE 
*
* Default DIYFB_SHIELD 
* NOTE: ESP32_WROVER_KIT can be used for debug workflows in VSCode / PlatformIO
* NOTE: If defining new board make sure to add board type definition to Hardware::begin
***/

#define WEMOS_D1_R32 // Using official Shield
// #define ARDUCAM_ESP32S // Using official Shield // TODO Finalise pin mapping (copied from WEMOS_D1_R32 but pins are not correctly mapped yet)
// #define ESP32DUINO // Generic pin mapping for ESP32 UNO style footprint. 
// #define ESP32_WROVER_KIT // DEBUG BUILD ONLY




/***********************************************************
* CONFIGURE FILESYSTEM
***/

#define FORMAT_FILESYSTEM_IF_FAILED true



/***********************************************************
* CONFIGURE COMMS
*
* Default comms port is 0 (U0UXD) - (USB programming port) This is used for API / Status Messages / Debugging
* Port 2 (U2UXD) is used to communicate with digital gauge for automated measurements & logging
*
***/

#define SERIAL0_ENABLED                                  // Default serial comms (API & status)
// #define SERIAL2_ENABLED                                  // Digital guage serial protocol

#define SERIAL0_BAUD 115200
#define SERIAL2_BAUD 9600

#define WEBSERVER_ENABLED




/***********************************************************
* SYSTEM SETTINGS
***/
#define BOOT_MESSAGE "May the flow be with you..."
#define PAGE_TITLE "DIY Flow Bench"
#define LANGUAGE_FILE "language/EN_language.h"
#define SHOW_ALARMS true
#define MIN_REFRESH_RATE 100
#define API_IS_ENABLED                                  
#define API_BLOB_LENGTH 1024
#define API_RESPONSE_LENGTH 64
#define API_STATUS_LENGTH 128
#define API_JSON_LENGTH 1020
#define API_SCAN_DELAY_MS 100
#define PRINT_BUFFER_LENGTH 128
//#define API_CHECKSUM_IS_ENABLED                           // Add checksum to serial API response TODO: UPDATE CHECKSUM TO NATIVE ESP32 CRC32
#define MAX_SEMAPHORE_DELAY 1000                          // Define max value rather than just use portMAX_DELAY
#define WEBSOCK_CLEAN_FREQ 600000
#define STATUS_UPDATE_RATE 100
#define FILESYSTEM SPIFFS



/***********************************************************
* BENCH SETTINGS
***/
#define MIN_TEST_PRESSURE_PERCENTAGE 80                     // Lowest test pressure bench will generate accurate results. Please see note in wiki

 


/***********************************************************
* GENERAL HARDWARE SETTINGS
*
* Adjustment figures for voltage regulators etc
***/

#define VCC_3V3_TRIMPOT 0.0                                 // volts
#define VCC_5V_TRIMPOT 0.0                                  // volts
#define USE_FIXED_3_3V_VALUE                                // Useful for testing
#define USE_FIXED_5V_VALUE                                  // Useful for testing





/***********************************************************
* CONFIGURE BME280
*
*  Default address 0x76 / Alternate is usually 0x77
*  Check the report shown in serial monitor on boot for addresses of I2C devices and update BME280_I2C_ADDR accordingly
***/

#define BME_IS_ENABLED                                      // Comment to disable BME related code

const int BME280_I2C_ADDR = 0x76;                           
// const int BME280_I2C_ADDR = 0x77;       
#define BME_SCAN_DELAY_MS 1000                              // Does not need to be faster than this as sensor read speed is around 30 secs!!       




/***********************************************************
* CONFIGURE ADC
*
*  standard shield can use ADS1015 (12 bit) or ADS1115 (16 bit) ADC's however ADS 1015 is untested with ADC1115-lite library
*  MAF / P-Ref / P-Diff / Pitot sensors are all connected through the ADC (see MAF / PDiff / PRef / Pitot sections for channel designations)
*  ADC is used at 5 Volts with an I2C level shifter to allow it to communicate on the 3.3v I2C bus of the ESP32. This allows for an easy way to run 5v sensors on the 3.3v ESP32
*  ADC communication can be found in Hardware->getADCRawData()
*
* Default device address is 0x48
* Connect ADR pin as below to set alternate addresses
* 0x48 (1001000) ADR -> GND
* 0x49 (1001001) ADR -> VDD
* 0x4A (1001010) ADR -> SDA
* 0x4B (1001011) ADR -> SCL
*
*  Check the report shown in serial monitor on boot for addresses of I2C devices and update ADC_I2C_ADDR accordingly
***/

#define ADC_IS_ENABLED                                      // Comment to disable ADC related code

const int ADC_I2C_ADDR = 0x48; 
#define ADC_SCAN_DELAY_MS 100                               // Need to allow enough time for ADC read (min 10ms)
#define ADC_MAX_RETRIES 10

#define ADC_TYPE_ADS1115 // 16 bit (188uV/bit)
// #define ADC_TYPE_ADS1015 // 12 bit (3 mV/bit)            // UNTESTED (Not supported in ADS1115_lite library but might work)




/***********************************************************
 * CONFIGURE MAF
 * If you want to modify the code to include additional MAF sensors
 * You will need to create your own MAF data file. Use exampleMafData.h as an example
 *
 * NOTE: RC level software has only been tested with default recommended sensor (ACDELCO_92281162)
 * NOTE: Currently onbly VOLTAGE based MAF sensors are working
 ***/

#define MAF_IS_ENABLED                                      // Comment to disable MAF related code
    
// Uncomment One sensor only
#define MAF_DATA_FILE "mafData/ACDELCO_92281162.cpp"          //default recommended sensor    
// #define MAF_DATA_FILE "mafData/ACDELCO_19330122.h" 
// #define MAF_DATA_FILE "mafData/VDO_AFM_043.h"
// #define MAF_DATA_FILE  "mafData/MH95_3000_100.h"         // PMAS MH95-3000 in 100mm housing              
// #define MAF_DATA_FILE  "mafData/SIEMENS_5WK9605.h"       // Data from Tonys tests
// #define MAF_DATA_FILE  "mafData/DELPHI_AF10118.h"        // kg/hr - Data from efidynotuning.com/maf.htm 
// #define MAF_DATA_FILE  "mafData/TEST.h"                  // Test Data


// Set signal source (Uncomment One line only)
// #define MAF_SRC_IS_PIN
#define MAF_SRC_IS_ADC


#define MAF_MV_TRIMPOT 0.0                                  // Millivolt offset
#define MAF_ADC_CHANNEL 0




/***********************************************************
* CONFIGURE REFERENCE PRESSURE SENSOR
* If you want to modify the code to include additional reference pressure sensors
* You will need to add your volts to kPa algorithm in the function sensors->getPRef()
*
* Recommended sensor is the MPXV7007DP
***/

#define PREF_IS_ENABLED                                       // Comment to disable reference pressure related code

#define FIXED_REF_PRESS_VALUE 1                           // Fixed pressure value in Pascals

// Set signal source (Uncomment One line only)
//#define PREF_SRC_PIN
#define PREF_SRC_ADC

// Set sensor type (Uncomment One line only)
// #define PREF_SENSOR_NOT_USED
// #define PREF_SENSOR_TYPE_LINEAR_ANALOG 
#define PREF_SENSOR_TYPE_MPXV7007        

#define PREF_MV_TRIMPOT 0.0                                 // Millivolt offset
#define PREF_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value
#define PREF_ADC_CHANNEL 2                                  // BUG: TEMP SWAPPED WITH PDIFF (ERROR ON PCB)





/***********************************************************
* CONFIGURE DIFFERENTIAL PRESSURE SENSOR
* If you want to modify the code to include additional reference pressure sensors
* You will need to add your volts to kPa algorithm in the function sensors->getPDiff()
* 
* Recommended sensor is the MPXV7007DP
***/

#define PDIFF_IS_ENABLED                                    //Comment to disable Differential pressure related code

#define FIXED_DIFF_PRESS_VALUE 1                            // Fixed pressure value in Pascals

// Set signal source (Uncomment One line only)
// #define PDIFF_SRC_IS_PIN
#define PDIFF_SRC_IS_ADC

// Set sensor type (Uncomment One line only)
// #define PDIFF_SENSOR_NOT_USED            
// #define PDIFF_SENSOR_TYPE_LINEAR_ANALOG 
#define PDIFF_SENSOR_TYPE_MPXV7007          

#define PDIFF_MV_TRIMPOT 0.0                                // Millivolt offset
#define PDIFF_ANALOG_SCALE 1.0                              // Scaling factor used for raw analog value
#define PDIFF_ADC_CHANNEL 1                                 // TODO: TEMP SWAPPED WITH PREF (ERROR ON PCB)




/***********************************************************
* CONFIGURE PITOT PRESSURE SENSOR
* If you want to modify the code to include additional pitot pressure sensors
* You will need to add your volts to kPa algorithm in the function sensors->getPitot()
* Note Pitot sensors need to be a differential pressure sensor (DP)
*
* Recommended sensor is the MPXV7007DP
***/

#define PITOT_IS_ENABLED                                    // Comment to disable pitot related code

// Set signal source (Uncomment One line only)
//#define PITOT_SRC_IS_PIN
#define PITOT_SRC_IS_ADC

// Set sensor type (Uncomment One line only)
// #define PITOT_SENSOR_NOT_USED
// #define PITOT_SENSOR_TYPE_LINEAR_ANALOG                  // Use analog signal from PITOT_PIN
#define PITOT_SENSOR_TYPE_MPXV7007DP

#define PITOT_MV_TRIMPOT 0.0                                // Millivolt offset
#define PITOT_ANALOG_SCALE 1.0                              // Scaling factor used for raw analog value
#define PITOT_ADC_CHANNEL 3





/***********************************************************
* CONFIGURE BARO SENSOR
*
* Default Baro 101.3529kpa - standard sea level baro pressure (14.7 psi) 
*
* Recommended sensor is the BME280
***/

#define BARO_IS_ENABLED                                     // Comment to disable Baro related code

// Uncomment One line only
// #define BARO_SENSOR_TYPE_FIXED_VALUE
// #define BARO_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from REF_BARO_PIN
#define BARO_SENSOR_TYPE_BME280
// #define BARO_SENSOR_TYPE_MPX4115                         // use absolute pressure sensor

#define FIXED_BARO_VALUE 101.3529                         // Default pressure in kPa
#define BARO_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value
#define startupBaroScalingFactor 1                          // scaling factor when using reference pressure sensor for baro correction
#define startupBaroScalingOffset 100                        // scaling offset when using reference pressure sensor for baro correction

#define BARO_MV_TRIMPOT 0.0                                 // Millivolt offset
#define BARO_FINE_ADJUST 0.0                                // Adjust end value

#define SEALEVELPRESSURE_HPA 1016.90                        // Change for your local value





/***********************************************************
* CONFIGURE TEMPERATURE SENSOR
*
* Recommended sensor is the BME280
***/

#define TEMP_IS_ENABLED                                     // Comment to disable temperature related code.

#define FIXED_TEMP_VALUE 21                               // Value to return if no sensor used
#define TEMP_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value

// Uncomment One line only
// #define TEMP_SENSOR_NOT_USED
// #define TEMP_SENSOR_TYPE_FIXED_VALUE
// #define TEMP_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from TEMPERATURE_PIN
#define TEMP_SENSOR_TYPE_BME280
// #define TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11

#define TEMP_MV_TRIMPOT 0.0                                 // Millivolt offset
#define TEMP_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value
#define TEMP_FINE_ADJUST 0.0                                // Adjust end value



/***********************************************************
* CONFIGURE HUMIDITY SENSOR
*
* Recommended sensor is the BME280
***/

#define RELH_IS_ENABLED                                     // Comment to disable humidity related code

#define FIXED_RELH_VALUE 36                               // Value to return if no sensor used
#define RELH_ANALOG_SCALE 1.0                               // Scaling factor for raw analog value

// Uncomment ONE of the following
// #define RELH_SENSOR_NOT_USED
// #define RELH_SENSOR_TYPE_FIXED_VALUE
// #define RELH_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from HUMIDITY_PIN
#define RELH_SENSOR_TYPE_BME280
// #define RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11

#define RELH_MV_TRIMPOT 0.0                                 // Millivolt offset
#define RELH_FINE_ADJUST 0.0                                // Adjust end value


// REVIEW
/***********************************************************
* ADVANCED CALIBRATION SETTINGS
*
* For a more detailed explanation of these settings
*
***/

// NOTE: This method is currently unused
// It is not clear if this method would produce usable results for all sensors.
// Suggest that once project is stable, method is implimented and results compared against known good data to validate 
// Potential for use of saw bench blade or similar for easily available orifice of known size (currently using CD)

// generate MAF data table using three point method
#define calibrationPlateHighCFM 100                         // Flow rate for large calibration orifice
#define calibrationPlateMidCFM 50                           // Flow rate for med calibration orifice
#define calibrationPlateLowCFM 10                           // Flow rate for small calibration orifice



// REVIEW
/***********************************************************
* What orifices are used?
* 
* TODO: Move into configuration data
*
* {diameter_in_mm, cfm_flow@15"}
***/
// double orificeData [][2] = {
//     {51.85,209.5},
//     {42.38,139.5},
//     {29.98,70.4},
//     {21.16,35.5},
//     {9.92,7.8}
// };


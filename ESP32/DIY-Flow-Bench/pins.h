/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* pins.h - Define pin mappings
*
* Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
* 
* For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
* Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions 
* You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
* 
* This project and all associated files are provided for use under the GNU GPL3 license:
* https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
* 
* 
***/
#pragma once

#include "configuration.h";

/***********************************************************
* OFFICIAL DIYFB Shield 
* 
* Uses Wemos D1R32 'ESPduino' Uno sized board or similar
*
***/
#ifdef DIYFB_SHIELD

    #define BOARD_TYPE              "DIYFB_SHIELD"

    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN           14                      // DAC speed reference for VFD
    #define VAC_BLEED_VALVE_PIN     27                      // DAC bleed valve control
    
    #define VAC_BANK_1              5                       // vac motor(s) on/off
    #define VAC_BANK_2              13                      // Provision for 2 stage Vac motor control
    #define VAC_BANK_3              12                      // Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL                18                     
    #define AVO_STEP                19
    #define AVO_DIR                 23

    #define VOLTAGE_PIN             0
    
    // SENSORS
    #define SPEED_SENSOR_PIN        25                       // turbine speed for turbo flow bench

    #define MAF_PIN                 2
    #define REF_PRESSURE_PIN        4
    #define DIFF_PRESSURE_PIN       35
    #define PITOT_PIN               34
    
    #define TEMPERATURE_PIN         26                      // NOTE: Not used by the DIYFB shield
    #define REF_BARO_PIN            36                      // NOTE: Not used by the DIYFB shield
    #define HUMIDITY_PIN            39                      // NOTE: Not used by the DIYFB shield

    
    // COMMS
    
    #define SERIAL0_TX              1                       // API
    #define SERIAL0_RX              3                       // API
    #define SERIAL2_TX              16                      // GAUGE PROTOCOL
    #define SERIAL2_TX              17                      // GAUGE PROTOCOL
    #define SCA_PIN                 21                      // BME280 etc
    #define SCL_PIN                 22                      // BME280 etc

    // SPARE 
    #define SPARE_PIN_1             15
    #define SPARE_PIN_2             32
    #define SPARE_PIN_3             33

#endif



/***********************************************************
* CUSTOM ESP32 / ESP32DUINO BASED SOLUTION
*
* NOTE: the ESPDuino has 18 available I/O pins
* On the Wemos D1R32 board there are also 3 additional I/O pins not on the header - 15/32/33
*
* Available GPIO Pins = 2/4/35/34/36/39 | 18/19/23/5/13/12 | 14/27/16/17/25/26 
* 1/3/21/22 reserved for Serial / I2C Additionally 16/17 reserved for gauge data connection
***/
#ifdef ESP32DUINO
    
    #define BOARD_TYPE              "ESP32DUINO"
    
    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN           14                      // DAC speed reference for VFD
    #define VAC_BLEED_VALVE_PIN     27                      // DAC bleed valve control
    
    #define VAC_BANK_1              5                       // vac motor(s) on/off
    #define VAC_BANK_2              13                      // Provision for 2 stage Vac motor control
    #define VAC_BANK_3              12                      // Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL                18                     
    #define AVO_STEP                19
    #define AVO_DIR                 23
    
    #define VOLTAGE_PIN             0
    
    // SENSORS
    #define SPEED_SENSOR_PIN        25                       // turbine speed for turbo flow bench
    
    #define MAF_PIN                 2
    #define REF_PRESSURE_PIN        4
    #define DIFF_PRESSURE_PIN       35
    #define PITOT_PIN               34
    
    #define TEMPERATURE_PIN         26                      // NOTE: these become spare if BME280 used
    #define REF_BARO_PIN            36                      // NOTE: these become spare if BME280 used
    #define HUMIDITY_PIN            39                      // NOTE: these become spare if BME280 used
    
    
    // COMMS
    
    #define SERIAL0_TX              1                       // API
    #define SERIAL0_RX              3                       // API
    #define SERIAL2_TX              16                      // GAUGE PROTOCOL
    #define SERIAL2_TX              17                      // GAUGE PROTOCOL
    #define SCA_PIN                 21                      // BME280 etc
    #define SCL_PIN                 22                      // BME280 etc
    
    
    // NOTE: Wemos D1R32 also has additional spare I/O points broken out on the board but not in the header (defined below)
    // Preference is NOT to use these in the core code as it ties the project to this specific board (other boards do not have these)
    // These are included here for for users making a custom shield that need additional I/O
    
    #define SPARE_PIN_1             15
    #define SPARE_PIN_2             32
    #define SPARE_PIN_3             33

    
#endif



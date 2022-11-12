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

#include "configuration.h"

/***********************************************************
* OFFICIAL DIYFB Shield 
* 
* Uses Wemos D1R32 'ESPduino' Uno sized board
* 
* Check pin assignments before using with other Uno style ESP32 boards
*
* NOTE: GPIO 5, 15, 16, 17, 18, 19, 23 cannot be used for ADC
***/
#ifdef DIYFB_SHIELD

    #define BOARD_TYPE              "DIYFB_SHIELD"

    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN           25                      // Built in DAC1 - used for speed reference for VFD
    #define VAC_BLEED_VALVE_PIN     26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1              5                       // vac motor(s) on/off
    #define VAC_BANK_2              13                      // Provision for 2 stage Vac motor control
    #define VAC_BANK_3              12                      // Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL                18                     
    #define AVO_STEP                19
    #define AVO_DIR                 23

    #define VOLTAGE_PIN             35                      // 10k-10k Voltage divider across 5v supply
    
    // SENSORS
    #define SPEED_SENSOR_PIN        0                       // turbine / rotor speed for turbo / blower flow bench

    #define MAF_PIN                 999                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN        999                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN       999                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN               999                     // NOTE: I2C ADC is used instead
    
    #define TEMPERATURE_PIN         999                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN            999                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN            999                     // NOTE: I2C BME280 used    
    
    // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1        34                      
    #define ORIFICE_BCD_BIT2        36                      
    #define ORIFICE_BCD_BIT3        39                      
    
    // COMMS    
    #define SERIAL0_TX              1                       // API
    #define SERIAL0_RX              3                       // API
    #define SERIAL2_TX              16                      // GAUGE PROTOCOL
    #define SERIAL2_TX              17                      // GAUGE PROTOCOL
    #define SCA_PIN                 21                      // BME280 etc
    #define SCL_PIN                 22                      // BME280 etc
    
    // SPARE
    #define SPARE_PIN_1             14                      
    #define SPARE_PIN_2             27
    
    // NOTE: Pins 15/32/33 are only available on Wemos D1 R32
    #define SPARE_PIN_3             15                      // NOTE: Pin 15 cannot be used for ADC (Analog in)
    #define SPARE_PIN_4             32
    #define SPARE_PIN_5             33

#endif



/***********************************************************
* CUSTOM ESP32 / ESP32DUINO BASED SOLUTION
*
* NOTE: the ESPDuino has 18 available I/O pins
* On the Wemos D1R32 board there are also 3 additional I/O pins not on the header - 15/32/33
*
* Available GPIO Pins = 2/4/35/34/36/39 | 18/19/23/5/13/12 | 14/27/16/17/25/26 
* 1/3/21/22 reserved for Serial / I2C Additionally 16/17 reserved for gauge data connection
* NOTE: GPIO 5, 15, 16, 17, 18, 19, 23 cannot be used for ADC
***/
#ifdef ESP32DUINO
    
    #define BOARD_TYPE              "ESP32DUINO"
    
    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN           25                      // Built in DAC1 - used for speed reference for VFD
    #define VAC_BLEED_VALVE_PIN     26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1              5                       // vac motor(s) on/off
    #define VAC_BANK_2              13                      // Provision for 2 stage Vac motor control
    #define VAC_BANK_3              12                      // Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL                18                     
    #define AVO_STEP                19
    #define AVO_DIR                 23
    
    #define VOLTAGE_PIN             35                      // 10k-10k Voltage divider across 5v supply
    
    // SENSORS
    #define SPEED_SENSOR_PIN        0                       // turbine speed for turbo flow bench
    
    #define MAF_PIN                 999                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN        999                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN       999                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN               999                     // NOTE: I2C ADC is used instead
    
    #define TEMPERATURE_PIN         999                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN            999                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN            999                     // NOTE: I2C BME280 used
    
    // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1        34                      
    #define ORIFICE_BCD_BIT2        36                      
    #define ORIFICE_BCD_BIT3        39                      

    // COMMS
    #define SERIAL0_TX              1                       // API
    #define SERIAL0_RX              3                       // API
    #define SERIAL2_TX              16                      // GAUGE PROTOCOL
    #define SERIAL2_RX              17                      // GAUGE PROTOCOL
    #define SCA_PIN                 21                      // BME280 etc
    #define SCL_PIN                 22                      // BME280 etc
    
    

    // SPARE
    #define SPARE_PIN_1             14                      
    #define SPARE_PIN_2             27
    
    // NOTE: Wemos D1R32 also has additional spare I/O points broken out on the board but not in the header (defined below)
    // Preference is NOT to use these in the core code as it ties the project to this specific board (other boards do not have these)
    // These are included here for for users making a custom shield that need additional I/O

    #define SPARE_PIN_3             15                      // NOTE: Pin 15 cannot be used for ADC (Analog in)
    #define SPARE_PIN_4             32
    #define SPARE_PIN_5             33

    
#endif



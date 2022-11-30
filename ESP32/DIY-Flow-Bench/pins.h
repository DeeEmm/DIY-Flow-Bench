/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file pins.h
 * 
 * @brief define pin mappings for ESP32 boards
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

#include "configuration.h"




/***********************************************************
* OFFICIAL DIYFB Shield 
* 
* Uses Wemos D1R32 'ESPduino' Uno sized board
* 
* Check pin assignments before using with other Uno style ESP32 boards
*
* NOTE: GPIO 5, 15, 16, 17, 18, 19, 23 cannot be used for ADC
* NOTE: 12. 13. 14, 15 may be used for JTAG Debugging provided that normal use is disabled
*
***/
#ifdef DIYFB_SHIELD

    #define BOARD_TYPE              "DIYFB_SHIELD"

    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN               25                      // Built in DAC1 - used for speed reference for VFD
    #define VAC_BLEED_VALVE_PIN         26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1_PIN              5                       // vac motor(s) on/off
    #define VAC_BANK_2_PIN              13                      // [JTAG TCK] Provision for 2 stage Vac motor control
    #define VAC_BANK_3_PIN              12                      // [JTAG TDI] Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL_PIN                18                     
    #define AVO_STEP_PIN                19
    #define AVO_DIR_PIN                 23

    #define VCC_3V3_PIN                 35                      // TODO: Define pin
    #define VCC_5V_PIN                  35                      // 10k-10k divider across 5v supply
    
    // SENSORS
    #define SPEED_SENSOR_PIN            0                       // turbine / rotor speed for turbo / blower flow bench

    // NOTE: these inputs are handled by ADC
    #define MAF_PIN                     33                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN            33                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN           33                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN                   3                     // NOTE: I2C ADC is used instead
    
    // NOTE: These inputs are handled by BME280
    #define TEMPERATURE_PIN             33                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN                33                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN                33                     // NOTE: I2C BME280 used    

    
    // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1_PIN        34                      
    #define ORIFICE_BCD_BIT2_PIN        36                      
    #define ORIFICE_BCD_BIT3_PIN        39                      
    
    // COMMS    
    #define SERIAL0_TX_PIN              1                       // API
    #define SERIAL0_RX_PIN              3                       // API
    #define SERIAL2_TX_PIN              16                      // GAUGE PROTOCOL
    #define SERIAL2_RX_PIN              17                      // GAUGE PROTOCOL
    #define SCA_PIN                     21                      // BME280 etc
    #define SCL_PIN                     22                      // BME280 etc
    
    // SPARE
    #define SPARE_PIN_1                 14                      // [JTAG TMS]                     
    #define SPARE_PIN_2                 27
    
    // NOTE: Pins 15/32/33 are only available on Wemos D1 R32
    #define SPARE_PIN_3                 15                      // [JTAG TDO] NOTE: Pin 15 cannot be used for ADC (Analog in)
    #define SPARE_PIN_4                 32
    #define SPARE_PIN_5                 33

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
* NOTE: 12. 13. 14, 15 may be used for JTAG Debugging provided that normal use is disabled
***/
#ifdef ESP32DUINO
    
    #define BOARD_TYPE              "ESP32DUINO"
    
    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN               25                      // Built in DAC1 - used for speed reference for VFD
    #define VAC_BLEED_VALVE_PIN         26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1_PIN              5                       // vac motor(s) on/off
    #define VAC_BANK_2_PIN              13                      // Provision for 2 stage Vac motor control
    #define VAC_BANK_3_PIN              12                      // Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL_PIN                18                     
    #define AVO_STEP_PIN                19
    #define AVO_DIR_PIN                23
    
    #define VCC_3V3_PIN                 35                      // TODO: Define pin
    #define VCC_5V_PIN                  35                      // 10k-10k divider across 5v across 5v supply
    
    // SENSORS
    #define SPEED_SENSOR_PIN            0                       // turbine speed for turbo flow bench
    
    // NOTE: these inputs are handled by ADC
    #define MAF_PIN                     33                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN            33                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN           33                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN                   33                     // NOTE: I2C ADC is used instead
    
    // NOTE: These inputs are handled by BME280
    #define TEMPERATURE_PIN             33                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN                33                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN                33                     // NOTE: I2C BME280 used

  
    
    // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1_PIN        34                      
    #define ORIFICE_BCD_BIT2_PIN        36                      
    #define ORIFICE_BCD_BIT3_PIN        39                      

    // COMMS
    #define SERIAL0_TX_PIN              1                       // API
    #define SERIAL0_RX_PIN              3                       // API
    #define SERIAL2_TX_PIN              16                      // GAUGE PROTOCOL
    #define SERIAL2_RX_PIN              17                      // GAUGE PROTOCOL
    #define SCA_PIN                     21                      // BME280 etc
    #define SCL_PIN                     22                      // BME280 etc
    
    

    // SPARE
    #define SPARE_PIN_1                 14                      
    #define SPARE_PIN_2                 27
    
    // NOTE: Wemos D1R32 also has additional spare I/O points broken out on the board but not in the header (defined below)
    // Preference is NOT to use these in the core code as it ties the project to this specific board (other boards do not have these)
    // These are included here for for users making a custom shield that need additional I/O

    #define SPARE_PIN_3                 15                      // NOTE: Pin 15 cannot be used for ADC (Analog in)
    #define SPARE_PIN_4                 32
    #define SPARE_PIN_5                 33

    
#endif




/***********************************************************
* ESP32 WROVER KIT
* NOTE: FOR DEBUGGING IN VSCode with PlatformIO
* Uses Officiel Expressif ESP32 WROVER Development KIT
* Do not use on other boards as pin configuration is modded for debugging
* NOTE: GPIO 12,13,14,15 are reserved for JTAG interface and so are unused in this map
***/
#ifdef ESP32_WROVER_KIT

    #define BOARD_TYPE                  "ESP32_WROVER_KIT"

    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN               25                      // Built in DAC1 - used for speed reference for VFD
    #define VAC_BLEED_VALVE_PIN         26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1_PIN              5                       // vac motor(s) on/off
    #define VAC_BANK_2_PIN              33                      // [JTAG TCK] Provision for 2 stage Vac motor control
    #define VAC_BANK_3_PIN              33                      // [JTAG TDI] Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL_PIN                18                     
    #define AVO_STEP_PIN                19
    #define AVO_DIR_PIN                 23

    #define VCC_3V3_PIN                 35                      // TODO: Define pin
    #define VCC_5V_PIN                  35                      // 10k-10k divider across 5v supply
    
    // SENSORS
    #define SPEED_SENSOR_PIN            0                       // turbine / rotor speed for turbo / blower flow bench

    // NOTE: these inputs are handled by ADC
    #define MAF_PIN                     33                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN            33                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN           33                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN                   3                      // NOTE: I2C ADC is used instead
    
    // NOTE: These inputs are handled by BME280
    #define TEMPERATURE_PIN             33                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN                33                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN                33                     // NOTE: I2C BME280 used    

    
    // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1_PIN        34                      
    #define ORIFICE_BCD_BIT2_PIN        36                      
    #define ORIFICE_BCD_BIT3_PIN        39                      
    
    // COMMS    
    #define SERIAL0_TX_PIN              1                       // API
    #define SERIAL0_RX_PIN              3                       // API
    #define SERIAL2_TX_PIN              16                      // GAUGE PROTOCOL
    #define SERIAL2_RX_PIN              17                      // GAUGE PROTOCOL
    #define SCA_PIN                     21                      // BME280 SCA
    #define SCL_PIN                     22                      // BME280 SCL
    
    // SPARE
    #define SPARE_PIN_1                 33                      // [JTAG TMS]                     
    #define SPARE_PIN_2                 27
    
    // NOTE: Pins 15/32/33 are only available on Wemos D1 R32
    #define SPARE_PIN_3                 33                      // [JTAG TDO] NOTE: Pin 15 cannot be used for ADC (Analog in)
    #define SPARE_PIN_4                 32
    #define SPARE_PIN_5                 33




#endif

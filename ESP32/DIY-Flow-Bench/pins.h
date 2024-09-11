/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file pins.h
 * 
 * @brief define pin mappings for ESP32 style boards
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @note There are some limitations to pin use due to be aware of. For example ADC2 pins cannot be used for analog readings when WiFi is on
 * There is further information available on the expresif website - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
 * 
 * @note Officially supported board for DIYFB Shield is the Wemos D1-R32.
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/
#pragma once

#include "configuration.h"






/***********************************************************
 * OFFICIAL DIYFB Shield With Wemos D1 R32
 *
 ***/
#ifdef WEMOS_D1_R32

    #define BOARD_TYPE              "WEMOS_D1_R32"

    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN               25                      // Built in DAC1 - used for speed reference for VFD (0-3v)
    #define VAC_BLEED_VALVE_PIN         26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1_PIN              13                      // vac motor(s) on/off
    #define VAC_BANK_2_PIN              12                      // Provision for 2 stage Vac motor control
    #define VAC_BANK_3_PIN              14                      // Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER                   
    #define AVO_STEP_PIN                15                      // NOTE: Pin 15 cannot be used for ADC (Analog in)
    #define AVO_DIR_PIN                 27                      
                  
    #define FLOW_VALVE_STEP_PIN         32
    #define FLOW_VALVE_DIR_PIN          33

    #define VCC_3V3_PIN                 99                      // Not required
    #define VCC_5V_PIN                  35                      // 10k-10k divider across 5v supply
    
    // SENSORS
    #define SPEED_SENSOR_PIN            2                       // turbine / rotor speed for turbo / blower flow bench 

    // SWIRL ENCODER
    #define SWIRL_ENCODER_PIN_A         2
    #define SWIRL_ENCODER_PIN_B         4

    // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1_PIN        34                                       
    #define ORIFICE_BCD_BIT2_PIN        36                                           
    #define ORIFICE_BCD_BIT3_PIN        39                         

    // NOTE: these inputs are handled by ADC
    #define MAF_PIN                     99                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN            99                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN           99                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN                   99                     // NOTE: I2C ADC is used instead
    
    // NOTE: These inputs are handled by BME280
    #define TEMPERATURE_PIN             99                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN                99                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN                99                     // NOTE: I2C BME280 used                     
    
    // COMMS    
    #define SERIAL0_TX_PIN              1                       // API
    #define SERIAL0_RX_PIN              3                       // API
    #define SERIAL2_TX_PIN              16                      // GAUGE PROTOCOL CLOCK
    #define SERIAL2_RX_PIN              17                      // GAUGE PROTOCOL DATA
    #define SDA_PIN                     21                      // BME280 etc
    #define SCL_PIN                     22                      // BME280 etc

    #define SD_CS_PIN                   5
    #define SD_MOSI_PIN                 23                 
    #define SD_MISO_PIN                 19             
    #define SD_SCK_PIN                  18                   


    #define WEMOS_SPARE_PIN_1           4                       // NOTE cannot be used for analog                 


#endif






/***********************************************************
 * GENERIC CUSTOM ESP32 / ESP32DUINO BASED SOLUTION
 * Copy or modify this section to create your own custom board mapping
 *
 * NOTE: the ESPDuino has 18 available I/O pins
 * On the Wemos D1R32 board there are also 3 additional I/O pins not on the header - 15/32/33
 * Some pins have limited use. See following URL for more info
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
 ***/
#ifdef ESP32DUINO
    
    #define BOARD_TYPE              "ESP32DUINO"
    
    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN               5                       // Built in DAC1 - used for speed reference for VFD (0-3v)
    #define VAC_BLEED_VALVE_PIN         26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1_PIN              13                      // vac motor(s) on/off
    #define VAC_BANK_2_PIN              12                      // Provision for 2 stage Vac motor control
    #define VAC_BANK_3_PIN              14                      // Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER                   
    #define AVO_STEP_PIN                19                      // NOTE 19 Also used by SD Card
    #define AVO_DIR_PIN                 23                      // NOTE 23 Also used by SD Card
                  
    #define FLOW_VALVE_STEP_PIN         27
    #define FLOW_VALVE_DIR_PIN          35

    #define VCC_3V3_PIN                 99                      
    #define VCC_5V_PIN                  2                      // 10k-10k divider across 5v supply
    
    // SENSORS
    #define SPEED_SENSOR_PIN            5                      // NOTE Also used by SD Card  // turbine / rotor speed for turbo / blower flow bench 

     // SWIRL ENCODER
    #define SWIRL_ENCODER_PIN_A         99
    #define SWIRL_ENCODER_PIN_B         99

   // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1_PIN        34                                       
    #define ORIFICE_BCD_BIT2_PIN        36                                           
    #define ORIFICE_BCD_BIT3_PIN        39                         

    // NOTE: these inputs are handled by ADC
    #define MAF_PIN                     99                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN            99                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN           99                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN                   99                     // NOTE: I2C ADC is used instead
    
    // NOTE: These inputs are handled by BME280
    #define TEMPERATURE_PIN             99                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN                99                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN                99                     // NOTE: I2C BME280 used                     
    
    // COMMS    
    #define SERIAL0_TX_PIN              1                       // API
    #define SERIAL0_RX_PIN              3                       // API
    #define SERIAL2_TX_PIN              16                      // GAUGE PROTOCOL CLOCK
    #define SERIAL2_RX_PIN              17                      // GAUGE PROTOCOL DATA
    #define SDA_PIN                     21                      // BME280 etc
    #define SCL_PIN                     22                      // BME280 etc

    // SD Card
    #define SD_CS_PIN                   5
    #define SD_MOSI_PIN                 23                 
    #define SD_MISO_PIN                 19             
    #define SD_SCK_PIN                  18                   

    
    // NOTE: Pins 15/32/33 are only available on Wemos D1 R32
    #define WEMOS_SPARE_PIN_1           15                      // NOTE: Pin 15 cannot be used for ADC (Analog in)
    #define WEMOS_SPARE_PIN_2           32
    #define WEMOS_SPARE_PIN_3           33         


    
#endif







/***********************************************************
 * Arducam ESP32S UNO board
 * UNTESTED
 * NOTE Arducam boards have onboard SD card on GPIOs 5/23/19/18
 *
  ***/
#ifdef ARDUCAM_ESP32S

    #define BOARD_TYPE              "ARDUCAM_ESP32S"

    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN               25                      // Built in DAC1 - used for speed reference for VFD (0-3v)
    #define VAC_BLEED_VALVE_PIN         26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1_PIN              0                       // vac motor(s) on/off
    #define VAC_BANK_2_PIN              2                       // Provision for 2 stage Vac motor control
    #define VAC_BANK_3_PIN              17                      // Provision for 3 stage Vac motor control
    
    // SENSORS
    #define SPEED_SENSOR_PIN            4                       // turbine / rotor speed for turbo / blower flow bench

    // SWIRL ENCODER
    #define SWIRL_ENCODER_PIN_A         99
    #define SWIRL_ENCODER_PIN_B         99

    // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1_PIN        33                      
    #define ORIFICE_BCD_BIT2_PIN        34                      
    #define ORIFICE_BCD_BIT3_PIN        35     

    //STEPPER MOTOR CONTROLLER                  
    #define FLOW_VALVE_STEP_PIN         16
    #define FLOW_VALVE_DIR_PIN          32
                  
    #define AVO_STEP_PIN                26
    #define AVO_DIR_PIN                 27

    #define VCC_3V3_PIN                 99                      // Unused
    #define VCC_5V_PIN                  36                      // 10k-10k divider across 5v supply
    
    // NOTE: these inputs are handled by ADC
    #define MAF_PIN                     99                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN            99                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN           99                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN                   99                     // NOTE: I2C ADC is used instead
    
    // NOTE: These inputs are handled by BME280
    #define TEMPERATURE_PIN             99                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN                99                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN                99                     // NOTE: I2C BME280 used                     
    
    // COMMS    
    #define SERIAL0_TX_PIN              1                      // API
    #define SERIAL0_RX_PIN              3                      // API
    #define SERIAL2_TX_PIN              15                     // GAUGE PROTOCOL CLOCK
    #define SERIAL2_RX_PIN              14                     // GAUGE PROTOCOL DATA
    #define SDA_PIN                     21                     // BME280 etc
    #define SCL_PIN                     22                     // BME280 etc
    
    // SD Card
    #define SD_CS_PIN                   5
    #define SD_MOSI_PIN                 23                 
    #define SD_MISO_PIN                 19             
    #define SD_SCK_PIN                  18                   

#endif




/***********************************************************
 * Arducam ESO32S LOTAI board
 * UNTESTED
 * NOTE Arducam boards have onboard SD card on GPIOs 5/23/19/18
 *
  ***/
#ifdef ARDUCAM_ESP32S

    #define BOARD_TYPE              "ARDUCAM_LOTAI"

    // Define Physical Pins
    
    // VAC CONTROL
    #define VAC_SPEED_PIN               25                      // Built in DAC1 - used for speed reference for VFD (0-3v)
    #define VAC_BLEED_VALVE_PIN         26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1_PIN              0                       // vac motor(s) on/off
    #define VAC_BANK_2_PIN              2                       // Provision for 2 stage Vac motor control
    #define VAC_BANK_3_PIN              5                       // Provision for 3 stage Vac motor control
    
    // SENSORS
    #define SPEED_SENSOR_PIN            4                       // turbine / rotor speed for turbo / blower flow bench

     // SWIRL ENCODER
    #define SWIRL_ENCODER_PIN_A         99
    #define SWIRL_ENCODER_PIN_B         99

   // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1_PIN        33                      
    #define ORIFICE_BCD_BIT2_PIN        34                      
    #define ORIFICE_BCD_BIT3_PIN        35     

    //STEPPER MOTOR CONTROLLER                  
    #define FLOW_VALVE_STEP_PIN         
    #define FLOW_VALVE_DIR_PIN          32
                  
    #define AVO_STEP_PIN                26
    #define AVO_DIR_PIN                 27

    #define VCC_3V3_PIN                 99                      // Unused
    #define VCC_5V_PIN                  36                      // 10k-10k divider across 5v supply
    
    // NOTE: these inputs are handled by ADC
    #define MAF_PIN                     99                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN            99                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN           99                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN                   99                     // NOTE: I2C ADC is used instead
    
    // NOTE: These inputs are handled by BME280
    #define TEMPERATURE_PIN             99                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN                99                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN                99                     // NOTE: I2C BME280 used                     
    
    // COMMS    
    #define SERIAL0_TX_PIN              1                      // API
    #define SERIAL0_RX_PIN              3                      // API
    #define SERIAL2_TX_PIN              15                     // GAUGE PROTOCOL CLOCK
    #define SERIAL2_RX_PIN              14                     // GAUGE PROTOCOL DATA
    #define SDA_PIN                     21                     // BME280 etc
    #define SCL_PIN                     22                     // BME280 etc
    
    #define SD_CS_PIN                   5
    #define SD_MOSI_PIN                 23                 
    #define SD_MISO_PIN                 19             
    #define SD_SCK_PIN                  18                   

#endif






/***********************************************************
 * ESP32 WROVER KIT
 * NOTE: FOR DEBUGGING IN VSCode with PlatformIO
 * Uses Officiel Expressif ESP32 WROVER Development KIT
 * Do not use configuration on other boards as pin mapping is modded for debugging
 * NOTE: GPIO 12,13,14,15 are reserved for JTAG interface and so are unused in this map
 * NOTE: 
 ***/
#ifdef ESP32_WROVER_KIT

    #define BOARD_TYPE                  "ESP32_WROVER_KIT"

    // Define Physical Pins
    
       // VAC CONTROL
    #define VAC_SPEED_PIN               25                      // Built in DAC1 - used for speed reference for VFD (0-3v)
    #define VAC_BLEED_VALVE_PIN         26                      // Built in DAC2 - used for bleed valve control
    
    #define VAC_BANK_1_PIN              99                      // vac motor(s) on/off
    #define VAC_BANK_2_PIN              99                      // Provision for 2 stage Vac motor control
    #define VAC_BANK_3_PIN              99                      // Provision for 3 stage Vac motor control
    
    //STEPPER MOTOR CONTROLLER                   
    #define AVO_STEP_PIN                99                      // NOTE: Pin 15 cannot be used for ADC (Analog in)
    #define AVO_DIR_PIN                 27                      
                  
    #define FLOW_VALVE_STEP_PIN         32
    #define FLOW_VALVE_DIR_PIN          33

    #define VCC_3V3_PIN                 99                      // Not required
    #define VCC_5V_PIN                  35                      // 10k-10k divider across 5v supply
    
    // SENSORS
    #define SPEED_SENSOR_PIN            2                       // turbine / rotor speed for turbo / blower flow bench 

     // SWIRL ENCODER
    #define SWIRL_ENCODER_PIN_A         99
    #define SWIRL_ENCODER_PIN_B         99

   // ORIFICE DETECTION                                                                                    
    #define ORIFICE_BCD_BIT1_PIN        34                                       
    #define ORIFICE_BCD_BIT2_PIN        36                                           
    #define ORIFICE_BCD_BIT3_PIN        39                         

    // NOTE: these inputs are handled by ADC
    #define MAF_PIN                     99                     // NOTE: I2C ADC is used instead
    #define REF_PRESSURE_PIN            99                     // NOTE: I2C ADC is used instead
    #define DIFF_PRESSURE_PIN           99                     // NOTE: I2C ADC is used instead
    #define PITOT_PIN                   99                     // NOTE: I2C ADC is used instead
    
    // NOTE: These inputs are handled by BME280
    #define TEMPERATURE_PIN             99                     // NOTE: I2C BME280 used
    #define REF_BARO_PIN                99                     // NOTE: I2C BME280 used
    #define HUMIDITY_PIN                99                     // NOTE: I2C BME280 used                     
    
    // COMMS    
    #define SERIAL0_TX_PIN              1                       // API
    #define SERIAL0_RX_PIN              3                       // API
    #define SERIAL2_TX_PIN              16                      // GAUGE PROTOCOL CLOCK
    #define SERIAL2_RX_PIN              17                      // GAUGE PROTOCOL DATA
    #define SDA_PIN                     21                      // BME280 etc
    #define SCL_PIN                     22                      // BME280 etc

    #define SD_CS_PIN                   5
    #define SD_MOSI_PIN                 23                 
    #define SD_MISO_PIN                 19             
    #define SD_SCK_PIN                  18                   


    #define WEMOS_SPARE_PIN_1           4                       // NOTE cannot be used for analog      


#endif

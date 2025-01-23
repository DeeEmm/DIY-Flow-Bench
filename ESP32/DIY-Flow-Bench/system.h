#pragma once
#include <string>
using namespace std;




/***********************************************************
* CONFIGURE FILESYSTEM
***/

#define FORMAT_FILESYSTEM_IF_FAILED


/***********************************************************
* CONFIGURE COMMS
*
* Default comms port is 0 (U0UXD) - (USB programming port) This is used for API / Status Messages / Debugging
* Port 2 (U2UXD) is used to communicate with digital gauge for automated measurements & logging
*
***/

#define SERIAL0_ENABLED                                         // Default serial comms (API & status)
// #define SERIAL2_ENABLED                                      // Digital guage serial protocol

#define SERIAL0_BAUD 115200
#define SERIAL2_BAUD 9600
// #define MAC_ADDRESS {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}    // MAC Address (uncomment to enable)
// #define STATIC_IP {192,168,1,222}                           // Static IP address (uncomment to enable)
#define SUBNET {192,168,1,1}                                    // Subnet (For static IP)
#define GATEWAY {255,255,0,0}                                   // Default gateway (For static IP)
#define WEBSERVER_ENABLED                                       // Disable to run headless


/***********************************************************
 * SYSTEM SETTINGS
 * NOTE: Some of these settings may break operation. Change with care.
 ***/
#define BOOT_MESSAGE "May the flow be with you..."
#define PAGE_TITLE "DIY Flow Bench"
#define FILESYSTEM SPIFFS
#define PRINT_BUFFER_LENGTH 128
#define iSHOW_ALARMS true
#define MIN_iREFRESH_RATE 250

// API data
#define API_IS_ENABLED                                  
// #define API_CHECKSUM_IS_ENABLED                       
#define API_BLOB_LENGTH 1300
#define API_RESPONSE_LENGTH 64

#define API_STATUS_LENGTH 128
// #define API_JSON_LENGTH 1024
#define API_SCAN_DELAY_MS 250
#define API_DELIMITER ":"
#define API_SERIAL_BAUD 115200

// Memory assignment
#define SENSOR_TASK_MEM_STACK 2800 // Free 776
#define ENVIRO_TASK_MEM_STACK 2200 // Free 640
#define LOOP_TASK_STACK_SIZE 9000 // 7000 // 15000 // SET_LOOP_TASK_STACK_SIZE( LOOP_TASK_STACK_SIZE )

// MAF Data Filters
#define ALPHA_MEDIAN 0.75f
#define ALPHA_AVERAGE 0.75f
#define ALPHA_MEAN 0.75f


// Loop Delays
#define VTASK_DELAY_ADC 1   
#define VTASK_DELAY_BME 1
#define VTASK_DELAY_SSE 1


// Poll timers
#define ADC_UPDATE_RATE 501 // 72 ms data read time, 501ms time between ADC read events in milliseconds | Previous 396
#define BME_UPDATE_RATE 1006 // 3 ms data read time, 1006ms time between BME read events in milliseconds | Previous 1103
#define SSE_UPDATE_RATE 554 // 129 ms data read time, 554ms time between SSE push in milliseconds  | Previous 559


// JSON memory allocation
#define DATA_JSON_SIZE 768
#define LANGUAGE_JSON_SIZE 8192 // TODO Need to test language override (mem size could be an issue)
#define CAL_DATA_JSON_SIZE 348
#define LIFT_DATA_JSON_SIZE 384
#define JSON_FILE_SIZE 6000 // 2800
// NOTE: Use ArduinJSON memory assistant to determine size requirements
// https://arduinojson.org/v6/assistant

/***********************************************************
* WEBUI SETTINGS
***/

#define GUI_COLOUR_UNSET "pink"
#define GUI_COLOUR_SET "#44b112"
#define GUI_COLOUR_ERROR "red"


/***********************************************************
* Precompile macro error flags
***/
#ifndef MAJOR_VERSION 
#error MAJOR_VERSION UNDEFINED
#endif
#ifndef MINOR_VERSION 
#error MINOR_VERSION UNDEFINED
#endif
#ifndef BUILD_NUMBER 
#error BUILD_NUMBER UNDEFINED
#endif
#ifndef RELEASE 
#error RELEASE UNDEFINED 
#endif
#ifndef DEV_BRANCH
#error DEV_BRANCH UNDEFINED
#endif
#ifndef UPDATE_SERVER
#error UPDATE_SERVER UNDEFINED
#endif

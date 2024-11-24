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

#define SERIAL0_ENABLED                                     // Default serial comms (API & status)
// #define SERIAL2_ENABLED                                  // Digital guage serial protocol

#define SERIAL0_BAUD 115200
#define SERIAL2_BAUD 9600
// #define MAC_ADDRESS {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}    // MAC Address (uncomment to enable)
// #define STATIC_IP {192,168,1,222}                           // Static IP address (uncomment to enable)
#define SUBNET {192,168,1,1}                                // Subnet (For static IP)
#define GATEWAY {255,255,0,0}                               // Default gateway (For static IP)
#define WEBSERVER_ENABLED                                   // Disable to run headless


/***********************************************************
 * SYSTEM SETTINGS
 * NOTE: Some of these settings may break operation. Change with care.
 ***/
#define BOOT_MESSAGE "May the flow be with you..."
#define PAGE_TITLE "DIY Flow Bench"
#define SHOW_ALARMS true
#define MIN_REFRESH_RATE 250
#define API_IS_ENABLED                                  
#define API_BLOB_LENGTH 1024
#define API_RESPONSE_LENGTH 64
#define API_STATUS_LENGTH 128
#define API_JSON_LENGTH 1020
#define API_SCAN_DELAY_MS 250
#define PRINT_BUFFER_LENGTH 128
// #define API_CHECKSUM_IS_ENABLED                       
#define FILESYSTEM SPIFFS
#define SENSOR_TASK_MEM_STACK 2400 // 2200
#define ENVIRO_TASK_MEM_STACK 2000 // 1800
#define DATA_JSON_SIZE 1500
#define SETTINGS_JSON_SIZE 1536 //1200 
#define LANGUAGE_JSON_SIZE 1200
#define CONFIG_JSON_SIZE 1700 //1536 //1200 
#define CAL_DATA_JSON_SIZE 128
#define LIFT_DATA_JSON_SIZE 384
#define MAF_JSON_SIZE 1500
#define VTASK_DELAY_ADC 500
#define VTASK_DELAY_BME 500
#define VTASK_DELAY_SSE 500


/***********************************************************
* WEBUI SETTINGS
***/
#define STATUS_UPDATE_RATE 500                              // time between SSE push in milliseconds

#define GUI_COLOUR_UNSET "pink"
#define GUI_COLOUR_SET "#44b112"
#define GUI_COLOUR_ERROR "red"



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

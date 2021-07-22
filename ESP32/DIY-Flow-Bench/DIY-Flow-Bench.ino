/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 *
 * DIY-Flow-Bench.ino - Main process
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
  * The standard project board is the ESP32DUINO  
 *   
 * Other ESP32 based boards can be made to work. 
 * You can define custom pin definitions in boards.h
 * 
 * Default temp / baro / RelH uses BME280 device (Sparkfun / clone)
 * I2C address for the BME280 is 0x77
 *
 * Default MAF unit recommended is the GM LS2 MAF (ACDELCO_ 92281162.h)
 * This will measure up to approx 277cfm
 * 
 * DEPENDENCIES
 * This program has a number of core libraries that must be available for it to work.
 *
 *
 ****/

 
/****************************************
 * CORE LIBRARIES
 ***/

#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include <ArduinoJson.h>
#include "configuration.h"
#include "boards.h"
#include "language/EN_Language.h"

/****************************************
 * DECLARE CONSTANTS
 ***/



/****************************************
 * DECLARE VARIABLES
 ***/
int serialData = 0;
extern int statusVal;


/****************************************
 * Create data structure for config params
 * 
 * Note that these settings are overwritten by config.txt stored in SPIFFS
 ***/
struct ConfigSettings {
  String wifi_ssid = "WIFI-SSID";            // Your Wifi SSID
  String wifi_pswd = "<WIFI-PSWD>";          // Your Wifi Password
  String wifi_ap_ssid = "DIYFB";            // Default Accesspoint name
  String wifi_ap_pswd = "123456789";        // Default Accesspoint Password
  String hostname = "diyfb";                 // Default Hostname
  char* api_delim = ":";                    // API Serial comms delimiter
  long serial_baud_rate = 115200;           // Default baud rate 
  int wifi_timeout = 30;                    // Time in seconds before falling back to AP mode
  int min_flow_rate = 3;                    // Flow rate at which bench is considered running
  int min_bench_pressure = 0;               // Minimum bench pressure at which bench is considered running
  int maf_min_millivolts = 100;             // Filter out results less than this
  int refresh_rate = 100;                   //  Screen refresh rate in milliseconds
};
ConfigSettings config;




/****************************************
 * Create AsyncWebServer object on port 80
 ***/
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");



/****************************************
 * INITIALISATION
 ***/
void setup (void)
{

extern ConfigSettings config;

  loadConfig();    
  initialiseServer();
  initialiseHardware();

}



/****************************************
 * MAIN PROGRAM LOOP
 ***/
void loop ()
{

    refPressureCheck();
    statusMessageHandler(statusVal);

    // If API enabled, parse serial data
    #if defined API_ENABLED         
        if (Serial.available() > 0) {
            serialData = Serial.read();
            parseAPI(serialData);
        }                            
    #endif

    ws.cleanupClients();

}
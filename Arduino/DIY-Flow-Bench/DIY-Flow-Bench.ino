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
 ***/

 
/****************************************
 * CORE LIBRARIES
 ***/

#include <EEPROM.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include "configuration.h"
#include "boards.h"
#include LANGUAGE_FILE



/****************************************
 * DECLARE CONSTANTS
 ***/



/****************************************
 * DECLARE VARIABLES
 ***/
int serialData = 0;
extern int statusVal;



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

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
#include <WebServer.h>
#include <SPIFFS.h>
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
extern int errorVal;
WebServer server(80);



/****************************************
 * INITIALISATION
 ***/
void setup (void)
{

    // Serial
    Serial.begin(SERIAL_BAUD_RATE);      
    Serial.println("\r\n");
    Serial.println("DIY Flow Bench ");
    Serial.println("Version: " RELEASE);
    Serial.println("Build: " BUILD_NUMBER);

    // Filesystem
    Serial.print(F("File System Inizializing "));
    if (SPIFFS.begin()){
      Serial.println(F("done."));
    }else{
      Serial.println(F("failed."));
    }

    // API
    #if defined API_ENABLED
      Serial.println("Serial API Enabled");
    #endif

    // WiFi    
    Serial.println("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSWD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
 
    Serial.print("Connected to ");
    Serial.println(WIFI_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    
    // Server
    server.on("/", []() {
      loadFromSPIFFS("/index.html");
    });
    server.on("/style.css", []() {
      loadFromSPIFFS("/style.css");
    });
    server.on("/favicon.png", []() {
      loadFromSPIFFS("favicon.png");
    });

//    server.on("/data.html", sendData );
    server.onNotFound(error404);
    server.begin();
    Serial.println("HTTP server started");

    // Hardware    
    initialiseHardware();

}



/****************************************
 * MAIN PROGRAM LOOP
 ***/
void loop ()
{

    //sendData();

    refPressureCheck();
    if (errorVal != 0) errorHandler(errorVal);

    // If API enabled, read serial data
    #if defined API_ENABLED         
        if (Serial.available() > 0) {
            serialData = Serial.read();
            parseAPI(serialData);
        }                            
    #endif

    server.handleClient();

}

/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
*
* DIY-Flow-Bench.ino - Main process
*
* Open source flow bench project to measure and display volumetric air flow using an ESP32
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
* You can define custom pin definitions in pins.h
* 
* Default temp / baro / RelH uses BME280 device (Sparkfun / Adafruit / clone)
* I2C address for the BME280 is 0x77
*
* Default MAF unit recommended is the GM LS2 MAF (ACDELCO_ 92281162.h)
* This will measure up to approx 277cfm
* Other MAF sensors are supported by creation of MAF Data file - See mafDATA/ for examples
*
* DEPENDENCIES
* This program has a number of core libraries that must be available for it to work.
*
* TODO: Add librarymanager references
* if using the Arduino IDE, click here: http://librarymanager#ArduinoJSON
*
*
****/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "constants.h"
#include "configuration.h"
#include "structs.h"
#include "mafData.h"

// Initiate Structs
ConfigSettings config;
CalibrationSettings calibration;
DeviceStatus status;
WebsocketData socketData;
FileUploadData fileUploadData;
//mafSensorData mafSensor;


#include MAF_SENSOR_FILE
#include LANGUAGE_FILE

//test
//#include "mafData/ACDELCO-92281162.h"

#include "settings.h"
#include "hardware.h"
#include "sensors.h"
#include "webserver.h"
#include "API.h"


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");



// Initiate Classes
Settings _settings;
Hardware _hardware;
Webserver _webserver;
Sensors _sensors;
API _api;



/***********************************************************
 * INITIALISATION
 ***/
void setup(void) {

  _webserver.Initialise();
  _hardware.Initialise();
  _sensors.Initialise();

}



/***********************************************************
 * MAIN PROGRAM LOOP
 ***/
void loop () {

    _hardware.checkRefPressure();
    
    // TODO: where do we send message
    // status.MessageHandler(statusVal);

    if (config.api_enabled) {        
        if (Serial.available() > 0) {
            status.serialData = Serial.read();
            _api.ParseMessage(status.serialData);
        }                            
    }
    
    // Stream data to browser
    if (config.refresh_rate < MIN_REFRESH_RATE) config.refresh_rate = MIN_REFRESH_RATE;
    if ((millis() > status.pollTimer) && status.liveStream) {
      status.pollTimer = millis() + config.refresh_rate; 
      _webserver.SendWebSocketMessage(_webserver.getDataJSON());
    }

    ws.cleanupClients();

}
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
 * TODO: Add librarymanager references
 * if using the Arduino IDE, click here: http://librarymanager#ArduinoJSON
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
#include "pins.h"
#include "language/EN_Language.h"

/****************************************
 * DECLARE CONSTANTS
 ***/

#define GET_FLOW_DATA 1
#define CONFIG 2
#define CALIBRATE 3
#define FILE_LIST 4
#define SYS_STATUS 5
#define SAVE_CONFIG 6 // NOTE: Need to be swapped this end
#define LOAD_CONFIG 7 // NOTE: Need to be swapped this end
#define FILE_DOWNLOAD 8
#define FILE_DELETE 9
#define FILE_UPLOAD 10
#define START_BENCH 11
#define STOP_BENCH 12

/****************************************
 * DECLARE VARIABLES
 ***/

extern int statusVal;



/****************************************
 * Create data structures
 ***/
struct ConfigSettings
{ 
  String wifi_ssid = "WIFI-SSID";           // Your Wifi SSID
  String wifi_pswd = "<WIFI-PSWD>";         // Your Wifi Password
  String wifi_ap_ssid = "DIYFB";            // Default Accesspoint name
  String wifi_ap_pswd = "123456789";        // Default Accesspoint Password
  String hostname = "diyfb";                // Default Hostname
  String api_delim = ":";                   // API Serial comms delimiter
  long serial_baud_rate = 115200;           // Default baud rate 
  int wifi_timeout = 30;                    // Time in seconds before falling back to AP mode
  int min_flow_rate = 3;                    // Flow rate at which bench is considered running
  int min_bench_pressure = 0;               // Minimum bench pressure at which bench is considered running
  int maf_min_millivolts = 100;             // Filter out results less than this
  int refresh_rate = 250;                   // Screen refresh rate in milliseconds (>180)
  float cal_ref_press = 10;                 // Calibration orifice refe pressure
  float cal_flow_rate = 14.4;               // Calibration orifica flow rate
  float cal_offset = 0;                     // Calibration offset
  int cyc_av_buffer = 5;                    // Number of scans over which to average output (helps stabilise results)
  int leak_test_tolerance = 2;              // Leak test tolerance
  bool show_alarms = true;                  // Display Alarms?
};
ConfigSettings config;

struct DeviceStatus {
  bool debug = true;
  int spiffs_mem_size = 0;
  int spiffs_mem_used = 0;
  String local_ip_address;
  String boardType = BOARD_TYPE;
  String mafSensor = MAF_SENSOR;
  String prefSensor = PREF_SENSOR;
  String tempSensor = TEMP_SENSOR;
  String relhSensor = RELH_SENSOR;
  String baroSensor = BARO_SENSOR;
  String pitotSensor = PITOT_SENSOR;
  int boot_time = millis();
  bool liveStream = false;
  int pollTimer = 0;
  int serialData = 0;
};
DeviceStatus status;

struct WebsocketData {
  String file_name;
  int length = 0;
};
WebsocketData socketData;

struct FileUploadData {
  String file_name;
  bool upload_error = false;
  int file_size = 0;
};
FileUploadData fileUploadData;




/****************************************
 * Create AsyncWebServer object on port 80
 ***/
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");


/****************************************
 * INITIALISATION
 ***/
void setup(void) {

  extern ConfigSettings config;
  extern String loadConfig();

  loadConfig();    
  initialiseServer();
  initialiseHardware();

}



/****************************************
 * MAIN PROGRAM LOOP
 ***/
void loop () {

    refPressureCheck();
    statusMessageHandler(statusVal);

    // If API enabled, parse serial data
    #if defined API_ENABLED         
        if (Serial.available() > 0) {
            status.serialData = Serial.read();
            parseAPI(status.serialData);
        }                            
    #endif
    
    // Stream data to browser
    if (config.refresh_rate < MIN_REFRESH_RATE) config.refresh_rate = MIN_REFRESH_RATE;
    if ((millis() > status.pollTimer) && status.liveStream) {
      status.pollTimer = millis() + config.refresh_rate; 
      notifyClients(getDataJson());
    }

    ws.cleanupClients();

}
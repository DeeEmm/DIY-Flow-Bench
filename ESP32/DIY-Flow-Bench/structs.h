/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* Structs.h - Data structure definitions header file
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

#include "Arduino.h"
#include "configuration.h"


/***********************************************************
 * Configuration Settings
 *
 * NOTE: Do not edit these! Used to create default config file! 
 * You can easily edit the config file via the browser after you connect!
 ***/
struct ConfigSettings {
  String wifi_ssid = "WIFI-SSID";           // Your Wifi SSID
  String wifi_pswd = "<WIFI-PSWD>";         // Your Wifi Password
  String wifi_ap_ssid = "DIYFB";            // Default Access Point name
  String wifi_ap_pswd = "123456789";        // Default Access Point Password
  String hostname = "diyfb";                // Default Hostname
  String api_delim = ":";                   // API Serial comms delimiter
  long serial_baud_rate = 115200;           // Default baud rate 
  unsigned long wifi_timeout = 10000;       // Time in millisec's before falling back to AP mode
  int min_flow_rate = 3;                    // Flow rate at which bench is considered running
  int min_bench_pressure = 0;               // Min bench pressure where bench is considered running
  int maf_min_millivolts = 100;             // Filter out results less than this
  int refresh_rate = 250;                   // Screen refresh rate in milliseconds (>180)
  float cal_ref_press = 10;                 // Calibration orifice refe pressure
  float cal_flow_rate = 14.4;               // Calibration orifica flow rate
  float cal_offset = 0;                     // Calibration offset
  int cyc_av_buffer = 5;                    // Scan # over which to average output (helps smooth results)
  int leak_test_tolerance = 2;              // Leak test tolerance
  int leak_test_threshold = 10;             // Value above which leak test activates (max pref - 2 x leak_test_tolerance is a good starting point)
  bool show_alarms = true;                  // Display Alarms?
  bool debug_mode = false;
  bool api_enabled = true;
  int tatltuae = 42;
  int parsecs = 12;
};

//ConfigSettings config;

/***********************************************************
 * Calibration Settings
 ***/
struct CalibrationSettings { 
  float flow_offset = 0.0;         
  float leak_cal_val = 0.0;  
};



/***********************************************************
 * Device status
 ***/
struct DeviceStatus {
  bool debug = true;
  int spiffs_mem_size = 0;
  int spiffs_mem_used = 0;
  String local_ip_address;
  String boardType;
  String mafSensor;
  String prefSensor;
  String pdiffSensor;
  String tempSensor;
  String relhSensor;
  String baroSensor;
  String pitotSensor;
  int boot_time = millis();
  bool liveStream = false;
  int pollTimer = 0;
  int serialData = 0;
  String statusMessage = BOOT_MESSAGE;
  bool apMode = false;
};



/***********************************************************
 * Websocket data
 ***/
struct WebsocketData {
 String file_name;
  int length = 0;
};



/***********************************************************
 * File upload data
 ***/
struct FileUploadData {
 String file_name;
  bool upload_error = false;
  int file_size = 0;
};



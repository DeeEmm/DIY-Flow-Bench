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
 * Default / Fallback Configuration Settings
 *
 * NOTE: Do not edit these! Please leave the default values.
 * These are used to generate the default config settings in the event of a system crash.
 * If you edit these, you may get locked out of your system or brick your device.
 * You can easily edit the config settings via the browser after you connect!
 ***/
struct ConfigSettings {
  int api_response_length = 64;                   // API Serial comms message length
  long serial_baud_rate = 115200;                 // Default baud rate 
  unsigned long wifi_timeout = 15000;             // Time in millisec's before falling back to AP mode
  int min_flow_rate = 3;                          // Flow rate at which bench is considered running
  int min_bench_pressure = 0;                     // Min bench pressure where bench is considered running
  int maf_min_millivolts = 100;                   // Filter out results less than this
  int refresh_rate = 200;                         // Screen refresh rate in milliseconds (>180)
  double cal_ref_press = 10;                       // Calibration orifice refe pressure
  double cal_flow_rate = 14.4;                     // Calibration orifica flow rate
  double cal_offset = 0;                           // Calibration offset
  int cyc_av_buffer = 3;                          // [5] Scan # over which to average output (helps smooth results)
  int leak_test_tolerance = 2;                    // Leak test tolerance
  int leak_test_threshold = 10;                   // Value above which leak test activates (max pref - 2 x leak_test_tolerance is a good starting point)
  bool show_alarms = true;                        // Display Alarms?
  bool debug_mode = false;                         // Global debug print override //REMOVE: (DISABLE)
  bool dev_mode = false;                          // Developer mode
  bool status_print_mode = false;                 // Stream status data to serial
  bool api_enabled = true;                        // Can disable serial API if required
  int tatltuae = 42;
  int parsecs = 12;
  char pageTitle[32] = "DIY Flow Bench";       // Display name for software
  char wifi_ssid[32] = "WIFI-SSID";            // Your Wifi SSID
  char wifi_pswd[32] = "<WIFI-PSWD>";          // Your Wifi Password
  char wifi_ap_pswd[32] = "123456789";         // Default Access Point Password
  char hostname[32] = "diyfb";                 // Default Hostname
  char api_delim[2] = ":";                    // API Serial comms delimiter
  char wifi_ap_ssid[32] = "DIYFB";             // Default Access Point name
};



/***********************************************************
 * Calibration Settings
 ***/
struct CalibrationSettings { 
  double flow_offset = 0.0;         
  double leak_cal_val = 0.0;  
};



/***********************************************************
 * Device status
 ***/
struct DeviceStatus {
  bool debug = true;
  int spiffs_mem_size = 0;
  int spiffs_mem_used = 0;
  String local_ip_address;
  String hostname;
  String boardType;
  String benchType;
  String mafSensor;
  String prefSensor;
  String pdiffSensor;
  String tempSensor;
  String relhSensor;
  String baroSensor;
  String pitotSensor;
  int boot_time = millis();
  bool liveStream = true;
  long adcPollTimer = 0;
  long bmePollTimer = 0;
  long apiPollTimer = 0;
  long wsCLeanPollTimer = 0;
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




/***********************************************************
 * Sensor data
 ***/
struct SensorData {
  double MAF;
  double MafMv;
  double TempDegC;
  double RelH;
  double BaroKPA;
  double PRefKPA;
  double PRefMv;
  double PDiffKPA;
  double PDiffMv;
  double PitotKPA;
  double PitotMv;
};

/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file structs.h
 * 
 * @brief Define Structs
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
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
  unsigned long wifi_timeout = 4000;              // Duration of Wifi connection attempt in millisec's
  unsigned long wifi_retries = 10;                // Number of attempts to connect to Wifi before creating AP
  int min_flow_rate = 1;                          // Flow rate at which bench is considered running in cfm
  int min_bench_pressure = 1;                     // Min bench pressure where bench is considered running (inches/h2o)
  double maf_min_volts = 0.1;                     // Filter out results less than this
  int refresh_rate = 500;                         // Screen refresh rate in milliseconds (>180)
  int adj_flow_depression = 28;                  // Adjusted flow depression in inches of water
  char rounding_type[12] = "NONE";                // Rounding type 
  int flow_decimal_length = 1;                    // Flow decimal number of places 
  int gen_decimal_length = 2;                     // General decimal number of places 
  char data_filter_type[12] = "NONE";             // Data filter type 
  int cyc_av_buffer = 5;                          // [5] Scan # over which to average output (helps smooth results)
  bool show_alarms = true;                        // Display Alarms?
  bool debug_mode = false;                        // Global debug print override
  bool dev_mode = false;                          // Developer mode
  bool status_print_mode = false;                 // Stream status data to serial
  bool api_enabled = true;                        // Can disable serial API if required
  char bench_type[8] = "MAF";                     // Default bench type
  int maf_housing_diameter = 0;                   // MAF Housing diameter
  int tatltuae = 42;
  int parsecs = 12;
  char pageTitle[32] = "DIY Flow Bench";          // Display name for software
  char wifi_ssid[32] = "WIFI-SSID";               // Your Wifi SSID
  char wifi_pswd[32] = "PASSWORD";                // Your Wifi Password
  char wifi_ap_pswd[32] = "123456789";            // Default Access Point Password
  char hostname[32] = "diyfb";                    // Default Hostname
  char api_delim[2] = ":";                        // API Serial comms delimiter
  char wifi_ap_ssid[32] = "DIYFB";                // Default Access Point name
  char temp_unit[11] = "Celcius";                 // Defalt display unit of temperature
  bool ap_mode = false;                           // Default WiFi connection mode is accesspoint mode
  double valveLiftInterval = 1.5;                 // Distance between valve lift data points (can be metric or imperial)
  double cal_ref_press = 10;                      // Calibration orifice ref pressure
  double cal_flow_rate = 14.4;                    // Calibration orifica flow rate
  double orificeOneFlow = 0.0;
  double orificeOneDepression = 0.0;
  double orificeTwoFlow = 0.0;
  double orificeTwoDepression = 0.0;
  double orificeThreeFlow = 0.0;
  double orificeThreeDepression = 0.0;
  double orificeFourFlow = 0.0;
  double orificeFourDepression = 0.0;
  double orificeFiveFlow = 0.0;
  double orificeFiveDepression = 0.0;
  double orificeSixFlow = 0.0;
  double orificeSixDepression = 0.0;
};






/***********************************************************
 * Calibration Data
 ***/
struct CalibrationData { 
  double flow_offset = 0.0;         
  double user_offset = 0.0;         
  double leak_cal_baseline = 0.0;  
  double leak_cal_offset = 0.0;  
  double leak_cal_baseline_rev = 0.0;  
  double leak_cal_offset_rev = 0.0;  
};







/***********************************************************
 * Device status
 ***/
struct DeviceStatus {
  bool debug = true;
  int spiffs_mem_size = 0;
  int spiffs_mem_used = 0;
  int pageSize = 0;
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
  long browserUpdateTimer = 0;
  long wsCLeanPollTimer = 0;
  int pollTimer = 0;
  int serialData = 0;
  String statusMessage = BOOT_MESSAGE;
  bool apMode = false;
  double HWMBME = 0.0;
  double HWMADC = 0.0;
  double HWMSSE = 0.0;
  long mafDataTableRows = 0;
  long mafDataValMax = 0;
  long mafDataKeyMax = 0;
  int mafUnits = 0;
  double mafScaling = 1;
  int mafDiameter = 0;
  String activeOrifice = "1";
  double activeOrificeFlowRate;
  double activeOrificeTestPressure;
  bool shouldReboot = false;
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
  long MafRAW = 0;
  long MafLookup = 0;
  double MedianCFM = 0.0;
  double AverageCFM = 0.0;
  double FlowKGH = 0.0;
  double FlowCFMraw = 0.0;
  double FlowCFMunc = 0.0;
  double FlowCFM = 0.0;
  double FlowADJ = 0.0;
  double MafMv = 0.0;
  double TempDegC = 0.0;
  double TempDegF = 0.0;
  double RelH = 0.0;
  long BaroPA = 0;
  double BaroKPA = 0.0;
  double BaroHPA = 0.0;
  double PRefKPA = 0.0;
  double PRefMv = 0.0;
  double PRefH2O = 0.0;
  double PDiffKPA = 0.0;
  double PDiffH2O = 0.0;
  double PDiffMv = 0.0;
  double PitotKPA = 0.0;
  double PitotH2O = 0.0;
  double PitotMv = 0.0;
  double Swirl = 0.0;
  double FDiff = 0.0;
  int FDiffType = 1;
  char FDiffTypeDesc[32] = "BASELINE";
};



/***********************************************************
 * Valve Lift data
 ***/
struct ValveLiftData {
  double LiftData1 = 0.0;
  double LiftData2 = 0.0;
  double LiftData3 = 0.0;
  double LiftData4 = 0.0;
  double LiftData5 = 0.0;
  double LiftData6 = 0.0;
  double LiftData7 = 0.0;
  double LiftData8 = 0.0;
  double LiftData9 = 0.0;
  double LiftData10 = 0.0;
  double LiftData11 = 0.0;
  double LiftData12 = 0.0;
};

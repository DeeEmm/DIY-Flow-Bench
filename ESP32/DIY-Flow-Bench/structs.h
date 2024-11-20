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
#include "system.h"
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
  int standardReference = 1;                      // Standard reference conditions (default ISO 1585)
  int std_adj_flow = 0;                           // Standardised adjusted flow
  int dataGraphMax = 0;                           // Data graph maximum Y value
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
  bool pinsLoaded = false;
  bool mafLoaded = false;
  bool configLoaded = false;
  String pinsFilename;
  String mafFilename;
  bool doBootLoop = false;
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
  double FlowSCFM = 0.0;
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
  int32_t test = 0;
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




/***********************************************************
 * Pin Data
 ***/
struct Pins {
  int VAC_SPEED_PIN;
  int VAC_BLEED_VALVE_PIN;
  int VAC_BANK_1_PIN;
  int VAC_BANK_2_PIN; 
  int VAC_BANK_3_PIN;
  int AVO_STEP_PIN;
  int AVO_DIR_PIN;
  int FLOW_VALVE_STEP_PIN;
  int FLOW_VALVE_DIR_PIN;
  int VCC_3V3_PIN;
  int VCC_5V_PIN;
  int SPEED_SENSOR_PIN;
  int SWIRL_ENCODER_PIN_A;
  int SWIRL_ENCODER_PIN_B;
  int ORIFICE_BCD_BIT1_PIN;
  int ORIFICE_BCD_BIT2_PIN;
  int ORIFICE_BCD_BIT3_PIN;
  int MAF_PIN;
  int REF_PRESSURE_PIN;
  int DIFF_PRESSURE_PIN;
  int PITOT_PIN;
  int TEMPERATURE_PIN;
  int REF_BARO_PIN;
  int HUMIDITY_PIN;
  int SERIAL0_TX_PIN;
  int SERIAL0_RX_PIN;
  int SERIAL2_TX_PIN;
  int SERIAL2_RX_PIN;
  int SDA_PIN;
  int SCL_PIN;
  int SD_CS_PIN;
  int SD_MOSI_PIN;
  int SD_MISO_PIN;             
  int SD_SCK_PIN;
  int WEMOS_SPARE_PIN_1;
};



/***********************************************************
 * @brief Language Data
 * @note default language is english but can beoverwritten by language.json 
 ***/
struct Language {
    char LANG_BLANK[50] = " "; 
    char LANG_NULL[50] = "NULL";
    char LANG_NO_ERROR[50] = "Status OK";
    char LANG_SERVER_RUNNING[50] = "Server Running";
    char LANG_WARNING[50] = "Warning!";
    char LANG_FLOW_LIMIT_EXCEEDED[50] = "Warning! Flow Limit Error";
    char LANG_REF_PRESS_LOW[50] = "Warning! Low Reference Pressure";
    char LANG_LEAK_TEST_PASS[50] = "Leak test OK";
    char LANG_LEAK_TEST_FAILED[50] = "Leak test fail";
    char LANG_ERROR_LOADING_CONFIG[50] = "Error loading config file";
    char LANG_ERROR_SAVING_CONFIG[50] = "Error saving config file";
    char LANG_SAVING_CONFIG[50] = "Saving config file";
    char LANG_SAVING_CALIBRATION[50] = "Error saving calibration file";
    char LANG_ERROR_LOADING_FILE[50] = "Error loading file";
    char LANG_DHT11_READ_FAIL[50] = "DHT11 Read fail";
    char LANG_BME280_READ_FAIL[50] = "BME280 Read fail";
    char LANG_LOW_FLOW_CAL_VAL[50] = "Low Cal Value: ";
    char LANG_HIGH_FLOW_CAL_VAL[50] = "High Cal Value: ";
    char LANG_REF_PRESS_VALUE[50] = "Ref Press Val: ";
    char LANG_NOT_ENABLED[50] = "Not Enabled";
    char LANG_START_REF_PRESSURE[50] = "Using Startup Ref Pressure";
    char LANG_FIXED_VALUE[50] = "Fixed value: ";
    char LANG_CALIBRATING[50] = "Calibrating FLow Offset...";
    char LANG_LEAK_CALIBRATING[50] = "Calibrating Leak Test...";
    char LANG_CAL_OFFET_VALUE[50] = "Cal Value: ";
    char LANG_LEAK_CAL_VALUE[50] = "Leak Cal Value: ";
    char LANG_RUN_BENCH_TO_CALIBRATE[50] = "Bench must be running to calibrate";
    char LANG_BENCH_RUNNING[50] = "Bench running";
    char LANG_BENCH_STOPPED[50] = "Bench stopped";
    char LANG_DEBUG_MODE[50] = "Debug Mode";
    char LANG_DEV_MODE[50] = "Developer Mode";
    char LANG_SYSTEM_REBOOTING[50] = "System Rebooting";
    char LANG_CANNOT_DELETE_INDEX[50] = "Cannot delete index.html (overwrite it instead!)";
    char LANG_DELETE_FAILED[50] = "File Delete Failed";
    char LANG_INVALID_ORIFICE_SELECTED[50] = "Invalid Orifice selected";
    char LANG_ORIFICE_CHANGE[50] = "Orifice Plate Changed";
    char LANG_UPLOAD_FAILED_NO_SPACE[50] = "Upload rejected, not enough space";
    char LANG_FILE_UPLOADED[50] = "File uploaded";
    char LANG_NO_BOARD_LOADED[50] = "No board loaded";  
    char LANG_INDEX_HTML[2500] = "<!DOCTYPE HTML> <html lang='en'> <HEAD> <title>DIY Flow Bench</title> <meta name='viewport' content='width=device-width, initial-scale=1'> <script> function onFileUpload(event) { this.setState({ file: event.target.files[0] }); const { file } = this.state; const data = new FormData; data.append('data', file); fetch('/upload', { method: 'POST', body: data }).catch(e => { console.log('Request failed', e); }); } </script> <style> body, html { height: 100%; margin: 0; font-family: Arial; font-size: 22px } a:link { color: #0A1128; text-decoration: none } a:visited, a:active { color: #0A1128; text-decoration: none } a:hover { color: #666; text-decoration: none } .headerbar { overflow: hidden; background-color: #0A1128; text-align: center } .headerbar h1 a:link, .headerbar h1 a:active, .headerbar h1 a:visited, .headerbar h1 a:hover { color: white; text-decoration: none } .align-center { text-align: center } .file-upload-button { padding: 12px 0px; text-align: center } .button { display: inline-block; background-color: #008CBA; border: none; border-radius: 4px; color: white; padding: 12px 12px; text-decoration: none; font-size: 22px; margin: 2px; cursor: pointer; width: 150px } #footer { clear: both; text-align: center } .file-upload-button { padding: 12px 0px; text-align: center } .file-submit-button { padding: 12px 0px; text-align: center; font-size: 15px; padding: 6px 6px; } .input_container { border: 1px solid #e5e5e5; } input[type=file]::file-selector-button { background-color: #fff; color: #000; border: 0px; border-right: 1px solid #e5e5e5; padding: 10px 15px; margin-right: 20px; transition: .5s; } input[type=file]::file-selector-button:hover { background-color: #eee; border: 0px; border-right: 1px solid #e5e5e5; } </style> </HEAD> <BODY> <div class='headerbar'> <h1><a href='/'>DIY Flow Bench</a></h1> </div> <br> <div class='align-center'> <p>Welcome to the DIY Flow Bench. Thank you for supporting our project.</p> <p>Please upload the following files to get started.</p> <p>~INDEX_STATUS~</p> <p>~PINS_STATUS~</p> <!--<p>~SETTINGS_STATUS~</p>--> <br> <form method=\"POST\" action='/upload' enctype=\"multipart/form-data\"> <div class=\"input_container\"> <input type=\"file\" name=\"file\" id=\"fileUpload\"> <input type='submit' value='Upload' class=\"button file-submit-button\"> </div> </form> </div> <br> <div id='footer'><a href='https://diyflowbench.com' target='new'>DIYFlowBench.com</a></div> <br> </BODY> </HTML>";
};


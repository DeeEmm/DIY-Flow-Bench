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
#include <vector>

#include "system.h"
#include "constants.h"
#include <ArduinoJson.h>

/***********************************************************
 * Default / Fallback Configuration Settings
 *
 * NOTE: Do not edit these! Please leave the default values.
 * These are used to generate the default config settings in the event of a system crash.
 * If you edit these, you may get locked out of your system or brick your device.
 * You can easily edit the config settings via the browser after you connect!
 ***/
struct BenchSettings {
  int api_response_length = 64;                   // API Serial comms message length
  long serial_baud_rate = 115200;                 // Default baud rate 
  unsigned long wifi_timeout = 4000;              // Duration of Wifi connection attempt in millisec's
  unsigned long wifi_retries = 10;                // Number of attempts to connect to Wifi before creating AP
  double min_flow_rate = 1;                       // Flow rate at which bench is considered running in cfm
  double min_bench_pressure = 1;                  // Min bench pressure where bench is considered running (inches/h2o)
  double maf_min_volts = 0.1;                     // Filter out results less than this
  int refresh_rate = 500;                         // Screen refresh rate in milliseconds (>180)
  int adj_flow_depression = 28;                   // Adjusted flow depression in inches of water
  int rounding_type = NONE;                       // Rounding type 
  int flow_decimal_length = 1;                    // Flow decimal number of places 
  int gen_decimal_length = 2;                     // General decimal number of places 
  int data_filter_type = NONE;                    // Data filter type 
  int data_capture_datatype = 1;                  // Datacapture datatype
  int standardReference = 1;                      // Standard reference conditions (default ISO 1585)
  int std_adj_flow = 0;                           // Standardised adjusted flow
  int dataGraphMax = 0;                           // Data graph maximum Y value
  int cyc_av_buffer = 5;                          // [5] Scan # over which to average output (helps smooth results)
  bool show_alarms = true;                        // Display Alarms?
  bool debug_mode = false;                        // Global debug print override
  bool dev_mode = false;                          // Developer mode
  bool function_mode = false;                     // Function mode
  bool status_print_mode = false;                 // Stream status data to serial
  bool verbose_print_mode = false;                // Stream verbose data to serial
  bool api_enabled = true;                        // Can disable serial API if required
  char AB_test = 'A';                             // A / B / C testing
  int bench_type = MAF_BENCH;                     // Default bench type
  int maf_housing_diameter = 0;                   // MAF Housing diameter
  int tatltuae = 42;
  int parsecs = 12;
  String pageTitle = "DIY Flow Bench";            // Display name for software
  String wifi_ssid = "WIFI-SSID";                 // Your Wifi SSID
  String wifi_pswd = "PASSWORD";                  // Your Wifi Password
  String wifi_ap_pswd = "123456789";              // Default Access Point Password
  String hostname = "diyfb";                      // Default Hostname
  String api_delim = ":";                         // API Serial comms delimiter
  String wifi_ap_ssid = "DIYFB";                  // Default Access Point name
  int temp_unit = CELCIUS;                        // Defalt display unit of temperature
  bool ap_mode = false;                           // Default WiFi connection mode is accesspoint mode
  double valveLiftInterval = 1.5;                 // Distance between valve lift data points (can be metric or imperial)
};







/***********************************************************
 * Configuration Data
 ***/
struct Configuration {
  bool bSD_ENABLED = false;
  int iMIN_PRESS_PCT = 80;
  double dPIPE_RAD_FT = 0.328084;

  double dVCC_3V3_TRIM = 0.0f;
  double dVCC_3V3_SRC = FIXED_VAL;
  bool bFIXED_3_3V = true;

  double dVCC_5V_TRIM = 0.0f;
  double dVCC_5V_SRC = FIXED_VAL;
  bool bFIXED_5V = true;

  int iBME_TYP = BOSCH_BME280;
  int iBME_ADDR = 118;

  int iADC_TYPE = ADS1115;
  int iADC_I2C_ADDR = 72; 
  int iADC_MAX_RETRY = 10;

  int iMAF_SRC_TYP = ADS_ADC;
  int iMAF_SENS_TYP = 0;
  double dMAF_MV_TRIM = 0.0f;
  int iMAF_ADC_CHAN = 0;

  int iPREF_SENS_TYP = MPXV7007;
  int iPREF_SRC_TYP = ADS_ADC;
  int iFIXED_PREF_VAL = 1;
  double dPREF_MV_TRIM =  0.0f;
  double dPREF_ALG_SCALE =  1.0f;
  int iPREF_ADC_CHAN = 1;

  int iPDIFF_SENS_TYP = MPXV7007; 
  int iPDIFF_SRC_TYP = ADS_ADC;
  int iFIXD_PDIFF_VAL = 1;
  double dPDIFF_MV_TRIM = 0.0f;
  double dPDIFF_SCALE = 1.0f;
  int iPDIFF_ADC_CHAN = 2;
  
  int iPITOT_SENS_TYP = MPXV7007;
  int iPITOT_SRC_TYP = ADS_ADC;
  double dPITOT_MV_TRIM = 0.0f;
  double dPITOT_SCALE = 1.0f;
  int iPITOT_ADC_CHAN = 3;

  int iBARO_SENS_TYP = BOSCH_BME280; //7
  double dFIXD_BARO_VAL = 101.3529f;
  double dBARO_ALG_SCALE = 1.0f;  
  double dBARO_SCALE = 1.0f;
  double dBARO_OFFSET = 0.0f;
  double dBARO_MV_TRIM = 0.0f;
  double dBARO_FINE_TUNE = 0.0f;
  double dSEALEVEL_PRESS = 1016.90f;
  int iBARO_ADC_CHAN = 4;

  int iTEMP_SENS_TYP = BOSCH_BME280; //7
  double dFIXED_TEMP_VAL = 21.0f;
  double dTEMP_ALG_SCALE = 1.0f;
  double dTEMP_MV_TRIM = 0.0f;
  double dTEMP_FINE_TUNE = 0.0f;

  int iRELH_SENS_TYP = BOSCH_BME280; //7
  double dFIXED_RELH_VAL = 36.0f;
  double dRELH_ALG_SCALE = 1.0f;
  double dRELH_MV_TRIM = 0.0f;
  double dRELH_FINE_TUNE = 0.0f;
  bool bSWIRL_ENBLD = false;

  float mafCoeff0 = 0.0f;
  float mafCoeff1 = 0.0f;
	float mafCoeff2 = 0.0f;
	float mafCoeff3 = 0.0f;
	float mafCoeff4 = 0.0f;
	float mafCoeff5 = 0.0f;
	float mafCoeff6 = 0.0f;
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
  double pdiff_cal_offset = 0.0;
  double pitot_cal_offset = 0.0;

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
  long ssePollTimer = 0;
  long wsCLeanPollTimer = 0;
  int pollTimer = 0;
  int serialData = 0;
  String statusMessage = BOOT_MESSAGE;
  bool apMode = false;
  double HWMBME = 0.0;
  double HWMADC = 0.0;
  double HWMSSE = 0.0;
  String activeOrifice = "1";
  double activeOrificeFlowRate;
  double activeOrificeTestPressure;
  bool shouldReboot = false;
  bool pinsLoaded = false;
  bool mafLoaded = false;
  bool configLoaded = false;
  bool GUIexists = false;
  String pinsFilename;
  String mafFilename;
  String indexFilename;
  bool doBootLoop = false;
  int ioError = -1;
  bool webserverIsRunning = false;
  int mafDataTableRows = 0;
  u_int mafDataValMax = 0;
  u_int mafDataKeyMax = 0;
  char mafUnits[5];
  double mafScaling = 1;
  int mafDiameter = 0;
  String mafSensorType;
  char mafOutputType[10];
  String mafLink;
  int GUIpage = 0;
  size_t nvmPins = 0;
  size_t nvmConfig = 0;
  size_t nvmSettings = 0;
  int loopScanTime = 0;
  int bmeScanTime = 0;
  int adcScanTime = 0;
  int bmeScanCountAverage = 1;
  int adcScanCountAverage = 1;
  int bmeScanCount = 1;
  int adcScanCount = 1;
  double bmeScanAlpha = 0.45;
  double adcScanAlpha = 0.45;
  String mafTypeDesc0 = "Spare";
  String mafTypeDesc1 = "Spare";
  String mafTypeDesc2 = "Spare";
  String mafTypeDesc3 = "Spare";
  String mafTypeDesc4 = "Spare";
  String mafTypeDesc5 = "Spare";
  String mafTypeDesc6 = "Spare";
  String mafTypeDesc7 = "Spare";
  String mafTypeDesc8 = "Spare";
  String mafTypeDesc9 = "Spare";
  String mafTypeDesc10 = "Spare";
  int mafStatus;
};





/***********************************************************
 * Sensor data
 ***/
struct SensorData {
  double VCC_3V3_BUS = 3.3f;
  double VCC_5V_BUS = 5.0f;
  long MafRAW = 0;
  long MafLookup = 0;
  double MedianCFM = 0.0;
  double ModeCFM = 0.0;
  double MeanCFM = 0.0;
  double AverageCFM = 0.0;
  double FlowKGH = 0.0;
  double FlowCFMraw = 0.0;
  double FlowCFMunc = 0.0;
  double FlowCFM = 0.0;
  double FlowSCFM = 0.0;
  double FlowADJ = 0.0;
  double FlowADJSCFM = 0.0;
  double MafVolts = 0.0;
  double TempDegC = 0.0;
  double TempDegF = 0.0;
  double RelH = 0.0;
  double BaroPA = 0;
  double BaroKPA = 0.0;
  double BaroHPA = 0.0;
  double PRefKPA = 0.0;
  double PRefVolts = 0.0;
  double PRefH2O = 0.0;
  double PDiffKPA = 0.0;
  double PDiffH2O = 0.0;
  double PDiffVolts = 0.0;
  double PitotKPA = 0.0;
  double PitotH2O = 0.0;
  double PitotVolts = 0.0;
  double PitotDelta = 0.0;
  double PitotVelocity = 0.0;
  double Swirl = 0.0;
  double FDiff = 0.0;
  int FDiffType = 1;
  char FDiffTypeDesc[32] = "BASELINE";
  int flowtile = 1;
  int32_t test = 0;
};




/***********************************************************
 * Valve Lift data
 ***/
struct ValveLiftData {
  double_t LiftData1 = 0.0;
  double_t LiftData2 = 0.0;
  double_t LiftData3 = 0.0;
  double_t LiftData4 = 0.0;
  double_t LiftData5 = 0.0;
  double_t LiftData6 = 0.0;
  double_t LiftData7 = 0.0;
  double_t LiftData8 = 0.0;
  double_t LiftData9 = 0.0;
  double_t LiftData10 = 0.0;
  double_t LiftData11 = 0.0;
  double_t LiftData12 = 0.0;
};




/***********************************************************
 * Pin Data
 * @note DEfault pin status is disabled to prevent issues on initial firmware flashing
 ***/
struct Pins {
  // Inputs
  int VCC_5V = -1;
  int VCC_3V3 = -1;
  int SPEED_SENS = -1;
  int ORIFICE_BCD_1 = -1;
  int ORIFICE_BCD_2 = -1;
  int ORIFICE_BCD_3 = -1;
  int MAF = -1;
  int PREF = -1;
  int PDIFF = -1;
  int PITOT = -1;
  int TEMPERATURE = -1;
  int HUMIDITY = -1;
  int REF_BARO = -1;
  int SWIRL_ENCODER_A = -1;
  int SWIRL_ENCODER_B = -1;
  int SERIAL0_RX = -1;
  int SERIAL2_RX = -1;
  int SDA = -1;
  int SCL = -1;
  int SD_CS = -1;
  int SD_MISO = -1;             
  int SD_SCK = -1;
  int SPARE_PIN_1 = -1;
  int SPARE_PIN_2 = -1;
  // Outputs
  int VAC_SPEED = -1;
  int VAC_BANK_1 = -1;
  int VAC_BANK_2 = -1; 
  int VAC_BANK_3 = -1;
  int VAC_BLEED_VALVE = -1;
  int AVO_STEP = -1;
  int AVO_DIR = -1;
  int FLOW_VALVE_STEP = -1;
  int FLOW_VALVE_DIR = -1;
  int SD_MOSI = -1;
  int SERIAL0_TX = -1;
  int SERIAL2_TX = -1;
};

// DEPRECATED Original pin assignemnt for wemos / official Shield
// struct Pins {
//   // Inputs
//   int VCC_5V = -1;
//   int VCC_3V3 = -1;
//   int SPEED_SENS = 15;
//   int ORIFICE_BCD_1 = 34;
//   int ORIFICE_BCD_2 = 36;
//   int ORIFICE_BCD_3 = 39;
//   int MAF = -1;
//   int PREF = -1;
//   int PDIFF = -1;
//   int PITOT = -1;
//   int TEMPERATURE = -1;
//   int HUMIDITY = -1;
//   int REF_BARO = -1;
//   int SWIRL_ENCODER_A = -1;
//   int SWIRL_ENCODER_B = -1;
//   int SERIAL0_RX = 3;
//   int SERIAL2_RX = 17;
//   int SDA = 21;
//   int SCL = 22;
//   int SD_CS = 5;
//   int SD_MISO = 19;             
//   int SD_SCK = 18;
//   int SPARE_PIN_1 = -1;
//   int SPARE_PIN_2 = -1;
//   // Outputs
//   int VAC_SPEED = 25;
//   int VAC_BANK_1 = 13;
//   int VAC_BANK_2 = 12; 
//   int VAC_BANK_3 = 14;
//   int VAC_BLEED_VALVE = 26;
//   int AVO_STEP = 32;
//   int AVO_DIR = 33;
//   int FLOW_VALVE_STEP = 27;
//   int FLOW_VALVE_DIR = 4;
//   int SD_MOSI = 23;
//   int SERIAL0_TX = 1;
//   int SERIAL2_TX = 16;
// };



/***********************************************************
 * @brief Language Data
 * @note default language is English but can be overwritten by uploading a language.json file to GUI
 * @note LANG_INDEX_HTML - HTML is intended to be rewritten. This is the On-boarding page to upload missing files. Full source can be found in src/preload.html
 ***/
struct Language {
    char LANG_INDEX_HTML[2500] = "<!DOCTYPE HTML> <html lang='en'> <HEAD> <title>DIY Flow Bench</title> <meta name='viewport' content='width=device-width, initial-scale=1'> <script> function onFileUpload(event) { this.setState({ file: event.target.files[0] }); const { file } = this.state; const data = new FormData; data.append('data', file); fetch('/api/file/upload', { method: 'POST', body: data }).catch(e => { console.log('Request failed', e); }); } </script> <style> body, html { height: 100%; margin: 0; font-family: Arial; font-size: 22px } a:link { color: #0A1128; text-decoration: none } a:visited, a:active { color: #0A1128; text-decoration: none } a:hover { color: #666; text-decoration: none } .headerbar { overflow: hidden; background-color: #0A1128; text-align: center } .headerbar h1 a:link, .headerbar h1 a:active, .headerbar h1 a:visited, .headerbar h1 a:hover { color: white; text-decoration: none } .align-center { text-align: center } .file-upload-button { padding: 12px 0px; text-align: center } .button { display: inline-block; background-color: #008CBA; border: none; border-radius: 4px; color: white; padding: 12px 12px; text-decoration: none; font-size: 22px; margin: 2px; cursor: pointer; width: 150px } #footer { clear: both; text-align: center } .file-upload-button { padding: 12px 0px; text-align: center } .file-submit-button { padding: 12px 0px; text-align: center; font-size: 15px; padding: 6px 6px; } .input_container { border: 1px solid #e5e5e5; } input[type=file]::file-selector-button { background-color: #fff; color: #000; border: 0px; border-right: 1px solid #e5e5e5; padding: 10px 15px; margin-right: 20px; transition: .5s; } input[type=file]::file-selector-button:hover { background-color: #eee; border: 0px; border-right: 1px solid #e5e5e5; } </style> </HEAD> <BODY> <div class='headerbar'> <h1><a href='/'>DIY Flow Bench</a></h1> </div> <br> <div class='align-center'> <p>Welcome to the DIY Flow Bench. Thank you for supporting our project.</p> <p>Please upload the following files to get started.</p> <p>~INDEX_STATUS~</p> <p>~CONFIGURATION_STATUS~</p> <p>~PINS_STATUS~</p> <p>~MAF_STATUS~</p> <br> <form method=\"POST\" action='/api/file/upload' enctype=\"multipart/form-data\"> <div class=\"input_container\"> <input type=\"file\" name=\"file\" id=\"fileUpload\"> <input type='submit' value='Upload' class=\"button file-submit-button\"> </div> </form> </div> <br> <div id='footer'><a href='https://diyflowbench.com' target='new'>DIYFlowBench.com</a></div> <br> </BODY> </HTML>";
    char LANG_GUI_SELECT_LIFT_VAL_BEFORE_CAPTURE[50] = "Select lift value before capture";
    char LANG_GUI_LIFT_VAL[50] = "Lift Value";
    char LANG_GUI_LIFT_CAPTURE[50] = "Capture";
    char LANG_GUI_UPLOAD_FIRMWARE_BINARY[50] = "Upload Firmware Binary (firmware_update.bin)";
    char LANG_GUI_FIRMWARE_UPDATE[50] = "Update";
    char LANG_GUI_USER_FLOW_TARGET_VAL[50] = "User Flow Target Value";
    char LANG_GUI_SAVE[50] = "Save";
    char LANG_GUI_CALIBRATE[50] = "Calibrate";
    char LANG_GUI_CAL_FLOW_OFFSET[50] = "Calibrate Flow Offset";
    char LANG_GUI_CAL_LEAK_TEST[50] = "Calibrate Leak Test";
    char LANG_GUI_LOAD_LIFT_PROFILE[50] = "Load Lift Profile";
    char LANG_GUI_LOAD_LIFT_PROFILE_LOAD[50] = "Load";
    char LANG_GUI_SAVE_LIFT_DATA[50] = "Save Lift Data";
    char LANG_GUI_SAVE_GRAPH_FILENAME[50] = "Filename";
    char LANG_GUI_FILE_MANAGER[50] = "File Manager";
    char LANG_GUI_FILEMANAGER_UPLOAD[50] = "Upload";
    char LANG_GUI_FIRMWARE[50] = "Firmware";
    char LANG_GUI_VERSION[50] = "Version";
    char LANG_GUI_BUILD[50] = "Build";
    char LANG_GUI_GUI[50] = "GUI";
    char LANG_GUI_MEM_SIZE[50] = "Memory Size";
    char LANG_GUI_MEM_USED[50] = "Memory Used";
    char LANG_GUI_STORAGE[50] = "Storage";
    char LANG_GUI_NETWORK[50] = "Network";
    char LANG_GUI_IP_ADDRESS[50] = "IP Address";
    char LANG_GUI_BENCH_TYPE[50] = "Bench Type";
    char LANG_GUI_BOARD_TYPE[50] = "Board Type";
    char LANG_GUI_AUTO[50] = "Auto";
    char LANG_GUI_SENS_CONFIG[50] = "Sensor Configuration";
    char LANG_GUI_MAF_DATA_FILE[50] = "MAF Data File";
    char LANG_GUI_REF_PRESSURE_SENSOR[50] = "Reference Pressure Sensor";
    char LANG_GUI_TEMP_SENSOR[50] = "Temperature Sensor";
    char LANG_GUI_HUMIDITY_SENSOR[50] = "Humidity Sensor";
    char LANG_GUI_BARO_SENSOR[50] = "Baro Sensor";
    char LANG_GUI_PITOT_SENSOR[50] = "Pitot Sensor";
    char LANG_GUI_DIFFERENTIAL_SENSOR[50] = "Differential Sensor";    
    char LANG_BLANK[50] = " "; 
    char LANG_NULL[50] = "NULL";
    char LANG_NO_ERROR[50] = "Status OK";
    char LANG_SERVER_RUNNING[50] = "Server Running";
    char LANG_WARNING[50] = "Warning!";
    char LANG_FLOW_LIMIT_EXCEEDED[50] = "Warning! Flow Limit Error";
    char LANG_REF_PRESS_LOW[50] = "Warning! Low Reference Pressure";
    char LANG_LEAK_TEST_PASS[50] = "Leak test OK";
    char LANG_LEAK_TEST_FAILED[50] = "Leak test fail";
    char LANG_ERROR_LOADING_CONFIG[50] = "Error loading config data";
    char LANG_ERROR_SAVING_CONFIG[50] = "Error saving config data";
    char LANG_SAVING_CONFIG[50] = "Saving config data";
    char LANG_SAVING_CALIBRATION[50] = "Saving calibration data";
    char LANG_ERROR_LOADING_FILE[50] = "Error loading file";
    char LANG_DHT11_READ_FAIL[50] = "DHT11 Read fail";
    char LANG_BME280_READ_FAIL[50] = "BME280 Read fail";
    char LANG_LOW_FLOW_CAL_VAL[50] = "Low Cal Value: ";
    char LANG_HIGH_FLOW_CAL_VAL[50] = "High Cal Value: ";
    char LANG_REF_PRESS_VAL[50] = "Ref Press Val: ";
    char LANG_NOT_ENABLED[50] = "Not Enabled";
    char LANG_START_REF_PRESSURE[50] = "Using Startup Ref Pressure";
    char LANG_FIXED_VAL[50] = "Fixed value: ";
    char LANG_CALIBRATING[50] = "Calibrating Flow Offset...";
    char LANG_LEAK_CALIBRATING[50] = "Calibrating Leak Test...";
    char LANG_CAL_OFFSET_VAL[50] = "Cal Value: ";
    char LANG_LEAK_CAL_VAL[50] = "Leak Cal Value: ";
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
    char LANG_GUI_PITOT[50] = "Pitot";  
    char LANG_GUI_PREF[50] = "Depression";  
    char LANG_GUI_PDIFF[50] = "Differential";
    char LANG_GUI_FLOW[50] = "Flow Rate";
    char LANG_GUI_AFLOW[50] = "Adjusted Flow";
    char LANG_GUI_SFLOW[50] = "Standard Flow";
    char LANG_GUI_MFLOW[50] = "MAF Flow";
    char LANG_GUI_SWIRL[50] = "Swirl";
    char LANG_GUI_FLOW_DIFF[50] = "Flow Diff";
    char LANG_GUI_TEMP[50] = "Temperature";
    char LANG_GUI_BARO[50] = "Baro Pressure";
    char LANG_GUI_HUMIDITY[50] = "Humidity";
    char LANG_GUI_START[50] = "Start Bench";
    char LANG_GUI_STOP[50] = "Stop Bench";
    char LANG_GUI_CLEAR_ALARM[1302] = "Clear Alarm";
    char LANG_GUI_CAPTURE[50] = "Capture";
    char LANG_GUI_DASHBOARD[50] = "Dashboard";
    char LANG_GUI_DATAGRAPH[50] = "Graph";
    char LANG_GUI_HARDWARE_CONFIG[50] = "Hardware Configuration";
    char LANG_GUI_HARDWARE[50] = "Hardware";
    char LANG_GUI_SYSTEM[50] = "System";
    char LANG_GUI_PINS[50] = "Pins";
    char LANG_GUI_CLEAR[50] = "Clear";
    char LANG_GUI_EXPORT[50] = "Export";
    char LANG_GUI_IMAGE[50] = "Image";
    char LANG_GUI_SYS_SETTINGS[50] = "System Settings";
    char LANG_GUI_WIFI_INFO[50] = "Wifi Info";
    char LANG_GUI_WIFI_SSID[50] = "WiFi SSID";
    char LANG_GUI_WIFI_PASS[50] = "WiFi Password";
    char LANG_GUI_WIFI_AP_SSID[50] = "WiFi AP SSID";
    char LANG_GUI_WIFI_AP_PASS[50] = "WiFi AP Password";
    char LANG_GUI_HOSTNAME[50] = "WiFi Hostname";
    char LANG_GUI_WIFI_TIMEOUT[50] = "WiFi Timeout";
    char LANG_GUI_GENERAL_SETTINGS[50] = "Bench Settings";
    char LANG_GUI_MAF_DIAMETER[50] = "MAF Diameter (mm)";
    char LANG_GUI_REFRESH_RATE[50] = "GUI Refresh Rate (ms)";
    char LANG_GUI_TEMPERATURE_UNIT[50] = "Temp Unit (&degC / &degF)";
    char LANG_GUI_LIFT_INTERVAL[50] = "Lift Interval (mm / inch)";
    char LANG_GUI_DATA_GRAPH_MAX_VAL[50] = "Max Flow Value";
    char LANG_GUI_RESOLUTION_AND_ACCURACY[50] = "Resolution and Accuracy";
    char LANG_GUI_FLOW_VAL_ROUNDING[50] = "Flow Value Rounding";
    char LANG_GUI_FLOW_DECIMAL_ACCURACY[50] = "Flow Accuracy";
    char LANG_GUI_GEN_DECIMAL_ACCURACY[50] = "General Accuracy";
    char LANG_GUI_DATA_FILTERS[50] = "Data Filters";
    char LANG_GUI_DATA_FLTR_TYP[50] = "Data Filter Type";
    char LANG_GUI_MIN_FLOW_RATE[50] = "Min Flow Rate (cfm)";
    char LANG_GUI_MIN_PRESSURE[50] = "Min Pressure (in/H2O)";
    char LANG_GUI_MAF_MIN_VOLTS[50] = "MAF Min volts";
    char LANG_GUI_CYCLIC_AVERAGE_BUFFER[50] = "Cyclical Average Buffer";
    char LANG_GUI_CONVERSION_SETTINGS[50] = "Conversion Settings";
    char LANG_GUI_ADJ_FLOW_DEP[50] = "Adj Flow pRef (in/H2O)";
    char LANG_GUI_STANDARD_REF_CONDITIONS[50] = "Ref Standard (SCFM)";
    char LANG_GUI_STANDARDISED_ADJ_FLOW[50] = "Std Adjusted Flow";
    char LANG_GUI_CAL_ORIFICE_SETTINGS[50] = "Calibration Orifice Settings";
    char LANG_GUI_CAL_ORIFICE_FLOW_RATE[50] = "Cal Orifice Flow Rate (cfm)";
    char LANG_GUI_CAL_ORIFICE_TEST_PRESS[50] = "Cal Orifice pRef (in/H2O)";
    char LANG_GUI_ORIFICE_DATA[50] = "Orifice Data";
    char LANG_GUI_ORIFICE1_FLOW[50] = "Orifice #1 Flow Rate (cfm)";
    char LANG_GUI_ORIFICE1_PRESSURE[50] = "Orifice #1 pRef (in/H2O)";
    char LANG_GUI_ORIFICE2_FLOW[50] = "Orifice #2 Flow Rate (cfm)";
    char LANG_GUI_ORIFICE2_PRESSURE[50] = "Orifice #2 pRef (in/H2O)";
    char LANG_GUI_ORIFICE3_FLOW[50] = "Orifice #3 Flow Rate (cfm)";
    char LANG_GUI_ORIFICE3_PRESSURE[50] = "Orifice #3 pRef (in/H2O)";
    char LANG_GUI_ORIFICE4_FLOW[50] = "Orifice #4 Flow Rate (cfm)";
    char LANG_GUI_ORIFICE4_PRESSURE[50] = "Orifice #4 pRef (in/H2O)";
    char LANG_GUI_ORIFICE5_FLOW[50] = "Orifice #5 Flow Rate (cfm)";
    char LANG_GUI_ORIFICE5_PRESSURE[50] = "Orifice #5 pRef (in/H2O)";
    char LANG_GUI_ORIFICE6_FLOW[50] = "Orifice #6 Flow Rate (cfm)";
    char LANG_GUI_ORIFICE6_PRESSURE[50] = "Orifice #6 pRef (in/H2O)";
    char LANG_GUI_API_SETTINGS[50] = "API Settings";
    char LANG_GUI_API_DELIMITER[50] = "API Delimiter";
    char LANG_GUI_SERIAL_BAUD[50] = "Serial Baud Rate";
    char LANG_GUI_CALIBRATION_DATA[50] = "Calibration Data";
    char LANG_GUI_CAL_OFFSET[50] = "Calibration Offset (cfm)";
    char LANG_GUI_LEAK_TEST_BASELINE[50] = "Leak Test Baseline (cfm)";
    char LANG_GUI_LEAK_TEST_OFFSET[50] = "Leak Test Offset (cfm)";
    char LANG_GUI_LEAK_TEST_BASELINE_REV[50] = "Leak Test Baseline Rev (cfm)";
    char LANG_GUI_LEAK_TEST_OFFSET_REV[50] = "Leak Test Offset Rev (cfm)";
    char LANG_GUI_OVERWRITE[50] = "Overwrite";  
    char LANG_GUI_DATA_CAPTURE_SETTINGS[50] = "DataGraph Settings";
    char LANG_GUI_CAPTURE_DATATYPE[50] = "Datatype";
    char LANG_GUI_MAF_VOLTS[50] = "MAF Volts";
    char LANG_GUI_PREF_VOLTS[50] = "pRef Volts";
    char LANG_GUI_PDIFF_VOLTS[50] = "pDiff Volts";
    char LANG_GUI_PITOT_VOLTS[50] = "Pitot Volts";
    char LANG_GUI_MAF_TYPE[50] = "MAF Type";
    char LANG_GUI_MIMIC[50] = "Mimic";
    char LANG_GUI_CALIBRATION[50] = "Calibration";
};


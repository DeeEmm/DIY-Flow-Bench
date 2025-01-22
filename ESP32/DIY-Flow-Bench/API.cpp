/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file api.cpp
 * 
 * @brief API Class - Serial API handler
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/

#include "constants.h"
#include "structs.h"

#include "API.h"
#include <esp32/rom/crc.h> 
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

#include "hardware.h"
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
#include "calibration.h"
#include "webserver.h"
#include "datahandler.h"
#include "comms.h"
#include <Preferences.h>

extern struct BenchSettings settings;


API::API() {
  
}


/***********************************************************
 * @name CREATE CRC32 CHECKSUM
 * @brief Compute checksum using ESP32 ROM CRC library
 * @returns CRC32 hash derived from *str
 * @implements <esp32/rom/crc.h>
 ***/
uint32_t API::calcCRC (const char* str) {

  CRC = (crc32_le(0, (const uint8_t*)str, 4));

  return CRC;
}


/***********************************************************
 * GET CONFIG SETTINGS IN JSON FORMAT
 ***/
String API::getConfigJSON() { 
  
  extern struct BenchSettings settings;
  String jsonString;
  StaticJsonDocument<1024> dataJson;    
  
  // Can add more settings as required
  dataJson["iMIN_FLOW_RATE"] = settings.min_flow_rate;
  dataJson["iMIN_PRESSURE"] = settings.min_bench_pressure;
  dataJson["iREFRESH_RATE"] = settings.refresh_rate;

  
  serializeJson(dataJson, jsonString);  
  return jsonString;
}






/***********************************************************
 * PARSE API
 *
 * Handles Serial API responses
 * See apiHelpText below for breakdown of API data mnemonics
 * 
 * Response anatomy:
 * API Response format 'V:1.1.20080705:48853'
 * Response Code:  'V'
 * Delimiter:      ':' 
 * Response:       '2.1.20080705'
 * Delimiter:      ':' 
 * CRC Checksum:   '48853'  
 *
 * Note: checksum is optional  
 ***/
void API::ParseMessage(char apiMessage) {

  extern struct BenchSettings settings;
  extern struct CalibrationData calVal;
  extern struct SensorData sensorVal;
  extern struct DeviceStatus status;
  extern struct Configuration config;

  Calculations _calculations;
  Sensors _sensors;
  Messages _message;    
  Hardware _hardware;
  Webserver _webserver;
  Calibration _calibration;
  DataHandler _data;
  Comms _comms;
  Preferences _prefs;
  
  extern TaskHandle_t sensorDataTask;
  extern TaskHandle_t enviroDataTask;
  
  // define char arrays for response strings
  char apiResponse[API_RESPONSE_LENGTH];  //64
  char apiResponseBlob[API_BLOB_LENGTH];  //1024
  // char charDataJSON[API_JSON_LENGTH];     //1020
  char fileListBlob[1024];

  // Initialise arrays
  apiResponse[0] = 0;
  apiResponseBlob[0] = 0;
  // charDataJSON[0] = 0;
  fileListBlob[0] = 0;
  

  String jsonString;
  long refValue;
  
  //API Response
  char apiHelpText[] = R"(
  DIYFB API Commands
  ==============================
  API Response Format
  Command : Value : Checksum
  ==============================
  3 : 3.3V Voltage Value
  5 : 5V Voltage Value
  A : ADC Voltage Values Maf:pRef:pDiff:Pitot
  a : ADC Raw Values Maf:pRef:pDiff:Pitot
  B : Barometric Pressure
  C : configuration.json
  D : Differential Pressure value inH2O
  E : Enum1 Flow:Ref:Temp:Humidity:Baro
  e : Enum2 Pitot:Swirl
  F : Flow Value in CFM
  f : Flow Value in KG/H
  H : Humidity Value (%)
  I : IP Address
  J : JSON Status Data
  j : JSON Configuration Data
  K : MAF Data Key Value
  k : MAF Data Lookup Value
  l : Lift Data
  M : MAF.json
  N : Hostname
  o : Active Orifice
  P : Pitot Value inH2O
  Q : MAF Data Max Value
  q : MAF Data Key Max Value
  R : Reference Pressure Value inH2O
  S : Status
  T : Temperature in Celcius
  t : Temperature in Fahrenheit
  U : Uptime in hhhh.mm
  V : Version
  v : GUI Version
  W : WiFi SSID
  X : xTask memory usage   
  x : Heap memory usage   
  ? : Help
  < : Last Error
  / : SPIFFS File List
  ~ : Restart ESP
  $ : Reset WiFi
  @ : Stream Status
  ! : Debug Mode
  + : Verbose Mode
  = : Status Mode
  ^ : FUNCTION MODE
  ============================== 
  )";

  //API Response for boot loop
  char apiFunctionHelpText[] = R"(
  DIYFB Function Commands
  ==============================
  & : Reset Pins
  % : Reset WiFi AP SSID & Password
  ============================== 
  )";


  //API Response for boot loop
  char bootHelpText[] = R"(
  DIYFB BOOT LOOP Commands
  ==============================
  API Response Format
  Command : Value : Checksum
  ==============================
  C : configuration.json
  I : IP Address
  J : JSON Status Data
  j : JSON Configuration Data
  M : MAF.json
  N : Hostname
  S : Status
  U : Uptime in hhhh.mm
  V : Version
  W : WiFi SSID
  X : xTask memory usage   
  x : Heap memory usage   
  ? : Help
  / : SPIFFS File List
  ~ : Restart ESP
  $ : Reset WiFi
  % : Reset WiFi AP SSID & Password
  & : Reset Pins
  @ : Stream Status
  ! : Debug Mode
  + : Verbose Mode
  = : Status Mode
  < : Last Error
  ============================== 
  )";





  switch (apiMessage)   {

      // NOTE: Responses are alphabetised

      case '0': // Bench Off
          if (status.doBootLoop) break;
          _hardware.benchOff();
      break;
      
      case '1': // Bench On
          if (status.doBootLoop) break;
          _hardware.benchOn();
      break;
      
      case '3': // Get 3v board supply voltage (mv) 'v.123.45\r\n'
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "3%s%f", settings.api_delim , sensorVal.VCC_5V_BUS);
      break;
      
      case '5': // Get 5v board supply voltage (mv) 'v.123.45\r\n'
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "5%s%f", settings.api_delim , sensorVal.VCC_5V_BUS);
      break;

      case 'A': // Report ADC voltage values
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "A%s%f%s%f%s%f%s%f", 
          settings.api_delim, _hardware.getADCVolts(config.iMAF_ADC_CHAN), 
          settings.api_delim, _hardware.getADCVolts(config.iPREF_ADC_CHAN), 
          settings.api_delim, _hardware.getADCVolts(config.iPDIFF_ADC_CHAN), 
          settings.api_delim, _hardware.getADCVolts(config.iPITOT_ADC_CHAN)); 
     break;


      case 'a': // Report Raw ADC values
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "a%s%u%s%u%s%u%s%u", 
          settings.api_delim, _hardware.getADCRawData(config.iMAF_ADC_CHAN), 
          settings.api_delim, _hardware.getADCRawData(config.iPREF_ADC_CHAN), 
          settings.api_delim, _hardware.getADCRawData(config.iPDIFF_ADC_CHAN), 
          settings.api_delim, _hardware.getADCRawData(config.iPITOT_ADC_CHAN)); 
     break;


      case 'B': // Get measured Baro Pressure in hPa'B.123.45\r\n'
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "B%s%f", settings.api_delim , sensorVal.BaroHPA);
      break;

      // DEPRECATED - Configuration no longer in JSON file
      case 'C': { // Show configuration.json  'C\r\n'        
          // StaticJsonDocument<CONFIG_JSON_SIZE> configurationJSON;
          // if (SPIFFS.exists("/configuration.json"))  {
          //   configurationJSON = _data.loadJSONFile("/configuration.json");
          // }
          // serializeJsonPretty(configurationJSON, Serial);
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", " "); // send an empty string to prevent Invalid Response
      break; }

      case 'D': // Differential Pressure value
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "D%s%d", settings.api_delim , _calculations.convertPressure(sensorVal.PDiffKPA, INH2O));
      break;      

      case 'E': // Enum1 - Flow:Ref:Temp:Humidity:Baro
          if (status.doBootLoop) break;          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "E%s%f%s%f%s%f%s%f%s%f", 
          settings.api_delim, sensorVal.FlowCFM, 
          settings.api_delim, _calculations.convertPressure(sensorVal.PRefKPA, KPA), 
          settings.api_delim, _calculations.convertTemperature(sensorVal.TempDegC, DEGC), 
          settings.api_delim, _calculations.convertRelativeHumidity(sensorVal.RelH, PERCENT), 
          settings.api_delim, sensorVal.BaroKPA);
      break;      

      
      case 'e': // Enum2 - Pitot:Swirl
          if (status.doBootLoop) break;          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "e%s%f%s%f", 
          settings.api_delim, sensorVal.PitotKPA, 
          settings.api_delim, sensorVal.Swirl); 
      break;      

      
      
      case 'F': // Get measured Flow in CFM 'F123.45\r\n'       
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "F%s%f", settings.api_delim , sensorVal.FlowCFM);
      break;

      case 'f': // Get measured Mass Flow 'F123.45\r\n'       
          if (status.doBootLoop) break;
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "f%s%f", settings.api_delim , sensorVal.FlowKGH);
          snprintf(apiResponse, API_RESPONSE_LENGTH, "f%s%f", settings.api_delim , _sensors.getMafFlow());
      break;

      case 'H': // Get measured Humidity 'H.123.45\r\n'
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "H%s%f", settings.api_delim , sensorVal.RelH);
      break;

      case 'I': // IP Address
          snprintf(apiResponse, API_RESPONSE_LENGTH, "I%s%s", settings.api_delim, status.local_ip_address.c_str());
      break;

      case 'J':{ // JSON SSE Data
          if (status.doBootLoop) break;
          StaticJsonDocument <DATA_JSON_SIZE> jsondoc;
          jsonString = _data.buildIndexSSEJsonData();
          deserializeJson(jsondoc, jsonString);
          serializeJsonPretty(jsondoc, Serial);
          // snprintf(apiResponseBlob, API_BLOB_LENGTH, "J%s%s", settings.api_delim, String(jsonString).c_str());
          snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", " "); // send an empty string to prevent Invalid Response
      break;}


      // DEPRECATED - config no longer in JSON file
      case 'j': // Current configuration in JSON
          // if (status.doBootLoop) break;
          // jsonString = this->getConfigJSON();
          // jsonString.toCharArray(charDataJSON, API_JSON_LENGTH);
          // snprintf(apiResponseBlob, API_BLOB_LENGTH, "C%s%s", settings.api_delim, charDataJSON);
      break;
      
      // DEPRECATED MAF Data now uses transfer function
      case 'K': // MAF Data Key Value A
          // if (status.doBootLoop) break;
          // // refValue =  map(_sensors.getMafVolts(), 0, 5, 0, status.mafDataKeyMax); 
          // refValue = (status.mafDataKeyMax / 5) * _sensors.getMafVolts();
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "K%s MAF DATA Key value: %d ", settings.api_delim , refValue); 
      break;

      case 'k': // MAF Data lookup value
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "k%s MAF DATA Lookup value: %d ", settings.api_delim , sensorVal.MafLookup); 
      break;

      case 'l': // Valve lift Data
          if (status.doBootLoop) break;
          jsonString = _webserver.getLiftDataJSON();
          // snprintf(apiResponseBlob, API_BLOB_LENGTH, "l%s%s", settings.api_delim, String(jsonString).c_str()); // TODO: Fix this - not working
      break;
      
      // TODO - MAF data now uses transfer function
      case 'M': { // Get MAF.json 
            // StaticJsonDocument<MAF_JSON_SIZE> mafJSON;
            // if (SPIFFS.exists(status.mafFilename))  {
            //   mafJSON = _data.loadJSONFile(status.mafFilename);
            // }
            // if (mafJSON.overflowed() == true) {
            //   _message.serialPrintf("MAF Data file - JsonDocument::overflowed()");
            // } else {
            //   serializeJsonPretty(mafJSON, Serial);
            // }
            // snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", " "); // send an empty string to prevent Invalid Response
      break; }
           
      case 'N': // Hostname
          snprintf(apiResponse, API_RESPONSE_LENGTH, "N%s%s", settings.api_delim, settings.hostname);
      break;
      
      case 'O': // Active orifice flow rate 'o\r\n'        
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "O%s%f", settings.api_delim , status.activeOrificeFlowRate);
      break;      

      case 'o': // Active orifice  'O\r\n'        
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "o%s%s", settings.api_delim , status.activeOrifice);
      break;      

      case 'P': // Get measured Pitot Pressure 'P.123.45\r\n'
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "P%s%f", settings.api_delim , _calculations.convertPressure(sensorVal.PitotKPA, INH2O));
      break;

      // DEPRECATED - No longer using MAF lookup table
      case 'Q': // mafdata max value
          // if (status.doBootLoop) break;
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "Q%s%u", settings.api_delim , status.mafDataValMax);
      break;      

      // DEPRECATED - No longer using MAF lookup table
      case 'q': // mafdata max key value
          // if (status.doBootLoop) break;
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "q%s%u", settings.api_delim , status.mafDataKeyMax);
      break;      

      case 'R': // Get measured Reference Pressure 'R.123.45\r\n'
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "R%s%f", settings.api_delim , _calculations.convertPressure(sensorVal.PRefKPA, INH2O));
      break;
      
  //     case 'S': { // Status  'S.123.45\r\n'

  //         _message.serialPrintf("debug = %d\n",status.debug ? "true" : "false");
  //         _message.serialPrintf("spiffs_mem_size = %i\n", status.spiffs_mem_size);
  //         _message.serialPrintf("spiffs_mem_used = %i\n", status.spiffs_mem_used);
  //         _message.serialPrintf("pageSize = %i\n", status.pageSize);
  //         _message.serialPrintf("local_ip_address = %s\n", status.local_ip_address);
  //         _message.serialPrintf("hostname = %s\n", status.hostname);
  //         _message.serialPrintf("boardType = %s\n", status.boardType);
  //         _message.serialPrintf("benchType = %s\n", status.benchType);
  //         _message.serialPrintf("mafSensor = %s\n", status.mafSensor);
  //         _message.serialPrintf("mafLink = %s \n", status.mafLink);
  //         _message.serialPrintf("prefSensor = %s\n", status.prefSensor);
  //         _message.serialPrintf("pdiffSensor = %s\n", status.pdiffSensor);
  //         _message.serialPrintf("tempSensor = %s\n", status.tempSensor);
  //         _message.serialPrintf("relhSensor = %s\n", status.relhSensor);
  //         _message.serialPrintf("baroSensor = %s\n", status.baroSensor);
  //         _message.serialPrintf("pitotSensor = %s\n", status.pitotSensor);
  //         _message.serialPrintf("boot_time = %i\n", status.boot_time);
  //         _message.serialPrintf("liveStream = %s\n", status.liveStream ? "true" : "false");
  //         _message.serialPrintf("adcPollTimer = %lu\n", status.adcPollTimer);
  //         _message.serialPrintf("bmePollTimer = %lu\n", status.bmePollTimer);
  //         _message.serialPrintf("apiPollTimer = %lu\n", status.apiPollTimer);
  //         _message.serialPrintf("ssePollTimer = %lu\n", status.ssePollTimer);
  //         _message.serialPrintf("wsCLeanPollTimer = %lu\n", status.wsCLeanPollTimer);
  //         _message.serialPrintf("pollTimer = %i\n", status.pollTimer);
  //         _message.serialPrintf("serialData = %i\n", status.serialData);;
  //         _message.serialPrintf("statusMessage = %s\n", status.statusMessage);
  //         _message.serialPrintf("apMode = %s\n",status.apMode ? "true" : "false");
  //         _message.serialPrintf("HWMBME = %d\n",status.HWMBME);
  //         _message.serialPrintf("HWMADC = %d\n",status.HWMADC);
  //         _message.serialPrintf("HWMSSE = %d\n",status.HWMSSE);
  //         _message.serialPrintf("activeOrifice =  %s\n", status.activeOrifice);

  //         _message.serialPrintf("activeOrificeFlowRate =  %d\n", status.activeOrifice);
  //         _message.serialPrintf("activeOrificeTestPressure =  %d\n", status.activeOrifice);
  //         _message.serialPrintf("shouldReboot  =  %s\n", status.activeOrifice ? "true" : "false");
  //         _message.serialPrintf("pinsLoaded  =  %s\n", status.activeOrifice ? "true" : "false");
  //         _message.serialPrintf("mafLoaded  =  %s\n", status.activeOrifice ? "true" : "false");
  //         _message.serialPrintf("configLoaded  =  %s\n", status.activeOrifice ? "true" : "false");
  //         _message.serialPrintf("GUIexists  =  %s", status.GUIexists ? "true" : "false");
  //         _message.serialPrintf("pinsFilename =  %s\n", status.pinsFilename);
  //         _message.serialPrintf("mafFilename =  %s\n", status.mafFilename);
  //         _message.serialPrintf("indexFilename =  %s\n", status.indexFilename);
  //         _message.serialPrintf("doBootLoop =  %s\n", status.doBootLoop ? "true" : "false");
  //         _message.serialPrintf("webserverIsRunning  =  %s\n", status.webserverIsRunning ? "true" : "false");
  //         _message.serialPrintf("mafDataTableRows  =  %i\n", status.mafDataTableRows);
  //         _message.serialPrintf("mafDataValMax  =  %s\n", status.mafDataValMax ? "true" : "false");
  //         _message.serialPrintf("mafDataKeyMax  =  %s\n", status.mafDataKeyMax ? "true" : "false");
  //         _message.serialPrintf("mafUnits =  %s\n", status.mafUnits);
  //         _message.serialPrintf("mafScaling  =  %d\n", status.mafScaling);
  //         _message.serialPrintf("mafDiameter  =  %i\n", status.mafDiameter);
  //         _message.serialPrintf("mafSensorType=  %s\n", status.mafSensorType);
  //         _message.serialPrintf("mafOutputType =  %s\n", status.mafOutputType);
  //         snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", " "); // send an empty string to prevent Invalid Response
  //     break;}
      
      case 't': // Get measured Temperature in Fahrenheit 'F.123.45\r\n'
          if (status.doBootLoop) break;
          double TdegF;
          TdegF = _calculations.convertTemperature(sensorVal.TempDegC, DEGF);
          snprintf(apiResponse, API_RESPONSE_LENGTH, "t%s%f", settings.api_delim , TdegF);
      break;      
      
      case 'T': // Get measured Temperature in Celcius 'T.123.45\r\n'
          if (status.doBootLoop) break;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "T%s%f", settings.api_delim , sensorVal.TempDegC);
      break;
      
      case 'u': // Uptime in minutes     
          snprintf(apiResponse, API_RESPONSE_LENGTH, "u%s%u", settings.api_delim , (millis() - status.boot_time) / 60000);
      break;

      case 'U': // Uptime in hhhh.mm      
          snprintf(apiResponse, API_RESPONSE_LENGTH, "U%s%g", settings.api_delim , _hardware.uptime() );
      break;

      case 'V': // Get Version 'VMmYYMMDDXX\r\n'          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "V%s%s.%s.%s", settings.api_delim , MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
      break;



      
      case 'W': // WiFi SSID
          if (status.apMode == true) {
            snprintf(apiResponse, API_RESPONSE_LENGTH, "W%s%s", settings.api_delim, settings.wifi_ap_ssid);
          } else {
            snprintf(apiResponse, API_RESPONSE_LENGTH, "W%s%s", settings.api_delim, settings.wifi_ssid);
          }
      break;

      case 'X': // Print xTask memory usage (Stack high water mark) to serial monitor 
          snprintf(apiResponse, API_RESPONSE_LENGTH,"X%sStack Free Memory EnviroTask=%s / SensorTask=%s ", settings.api_delim , _calculations.byteDecode(uxTaskGetStackHighWaterMark(enviroDataTask)), _calculations.byteDecode(uxTaskGetStackHighWaterMark(sensorDataTask))); 
      break;
      
      case 'x': // Print Heap memory usage to serial monitor 
          // snprintf(apiResponse, API_RESPONSE_LENGTH,"x%sFree Heap=%s / Max Allocated Heap=%s ", settings.api_delim , _calculations.byteDecode(ESP.getFreeHeap()), _calculations.byteDecode(ESP.getMaxAllocHeap())); 
          snprintf(apiResponse, API_RESPONSE_LENGTH, "Free Stack: EnviroTask=%s  \n", _calculations.byteDecode(uxTaskGetStackHighWaterMark(enviroDataTask))); 
          snprintf(apiResponse, API_RESPONSE_LENGTH, "Free Stack: SensorTask=%s  \n", _calculations.byteDecode(uxTaskGetStackHighWaterMark(sensorDataTask))); 
          snprintf(apiResponse, API_RESPONSE_LENGTH, "Free Stack: LoopTask=%s    \n", _calculations.byteDecode(uxTaskGetStackHighWaterMark(NULL))); 
      break;

      
  //     case '@': // Status Print Mode (Stream status messages to serial)
  //         if (status.doBootLoop) break;
  //         if (settings.status_print_mode == true){
  //           settings.status_print_mode = false;
  //           snprintf(apiResponse, API_RESPONSE_LENGTH, "@%s%s", settings.api_delim, "Status Data Disabled" ); 
  //         } else {
  //           settings.status_print_mode = true;
  //           snprintf(apiResponse, API_RESPONSE_LENGTH, "@%s%s", settings.api_delim, "Status Data Enabled~" ); 
  //         }
  //     break;
      
      

      case '?': // Help      
          if (status.doBootLoop) {
            snprintf(apiResponseBlob, API_BLOB_LENGTH, "\n%s", bootHelpText);            
          } else if (settings.function_mode == true) {
            snprintf(apiResponseBlob, API_BLOB_LENGTH, "\n%s", apiFunctionHelpText);
          } else {
            snprintf(apiResponseBlob, API_BLOB_LENGTH, "\n%s", apiHelpText);
          }
      break;
      
      case '/': // SPIFFS File List
        {
          FILESYSTEM.begin();
          File root = FILESYSTEM.open("/");
          File file = root.openNextFile();
          const char* spiffsFileName;
          int spiffsFileSize;

          status.spiffs_mem_size = SPIFFS.totalBytes();
          status.spiffs_mem_used = SPIFFS.usedBytes();

          strcat(fileListBlob, "Spiffs File List\n");
          strcat(fileListBlob, "================\n");

          while (file)  {
            spiffsFileName = file.name();
            spiffsFileSize = file.size(); 
            strcat(fileListBlob, spiffsFileName);
            strcat(fileListBlob, "  ");
            strcat(fileListBlob, _calculations.byteDecode(spiffsFileSize).c_str());
            strcat(fileListBlob, "\n");
            file = root.openNextFile();
          }

          strcat(fileListBlob, "================\n");
          strcat(fileListBlob, "\nTotal space:      ");
          strcat(fileListBlob, _calculations.byteDecode(status.spiffs_mem_size).c_str());
          strcat(fileListBlob, "\n");
          strcat(fileListBlob, "Total space used: ");
          strcat(fileListBlob, _calculations.byteDecode(status.spiffs_mem_used).c_str());
          strcat(fileListBlob, "\n");
          strcat(fileListBlob, "================\n");

          snprintf(apiResponseBlob, API_BLOB_LENGTH, "\n%s" , fileListBlob);


          // FILESYSTEM.end();
        }
      break;

      case '!': // Debug Mode
        if (settings.debug_mode == true){
          settings.debug_mode = false;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "!%s%s", settings.api_delim, "Debug Mode Off" ); 
        } else {
          settings.debug_mode = true;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "!%s%s", settings.api_delim, "Debug Mode On" ); 
        }
      break;
      
      case '+': // Verbose Print Mode 
        if (settings.verbose_print_mode == true){
          settings.verbose_print_mode = false;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "!%s%s", settings.api_delim, "Verbose Print Mode Off" ); 
        } else {
          settings.verbose_print_mode = true;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "!%s%s", settings.api_delim, "Verbose Print Mode On" ); 
        }
      break;
      
      case '=': // Status Print Mode 
        if (settings.status_print_mode == true){
          settings.status_print_mode = false;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "!%s%s", settings.api_delim, "Status Print Mode Off" ); 
        } else {
          settings.status_print_mode = true;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "!%s%s", settings.api_delim, "Status Print Mode On" ); 
        }
      break;
      
      case '#': // Developer Mode (Enable additional developer tools)
        if (settings.dev_mode == true) {
          settings.dev_mode = false;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", settings.api_delim, "Developer Mode Off" ); 
        } else {
          settings.dev_mode = true;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", settings.api_delim, "Developer Mode On" ); 
        }
      break;
      
      case '^': // Function Mode (Enable additional tools)
        if (settings.function_mode == true) {
          settings.function_mode = false;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", settings.api_delim, "Function Mode Off" ); 
        } else {
          settings.function_mode = true;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", settings.api_delim, "Function Mode On" ); 
        }
      break;
      
      case '<': // Display last error
        snprintf(apiResponseBlob, API_BLOB_LENGTH, "%s", "No error in buffer");
      break;
      
      case '~': //ESP Restart
        snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", "Restarting...");
        ESP.restart();
      break;

      case '$': // Recover WiFi
          snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", "Attempting to recover WiFi Connection");
          // settings.api_enabled = false;
          _comms.wifiReconnect();
          // settings.api_enabled = true;
      break;

      case '%':{ // Reset WiFi passwords
        if (settings.function_mode == true) {
          snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", "Attempting to reset WiFi passwords");
          _prefs.begin("settings");
          _prefs.putString("sWIFI_AP_SSID", static_cast<String>("DIYFB"));
          _prefs.putString("sWIFI_AP_PSWD", static_cast<String>("123456789"));
          _prefs.end();
          settings.function_mode = false;
        }
        break;
      }

      case '&':{ // Reset Pins
        if (settings.function_mode == true) {
          _hardware.resetPins();
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", settings.api_delim, "Pins Reset" ); 
          settings.function_mode = false;
        }
        break;
      }


      case '\\': { //AB_test
        if (settings.AB_test == 'A') {
          settings.AB_test = 'B';
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", settings.api_delim, "A/B test B" ); 
        } else if (settings.AB_test == 'B'){
          settings.AB_test = 'C';
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", settings.api_delim, "A/B test C" ); 
        } else if (settings.AB_test == 'C'){
          settings.AB_test = 'A';
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", settings.api_delim, "A/B test A" ); 
        }
      }

      case ' ': // <<<<(TEST [space] exclude from API listing)<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 
           
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "Z%s%d", settings.api_delim , status.mafScaling);
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "Z%s%d", settings.api_delim , status.mafUnits);
          // _hardware.stepperTest();

          // BME680 Tests
          // if (status.debug == true) {
          //   // snprintf(apiResponse, API_RESPONSE_LENGTH, "Temp: %f Baro: %f RelH: %f \n" , sensorVal.TempDegC, sensorVal.BaroHPA, sensorVal.RelH);
          //   snprintf(apiResponse, API_RESPONSE_LENGTH, "Temp: %f Baro: %f RelH: %f \n" , sensorVal.test, sensorVal.BaroHPA, sensorVal.RelH);
          // } else {
          //   snprintf(apiResponse, API_RESPONSE_LENGTH, "Temp: %f Baro: %f RelH: %f \n", _sensors.getTempValue(), _sensors.getBaroValue(), _sensors.getRelHValue() );
          // }


          // pin assignment tests
          // _data.loadPinsData();
          // _data.writeJSONFile();


      break; // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


      // TODO - Calibration API commands
      // case '}': // Flow Offset Calibration  'O\r\n'        
      //     if (status.doBootLoop) break;
      //     // _calibration.setFlowOffset();
      //     snprintf(apiResponse, API_RESPONSE_LENGTH, "C%s%f", settings.api_delim , calVal.flow_offset);
      //     // TODO: confirm Flow Offset Calibration success in response
      // break;      

      // case 'L': // Perform Leak Test Calibration 'L\r\n'
      //     if (status.doBootLoop) break;
      //     _calibration.setLeakOffset();
      //     snprintf(apiResponse, API_RESPONSE_LENGTH, "L%s%F", settings.api_delim , calVal.leak_cal_offset );
      //     // TODO: confirm Leak Test Calibration success in response
      // break;
      
      // case 'l': // Perform Leak Test 'l\r\n'      
      //     if (status.doBootLoop) break;
      //     // TODO: apiResponse = ("l") + settings.api_delim + leakTest();
      //     // TODO: confirm Leak Test success in response
      // break;


      // We've got here without a valid API request so lets get outta here before we send garbage to the serial comms
      default:
          return;
      break;

      
  }
  
  
  
  // Send API Response
  #if defined API_CHECKSUM_IS_ENABLED

      uint32_t crcValue = calcCRC(apiResponse);

      if (*apiResponseBlob != 0)   {
        _message.blobPrintf("%s%s%s\n", apiResponseBlob, settings.api_delim, (String)crcValue);              
      } else if (*apiResponse != 0) {
        _message.serialPrintf("%s%s%s\n", apiResponse, settings.api_delim, (String)crcValue);      
      } else {
        //invalid response
        _message.serialPrintf("%s%s%s\n", "Invalid Response", settings.api_delim, (String)crcValue);
      }
  #else
      if (*apiResponseBlob != 0) {
        _message.blobPrintf("%s\n", apiResponseBlob);     
        _message.serialPrintf("\n\n", "");    
     
      } else if (*apiResponse != 0) {
        _message.serialPrintf("%s\n", apiResponse);    
      
      } else {
        //invalid response
        _message.serialPrintf("%s\n", "Invalid Response");
      }
  #endif

}




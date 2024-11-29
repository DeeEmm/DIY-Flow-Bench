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
#include "configuration.h"
// #include " version.h"

#include "API.h"
#include <esp32/rom/crc.h> 
// #include "pins.h"
#include "hardware.h"
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
#include "calibration.h"
#include "webserver.h"
#include "datahandler.h"
#include "comms.h"
// // #include LANGUAGE_FILE

extern struct ConfigSettings config;


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
  
  extern struct ConfigSettings config;
  String jsonString;
  StaticJsonDocument<1024> dataJson;    
  
  // Can add more settings as required
  dataJson["MIN_FLOW_RATE"] = config.min_flow_rate;
  dataJson["MIN_BENCH_PRESSURE"] = config.min_bench_pressure;
  dataJson["REFRESH_RATE"] = config.refresh_rate;

  
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

  extern struct ConfigSettings config;
  extern struct CalibrationData calVal;
  extern struct SensorData sensorVal;
  extern struct DeviceStatus status;

  Calculations _calculations;
  Sensors _sensors;
  Messages _message;    
  Hardware _hardware;
  Webserver _webserver;
  Calibration _calibration;
  DataHandler _data;
  Comms _comms;
  
  extern TaskHandle_t sensorDataTask;
  extern TaskHandle_t enviroDataTask;
  
  // define char arrays for response strings
  char apiResponse[API_RESPONSE_LENGTH];  //64
  char apiResponseBlob[API_BLOB_LENGTH];  //1024
  char charDataJSON[API_JSON_LENGTH];     //1020
  char fileListBlob[1024];

  // Initialise arrays
  apiResponse[0] = 0;
  apiResponseBlob[0] = 0;
  charDataJSON[0] = 0;
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
  C : Flow Offset Calibration
  D : Differential Pressure value
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
  L : Leak Test Calibration
  l : Leak Test
  M : MAF RAW ADC Value
  m : MAF Voltage
  N : Hostname
  o : Active Orifice
  P : Pitot Value
  Q : MAF Data Max Value
  q : MAF Data Key Max Value
  R : Reference Pressure Value
  S : Swirl value
  T : Temperature in Celcius
  t : Temperature in Fahrenheit
  U : Uptime in hhhh.mm
  V : Version
  v : Valve lift data in JSON format
  W : WiFi SSID
  X : xTask memory usage   
  ? : Help
  / : SPIFFS File List
  ~ : Restart ESP
  $ : Reset WiFi
  @ : Stream Status
  ! : Debug Mode
  < : Last Error
  ============================== 
  )";


  switch (apiMessage)   {

      // NOTE: Responses are alphabetised

      case '0': // Bench Off
          _hardware.benchOff();
      break;
      
      case '1': // Bench On
          _hardware.benchOn();
      break;
      
      case '3': // Get 3v board supply voltage (mv) 'v.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "3%s%f", config.api_delim ,_hardware.get3v3SupplyVolts());
      break;
      
      case '5': // Get 5v board supply voltage (mv) 'v.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "5%s%f", config.api_delim , _hardware.get5vSupplyVolts());
      break;

      case 'A': // Report ADC voltage values
          snprintf(apiResponse, API_RESPONSE_LENGTH, "A%s%f%s%f%s%f%s%f", 
          config.api_delim, _hardware.getADCVolts(MAF_ADC_CHANNEL), 
          config.api_delim, _hardware.getADCVolts(PREF_ADC_CHANNEL), 
          config.api_delim, _hardware.getADCVolts(PDIFF_ADC_CHANNEL), 
          config.api_delim, _hardware.getADCVolts(PITOT_ADC_CHANNEL)); 
     break;


      case 'a': // Report Raw ADC values
          snprintf(apiResponse, API_RESPONSE_LENGTH, "a%s%u%s%u%s%u%s%u", 
          config.api_delim, _hardware.getADCRawData(MAF_ADC_CHANNEL), 
          config.api_delim, _hardware.getADCRawData(PREF_ADC_CHANNEL), 
          config.api_delim, _hardware.getADCRawData(PDIFF_ADC_CHANNEL), 
          config.api_delim, _hardware.getADCRawData(PITOT_ADC_CHANNEL)); 
     break;


      case 'B': // Get measured Baro Pressure in hPa'B.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "B%s%f", config.api_delim , sensorVal.BaroHPA);
      break;

      case 'C': // Flow Offset Calibration  'O\r\n'        
          // _calibration.setFlowOffset();
          snprintf(apiResponse, API_RESPONSE_LENGTH, "C%s%f", config.api_delim , calVal.flow_offset);
          // TODO: confirm Flow Offset Calibration success in response
      break;      

      case 'D': // Differential Pressure value
          snprintf(apiResponse, API_RESPONSE_LENGTH, "D%s%u", config.api_delim , _calculations.convertPressure(sensorVal.PRefKPA, INH2O));
      break;      

      // DEPRECATED 
      // case 'd': // Differential pressure sensor voltage
      //     snprintf(apiResponse, API_RESPONSE_LENGTH, "d%s%u", config.api_delim , sensorVal.PDiffMv);
      // break;      

      case 'E': // Enum1 - Flow:Ref:Temp:Humidity:Baro
          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "E%s%f%s%f%s%f%s%f%s%f", 
          config.api_delim, sensorVal.FlowCFM, 
          config.api_delim, _calculations.convertPressure(sensorVal.PRefKPA, KPA), 
          config.api_delim, _calculations.convertTemperature(sensorVal.TempDegC, DEGC), 
          config.api_delim, _calculations.convertRelativeHumidity(sensorVal.RelH, PERCENT), 
          config.api_delim, sensorVal.BaroKPA);
      break;      

      
      case 'e': // Enum2 - Pitot:Swirl
          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "E%s%f%s%f", 
          config.api_delim, sensorVal.PitotKPA, 
          config.api_delim, sensorVal.Swirl); 
      break;      

      
      
      case 'F': // Get measured Flow in CFM 'F123.45\r\n'       
          snprintf(apiResponse, API_RESPONSE_LENGTH, "F%s%f", config.api_delim , sensorVal.FlowCFM);
      break;

      case 'f': // Get measured Mass Flow 'F123.45\r\n'       
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "f%s%f", config.api_delim , sensorVal.FlowKGH);
          snprintf(apiResponse, API_RESPONSE_LENGTH, "f%s%f", config.api_delim , _sensors.getMafFlow());
      break;

      case 'H': // Get measured Humidity 'H.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "H%s%f", config.api_delim , sensorVal.RelH);
      break;

      case 'I': // IP Address
          snprintf(apiResponse, API_RESPONSE_LENGTH, "I%s%s", config.api_delim, status.local_ip_address.c_str());
      break;

      case 'J': // JSON Data
          jsonString = _data.getDataJSON();
          snprintf(apiResponseBlob, API_BLOB_LENGTH, "J%s%s", config.api_delim, String(jsonString).c_str());
      break;
      
      case 'j': // Current configuration in JSON
          jsonString = this->getConfigJSON();
          jsonString.toCharArray(charDataJSON, API_JSON_LENGTH);
          snprintf(apiResponseBlob, API_BLOB_LENGTH, "C%s%s", config.api_delim, charDataJSON);
      break;
      
      case 'K': // MAF Data Key Value 
          // refValue =  map(_sensors.getMafVolts(), 0, 5, 0, status.mafDataKeyMax); 
          refValue = (status.mafDataKeyMax / 5) * _sensors.getMafVolts();
          snprintf(apiResponse, API_RESPONSE_LENGTH, "K%s MAF DATA Key value: %d ", config.api_delim , refValue); 
      break;

      case 'k': // MAF Data lookup value
          snprintf(apiResponse, API_RESPONSE_LENGTH, "k%s MAF DATA Lookup value: %d ", config.api_delim , sensorVal.MafLookup); 
      break;

      case 'L': // Perform Leak Test Calibration 'L\r\n'
          _calibration.setLeakOffset();
          snprintf(apiResponse, API_RESPONSE_LENGTH, "L%s%F", config.api_delim , calVal.leak_cal_offset );
          // TODO: confirm Leak Test Calibration success in response
      break;
      
      case 'l': // Perform Leak Test 'l\r\n'      
          // TODO: apiResponse = ("l") + config.api_delim + leakTest();
          // TODO: confirm Leak Test success in response
      break;
      
      case 'M': // Get MAF raw sensor data'  
          snprintf(apiResponse, API_RESPONSE_LENGTH, "M%s%u", config.api_delim, _hardware.getADCRawData(MAF_ADC_CHANNEL));   
      break;
      
      // DEPRECATED 
      // case 'm': // Get MAF output voltage'
      //     snprintf(apiResponse, API_RESPONSE_LENGTH, "m%s%f", config.api_delim , _sensors.getMafVolts());
      // break;     
      
      case 'N': // Hostname
          snprintf(apiResponse, API_RESPONSE_LENGTH, "N%s%s", config.api_delim, config.hostname);
      break;
      
      case 'O': // Active orifice flow rate 'o\r\n'        
          snprintf(apiResponse, API_RESPONSE_LENGTH, "O%s%f", config.api_delim , status.activeOrificeFlowRate);
      break;      

      case 'o': // Active orifice  'O\r\n'        
          snprintf(apiResponse, API_RESPONSE_LENGTH, "o%s%s", config.api_delim , status.activeOrifice);
      break;      

      case 'P': // Get measured Pitot Pressure 'P.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "R%s%f", config.api_delim , _calculations.convertPressure(sensorVal.PitotKPA, INH2O));
      break;
      
      // DEPRECATED 
      // case 'p': // Get Pitot sensor output voltage          
      //     snprintf(apiResponse, API_RESPONSE_LENGTH, "r%s%f", config.api_delim , _sensors.getPitotVolts());
      // break;   

      case 'Q': // mafdata max value
          snprintf(apiResponse, API_RESPONSE_LENGTH, "Q%s%u", config.api_delim , status.mafDataValMax);
      break;      

      case 'q': // mafdata max key value
          snprintf(apiResponse, API_RESPONSE_LENGTH, "q%s%u", config.api_delim , status.mafDataKeyMax);
      break;      

      case 'R': // Get measured Reference Pressure 'R.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "R%s%f", config.api_delim , _calculations.convertPressure(sensorVal.PRefKPA, INH2O));
      break;
      
      // DEPRECATED 
      // case 'r': // Get Reference Pressure sensor output voltage          
      //     snprintf(apiResponse, API_RESPONSE_LENGTH, "r%s%f", config.api_delim , _sensors.getPRefVolts());
      // break;      
      
      case 'S': // Get Swirl Value (rpm) 'S.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "S%s%f", config.api_delim , sensorVal.Swirl);
      break;
      
      case 't': // Get measured Temperature in Fahrenheit 'F.123.45\r\n'
          double TdegF;
          TdegF = _calculations.convertTemperature(sensorVal.TempDegC, DEGF);
          snprintf(apiResponse, API_RESPONSE_LENGTH, "t%s%f", config.api_delim , TdegF);
      break;      
      
      case 'T': // Get measured Temperature in Celcius 'T.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "T%s%f", config.api_delim , sensorVal.TempDegC);
      break;
      
      case 'u': // Uptime in minutes     
          snprintf(apiResponse, API_RESPONSE_LENGTH, "u%s%u", config.api_delim , (millis() - status.boot_time) / 60000);
      break;

      case 'U': // Uptime in hhhh.mm      
          snprintf(apiResponse, API_RESPONSE_LENGTH, "U%s%g", config.api_delim , _hardware.uptime() );
      break;

      case 'V': // Get Version 'VMmYYMMDDXX\r\n'          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "V%s%s.%s.%s", config.api_delim , MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
      break;

      case 'v': // Valve lift Data
          jsonString = _webserver.getValveDataJSON();
          snprintf(apiResponseBlob, API_BLOB_LENGTH, "v%s%s", config.api_delim, String(jsonString).c_str());
      break;
      
      case 'W': // WiFi SSID
          if (status.apMode == true) {
            snprintf(apiResponse, API_RESPONSE_LENGTH, "W%s%s", config.api_delim, config.wifi_ap_ssid);
          } else {
            snprintf(apiResponse, API_RESPONSE_LENGTH, "W%s%s", config.api_delim, config.wifi_ssid);
          }
      break;

      case 'X': // Print xTask memory usage (Stack high water mark) to serial monitor 
          snprintf(apiResponse, API_RESPONSE_LENGTH,"X%sStack Free Memory EnviroTask=%d / SensorTask=%d ", config.api_delim , uxTaskGetStackHighWaterMark(enviroDataTask), uxTaskGetStackHighWaterMark(sensorDataTask)); 
      break;
      
      case '@': // Status Print Mode (Stream status messages to serial)
        if (config.status_print_mode == true){
          config.status_print_mode = false;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "@%s%s", config.api_delim, "Status Data Disabled" ); 
        } else {
          config.status_print_mode = true;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "@%s%s", config.api_delim, "Status Data Enabled~" ); 
        }
      break;
      
      

      case '?': // Help      
        snprintf(apiResponseBlob, API_BLOB_LENGTH, "\n%s", apiHelpText);
      break;
      
      case '/': // SPIFFS File List
        {
          FILESYSTEM.begin();
          File root = FILESYSTEM.open("/");
          File file = root.openNextFile();
          const char* spiffsFileName;
          String spiffsFileSize;
          while (file)  {
            spiffsFileName = file.name();
            spiffsFileSize = file.size(); // TODO what about file size????!?!?!
            strcat(fileListBlob, spiffsFileName);
            strcat(fileListBlob, "\n");
            file = root.openNextFile();
          }
          snprintf(apiResponseBlob, API_BLOB_LENGTH, "\n%s" , fileListBlob);
          // FILESYSTEM.end();
        }
      break;

      case '!': // Debug Mode (enable verbose debug messages)
        if (config.debug_mode == true){
          config.debug_mode = false;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "!%s%s", config.api_delim, "Debug Mode Off" ); 
        } else {
          config.debug_mode = true;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "!%s%s", config.api_delim, "Debug Mode On" ); 
        }
      break;
      
      case '#': // Developer Mode (Enable additional developer tools)
        if (config.dev_mode == true){
          config.dev_mode = false;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", config.api_delim, "Developer Mode Off" ); 
        } else {
          config.dev_mode = true;
          snprintf(apiResponse, API_RESPONSE_LENGTH, "#%s%s", config.api_delim, "Developer Mode On" ); 
        }
      break;
      
      case '<': // Display last error
        snprintf(apiResponseBlob, API_BLOB_LENGTH, "%s", "No error in buffer");
      break;
      
      case '~': //ESP Restart
        snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", "Restarting...");
        ESP.restart();
      break;

      case '$': // Recover server
          snprintf(apiResponse, API_RESPONSE_LENGTH, "%s", "Attempting to recover WiFi Connection");
          // config.api_enabled = false;
          _comms.wifiReconnect();
          // config.api_enabled = true;
      break;

      case ' ': // <<<<(TEST [space] exclude from API listing)<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 
           
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "Z%s%d", config.api_delim , status.mafScaling);
          // snprintf(apiResponse, API_RESPONSE_LENGTH, "Z%s%d", config.api_delim , status.mafUnits);
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


      // We've got here without a valid API request so lets get outta here before we send garbage to the serial comms
      default:
          return;
      break;

      
  }
  
  
  
  // Send API Response
  #if defined API_CHECKSUM_IS_ENABLED

      uint32_t crcValue = calcCRC(apiResponse);

      if (*apiResponseBlob != 0)   {
        _message.blobPrintf("%s%s%s\n", apiResponseBlob, config.api_delim, (String)crcValue);              
      } else if (*apiResponse != 0) {
        _message.serialPrintf("%s%s%s\n", apiResponse, config.api_delim, (String)crcValue);      
      } else {
        //invalid response
        _message.serialPrintf("%s%s%s\n", "Invalid Response", config.api_delim, (String)crcValue);
      }
  #else
      if (*apiResponseBlob != 0) {
        _message.blobPrintf("%s\n", apiResponseBlob);     
        _message.serialPrintf("\n\n", "");    
      } else if (*apiResponse != 0) {
        _message.serialPrintf("%s\n", apiResponse);    
      }else {
        //invalid response
        _message.serialPrintf("%s\n", "Invalid Response");
      }
  #endif

}




/***********************************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * API.ino - API functions
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

#include "constants.h"
#include "structs.h"
#include "configuration.h"

#include "API.h"
#include <esp32/rom/crc.h>
#include "pins.h"
#include "hardware.h"
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
// #include "calibration.h"
#include "webserver.h"
// #include LANGUAGE_FILE

extern struct ConfigSettings config;


API::API() {
  
}


/***********************************************************
 * CREATE CRC32 CHECKSUM
 *
 * Source: https://techoverflow.net/2022/08/05/how-to-compute-crc32-with-ethernet-polynomial-0x04c11db7-on-esp32-crc-h/
 *
 ***/
uint16_t API::calcCRC (char* str) {

  CRC = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)str, 8))^0xffffffFF;

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
  dataJson["LEAK_TEST_TOLERANCE"] = config.leak_test_tolerance;
  dataJson["LEAK_TEST_THRESHOLD"] = config.leak_test_threshold;
  
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
  extern struct DeviceStatus status;
  
  Calculations _calculations;
  Sensors _sensors;
  Messages _message;
  Hardware _hardware;
  Webserver _webserver;
  
  // define char arrays for response strings
  char apiResponse[API_RESPONSE_LENGTH];  //64
  char apiResponseBlob[API_BLOB_LENGTH];  //1024
  char charDataJSON[API_JSON_LENGTH];     //1020
  // Initialise arrays
  apiResponse[0] = 0;
  apiResponseBlob[0] = 0;
  charDataJSON[0] = 0;
  
  String jsonString;
  
  //API Response
  char apiHelpText[] = R"(
  DIYFB API Commands
  ==============================
  API Response Format
  Command : Value : Checksum
  ==============================
  3 : 3.3V Voltage Value
  5 : 5V Voltage Value
  B : Barometric Pressure
  C : JSON Configuration Data
  d : Debug On / Off
  E : Enum
  F : Flow Value
  H : Humidity Value
  I : IP Address
  J : JSON Status Data
  L : Leak Test Calibration
  l : Leak Test
  M : MAF Value
  m : MAF Voltage
  N : Hostname
  O : Flow Offset Calibration
  R : Reference Pressure Value
  r : Reference Pressure Voltage
  S : WiFi SSID
  T : Temperature in Celcius
  t : Temperature in Fahrenheit
  V : Version
  ? : Help
  ~ : Restart
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
          snprintf(apiResponse, API_RESPONSE_LENGTH, "3%s%f", config.api_delim ,_hardware.get3v3SupplyMillivolts());
      break;
      
      case '5': // Get 5v board supply voltage (mv) 'v.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "5%s%f", config.api_delim , _hardware.get5vSupplyMillivolts());
      break;
      
      case 'B': // Get measured Baro Pressure in hPa'B.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "B%s%f", config.api_delim , _calculations.convertPressure(_sensors.getBaroValue(), HPA));
      break;

      case 'C': // Current configuration in JSON
          jsonString = this->getConfigJSON();
          jsonString.toCharArray(charDataJSON, API_JSON_LENGTH);
          snprintf(apiResponseBlob, API_BLOB_LENGTH, "C%s%s", config.api_delim, charDataJSON);
      break;
      
      case 'd': // DEBUG ON/OFF'
        if (streamMafData == false) {
          snprintf(apiResponse, API_RESPONSE_LENGTH, "d%s%s", config.api_delim, "Debug Mode On" );
          streamMafData = true;
        } else {
          snprintf(apiResponse, API_RESPONSE_LENGTH, "d%s%s", config.api_delim, "Debug Mode Off" );
          streamMafData = false;
        }
      break;      

      case 'E': // Enum - Flow:Ref:Temp:Humidity:Baro
          // // Flow
          // apiResponse = ("E") + config.api_delim ;        
          // // Truncate to 2 decimal places
          // flowCFM = _calculations.calculateFlowCFM() * 100;
          // apiResponse += (flowCFM / 100) + (config.api_delim);
          // // Reference Pressure
          // apiResponse += _calculations.convertPressure(_sensors.getPRefValue(), KPA) + (config.api_delim);
          // // Temperature
          // apiResponse += _calculations.convertTemperature(_sensors.getTempValue(), DEGC) + (config.api_delim);
          // // Humidity
          // apiResponse += _calculations.convertRelativeHumidity(_sensors.getRelHValue(), PERCENT) + (config.api_delim);
          // // Barometric Pressure
          // apiResponse += _calculations.convertPressure(_sensors.getBaroValue(), KPA);
          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "E%s%f%s%f%s%f%s%f%s%f", 
          config.api_delim, _calculations.calculateFlowCFM() * 100, 
          config.api_delim, _calculations.convertPressure(_sensors.getPRefValue(), KPA), 
          config.api_delim, _calculations.convertTemperature(_sensors.getTempValue(), DEGC), 
          config.api_delim, _calculations.convertRelativeHumidity(_sensors.getRelHValue(), PERCENT), 
          config.api_delim, _calculations.convertPressure(_sensors.getBaroValue(), KPA));
      break;      
      
      case 'F': // Get measured Flow 'F123.45\r\n'       
          snprintf(apiResponse, API_RESPONSE_LENGTH, "F%s%f", config.api_delim , _calculations.calculateFlowCFM());
      break;

      case 'H': // Get measured Humidity 'H.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "H%s%f", config.api_delim , _sensors.getRelHValue());
      break;

      case 'I': // IP Address
          snprintf(apiResponse, API_RESPONSE_LENGTH, "I%s%s", config.api_delim, status.local_ip_address.c_str());
      break;

      case 'J': // JSON Data
          // _webserver.getDataJSON().toCharArray(charDataJSON, API_JSON_LENGTH);
          // snprintf(apiResponseBlob, API_BLOB_LENGTH, "J%s%s", config.api_delim, charDataJSON);
          jsonString = _webserver.getDataJSON();
          jsonString.toCharArray(charDataJSON, API_JSON_LENGTH);
          snprintf(apiResponseBlob, API_BLOB_LENGTH, "J%s%s", config.api_delim, charDataJSON);

      break;
      
      case 'L': // Perform Leak Test Calibration 'L\r\n'
          // TODO: apiResponse = ("L") + config.api_delim + leakTestCalibration();
          // TODO: confirm Leak Test Calibration success in response
      break;
      
      case 'l': // Perform Leak Test 'l\r\n'      
          // TODO: apiResponse = ("l") + config.api_delim + leakTest();
          // TODO: confirm Leak Test success in response
      break;
      
      case 'M': // Get MAF sensor data'  
          snprintf(apiResponse, API_RESPONSE_LENGTH, "M%s", config.api_delim );   
          if (streamMafData == false) {
              streamMafData = true;
              _calculations.calculateFlowCFM();
              streamMafData = false;         
          }
      break;
      
      case 'm': // Get MAF output voltage'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "m%s%f", config.api_delim , ((_sensors.getMafMillivolts() * (_hardware.get5vSupplyMillivolts() / 4095.0)) * 1000));
      break;     
      
      case 'N': // Hostname
          snprintf(apiResponse, API_RESPONSE_LENGTH, "N%s%s", config.api_delim, config.hostname);
      break;
      
      case 'O': // Flow Offset Calibration  'O\r\n'        
          // TODO: setCalibrationOffset();
          // TODO: calibration.flow_offset
          snprintf(apiResponse, API_RESPONSE_LENGTH, "O%s%f", config.api_delim , config.cal_offset);
          // TODO: confirm Flow Offset Calibration success in response
      break;      
  
      case 'R': // Get measured Reference Pressure 'R.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "R%s%f", config.api_delim , _calculations.convertPressure(_sensors.getPRefValue(), INWG));
      break;
      
      case 'r': // Get Reference Pressure sensor output voltage          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "r%s%f", config.api_delim , _sensors.getPRefMillivolts());
      break;      
      
      case 'S': // WiFi SSID
          if (status.apMode == true) {
            snprintf(apiResponse, API_RESPONSE_LENGTH, "S%s%s", config.api_delim, config.wifi_ap_ssid);
          } else {
            snprintf(apiResponse, API_RESPONSE_LENGTH, "S%s%s", config.api_delim, config.wifi_ssid);
          }
      break;
      
      case 'T': // Get measured Temperature in Celcius 'T.123.45\r\n'
          snprintf(apiResponse, API_RESPONSE_LENGTH, "T%s%f", config.api_delim , _sensors.getTempValue());
      break;
      
      case 't': // Get measured Temperature in Fahrenheit 'F.123.45\r\n'
          double TdegF;
          TdegF = _calculations.convertTemperature(_sensors.getTempValue(), DEGF);
          snprintf(apiResponse, API_RESPONSE_LENGTH, "t%s%f", config.api_delim , TdegF);
      break;      
      
      case 'V': // Get Version 'VMmYYMMDDXX\r\n'          
          snprintf(apiResponse, API_RESPONSE_LENGTH, "V%s%s.%s.%s", config.api_delim , MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
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
      
      case '?': // Help      
        snprintf(apiResponseBlob, API_BLOB_LENGTH, "\n%s\n", apiHelpText);
      break;
      
      case '<': //Display last error
        snprintf(apiResponseBlob, API_BLOB_LENGTH, "\n%s\n", "No error in buffer");
      break;
      
      case '~': //Restart
        snprintf(apiResponse, API_RESPONSE_LENGTH, "\n%s\n", "Restarting...");
        ESP.restart();
      break;
      
      // We've got here without a valid API request so lets get outta here before we send garbage to the serial comms
      default:
          return;
      break;

      
  }
  
  
  
  // Send API Response
  #if defined API_CHECKSUM_IS_ENABLED
      // TODO: FIX CHECKSUM
      crcValue = calcCRC(apiResponse);
      if (*apiResponseBlob != 0)   {
        _message.blobPrintf("%s%s%s\n", apiResponseBlob, config.api_delim, crcValue);              
      } else if (*apiResponse != 0) {
        _message.serialPrintf("%s%s%s\n", apiResponse, config.api_delim, crcValue);      
      } else {
        //invalid response
        _message.serialPrintf("%s%s%s\n", "Invalid Response", config.api_delim, crcValue);
      }
  #else
      if (*apiResponseBlob != 0) {
        _message.blobPrintf("%s\n", apiResponseBlob);     
      } else if (*apiResponse != 0) {
        _message.serialPrintf("%s\n", apiResponse);    
      }else {
        //invalid response
        _message.serialPrintf("%s\n", "Invalid Response");
      }
  #endif

}




/****************************************
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

#include "API.h";

#include "constants.h"
#include "structs.h"
#include "configuration.h"
#include "pins.h"
#include "hardware.h"
// #include "sensors.h"
#include "maths.h"
#include "messages.h"
// #include "calibration.h"
#include "webserver.h"

#include LANGUAGE_FILE

// extern struct ConfigSettings config;
// extern struct CalibrationSettings calibrationSettings;
// extern struct DeviceStatus status;

API::API() {
  
}





/****************************************
 * CREATE CHECKSUM
 *
 * Source: https://forum.arduino.cc/index.php?topic=311293.msg2158081#msg2158081
 * Usage: myVar = calcCRC(str);
 *
 ***/
uint16_t API::calcCRC (char* str) {
    // Initialise CRC
    uint16_t CRC = 0; 
    // Traverse each character in the string
    for (int i=0;i<strlen(str);i++) {
        // TODO: update the CRC value using the built in CRC32 function
//        CRC= _crc16_update (CRC, str[i]);  // NOTE: OLD CRC16 Library!!
    }
    return CRC;
}




/****************************************
 * PARSE API
 *
 * handle API responses:
 * 
 * General Commmands
 * 'V' - Return firmware version
 * 'F' - Return flow value in cfm
 * 'T' - Return temperature value in deg C
 * 'H' - Return humidity value in RH
 * 'R' - Return reference pressure value in in/h2o
 * 'I' - Return IP Address
 * 'S' - Return WiFi SSID
 * 'N' - Return HOSTNAME
 * 'J' - Return JSON
 * Debug Commands
 * 'M' - Return MAF Data (NOTE: will only return data if flow > 0)
 * 'D' - Debug MAF on
 * 'd' - Debug MAF off
 * 'v' - System voltage
 * 'm' - Return MAF sensor voltage
 * 'b' - Return Baro sensor voltage
 * 'r' - Return reference pressure sensor voltage
 * 'h' - Return humidity sensor voltage
 * 't' - Return temperature sensor voltage
 * Calibration commands
 * 'L' - Perform Leak test calibration [+return ok/nok]
 * 'l' - Perform leak test [+return ok/nok]
 * 'O' - Perform offset calibration [+return ok/nok]
 * 
 *  Response anatomy:
 *  API Response format 'V:1.1.20080705:48853'
 *  Response Code:  'V'
 *  Delimiter:      ':' 
 *  Response:       '2.1.20080705'
 *  Delimiter:      ':' 
 *  CRC Checksum:   '48853'  
 *  
 ***/
void API::ParseMessage(char apiMessage) {

  extern struct ConfigSettings config;
  extern struct DeviceStatus status;
  
  Maths _maths;
  Messages _message;
  Hardware _hardware;
  Webserver _webserver;

  char serialResponse[30];
  double flowCFM = 0.01;


  switch (apiMessage)
  {
      case 'C': // Test Checksum - somewhere to test custom responses
          status.statusMessage = String("V") + config.api_delim + "2" + "." + MAJOR_VERSION + "." + BUILD_NUMBER;
      break;
      

      case 'V': // Get Version 'VMmYYMMDDXX\r\n'
          status.statusMessage = String("V") + config.api_delim + MAJOR_VERSION + "." + MINOR_VERSION + "." + BUILD_NUMBER;
      break;

      case 'L': // Perform Leak Test Calibration 'L\r\n'
// TODO:          status.statusMessage = String("L") + config.api_delim + leakTestCalibration();
          // TODO: confirm Leak Test Calibration success in response
      break;

      case 'l': // Perform Leak Test 'l\r\n'      
// TODO:         status.statusMessage = String("l") + config.api_delim + leakTest();
          // TODO: confirm Leak Test success in response
      break;

      case 'O': // Flow Offset Calibration  'O\r\n'        
// TODO:          setCalibrationOffset();
// TODO:          calibration.flow_offset
          status.statusMessage = String("O") + config.api_delim + config.cal_offset;
          // TODO: confirm Flow Offset Calibration success in response
      break;

      case 'F': // Get measured Flow 'F123.45\r\n'
          status.statusMessage = String("F") + config.api_delim ;        
          // Truncate to 2 decimal places
          flowCFM = _maths.calculateMafFlowCFM() * 100;
          status.statusMessage += flowCFM / 100;
      break;

      case 'M': // Get MAF sensor data'
          status.statusMessage = String("M") + config.api_delim ;        
          if (streamMafData == false) {
              streamMafData = true;
              _maths.calculateMafFlowCFM();
              streamMafData = false;         
          }
      break;

      case 'm': // Get MAF output voltage'
          status.statusMessage = String("m") + config.api_delim + ((analogRead(MAF_PIN) * (5.0 / 1024.0)) * 1000);        
      break;

      case 'T': // Get measured Temperature 'T.123.45\r\n'
          status.statusMessage = String("T") + config.api_delim + _maths.calculateTemperature(DEGC);
      break;

      case 't': // Get Temperature sensor output voltage'
          status.statusMessage = String("t") + config.api_delim + ((analogRead(TEMPERATURE_PIN) * (5.0 / 1024.0)) * 1000);
      break;

      case 'H': // Get measured Humidity 'H.123.45\r\n'
          status.statusMessage = String("H") + config.api_delim + _maths.calculateRelativeHumidity(PERCENT);
      break;

      case 'h': // Get Humidity sensor output voltage'
          status.statusMessage = String("h") + config.api_delim + ((analogRead(HUMIDITY_PIN) * (5.0 / 1024.0)) * 1000);
      break;

      case 'R': // Get measured Reference Pressure 'R.123.45\r\n'
          status.statusMessage = String("R") + config.api_delim + _maths.calculateRefPressure(KPA);
      break;

      case 'r': // Get Reference Pressure sensor output voltage'
          status.statusMessage = String("r") + config.api_delim + ((analogRead(REF_PRESSURE_PIN) * (5.0 / 1024.0)) * 1000);
      break;

      case 'B': // Get measured Baro Pressure 'B.123.45\r\n'
          status.statusMessage = String("B") + config.api_delim + _maths.calculateBaroPressure(KPA);
      break;

      case 'b': // Get Baro Pressure sensor output voltage'
          status.statusMessage = String("b") + config.api_delim + ((analogRead(REF_BARO_PIN) * (5.0 / 1024.0)) * 1000);
      break;

      case 'v': // Get board supply voltage (mv) 'v.123.45\r\n'
          status.statusMessage = String("v") + config.api_delim + _hardware.getSupplyMillivolts();
      break;
      
      case 'D': // DEBUG MAF'
          status.statusMessage = String("D") + config.api_delim ;
          streamMafData = true;
      break;

      case 'd': // DEBUG OFF'
          status.statusMessage = String("d") + config.api_delim;
          streamMafData = false;
      break;

      case 'E': // Enum - Flow:Ref:Temp:Humidity:Baro
          // Flow
          status.statusMessage = String("E") + config.api_delim ;        
          // Truncate to 2 decimal places
          flowCFM = _maths.calculateMafFlowCFM() * 100;
          status.statusMessage += (flowCFM / 100) + String(config.api_delim);
          // Reference Pressure
          status.statusMessage += _maths.calculateRefPressure(KPA) + String(config.api_delim);
          // Temperature
          status.statusMessage += _maths.calculateTemperature(DEGC) + String(config.api_delim);
          // Humidity
          status.statusMessage += _maths.calculateRelativeHumidity(PERCENT) + String(config.api_delim);
          // Barometric Pressure
          status.statusMessage += _maths.calculateBaroPressure(KPA);
      break;

      case 'I': // IP Address
          status.statusMessage = String("I") + config.api_delim + status.local_ip_address;
      break;

      case 'N': // Hostname
          status.statusMessage = String("I") + config.api_delim + config.hostname;
      break;

      case 'S': // WiFi SSID
          if (status.apMode == true) {
            status.statusMessage = String("I") + config.api_delim + config.wifi_ap_ssid;            
          } else {
            status.statusMessage = String("I") + config.api_delim + config.wifi_ssid;            
          }
      break;

      case 'J': // JSON Data
          status.statusMessage = String("J") + config.api_delim + _webserver.getDataJSON();
      break;


      // We've got here without a valid API request so lets get outta here before we send garbage to the serial comms
      default:
          return;
      break;

      
  }

  // Append delimiter to message data
  status.statusMessage += config.api_delim ;

  // Convert message data to char array for CRC function
  status.statusMessage.toCharArray(serialResponse, sizeof(serialResponse));

  // Send API Response
  #if defined DISABLE_API_CHECKSUM
        _message.DebugPrint(status.statusMessage + "\r\n");
        _message.Handler(status.statusMessage + "\r\n");
  #else
        _message.DebugPrint(status.statusMessage + calcCRC(serialResponse) + "\r\n");
        _message.Handler(status.statusMessage + calcCRC(serialResponse) +  "\r\n");
  #endif

}
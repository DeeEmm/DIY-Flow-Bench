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



/****************************************
 * DECLARE CONSTANTS
 ***/
// standard units
extern const int INWG; 
extern const int KPA; 
extern const int PSIA; 
extern const int DEGC; 
extern const int DEGF; 
extern const int RANKINE; 
extern const int PERCENT; 
extern const int BAR; 




 /****************************************
 * DECLARE GLOBALS
 ***/
extern bool streamMafData;
extern float getRelativeHumidity(int);
extern String localIpAddress;
extern bool apMode;


// extern int MAF_PIN;
// extern int TEMPERATURE_PIN;
// extern int HUMIDITY_PIN;
// extern int REF_PRESSURE_PIN;
// extern int REF_BARO_PIN;
// extern int VOLTAGE_PIN;
// extern int PITOT_PIN;




/****************************************
 * CREATE CHECKSUM
 *
 * Source: https://forum.arduino.cc/index.php?topic=311293.msg2158081#msg2158081
 * Usage: myVar = calcCRC(str);
 *
 ***/
uint16_t calcCRC(char* str) {
    // Initialise CRC
    uint16_t CRC = 0; 
    // Traverse each character in the string
    for (int i=0;i<strlen(str);i++) {
        // TODO update the CRC value using the built in CRC32 function
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
void parseAPI(char serialData)
{

    String messageData;
    char serialResponse[30];
    double flowCFM = 0.01;

    extern ConfigSettings config;

    switch (serialData)
    {
        case 'C': // Test Checksum - somewhere to test custom responses
            messageData = String("V") + config.api_delim + "2" + "." + MAJOR_VERSION + "." + BUILD_NUMBER;
        break;
        

        case 'V': // Get Version 'VMmYYMMDDXX\r\n'
            messageData = String("V") + config.api_delim + MAJOR_VERSION + "." + MINOR_VERSION + "." + BUILD_NUMBER;
        break;

        case 'L': // Perform Leak Test Calibration 'L\r\n'
            messageData = String("L") + config.api_delim + leakTestCalibration();
            // TODO confirm Leak Test Calibration success in response
        break;

        case 'l': // Perform Leak Test 'l\r\n'      
            messageData = String("l") + config.api_delim + leakTest();
            // TODO confirm Leak Test success in response
        break;

        case 'O': // Flow Offset Calibration  'O\r\n'        
            messageData = String("O") + config.api_delim + setCalibrationOffset();
            // TODO confirm Flow Offset Calibration success in response
        break;

        case 'F': // Get measured Flow 'F123.45\r\n'
            messageData = String("F") + config.api_delim ;        
            // Truncate to 2 decimal places
            flowCFM = getMafFlowCFM() * 100;
            messageData += flowCFM / 100;
        break;

        case 'M': // Get MAF sensor data'
            messageData = String("M") + config.api_delim ;        
            if (streamMafData == false) {
                streamMafData = true;
                getMafFlowCFM();
                streamMafData = false;         
            }
        break;

        case 'm': // Get MAF output voltage'
            messageData = String("m") + config.api_delim + ((analogRead(MAF_PIN) * (5.0 / 1024.0)) * 1000);        
        break;

        case 'T': // Get measured Temperature 'T.123.45\r\n'
            messageData = String("T") + config.api_delim + getTemp(DEGC);
        break;

        case 't': // Get Temperature sensor output voltage'
            messageData = String("t") + config.api_delim + ((analogRead(TEMPERATURE_PIN) * (5.0 / 1024.0)) * 1000);
        break;

        case 'H': // Get measured Humidity 'H.123.45\r\n'
            messageData = String("H") + config.api_delim + getRelativeHumidity(PERCENT);
        break;

        case 'h': // Get Humidity sensor output voltage'
            messageData = String("h") + config.api_delim + ((analogRead(HUMIDITY_PIN) * (5.0 / 1024.0)) * 1000);
        break;

        case 'R': // Get measured Reference Pressure 'R.123.45\r\n'
            messageData = String("R") + config.api_delim + getRefPressure(KPA);
        break;

        case 'r': // Get Reference Pressure sensor output voltage'
            messageData = String("r") + config.api_delim + ((analogRead(REF_PRESSURE_PIN) * (5.0 / 1024.0)) * 1000);
        break;

        case 'B': // Get measured Baro Pressure 'B.123.45\r\n'
            messageData = String("B") + config.api_delim + getBaroPressure(KPA);
        break;

        case 'b': // Get Baro Pressure sensor output voltage'
            messageData = String("b") + config.api_delim + ((analogRead(REF_BARO_PIN) * (5.0 / 1024.0)) * 1000);
        break;

        case 'v': // Get board supply voltage (mv) 'v.123.45\r\n'
            messageData = String("v") + config.api_delim + getSupplyMillivolts();
        break;
        
        case 'D': // DEBUG MAF'
            messageData = String("D") + config.api_delim ;
            streamMafData = true;
        break;

        case 'd': // DEBUG OFF'
            messageData = String("d") + config.api_delim;
            streamMafData = false;
        break;

        case 'E': // Enum - Flow:Ref:Temp:Humidity:Baro
            // Flow
            messageData = String("E") + config.api_delim ;        
            // Truncate to 2 decimal places
            flowCFM = getMafFlowCFM() * 100;
            messageData += (flowCFM / 100) + String(config.api_delim);
            // Reference Pressure
            messageData += getRefPressure(KPA) + String(config.api_delim);
            // Temperature
            messageData += getTemp(DEGC) + String(config.api_delim);
            // Humidity
            messageData += getRelativeHumidity(PERCENT) + String(config.api_delim);
            // Barometric Pressure
            messageData += getBaroPressure(KPA);
        break;

        case 'I': // IP Address
            messageData = String("I") + config.api_delim + localIpAddress;
        break;

        case 'N': // Hostname
            messageData = String("I") + config.api_delim + config.hostname;
        break;

        case 'S': // WiFi SSID
            if (apMode == true) {
              messageData = String("I") + config.api_delim + config.wifi_ap_ssid;            
            } else {
              messageData = String("I") + config.api_delim + config.wifi_ssid;            
            }
        break;

        case 'J': // JSON Data
            messageData = String("J") + config.api_delim + getDataJson();
        break;


        // We've got here without a valid API request so lets get outta here before we send garbage to the serial comms
        default:
            return;
        break;

        
    }

    // Append delimiter to message data
    messageData += config.api_delim ;

    // Convert message data to char array for CRC function
    messageData.toCharArray(serialResponse, sizeof(serialResponse));

    // Send API Response
    #if defined DISABLE_API_CHECKSUM
//        Serial.print(messageData + "\r\n");
        sendSerial(messageData + "\r\n");
    #else
//        Serial.print(messageData + calcCRC(serialResponse) + "\r\n");
//        sendSerial(messageData + calcCRC(serialResponse) +  "\r\n");
    #endif


}
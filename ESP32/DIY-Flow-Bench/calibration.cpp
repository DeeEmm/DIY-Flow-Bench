/***********************************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * Calibration.cpp - Calibration class
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

#include "calibration.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "configuration.h"
#include "constants.h"
#include "structs.h"
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
#include "webserver.h"
#include LANGUAGE_FILE


Calibration::Calibration () {

}



/***********************************************************
* Perform Flow Calibration
*
* Called from API.ino
* Called from processWebSocketMessage in server.ino
*
* TODO: #75 The calibration handling needs to be rewritten. https://github.com/DeeEmm/DIY-Flow-Bench/issues/75
* 
***/
bool Calibration::setFlowOffset() {

  extern struct CalibrationSettings calibration;
  extern struct ConfigSettings config;
  
  Sensors _sensors; 
  Calculations _calculations;
  
  // Get the current flow value
  // TODO: need to determine what flow sensors are active (MAF / Orifice / Pitot)
  float MafFlowCFM = _calculations.calculateFlowCFM();
  // Get the current reference pressure
  float RefPressure = _calculations.convertPressure(_sensors.getPRefValue(), INWG);
  
  // convert the calibration orifice flow value  to our current ref pressure  
  float convertedOrificeFlowCFM = _calculations.convertFlowDepression(config.cal_ref_press, RefPressure,  config.cal_flow_rate);
 
  // compare it to the measured flow to generate our flow offset
  float flowCalibrationOffset = convertedOrificeFlowCFM - MafFlowCFM;
  
  // update config var
  calibration.flow_offset = flowCalibrationOffset;
  
  // TODO: Save cal.json
  saveCalibration();    
  
  return true;
  
}



/***********************************************************
* Get Flow offset
* 
***/
float Calibration::getFlowOffset() {

  extern struct CalibrationSettings calibration;
  
  loadCalibration();

  return calibration.flow_offset;

}






/***********************************************************
* Perform leakTestCalibration
***/
bool Calibration::setLeakTestPressure() {
  
  extern struct CalibrationSettings calibration;
  Calculations _calculations;
  Sensors _sensors;
  
  calibration.leak_cal_val = _calculations.convertPressure(_sensors.getPRefValue(), INWG);  

  return true;
}



/***********************************************************
* Get leakTest
***/
float Calibration::getLeakTestPressure() {

  extern struct CalibrationSettings calibration;
  
  loadCalibration();
  
  return calibration.leak_cal_val;

}




/***********************************************************
* createConfig
* 
* Create configuration json file
* Called from Webserver::Initialise() if config.json not found
***/
void Calibration::createCalibrationFile () {

  extern struct CalibrationSettings calibration;
  Webserver _webserver;
  Messages _message;
  String jsonString;
  StaticJsonDocument<1024> calibrationData;
  
  _message.statusPrintf((char*)"Creating cal.json file... \n"); 
  
  calibrationData["FLOW_OFFSET"] = calibration.flow_offset;
  calibrationData["LEAK_CAL_VAL"] = calibration.leak_cal_val;
  
  serializeJsonPretty(calibrationData, jsonString);
  _webserver.writeJSONFile(jsonString, "/cal.json");
  
}



/***********************************************************
* saveCalibration 
* write calibration data to cal.json file
***/
void Calibration::saveCalibration() {
  
  extern struct CalibrationSettings calibration;
  extern struct translator translate;
  
  Messages _message;
  Webserver _webserver;
  
  String jsonString;
  StaticJsonDocument<1024> calibrationData;
    
  calibrationData["flow_offset"] = calibration.flow_offset; 
  calibrationData["leak_cal_val"] = calibration.leak_cal_val; 

  _message.Handler(translate.LANG_VAL_SAVING_CALIBRATION);
  
  serializeJsonPretty(calibrationData, jsonString);
  
  _webserver.writeJSONFile(jsonString, "/cal.json");

}




/***********************************************************
* Parse Calibration Data
***/
void Calibration::parseCalibrationData(StaticJsonDocument<1024> calibrationData) {

  extern struct CalibrationSettings calibration;  
  Messages _message;
  _message.statusPrintf((char*)"Calibration::parseCalibrationData \n");
  
  calibration.flow_offset = calibrationData["FLOW_OFFSET"].as<float>();
  calibration.leak_cal_val = calibrationData["LEAK_CAL_VAL"].as<float>();
  
}



/***********************************************************
* loadCalibration
* Read calibration data from cal.json file
***/
StaticJsonDocument<1024> Calibration::loadCalibration () {

  Webserver _webserver;
  Messages _message;
  _message.statusPrintf((char*)"Calibration::loadCalibration \n");
  
  StaticJsonDocument<1024> calibrationData;
  calibrationData = _webserver.loadJSONFile("/cal.json");
  parseCalibrationData(calibrationData);
  return calibrationData;
}

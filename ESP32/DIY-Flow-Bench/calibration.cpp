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
#include "maths.h"
#include "messages.h"
#include "webserver.h"
#include LANGUAGE_FILE


Calibration::Calibration () {

}





/***********************************************************
* Perform Flow Calibration
*
* Called from API.ino
* Called from ProcessWebSocketMessage in server.ino
*
* TODO: #75 The calibration handling needs to be rewritten. https://github.com/DeeEmm/DIY-Flow-Bench/issues/75
* 
***/
bool Calibration::setFlowOffset() {

  extern struct CalibrationSettings calibration;
  extern struct ConfigSettings config;
  
  Sensors _sensor; 
  Maths _maths;
  Webserver _webserver;
  
  // Get the current flow value
  // TODO: need to determine what flow sensors are active (MAF / Orifice / Pitot)
  float MafFlowCFM = _maths.calculateMafFlowCFM();
  // Get the current reference pressure
  float RefPressure = _maths.calculateRefPressure(INWG);
  
  // convert the calibration orifice flow value  to our current ref pressure  
  float convertedOrificeFlowCFM = _maths.convertFlowDepression(config.cal_ref_press, RefPressure,  config.cal_flow_rate);
 
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
  Maths _maths;
  
  calibration.leak_test = _maths.calculateRefPressure(INWG);  

  return true;
}



/***********************************************************
* Get leakTest
***/
float Calibration::getLeakTestPressure() {

  extern struct CalibrationSettings calibration;
  
  loadCalibration();
  
  return calibration.leak_test;

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
  
  _message.DebugPrint("Creating cal.json file..."); 
  
  calibrationData["FLOW_OFFSET"] = calibration.flow_offset;
  calibrationData["LEAK_TEST"] = calibration.leak_test;
  
  serializeJsonPretty(calibrationData, jsonString);
  _webserver.writeJSONFile(jsonString, "/cal.json");
  
}



/***********************************************************
* saveCalibration 
* write calibration data to cal.json file
***/
void Calibration::saveCalibration() {
  
  extern struct CalibrationSettings calibration;
  
  Messages _message;
  Webserver _webserver;
  
  String jsonString;
  StaticJsonDocument<1024> calibrationData;
    
  calibrationData["flow_offset"] = calibration.flow_offset; 
  calibrationData["leak_test"] = calibration.leak_test; 

  _message.Handler(LANG_SAVING_CALIBRATION);
  
  serializeJsonPretty(calibrationData, jsonString);
  
  _webserver.writeJSONFile(jsonString, "/cal.json");

}




/***********************************************************
* Parse Calibration Data
***/
void Calibration::parseCalibrationData(StaticJsonDocument<1024>  calibrationData) {

  extern struct CalibrationSettings calibration;  
  
  calibration.flow_offset = calibrationData["FLOW_OFFSET"].as<float>();
  calibration.leak_test = calibrationData["LEAK_TEST"].as<float>();
  
}



/***********************************************************
* loadCalibration
* Read calibration data from cal.json file
***/
void Calibration::loadCalibration () {

  Webserver _webserver;
  
  StaticJsonDocument<1024> calibrationData;
  
  calibrationData = _webserver.loadJSONFile("/cal.json");
  
  parseCalibrationData(calibrationData);

}



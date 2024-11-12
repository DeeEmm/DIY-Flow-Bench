/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file calibration.app
 * 
 * @brief Calibration class
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/

#include "configuration.h"
#include "constants.h"
#include "structs.h"

#include "calibration.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
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

  extern struct ConfigSettings config;
  extern struct CalibrationData calVal;
  extern struct Translator translate;
  extern struct SensorData sensorVal;
  
  Sensors _sensors; 
  Calculations _calculations;
  Messages _message;
 
  // update config var
  calVal.flow_offset = sensorVal.FlowCFM - config.cal_flow_rate;
  
  _message.debugPrintf("Calibration::setFlowOffset $ \n", calVal.flow_offset);

  saveCalibrationData();    

  _message.Handler(translate.LANG_CAL_OFFET_VALUE + calVal.flow_offset);
  
  return true;
  
}



/***********************************************************
* Get Flow offset
* 
***/
double Calibration::getFlowOffset() {

  extern struct CalibrationData calVal;
  
  loadCalibrationFile();

  return calVal.flow_offset;

}



/***********************************************************
* Perform Leak Offset Calibration
***/
bool Calibration::setLeakOffset() {
  
  Calculations _calculations;
  Sensors _sensors;
  Messages _message;

  extern struct CalibrationData calVal;
  extern struct Translator translate;
  extern struct SensorData sensorVal;

  // load current calibration data
  loadCalibrationData();

  _message.debugPrintf("Calibration::setLeakTest \n");

  //TODO - Reverse flow calibration. Need to understand reverse flow characetitics of sensor
  // Assumed negative flow = -ve value BUT suspect that sensor -> ADC may need to be wired as differential
  // Possibility that pressure sensor circuits on PCB may need to be redesigned?!?!?!

  // De we have Pressure or Vacuum | Forwards or Reverse Flow?
  // if (_calculations.convertPressure(_sensors.getPRefValue(), INH2O) > 0) {
  //   calVal.leak_cal_offset_rev = _calculations.convertPressure(_sensors.getPRefValue(), INH2O);  
  //   _message.Handler(translate.LANG_LEAK_CAL_VALUE + calVal.leak_cal_offset_rev);
  // } else {
    // calVal.leak_cal_offset = _calculations.convertPressure(_sensors.getPRefValue(), INH2O);  
    // _message.Handler(translate.LANG_LEAK_CAL_VALUE + calVal.leak_cal_offset);
  // }

    calVal.leak_cal_offset = sensorVal.FlowCFM;
    _message.Handler(translate.LANG_LEAK_CAL_VALUE + calVal.leak_cal_offset);

  saveCalibrationData();    
  
  return true;
}





/***********************************************************
* Get leakTest
***/
double Calibration::getLeakOffset() {

  extern struct CalibrationData calVal;
  loadCalibrationFile();
  
  return calVal.leak_cal_offset;

}




/***********************************************************
* Get leakTest
***/
double Calibration::getLeakOffsetReverse() {

  extern struct CalibrationData calVal;
  
  loadCalibrationFile();
  
  return calVal.leak_cal_offset_rev;

}





/***********************************************************
* @brief createCalibration File
* @details Create configuration json file
* @note Called from Webserver::Initialise() if config.json not found
***/
void Calibration::createCalibrationFile () {

  extern struct CalibrationData calVal;
  Webserver _webserver;
  Messages _message;
  String jsonString;
  StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;
  
  _message.debugPrintf("Creating cal.json file... \n"); 
  
  calData["FLOW_OFFSET"] = calVal.flow_offset;
  calData["USER_OFFSET"] = calVal.user_offset;
  calData["LEAK_CAL_BASELINE"] = calVal.leak_cal_baseline;
  calData["LEAK_CAL_BASELINE_REV"] = calVal.leak_cal_baseline_rev;
  calData["LEAK_CAL_OFFSET"] = calVal.leak_cal_offset;
  calData["LEAK_CAL_OFFSET_REV"] = calVal.leak_cal_offset_rev;

  serializeJsonPretty(calData, jsonString);

  File outputFile = SPIFFS.open("/cal.json", FILE_WRITE);
  serializeJsonPretty(calData, outputFile);
  outputFile.close();
  
}





/***********************************************************
* @brief saveCalibration 
* @details write calibration data to cal.json file
***/
void Calibration::saveCalibrationData() {
  
  Messages _message;

  Webserver _webserver;
  String jsonString;
  StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;

  extern struct CalibrationData calVal;
  extern struct Translator translate;

  _message.debugPrintf("Writing to cal.json file... \n");
    
  // Populate JSON
  calData["FLOW_OFFSET"] = calVal.flow_offset;
  calData["USER_OFFSET"] = calVal.user_offset;
  calData["LEAK_CAL_BASELINE"] = calVal.leak_cal_baseline;
  calData["LEAK_CAL_BASELINE_REV"] = calVal.leak_cal_baseline_rev;
  calData["LEAK_CAL_OFFSET"] = calVal.leak_cal_offset;
  calData["LEAK_CAL_OFFSET_REV"] = calVal.leak_cal_offset_rev;

  _message.Handler(translate.LANG_SAVING_CALIBRATION);
  
  serializeJsonPretty(calData, jsonString);

  if (SPIFFS.exists("/cal.json"))  {
    SPIFFS.remove("/cal.json");
  }
  File outputFile = SPIFFS.open("/cal.json", FILE_WRITE);
  serializeJsonPretty(calData, outputFile);
  outputFile.close();
  
  _webserver.writeJSONFile(jsonString, "/cal.json", CAL_DATA_JSON_SIZE);

  _message.debugPrintf("Calibration Saved \n");


}




/***********************************************************
* loadCalibration
* Read calibration data from cal.json file
***/
StaticJsonDocument<1024> Calibration::loadCalibrationData () {

  Webserver _webserver;
  Messages _message;
  _message.debugPrintf("Calibration::loadCalibration \n");
  
  StaticJsonDocument<1024> calibrationData;
  calibrationData = _webserver.loadJSONFile("/cal.json");
  parseCalibrationData(calibrationData);
  return calibrationData;
}





/***********************************************************
* @brief Parse Calibration Data
* @param calibrationData JSON document containing calibration data
***/
void Calibration::parseCalibrationData(StaticJsonDocument<1024> calData) {

  extern struct CalibrationData calVal;

  calVal.flow_offset = calData["FLOW_OFFSET"];
  calVal.user_offset = calData["USER_OFFSET"];
  calVal.leak_cal_baseline = calData["LEAK_CAL_BASELINE"];
  calVal.leak_cal_baseline_rev = calData["LEAK_CAL_BASELINE_REV"];
  calVal.leak_cal_offset = calData["LEAK_CAL_OFFSET"];
  calVal.leak_cal_offset_rev = calData["LEAK_CAL_OFFSET_REV"];
}




// TODO if calibration flow is substantially less than calibrated orifice flow then vac source not enough do we need to test for this?????????





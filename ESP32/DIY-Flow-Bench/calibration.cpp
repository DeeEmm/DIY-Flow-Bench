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
#include "datahandler.h"

#include "calibration.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
#include "webserver.h"
// // #include LANGUAGE_FILE


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
  extern struct Language language;
  extern struct SensorData sensorVal;
  
  Sensors _sensors; 
  Calculations _calculations;
  Messages _message;
 
  // update config var
  calVal.flow_offset = sensorVal.FlowCFM - config.cal_flow_rate;
  
  _message.debugPrintf("Calibration::setFlowOffset $ \n", calVal.flow_offset);

  saveCalibrationData();    

  // _message.Handler(language.LANG_CAL_OFFET_VALUE + calVal.flow_offset);
  
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
  DataHandler _data;



  extern struct CalibrationData calVal;
  extern struct Language language;
  extern struct SensorData sensorVal;

  // load current calibration data
  _data.loadCalibrationData();

  _message.debugPrintf("Calibration::setLeakTest \n");

  //TODO - Reverse flow calibration. Need to understand reverse flow characetitics of sensor
  // Assumed negative flow = -ve value BUT suspect that sensor -> ADC may need to be wired as differential
  // Possibility that pressure sensor circuits on PCB may need to be redesigned?!?!?!

  // De we have Pressure or Vacuum | Forwards or Reverse Flow?
  // if (_calculations.convertPressure(_sensors.getPRefValue(), INH2O) > 0) {
  //   calVal.leak_cal_offset_rev = _calculations.convertPressure(_sensors.getPRefValue(), INH2O);  
  //   _message.Handler(language.LANG_LEAK_CAL_VALUE + calVal.leak_cal_offset_rev);
  // } else {
    // calVal.leak_cal_offset = _calculations.convertPressure(_sensors.getPRefValue(), INH2O);  
    // _message.Handler(language.LANG_LEAK_CAL_VALUE + calVal.leak_cal_offset);
  // }

    calVal.leak_cal_offset = sensorVal.FlowCFM;
    // _message.Handler(language.LANG_LEAK_CAL_VALUE + calVal.leak_cal_offset);

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




// TODO - #205 convert from pre-compile to post-compile 
// /***********************************************************
// * @brief createCalibration File
// * @details Create configuration json file
// * @note Called from Webserver::Initialise() if config.json not found
// ***/
// void Calibration::createCalibrationFile () {

//   extern struct CalibrationData calVal;
//   Webserver _webserver;
//   Messages _message;
//   String jsonString;
//   StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;
  
//   _message.debugPrintf("Creating cal.json file... \n"); 
  
//   calData["FLOW_OFFSET"] = calVal.flow_offset;
//   calData["USER_OFFSET"] = calVal.user_offset;
//   calData["LEAK_CAL_BASELINE"] = calVal.leak_cal_baseline;
//   calData["LEAK_CAL_BASELINE_REV"] = calVal.leak_cal_baseline_rev;
//   calData["LEAK_CAL_OFFSET"] = calVal.leak_cal_offset;
//   calData["LEAK_CAL_OFFSET_REV"] = calVal.leak_cal_offset_rev;

//   serializeJsonPretty(calData, jsonString);

//   File outputFile = SPIFFS.open("/cal.json", FILE_WRITE);
//   serializeJsonPretty(calData, outputFile);
//   outputFile.close();
  
// }







/***********************************************************
* Zero pDiff value
* 
***/
bool Calibration::setPdiffCalOffset() {

  extern struct ConfigSettings config;
  extern struct CalibrationData calVal;
  extern struct Language language;
  extern struct SensorData sensorVal;
  extern struct DeviceStatus status;
  
  Sensors _sensors; 
  Calculations _calculations;
  Messages _message;
 
 
  if (calVal.pdiff_cal_offset == 0) {
    // update config var
    calVal.pdiff_cal_offset = sensorVal.PDiffH2O;
    _message.debugPrintf("Calibration::setPdiffOffset %d \n", calVal.pdiff_cal_offset);
  } else {
    // update config var
    calVal.pdiff_cal_offset = 0.0;
    _message.debugPrintf("Calibration::resetPdiffOffset %d \n", 0.0);
  }

 
 
 
 
  saveCalibrationData();    

  // _message.Handler(language.LANG_CAL_OFFET_VALUE + calVal.flow_offset);
  
  return true;
  
}



/***********************************************************
* Get pDiff offset
* 
***/
double Calibration::getPdiffCalOffset() {

  extern struct CalibrationData calVal;
  
  loadCalibrationFile();

  return calVal.pdiff_cal_offset;

}





/***********************************************************
* Zero Pitot value
*  
***/
bool Calibration::setPitotCalOffset() {

  extern struct ConfigSettings config;
  extern struct CalibrationData calVal;
  extern struct Language language;
  extern struct SensorData sensorVal;
  extern struct DeviceStatus status;
  
  Sensors _sensors; 
  Calculations _calculations;
  Messages _message;
 

  if (calVal.pitot_cal_offset == 0) {
    // update config var
    calVal.pitot_cal_offset = sensorVal.PitotKPA;
    _message.debugPrintf("Calibration::setPitotDeltaOffset %d kPa\n", calVal.pitot_cal_offset);
  } else {
    // update config var
    calVal.pitot_cal_offset = 0.0;
    _message.debugPrintf("Calibration::resetPitotDeltaOffset %d \n", 0.0);
  }

  saveCalibrationData();    

  // _message.Handler(language.LANG_CAL_OFFET_VALUE + calVal.flow_offset);
  
  return true;
  
}



/***********************************************************
* Get pDiff offset
* 
***/
double Calibration::getPitotCalOffset() {

  extern struct CalibrationData calVal;
  
  loadCalibrationFile();

  return calVal.pdiff_cal_offset;

}




/***********************************************************
* @brief saveCalibration 
* @details write calibration data to cal.json file
***/
void Calibration::saveCalibrationData() {
  
  Messages _message;

  DataHandler _data;
  String jsonString;
  StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;

  extern struct CalibrationData calVal;
  extern struct Language language;

  _message.debugPrintf("Writing to cal.json file... \n");
    
  // Populate JSON
  calData["FLOW_OFFSET"] = calVal.flow_offset;
  calData["USER_OFFSET"] = calVal.user_offset;
  calData["LEAK_CAL_BASELINE"] = calVal.leak_cal_baseline;
  calData["LEAK_CAL_BASELINE_REV"] = calVal.leak_cal_baseline_rev;
  calData["LEAK_CAL_OFFSET"] = calVal.leak_cal_offset;
  calData["LEAK_CAL_OFFSET_REV"] = calVal.leak_cal_offset_rev;
  calData["PDIFF_CAL_OFFSET"] = calVal.pitot_cal_offset;
  calData["PITOT_CAL_OFFSET"] = calVal.pdiff_cal_offset;

  _message.Handler(language.LANG_SAVING_CALIBRATION);
  
  serializeJsonPretty(calData, jsonString);

  if (SPIFFS.exists("/cal.json"))  {
    SPIFFS.remove("/cal.json");
  }
  File outputFile = SPIFFS.open("/cal.json", FILE_WRITE);
  serializeJsonPretty(calData, outputFile);
  outputFile.close();
  
  _data.writeJSONFile(jsonString, "/cal.json", CAL_DATA_JSON_SIZE);

  _message.debugPrintf("Calibration Saved \n");


}






// TODO if calibration flow is substantially less than calibrated orifice flow then vac source not enough do we need to test for this?????????





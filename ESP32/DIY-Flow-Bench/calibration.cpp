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

#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include "constants.h"
#include "structs.h"
#include "datahandler.h"

#include "calibration.h"
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
#include "webserver.h"


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

  extern struct BenchSettings settings;
  extern struct CalibrationData calVal;
  extern struct Language language;
  extern struct SensorData sensorVal;
  
  Sensors _sensors; 
  Calculations _calculations;
  Messages _message;
 
  // update config var
  calVal.flow_offset = sensorVal.FlowCFM - settings.cal_flow_rate;
  
  _message.debugPrintf("Calibration::setFlowOffset $ \n", calVal.flow_offset);

  saveCalibrationData();    

  // _message.Handler(language.LANG_CAL_OFFSET_VAL + calVal.flow_offset);
  
  return true;
  
}



/***********************************************************
* Get Flow offset
* 
***/
double Calibration::getFlowOffset() {

  extern struct CalibrationData calVal;
  
  loadCalibrationData();

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
  this->loadCalibrationData();

  _message.debugPrintf("Calibration::setLeakTest \n");

  //TODO - Reverse flow calibration. Need to understand reverse flow characetitics of sensor
  // Assumed negative flow = -ve value BUT suspect that sensor -> ADC may need to be wired as differential
  // Possibility that pressure sensor circuits on PCB may need to be redesigned?!?!?!

  // De we have Pressure or Vacuum | Forwards or Reverse Flow?
  // if (_calculations.convertPressure(_sensors.getPRefValue(), INH2O) > 0) {
  //   calVal.leak_cal_offset_rev = _calculations.convertPressure(_sensors.getPRefValue(), INH2O);  
  //   _message.Handler(language.LANG_LEAK_CAL_VAL + calVal.leak_cal_offset_rev);
  // } else {
    // calVal.leak_cal_offset = _calculations.convertPressure(_sensors.getPRefValue(), INH2O);  
    // _message.Handler(language.LANG_LEAK_CAL_VAL + calVal.leak_cal_offset);
  // }

    calVal.leak_cal_offset = sensorVal.FlowCFM;
    // _message.Handler(language.LANG_LEAK_CAL_VAL + calVal.leak_cal_offset);

  saveCalibrationData();    
  
  return true;
}





/***********************************************************
* Get leakTest
***/
double Calibration::getLeakOffset() {

  extern struct CalibrationData calVal;
  loadCalibrationData();
  
  return calVal.leak_cal_offset;

}




/***********************************************************
* Get leakTest
***/
double Calibration::getLeakOffsetReverse() {

  extern struct CalibrationData calVal;
  
  loadCalibrationData();
  
  return calVal.leak_cal_offset_rev;

}










/***********************************************************
* Zero pDiff value
* 
***/
bool Calibration::setPdiffCalOffset() {

  extern struct BenchSettings settings;
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

  // _message.Handler(language.LANG_CAL_OFFSET_VAL + calVal.flow_offset);
  
  return true;
  
}



/***********************************************************
* Get pDiff offset
* 
***/
double Calibration::getPdiffCalOffset() {

  extern struct CalibrationData calVal;
  
  loadCalibrationData();

  return calVal.pdiff_cal_offset;

}





/***********************************************************
* Zero Pitot value
*  
***/
bool Calibration::setPitotCalOffset() {

  extern struct BenchSettings settings;
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

  // _message.Handler(language.LANG_CAL_OFFSET_VAL + calVal.flow_offset);
  
  return true;
  
}



/***********************************************************
* Get pDiff offset
* 
***/
double Calibration::getPitotCalOffset() {

  extern struct CalibrationData calVal;
  
  loadCalibrationData();

  return calVal.pdiff_cal_offset;

}






/***********************************************************
* @brief initialiseLiftData
* @note - Initialise settings in NVM if they do not exist
* @note Key must be 15 chars or shorter.
***/ 
void Calibration::initialiseCalibrationData () {

  Messages _message;
  Preferences _prefs;

  _message.serialPrintf("Loading Bench Settings \n");    
  
  _prefs.begin("calibration");

  if (!_prefs.isKey("FLOW_OFFSET")) _prefs.putDouble("FLOW_OFFSET", 0.0);
  if (!_prefs.isKey("USER_OFFSET")) _prefs.putDouble("USER_OFFSET", 0.0);
  if (!_prefs.isKey("LEAK_BASE")) _prefs.putDouble("LEAK_BASE", 0.0);
  if (!_prefs.isKey("LEAK_BASE_REV")) _prefs.putDouble("LEAK_BASE_REV", 0.0);
  if (!_prefs.isKey("LEAK_OFFSET")) _prefs.putDouble("LEAK_OFFSET", 0.0);
  if (!_prefs.isKey("LEAK_OFFSET_REV")) _prefs.putDouble("LEAK_OFFSET_REV", 0.0);
  if (!_prefs.isKey("PDIFF_OFFSET")) _prefs.putDouble("PDIFF_OFFSET", 0.0);
  if (!_prefs.isKey("PITOT_OFFSET")) _prefs.putDouble("PITOT_OFFSET", 0.0);

  _prefs.end();
}





/***********************************************************
* @brief loadCalibration 
* @details load calibration data from NVM into struct
***/
void Calibration::loadCalibrationData() {
  
  Messages _message;
  Preferences _prefs;

  DataHandler _data;
  String jsonString;
  StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;

  extern struct CalibrationData calVal;
  extern struct Language language;

  _message.serialPrintf("Loading Calibration Data \n");    
  
  _prefs.begin("calibration");

  calVal.flow_offset = _prefs.getDouble("FLOW_OFFSET", 0.0);
  calVal.user_offset = _prefs.getDouble("USER_OFFSET", 0.0);
  calVal.leak_cal_baseline = _prefs.getDouble("LEAK_BASE", 0.0);
  calVal.leak_cal_baseline_rev = _prefs.getDouble("LEAK_BASE_REV", 0.0);
  calVal.leak_cal_offset = _prefs.getDouble("LEAK_OFFSET", 0.0);
  calVal.leak_cal_offset_rev = _prefs.getDouble("LEAK_OFFSET_REV", 0.0);
  calVal.pdiff_cal_offset = _prefs.getDouble("PDIFF_OFFSET", 0.0);
  calVal.pitot_cal_offset = _prefs.getDouble("PITOT_OFFSET", 0.0);

  _prefs.end();
}



/***********************************************************
* @brief saveCalibration 
* @details write calibration data to NVM
***/
void Calibration::saveCalibrationData() {
  
  Preferences _prefs;
  Messages _message;
  DataHandler _data;

  extern struct CalibrationData calVal;
  extern struct Language language;

  _message.Handler(language.LANG_SAVING_CALIBRATION);

  _prefs.begin("calibration");

  _prefs.putDouble("FLOW_OFFSET", calVal.flow_offset);
  _prefs.putDouble("USER_OFFSET", calVal.user_offset);
  _prefs.putDouble("LEAK_BASE", calVal.leak_cal_baseline);
  _prefs.putDouble("LEAK_BASE_REV", calVal.leak_cal_baseline_rev);
  _prefs.putDouble("LEAK_OFFSET", calVal.leak_cal_offset);
  _prefs.putDouble("LEAK_OFFSET_REV", calVal.leak_cal_offset_rev);
  _prefs.putDouble("PDIFF_OFFSET", calVal.pdiff_cal_offset);
  _prefs.putDouble("PITOT_OFFSET", calVal.pitot_cal_offset);
    
  _prefs.end();

}






// TODO #77 if calibration flow is substantially less than calibrated orifice flow then vac source not enough do we need to test for this?????????





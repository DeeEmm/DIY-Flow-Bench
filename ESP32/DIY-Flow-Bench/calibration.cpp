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

  double flowVal = 0.0f;

  // Get flow type based on currently visible tile
  switch (sensorVal.flowtile) {
    case MAFFLOW_TILE:
      flowVal = sensorVal.FlowCFM;
      // TODO ADD SYSTEM Warning !!!
    break;

    case ACFM_TILE:
      flowVal = sensorVal.FlowCFM;
    break;

    case ADJCFM_TILE:
      flowVal = sensorVal.FlowADJ;
    break;

    case SCFM_TILE:
      flowVal = sensorVal.FlowSCFM;
    break;

  }
 
  // update config var
  calVal.flow_offset = flowVal - calVal.cal_flow_rate;
  
  _message.debugPrintf("Calibration::setFlowOffset %d \n", calVal.flow_offset);

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

  if (!_prefs.isKey("dCAL_FLW_RATE")) _prefs.putDouble("dCAL_FLW_RATE", 14.4F);
  if (!_prefs.isKey("dCAL_REF_PRESS")) _prefs.putDouble("dCAL_REF_PRESS", 10.0F);
  if (!_prefs.isKey("dORIFICE1_FLOW")) _prefs.putDouble("dORIFICE1_FLOW", 0.0F);
  if (!_prefs.isKey("dORIFICE1_PRESS")) _prefs.putDouble("dORIFICE1_PRESS", 0.0F);
  if (!_prefs.isKey("dORIFICE2_FLOW")) _prefs.putDouble("dORIFICE2_FLOW", 0.0F);
  if (!_prefs.isKey("dORIFICE2_PRESS")) _prefs.putDouble("dORIFICE2_PRESS", 0.0F);
  if (!_prefs.isKey("dORIFICE3_FLOW")) _prefs.putDouble("dORIFICE3_FLOW", 0.0F);
  if (!_prefs.isKey("dORIFICE3_PRESS")) _prefs.putDouble("dORIFICE3_PRESS", 0.0F);
  if (!_prefs.isKey("dORIFICE4_FLOW")) _prefs.putDouble("dORIFICE4_FLOW", 0.0F);
  if (!_prefs.isKey("dORIFICE4_PRESS")) _prefs.putDouble("dORIFICE4_PRESS", 0.0F);
  if (!_prefs.isKey("dORIFICE5_FLOW")) _prefs.putDouble("dORIFICE5_FLOW", 0.0F);
  if (!_prefs.isKey("dORIFICE5_PRESS")) _prefs.putDouble("dORIFICE5_PRESS", 0.0F);
  if (!_prefs.isKey("dORIFICE6_FLOW")) _prefs.putDouble("dORIFICE6_FLOW", 0.0F);
  if (!_prefs.isKey("dORIFICE6_PRESS")) _prefs.putDouble("dORIFICE6_PRESS", 0.0F);

  _prefs.end();
}





/***********************************************************
* @brief loadCalibration 
* @details load calibration data from NVM into struct
***/
void Calibration::loadCalibrationData() {
  
  Messages _message;
  Preferences _prefs;

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

  calVal.cal_flow_rate = _prefs.getDouble("dCAL_FLW_RATE", 14.4F );
  calVal.cal_ref_press = _prefs.getDouble("dCAL_REF_PRESS", 10.0F );
  calVal.orificeOneFlow = _prefs.getDouble("dORIFICE1_FLOW", 0.0F );
  calVal.orificeOneDepression = _prefs.getDouble("dORIFICE1_PRESS", 0.0F );
  calVal.orificeTwoFlow = _prefs.getDouble("dORIFICE2_FLOW", 0.0F );
  calVal.orificeTwoDepression = _prefs.getDouble("dORIFICE2_PRESS", 0.0F );
  calVal.orificeThreeFlow = _prefs.getDouble("dORIFICE3_FLOW", 0.0F );
  calVal.orificeThreeDepression = _prefs.getDouble("dORIFICE3_PRESS", 0.0F );
  calVal.orificeFourFlow = _prefs.getDouble("dORIFICE4_FLOW", 0.0F );
  calVal.orificeFourDepression = _prefs.getDouble("dORIFICE4_PRESS", 0.0F );
  calVal.orificeFiveFlow = _prefs.getDouble("dORIFICE5_FLOW", 0.0F );
  calVal.orificeFiveDepression = _prefs.getDouble("dORIFICE5_PRESS", 0.0F );
  calVal.orificeSixFlow = _prefs.getDouble("dORIFICE6_FLOW", 0.0F );
  calVal.orificeSixDepression = _prefs.getDouble("dORIFICE6_PRESS",  0.0F);

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

  _message.Handler(language.LANG_SAVING_CALIBRATION);

}






// TODO #77 if calibration flow is substantially less than calibrated orifice flow then vac source not enough do we need to test for this?????????





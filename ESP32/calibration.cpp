/****************************************
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

#include "Arduino.h"
#include "calibration.h"

Calibration::Calibration () {



}



/****************************************
* SET CALIBRATION OFFSET
*
* Called from API.ino
* Called from handleWebSocketMessage in server.ino
*
* TODO: #75 The calibration handling needs to be rewritten. https://github.com/DeeEmm/DIY-Flow-Bench/issues/75
* 
***/
bool Calibration::setFlowOffset() {
   
  extern void notifyClients(String jsonValues);
  extern String loadConfig();
  extern String saveCalobration();
  
  float MafFlowCFM = getMafFlowCFM();
  float RefPressure = getRefPressure(INWG);
  float convertedMafFlowCFM = convertFlowDepression(RefPressure, config.cal_ref_press,  MafFlowCFM);
  float flowCalibrationOffset = config.cal_flow_rate - convertedMafFlowCFM;
  
  char flowCalibrationOffsetText[12]; // Buffer big enough?
  dtostrf(flowCalibrationOffset, 6, 2, flowCalibrationOffsetText); // Leave room for too large numbers!
  
  // update config var
  config.cal_offset  = flowCalibrationOffset;
  // send new config to the browser
  notifyClients(loadConfig());
  // invoke a config save
  notifyClients(saveCalibration());
  
  return true;
  
}



/****************************************
* GET CALIBRATION OFFSET
* 
***/
float Calibration::getFlowOffset() {

// TODO: - cal data loaded with json config

}






/****************************************
* leakTestCalibration
***/
bool Calibration::setLeakTestPressure() {

  float RefPressure = getRefPressure(INWG);  
  char RefPressureText[12]; // Buffer big enough?
  dtostrf(RefPressure, 6, 2, RefPressureText); // Leave room for too large numbers!
  
  //Store data in EEPROM
//    EEPROM.write(NVM_LEAK_CAL_ADDR, RefPressure);

  // Display the value on the main screen
  statusMessage = LANG_LEAK_CAL_VALUE, RefPressureText;  

  return true;
}


/****************************************
* leakTest
***/
float  Calibration::getLeakTestPressure() {

	float leakTestPressure = 0.0;
	int leakCalibrationValue = 0; 
	
//    leakCalibrationValue = EEPROM.read(NVM_LEAK_CAL_ADDR);
  int refPressure = getRefPressure(INWG);

  //compare calibration data from NVM
  if (leakCalibrationValue > (refPressure - config.leak_test_tolerance))
  {   
	 return LEAK_TEST_FAILED;
  } else {     
	 return LEAK_TEST_PASS;
  }
  
  return leakTestPressure;

}


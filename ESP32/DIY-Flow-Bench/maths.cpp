/***********************************************************
   The DIY Flow Bench project
   https://diyflowbench.com

   maths.cpp - Core mathematical functions class

   Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.

   For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
   Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
   You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/

   This project and all associated files are provided for use under the GNU GPL3 license:
   https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE


 ***/

#include <Arduino.h>
#include "maths.h"

#include "constants.h"
#include "pins.h"
#include "sensors.h"
#include "structs.h"
#include "hardware.h"
#include "messages.h"

const float MOLECULAR_WEIGHT_DRY_AIR = 28.964;

Maths::Maths() {

  extern String mafSensorType;
  extern int MAFdataUnit;
  extern long mafLookupTable[][2];

  //  this->_mafLookupTable[][2] = mafLookupTable[][2];
  this->_mafDataUnit = MAFdataUnit;

  // lets copy our MAF lookup table into local scope
  memcpy(this->_mafLookupTable, &mafLookupTable, sizeof mafLookupTable[2]);

  //memcpy (b, a, 50*50*sizeof(float));
  //memcpy(p, q, 13*15*sizeof(*p));
  //this->_mafLookupTable[][2] = mafLookupTable[][2];
}



/***********************************************************
   CALCULATE BAROMETRIC pressure (kPa)
 ***/
float Maths::calculateBaroPressure(int units) {

  Hardware _hardware;
  Sensors _sensors;

  float baroPressurePsia;
  float baroPressureKpa = _sensors.getBaroValue();

  switch (units)
  {
    case KPA:
      return baroPressureKpa;
      break;

    case PSIA:
      baroPressurePsia = baroPressureKpa * 0.145038;
      return baroPressurePsia;
      break;
  }

  return baroPressureKpa;
}



/***********************************************************
   CALCULATE REFERENCE PRESSURE
 ***/
float Maths::calculateRefPressure(int units) {

  Sensors _sensors;

  float refPressureInWg;
  float refPressureKpa = _sensors.getPRefValue();

  switch (units)
  {
    case INWG:
      // convert from kPa to inches Water
      refPressureInWg = refPressureKpa * 4.0147421331128;
      return refPressureInWg;
      break;

    case KPA:
      return refPressureKpa;
      break;

    case BAR:
      // 1kpa = 0.01 bar
      return refPressureKpa  * 0.01 ;
      break;

    case PSIA:
      refPressureKpa = refPressureKpa * 0.145038;
      return refPressureKpa  * 0.01 ;
      break;
  }

  return refPressureKpa;

}





/***********************************************************
   CALCULATE DIFFERENTIAL PRESSURE
 ***/
float Maths::calculateDiffPressure(int units) {

  Sensors _sensors;

  float diffPressureInWg;
  float diffPressureKpa = _sensors.getPDiffValue();

  switch (units)
  {
    case INWG:
      // convert from kPa to inches Water
      diffPressureInWg = diffPressureKpa * 4.0147421331128;
      return diffPressureInWg;
      break;

    case KPA:
      return diffPressureKpa;
      break;

    case BAR:
      // 1kpa = 0.01 bar
      return diffPressureKpa  * 0.01 ;
      break;

    case PSIA:
      diffPressureKpa = diffPressureKpa * 0.145038;
      return diffPressureKpa  * 0.01 ;
      break;
  }

  return diffPressureKpa;

}



/***********************************************************
   CALCULATE TEMPERATURE
 ***/
float Maths::calculateTemperature(int units) {

  Sensors _sensors;

  float refTempDegF;
  float refTempRankine;
  float refTempDegC = _sensors.getTempValue();

  switch (units) 	{

    case DEGC:
      return refTempDegC;
      break;

    case DEGF:
      refTempDegF = refTempDegC * 1.8;
      return refTempDegF;
      break;

    case RANKINE:
      refTempRankine = (refTempDegC + 273.15 ) * 9 / 5;
      return refTempRankine;
      break;
  }

  return refTempDegC;
}



/***********************************************************
   CALCULATE RELATIVE HUMIDITY %
 ***/
float Maths::calculateRelativeHumidity(int units) {

  Sensors _sensors;

  float relativeHumidity = _sensors.getRelHValue();

  switch (units)
  {
    case DECI:
      return relativeHumidity / 100;
      break;

    case PERCENT:
      return relativeHumidity;
      break;

  }

  return relativeHumidity;

}




/***********************************************************
   CALCULATE VAPOUR PRESSURE
 ***/
float Maths::calculateVaporPressure(int units) {

  float airTemp = this->calculateTemperature(DEGC);
  float vapourPressureKpa = (0.61078 * exp((17.27 * airTemp) / (airTemp + 237.3))); // Tetans Equation
  float vapourPressurePsia;

  switch (units)
  {
    case KPA:
      return vapourPressureKpa;
      break;

    case PSIA:
      vapourPressurePsia = vapourPressureKpa * 0.145038;
      return vapourPressurePsia;
      break;
  }

  return vapourPressureKpa;

}




/***********************************************************
   CALCULATE SPECIFIC GRAVITY
 ***/
float Maths::calculateSpecificGravity() {

  float specificGravity;
  float relativeHumidity = this->calculateRelativeHumidity(DECI);
  float vaporPressurePsia = this->calculateVaporPressure(PSIA);
  float baroPressurePsia = this->calculateBaroPressure(PSIA);

  specificGravity = (1 - (0.378 * relativeHumidity * vaporPressurePsia) / baroPressurePsia);

  return specificGravity;

}




/***********************************************************
   CONVERT MASS FLOW TO VOLUMETRIC FLOW

   Calculated using ideal gas law:
   https://www.pdblowers.com/tech-talk/volume-and-mass-flow-calculations-for-gases/
 ***/
float Maths::convertMassFlowToVolumetric(float massFlowKgh) {
  float mafFlowCFM;
  float gasPressure;
  float tempInRankine = this->calculateTemperature(RANKINE); //tested ok
  float specificGravity = this->calculateSpecificGravity(); //tested ok
  float molecularWeight = MOLECULAR_WEIGHT_DRY_AIR * specificGravity; //tested ok
  float baroPressure = this->calculateBaroPressure(PSIA);
  float refPressure = this->calculateRefPressure(PSIA);
  float massFlowLbm = massFlowKgh * 0.03674371036415;

  gasPressure = baroPressure + refPressure; // TODO: need to validate refPressure (should be a negative number)

  mafFlowCFM = ((massFlowLbm * 1545 * tempInRankine) / (molecularWeight * 144 * gasPressure));

  return mafFlowCFM;


}




/***********************************************************
   GET FLOW in CFM
 ***/
float Maths::calculateFlowCFM() {

  extern CalibrationSettings calibration;
  Sensors _sensors;
  Hardware _hardware;
  Messages _message;

  float flowRateCFM = 0;
  float flowRateKGH = 0;
  double flowRateRAW = 0;


  /*
    MAF Style Bench
  ***/
#if defined MAF_STYLE_BENCH

  int lookupValue;
  int numRows;
  lookupValue = _sensors.getMafValue();

  //Set size of array
  numRows = (sizeof(this->_mafLookupTable[0])) - 1;

  // Traverse the array until we find the lookupValue
  for (int rowNum = 0; rowNum <= numRows; rowNum++) {

    // Lets check to see if exact match is found
    if (lookupValue == this->_mafLookupTable[rowNum][0]) {
      // we've got the exact value
      flowRateRAW = this->_mafLookupTable[rowNum][1];
      break;

      // We've overshot so lets use the previous value
    } else if ( this->_mafLookupTable[rowNum][0] > lookupValue ) {

      if (rowNum == 0) {
        // we were on the first row so lets set the value to zero and consider it no flow
        return 0;
      } else {
        // Flow value is valid so let's convert it.
        // We use a linear interpolation formula to calculate the actual value
        // NOTE: Y=Y0+(((X-X0)(Y1-Y0))/(X1-X0)) where Y = flow and X = Volts
        flowRateRAW = this->_mafLookupTable[rowNum - 1][1] + (((lookupValue - this->_mafLookupTable[rowNum - 1][0]) * (this->_mafLookupTable[rowNum][1] - this->_mafLookupTable[rowNum - 1][1])) / (this->_mafLookupTable[rowNum][0] - this->_mafLookupTable[rowNum - 1][0]));
      }
      break;
    }
  }

  // Now that we have a flow value, we need to scale it and convert it.
  if (this->_mafDataUnit == KG_H) {

    // convert RAW datavalue back into kg/h
    flowRateKGH = float(flowRateRAW / 1000);

  } else if (this->_mafDataUnit == MG_S) {

    //  convert mg/s value into kg/h
    flowRateKGH = float(flowRateRAW * 0.0036);
  }

  // Now we need to convert from kg/h into cfm (NOTE this is approx 0.4803099 cfm per kg/h @ sea level)
  flowRateCFM = convertMassFlowToVolumetric(flowRateKGH) + calibration.flow_offset;

  // TODO: we should also report back the MASS flow as ultimately this is more useful than CFM


  /*
    Orifice Style Bench
  ***/
#elif defined ORIFICE_STYLE_BENCH //ratiometric

  // We use the ref pressure + differential pressure (pressure drop) across the orifice to calculate the flow. The pressure drop across an orifice is to the square of the flow. If flow doubles, pressure goes up 4 times, therefore the scale is logarithmic.

  // Assuming that the test pressure = pressure that orifices were calibrated at...
  // If diff pressure = 0 and the bench is running we must be flowing 100% of calibrated orifice flow rate
  // If diff pressure is equal to the ref pressure we must be flowing zero
  // All other values are somewhere in between and can be expressed as a percentage (ratio) of the orifice's calibrated flow rate

  // #1 We need to know what orifice we are using - Get selected orifice flow + ref pressure it was calibrated at

  // #2 Convert orifice flow for current test (ref) pressure

  // #3 Calculate flow as percentage of the reference orifice flow rate

  flowRateCFM = 0;

  // TODO: we should convert CFM to MASS flow and also report this figure as ultimately it is more useful


  /*
    Venturi Style Bench
  ***/
#elif defined VENTURI_STYLE_BENCH  //ratiometric

  // same process as per orifice style bench. Different size orifices are used for different flow rates


  flowRateCFM = 0;

  // TODO: we should convert CFM to MASS flow and also report this figure as ultimately it is more useful


  /*
    Pitot Style Bench
  ***/
#elif defined PITOT_STYLE_BENCH //ratiometric

  // Similar process to both orifice and Venturi but requires static pressure data



  flowRateCFM = 0;

  // TODO: we should convert CFM to MASS flow and also report this figure as ultimately it is more useful

#endif



  // Send data to the serial port if requested.
  if (streamMafData == true) {
    _message.DebugPrint(String(lookupValue));
    _message.DebugPrintLn(" (raw) = ");
    if (this->_mafDataUnit == KG_H) {
      _message.DebugPrint(String(flowRateRAW / 1000));
      _message.DebugPrintLn("kg/h = ");
    } else if (this->_mafDataUnit == MG_S) {
      _message.DebugPrint(String(flowRateRAW));
      _message.DebugPrintLn("mg/s = ");
    }
    _message.DebugPrint(String(flowRateCFM ));
    _message.DebugPrintLn("cfm \r\n");
  }

  return flowRateCFM;
}



/***********************************************************
   CALCULATE PITOT PROBE VALUE
 ***/
float Maths::calculatePitotPressure(int units) {

  Hardware _hardware;

  float pitotPressureKpa = 0.0;
  float pitotPressureInWg;

  switch (units)
  {
    case INWG:
      // convert from kPa to inches Water
      pitotPressureInWg = pitotPressureKpa * 4.0147421331128;
      return pitotPressureInWg;
      break;

    case KPA:
      return pitotPressureKpa;
      break;
  }

  return pitotPressureKpa;

}



/***********************************************************
  CONVERT FLOW

  Convert flow values between different reference pressures
  Flow at the new pressure drop = (the square root of (new pressure drop/old pressure drop)) times CFM at the old pressure drop.
  An example of the above formula would be to convert flow numbers taken at 28" of water to those which would occur at 25" of water.
  (25/28) = .89286
  Using the square root key on your calculator and inputting the above number gives .94489 which can be rounded off to .945.
  We can now multiply our CFM values at 28" of water by .945 to obtain the theoretical CFM values at 25" of water.
  Source: http://www.flowspeed.com/cfm-numbers.htm
*/

double Maths::convertFlowDepression(float oldPressure, int newPressure, float inputFlow) {

  double outputFlow;
  double pressureRatio = (newPressure / oldPressure);
  outputFlow = (sqrt(pressureRatio) * inputFlow);

  return outputFlow;

}

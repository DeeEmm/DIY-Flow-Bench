/***********************************************************
   The DIY Flow Bench project
   https://diyflowbench.com

   calculations.cpp - Core mathematical functions class

   Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.

   For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
   Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
   You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/

   This project and all associated files are provided for use under the GNU GPL3 license:
   https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE


 ***/

#include <Arduino.h>

#include "constants.h"
#include "structs.h"
#include "pins.h"

#include "calculations.h"
#include "sensors.h"
#include "hardware.h"
#include "messages.h"
#include MAF_DATA_FILE

const double MOLECULAR_WEIGHT_DRY_AIR = 28.964;

Calculations::Calculations() {
}





/***********************************************************
* CONVERT PRESSURE
*
* Accepts input in kPa returns converted pressure 
* Units - HPA / BAR / PSIA / INWG
*
***/
double Calculations::convertPressure(double pressureKpa, int units) {

  Sensors _sensors;

  double refPressureInWg;

  switch (units)
  {
    case HPA:
      return pressureKpa * 0.1;
      break;

    case BAR:
      // 1kpa = 0.01 bar
      return pressureKpa  * 0.01 ;
      break;

    case PSIA:
      pressureKpa = pressureKpa * 0.145038;
      return pressureKpa  * 0.01 ;
      break;
      
    case INWG:
    default:
          // convert from kPa to inches Water
      refPressureInWg = pressureKpa * 4.0146307866177;
      return refPressureInWg;
      break;
  }

  return pressureKpa;

}





/***********************************************************
   CONVERT TEMPERATURE
 ***/
double Calculations::convertTemperature(double refTempDegC, int units) {

  Sensors _sensors;

  double refTempDegF;
  double refTempRankine;

  switch (units) 	{

    case DEGF:
      refTempDegF = refTempDegC * 1.8;
      return refTempDegF;
      break;

    case RANKINE:
      refTempRankine = (refTempDegC + 273.15 ) * 9 / 5;
      return refTempRankine;
      break;

    case DEGC:
    default:
      return refTempDegC;
      break;

  }

  return refTempDegC;
}



/***********************************************************
   CONVERT RELATIVE HUMIDITY %
 ***/
double Calculations::convertRelativeHumidity(double relativeHumidity, int units) {

  Sensors _sensors;

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
double Calculations::calculateVaporPressure(int units) {

  Sensors _sensors;

  double airTemp;
  double vapourPressureKpa;
  double vapourPressurePsia;

  airTemp = this->convertTemperature(_sensors.getTempValue(), DEGC);
  vapourPressureKpa = (0.61078 * exp((17.27 * airTemp) / (airTemp + 237.3))); // Tetans Equation

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
double Calculations::calculateSpecificGravity() {
  
  Sensors _sensors;

  double specificGravity;
  double relativeHumidity = this->convertRelativeHumidity(_sensors.getRelHValue(), DECI);
  double vaporPressurePsia = this->calculateVaporPressure(PSIA);
  double baroPressurePsia = this->convertPressure(_sensors.getBaroValue(), PSIA);

  specificGravity = (1 - (0.378 * relativeHumidity * vaporPressurePsia) / baroPressurePsia);

  return specificGravity;

}




/***********************************************************
   CONVERT MASS FLOW TO VOLUMETRIC FLOW

   Calculated using ideal gas law:
   https://www.pdblowers.com/tech-talk/volume-and-mass-flow-calculations-for-gases/
 ***/
double Calculations::convertMassFlowToVolumetric(double massFlowKgh) {
  
  Sensors _sensors;

  double mafFlowCFM;
  double gasPressure;
  double tempInRankine = this->convertTemperature(_sensors.getTempValue(), RANKINE); //tested ok
  double specificGravity = this->calculateSpecificGravity(); //tested ok
  double molecularWeight = MOLECULAR_WEIGHT_DRY_AIR * specificGravity; //tested ok
  double baroPressure = this->convertPressure(_sensors.getBaroValue(), PSIA);
  double refPressure = this->convertPressure(_sensors.getPRefValue(), PSIA);
  double massFlowLbm = massFlowKgh * 0.03674371036415;

  gasPressure = baroPressure + refPressure; // TODO: need to validate refPressure (should be a negative number)

  mafFlowCFM = ((massFlowLbm * 1545 * tempInRankine) / (molecularWeight * 144 * gasPressure));

  return mafFlowCFM;

}




/***********************************************************
  CALCULATE FLOW in CFM
  
  Supported bench types - MAF / Orifice / Pitot
   
 ***/
double Calculations::calculateFlowCFM() {

  extern CalibrationSettings calibration;
  
  Sensors _sensors;
  Hardware _hardware;
  Messages _message;
  MafData _mafdata;

  double flowRateCFM = 0;
  double flowRateKGH = 0;
  double flowRateRAW = 0;



  /*
    MAF Style Bench
  ***/
#if defined MAF_STYLE_BENCH

  int lookupValue;
  int numRows;
  
  lookupValue = _sensors.getMafValue();

  //Check MAF data is valid
  if (_mafdata.mafLookupTable == nullptr) {
    _message.serialPrintf("Invalid MAF Data  \n");
  }

  //Set size of array
  numRows = (sizeof(_mafdata.mafLookupTable));

  // Traverse the array until we find the lookupValue
  for (int rowNum = 0; rowNum <= numRows; rowNum++) {

    // Lets check to see if exact match is found
    if (lookupValue == _mafdata.mafLookupTable[rowNum][0]) {
      // we've got the exact value
      flowRateRAW = _mafdata.mafLookupTable[rowNum][1];
      break;

      // We've overshot so lets use the previous value
    } else if ( _mafdata.mafLookupTable[rowNum][0] > lookupValue ) {

      if (rowNum == 0) {
        // we were on the first row so lets set the value to zero and consider it no flow
        return 0;
      } else {
        // Flow value is valid so let's convert it.
        // We use a linear interpolation formula to calculate the actual value
        // NOTE: Y=Y0+(((X-X0)(Y1-Y0))/(X1-X0)) where Y = flow and X = Volts
        flowRateRAW = _mafdata.mafLookupTable[rowNum - 1][1] + (((lookupValue - _mafdata.mafLookupTable[rowNum - 1][0]) * (_mafdata.mafLookupTable[rowNum][1] - _mafdata.mafLookupTable[rowNum - 1][1])) / (_mafdata.mafLookupTable[rowNum][0] - _mafdata.mafLookupTable[rowNum - 1][0]));
      }
      break;
    }
  }

  // Now that we have a flow value, we need to scale it and convert it.
  if (_mafdata.mafDataUnit == KG_H) {

    // convert RAW datavalue back into kg/h
    flowRateKGH = double(flowRateRAW / 1000);

  } else if (_mafdata.mafDataUnit == MG_S) {

    //  convert mg/s value into kg/h
    // flowRateKGH = float(flowRateRAW * 0.0036); // TEST  : is divide by float an issue????
    flowRateKGH = double(flowRateRAW / 277.8);
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
    if (_mafdata.mafDataUnit == KG_H) {
      _message.serialPrintf("%s (raw) = kg/h = %c \n", lookupValue, dtostrf((flowRateRAW / 1000), 7, 2, _message.floatBuffer));
    } else if (_mafdata.mafDataUnit == MG_S) {
      _message.serialPrintf("%s (raw) = mg/s = %d \n", lookupValue, dtostrf((flowRateRAW / 1000), 7, 2, _message.floatBuffer));
    }
    _message.serialPrintf("%s (raw) = cfm = %d \n", lookupValue,dtostrf((flowRateCFM / 1000), 7, 2, _message.floatBuffer));
  }

  return flowRateCFM;
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

double Calculations::convertFlowDepression(double oldPressure, double newPressure, double inputFlow) {

  //double outputFlow;
  //double pressureRatio = (newPressure / oldPressure);
  //outputFlow = (sqrt(pressureRatio) * inputFlow);

  //return outputFlow;
  
  return 1;

}
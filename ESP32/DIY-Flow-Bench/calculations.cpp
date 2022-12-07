/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file calculations.cpp
 * 
 * @brief Core mathematical functions class
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/

#include <Arduino.h>
#include <vector>

#include "constants.h"
#include "structs.h"
#include "pins.h"

#include "calculations.h"
#include "sensors.h"
#include "hardware.h"
#include "messages.h"

#ifdef MAF_IS_ENABLED
#include MAF_DATA_FILE
#endif




/***********************************************************
 * @brief Class constructor
*/
Calculations::Calculations() {
}




/***********************************************************
* @brief CONVERT PRESSURE
*
* @note Accepts input in kPa returns converted pressure 
* @note Units - HPA / BAR / PSIA / INWG
***/
double Calculations::convertPressure(double pressureKpa, int units) {

  double convertedPressure;


  switch (units)
  {
    case HPA:
      convertedPressure = pressureKpa * 10;
      return convertedPressure;
      break;

    case BAR:
      convertedPressure = pressureKpa * 0.01 ;
      return convertedPressure;
      break;

    case PSIA:
      convertedPressure = pressureKpa * 0.145038;
      return convertedPressure;
      break;
      
    case INWG:
    default:
      convertedPressure = pressureKpa * 4.0146307866177;
      return convertedPressure;
      break;
  }

  return pressureKpa;

}




/***********************************************************
 * @brief CONVERT TEMPERATURE
 ***/
double Calculations::convertTemperature(double refTempDegC, int units) {

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

    case KELVIN:
    default:
      return refTempDegC + 273.15;
      break;

  }

  return refTempDegC;
}



/***********************************************************
 * @brief CONVERT RELATIVE HUMIDITY %
 ***/
double Calculations::convertRelativeHumidity(double relativeHumidity, int units) {

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
 * @brief CALCULATE VAPOUR PRESSURE
 * 
 * @note Tetans Equation
 * @example P(kPa) = 0.61078EXP * ((17.27 * Temp(C) / Temp(C) + 237.3)
 ***/
double Calculations::calculateVaporPressure(int units) {

  extern struct SensorData sensorVal;

  double vapourPressureKpa;
  double vapourPressurePsia;

  vapourPressureKpa = 0.61078 * exp ((17.27 * sensorVal.TempDegC) / (sensorVal.TempDegC + 237.3));

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
 * @brief CALCULATE SPECIFIC GRAVITY
 * 
 * @example SG = 1-(0.378 * RH(%) * vaporPressure(psia)) / baroPressure(psia)
 ***/
double Calculations::calculateSpecificGravity() {

  extern struct SensorData sensorVal;
  double specificGravity;

  double vaporPressurePsia = this->calculateVaporPressure();
  double relativeHumidity = this->convertRelativeHumidity(sensorVal.RelH, DECI);
  double baroPressurePsia = this->convertPressure(sensorVal.BaroKPA, PSIA);

  specificGravity = (1 - (0.378 * relativeHumidity * vaporPressurePsia) / baroPressurePsia);

  return specificGravity;

}



/***********************************************************
 * @brief CALCULATE DENSITY OF AIR
 * 
 * @note Ideal gas law
 * @example airDensity(kg/m3) = absPressurePa / (SPECIFIC_GAS_CONSTANT_DRY_AIR * absTempKelvin)
 ***/
double Calculations::calculateAirDensity() {

  extern struct SensorData sensorVal;

  // double airDensity = (sensorVal.BaroKPA / 100) / (SPECIFIC_GAS_CONSTANT_DRY_AIR * convertTemperature(sensorVal.TempDegC, KELVIN)); // density of dry air

  double airDensity = ((convertPressure(sensorVal.BaroKPA, PSIA)) * MOLAR_MASS_DRY_AIR + calculateVaporPressure(PSIA) * MOLAR_MASS_WATER_VAPOUR) / UNIVERSAL_GAS_FLOW_CONSTANT * convertTemperature(sensorVal.TempDegC, KELVIN);

  return airDensity;

}




/***********************************************************
 * @brief CONVERT MASS FLOW TO VOLUMETRIC FLOW
 * @param massflowKgh Mass flow in KG/h
 * @note Calculate flow using ideal gas law:
 * @ref https://www.pdblowers.com/tech-talk/volume-and-mass-flow-calculations-for-gases/
 * @ref flowCFM = (massFlow * UniversalGasFlowConstant * GasTempInRankine * CompressibilityFactor(1))/ 144 * PressurePSIA
 ***/
double Calculations::convertMassFlowToVolumetric(double massFlowKgh) {
  
  extern struct SensorData sensorVal;

  double mafFlowCFM;
  double gasPressure;

  double tempInRankine = this->convertTemperature(sensorVal.TempDegC, RANKINE);
  double specificGravity = this->calculateSpecificGravity(); //tested ok
  double molecularWeight = MOLAR_MASS_DRY_AIR * specificGravity; //tested ok
  double baroPressurePsia = this->convertPressure(sensorVal.BaroKPA, PSIA);
  double refPressure = this->convertPressure(sensorVal.PRefKPA, PSIA);
  double massFlowLbm = massFlowKgh * 0.03674371036415;

  // gasPressure = baroPressure + refPressure; // TODO: need to validate refPressure (should be a negative number)
  gasPressure = baroPressurePsia;

  mafFlowCFM = ((massFlowLbm * 1545 * tempInRankine) / (molecularWeight * 144 * gasPressure)); 

  return mafFlowCFM;

}




/***********************************************************
 * @brief CALCULATE FLOW in CFM
 * 
 * @param mafValue RAW reference value (15bit value from ADS1115 ADC)
 * @param maxValue Max reference value (default 32767 for 15 bit resolution)
 * @return flowRateCFM
 * @note  mafLookupTable is lookup vector from MafData file (val > flow data pairs)
 * @note Supported bench types - MAF 
 * @note Future support for Orifice / Pitot / Venturi benches planned
 * 
 ***/
double Calculations::calculateFlowCFM(int mafValue, int maxValue ) {

  extern CalibrationSettings calibration;
  extern struct SensorData sensorVal;
  
	#ifdef MAF_IS_ENABLED
	Maf _maf;
	#endif

  double flowRateCFM = 0.0;
  double flowRateMGS = 0.0;
  double flowRateKGH = 0.0;
  double flowRateRAW = 0.0;


/*** MAF Style Bench ***/
#if defined MAF_STYLE_BENCH && defined MAF_IS_ENABLED

  int refValue;
  int numRows;

  // get size of the data table
  numRows = mafLookupTable.size();

  // scale sensor reading to data table size using map function
  refValue =  map(mafValue, 0, maxValue, 0, numRows -1 );
 
  for (int rowNum = 0; rowNum < numRows; rowNum++) { // iterate the data table comparing the Lookup Value to the refValue for each row

    double LV = mafLookupTable.at(rowNum).at(0); // Lookup Value for this row
    double FV = mafLookupTable.at(rowNum).at(1); // Flow Value for this row

    // Did we get a match??
    if (refValue == LV) { // Great!!! we've got the exact value
      
      flowRateRAW = FV;
      break;
      
    } else if ( LV > refValue ) { // we've passed our refValue so we need to interpolate 

      if (rowNum == 0) { // we were only on the first row so there is no previous value to interpolate with, so lets set the flow value to zero and consider it no flow

        flowRateRAW = 0.0;
        return flowRateRAW;
        break;

      } else { // The value is somewhere between this and the previous value so let's use linear interpolation to calculate the actual value: 

        double LVP = mafLookupTable.at(rowNum - 1).at(0); // Lookup value for the previous row
        double FVP = mafLookupTable.at(rowNum - 1).at(1); // Flow value for the previous row

        flowRateRAW = FVP + (refValue - LVP) * ((FV-FVP)/(LV-LVP)); // Linear interpolation
        
      }

      break;
    }

  } //endfor


  // Now that we have a flow value, we need to scale it and convert it.
  if (_maf.mafDataUnit == KG_H) {

    // convert RAW datavalue back into kg/h
    flowRateKGH = double(flowRateRAW / 1000);

  } else if (_maf.mafDataUnit == MG_S) {

    // Convert RAW datavalue back into mg/s
    flowRateMGS = flowRateRAW / 10;
    //  convert mg/s value into kg/h
    flowRateKGH = flowRateMGS / 277.8;
  }

  sensorVal.FlowMASS = flowRateKGH;

  // Now we need to convert from kg/h into cfm (NOTE this is approx 0.4803099 cfm per kg/h @ sea level)
  flowRateCFM = convertMassFlowToVolumetric(flowRateKGH);

/*** Orifice Style Bench ***/
#elif defined ORIFICE_STYLE_BENCH //ratiometric

  // We use the ref pressure + differential pressure (pressure drop) across the orifice to calculate the flow. The pressure drop across an orifice is the square of the flow. If flow doubles, pressure goes up 4 times, therefore the scale is exponential.
  // Assuming that the test pressure = pressure that orifices were calibrated at...
  // If diff pressure = 0 and the bench is running we must be flowing 100% of calibrated orifice flow rate
  // If diff pressure is equal to the ref pressure we must be flowing zero
  // All other values are somewhere in between and can be expressed as an exponent (ratio) of the orifice's calibrated flow rate
  // #1 We need to know what orifice we are using - Get selected orifice flow + ref pressure it was calibrated at
  // #2 Convert orifice flow for current test (ref) pressure
  // #3 Calculate flow as percentage of the reference orifice flow rate

  flowRateCFM = 0;

  // TODO: we should convert CFM to MASS flow and also report this figure as ultimately it is more useful


/*** Venturi Style Bench ***/
#elif defined VENTURI_STYLE_BENCH  //ratiometric

  // same process as per orifice style bench. Different size orifices are used for different flow rates

  flowRateCFM = 0;

  // TODO: we should convert CFM to MASS flow and also report this figure as ultimately it is more useful


/*** Pitot Style Bench ***/
#elif defined PITOT_STYLE_BENCH //ratiometric

  // Similar process to both orifice and Venturi but requires static pressure data

  flowRateCFM = 0;

  // TODO: we should convert CFM to MASS flow and also report this figure as ultimately it is more useful

#endif




  flowRateCFM += calibration.flow_offset;

  sensorVal.FlowCFM = flowRateCFM;

   return flowRateCFM;
}





/***********************************************************
 * @brief Convert flow values between different reference pressures
 * @param oldPressure Reference pressure to convert from
 * @param newPressure Reference pressure to convert to
 * @param inputFlow Value to convert
 * @return outputFlow converted flow value at newPressure
 * 
 * @note Flow at the new pressure drop = (the square root of (new pressure drop/old pressure drop)) times CFM at the old pressure drop.
 * An example of the above formula would be to convert flow numbers taken at 28" of water to those which would occur at 25" of water.
 * (25/28) = .89286
 * Using the square root key on your calculator and inputting the above number gives .94489 which can be rounded off to .945.
 * We can now multiply our CFM values at 28" of water by .945 to obtain the theoretical CFM values at 25" of water.
 * Source: http://www.flowspeed.com/cfm-numbers.htm
*/
// TODO re-enable and...
// TEST
double Calculations::convertFlowDepression(double oldPressure, double newPressure, double inputFlow) {

  //double outputFlow;
  //double pressureRatio = (newPressure / oldPressure);
  //outputFlow = (sqrt(pressureRatio) * inputFlow);

  //return outputFlow;
  
  return 1;

}
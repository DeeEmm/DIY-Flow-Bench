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
#include <math.h>

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
 * @param inputPressure Input value to be converted
 * @param unitsIn Input value units (default kPa)
 * @param unitsOut Desired output format (default INH2O)
 * ***/
double Calculations::convertPressure(double inputPressure, int unitsOut, int unitsIn) {

  double inputPressureKpa;
  double convertedPressure;

  // First convert input pressure to kPa
  switch (unitsIn)
  {
    case HPA:
      inputPressureKpa = inputPressure * 0.1;
      break;

    case BAR:
      inputPressureKpa = inputPressure * 100;
      break;

    case PSIA:
      inputPressureKpa = inputPressure * 6.89476;
      break;

    case INH2O:
      inputPressureKpa = inputPressure * 0.24884;
      break;

    default:
      inputPressureKpa = inputPressure;
      break;

  }

  // Then convert kPa to required output and return result
  switch (unitsOut)
  {
    case HPA:
      convertedPressure = inputPressureKpa * 10;
      return convertedPressure;
      break;

    case BAR:
      convertedPressure = inputPressureKpa * 0.01;
      return convertedPressure;
      break;

    case PSIA:
      convertedPressure = inputPressureKpa * 0.145037738;
      return convertedPressure;
      break;
      
    case KPA:
      return inputPressureKpa;
      break;

    case INH2O:
    default:
      convertedPressure = inputPressureKpa * 4.01463;
      return convertedPressure;
      break;

  }

  return 0.00001; // else return small non zero value which will get filtered out as zero

}




/***********************************************************
 * @brief CONVERT TEMPERATURE
 * @param refTemp Input value to be converted
 * @param unitsOut Desired output format (default degC)
 * @param unitsIn Input value units (default degC)
 ***/
double Calculations::convertTemperature(double refTemp, int unitsOut, int unitsIn) {

  double refTempDegF;
  double refTempRankine;

  double inputTempDegC;
  double convertedTemp;


    // First convert input Temperature to DegC
  switch (unitsIn) {

    case DEGF:
      inputTempDegC = (refTemp - 32) * 0.5556;
      break;

    case RANKINE:
      inputTempDegC = (refTemp - 491.67) * 0.5556;
      break;

    case KELVIN:
      inputTempDegC = refTemp - 273.15;
      break;

    default: // DEGC
      inputTempDegC = refTemp;
      break;


  }

  // Then convert DegC to required value and return
  switch (unitsOut) 	{

    case DEGF:
      convertedTemp = (inputTempDegC * 1.8) + 32;
      return convertedTemp;
      break;

    case RANKINE:
      convertedTemp = (inputTempDegC + 273.15 ) * 1.8;
      return convertedTemp;
      break;

    case KELVIN:
      convertedTemp = inputTempDegC + 273.15;
      return convertedTemp;
      break;

    case DEGC:
    default:
      return inputTempDegC;
      break;

  }

  return 0.00001; // else return small non zero value which will get filtered out as zero
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
 * @param mafRAW RAW reference value (15bit value from ADS1115 ADC)
 * @param maxValue Max reference value (default 32767 for 15 bit resolution)
 * @return flowRateCFM
 * @note mafLookupTable is lookup vector from MafData file (val > flow data pairs)
 * @note Supported bench types - MAF 
 * @note Future support for Orifice / Pitot / Venturi benches planned
 * 
 ***/
double Calculations::calculateFlowCFM(long mafRAW, long maxADC ) {

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

  // get highest MAF input value from data table
  long maxMAF =  mafLookupTable[numRows -1][0];

  // scale sensor reading to data table size using map function (0-ADC Max -> 0-MAF Max))
  refValue =  map(mafRAW, 0, maxADC, 0, maxMAF );

  for (int rowNum = 0; rowNum < numRows; rowNum++) { // iterate the data table comparing the Lookup Value to the refValue for each row

    long LV = mafLookupTable[rowNum][0]; // Lookup Value for this row (x2)
    long FV = mafLookupTable[rowNum][1]; // Flow Value for this row (y2)

    // Did we get a match??
    if (refValue == LV) { // Great!!! we've got the exact value

      flowRateRAW = FV;
      break;
      
    // } else if ( LV > refValue && rowNum == 0) { // we were only on the first row so there is no previous value to interpolate with, so lets set the flow value to zero and consider it no flow

    //   flowRateRAW = 0.0;
    //   return flowRateRAW;
    //   break;

    } else if (LV > refValue) { // The value is somewhere between this and the previous value so let's use linear interpolation to calculate the actual value: 

      long LVP = mafLookupTable[rowNum - 1][0]; // Lookup value for the previous row (x1)
      long FVP = mafLookupTable[rowNum - 1][1]; // Flow value for the previous row (y1)

      // Linear interpolation y = y1 + (x-x1)((y2-y1)/(x2-x1)) where x1+y1 are coord1 and x2_y2 are coord2
      flowRateRAW = FVP + (refValue - LVP)*((FV-FVP)/(LV-LVP));
      break;   
    }

  } //endfor


  // Now that we have a flow value, we need to scale it and convert it.
  if (_maf.mafUnits() == KG_H) {

    // convert RAW datavalue back into kg/h
    flowRateKGH = double(flowRateRAW / 1000);

  } else if (_maf.mafUnits() == MG_S) {

    // Convert RAW datavalue back into mg/s
    flowRateMGS = flowRateRAW / 10;

    // convert mg/s value into kg/h
    flowRateKGH = flowRateMGS / 277.8;
  }

  sensorVal.FlowMASS = flowRateKGH;

  // Now we need to convert from kg/h into cfm (NOTE this is approx 0.4803099 cfm per kg/h @ sea level)
  flowRateCFM = convertMassFlowToVolumetric(flowRateKGH);

/*** Orifice Style Bench ***/
// NOTE Ideally orifice should not be larger than 50% of your pipe size
#elif defined ORIFICE_STYLE_BENCH // ratiometric


  double refPressure = sensorVal.PRefKPA;
  double diffPressure = sensorVal.PDiffKPA;

  //get currently selected orifice + calibration depression

  //Simplified Flow calculation. Source - https://www.youtube.com/watch?v=Dn16HZ_oHEo
  // flowRateCFM = OrificeCalibratedflowCFM * SQRT(diffPressure / OrificeCalibratedPressure);



/*** Venturi Style Bench ***/
// NOTE Ideally venturi should not be larger than 50% of your pipe size
// https://www.engineersedge.com/fluid_flow/venturi_flow_equation_and_calculator_14001.htm
#elif defined VENTURI_STYLE_BENCH  // ratiometric

  double refPressure = sensorVal.PRef;
  double diffPressure = sensorVal.PDiff;

  //Simplified Flow calculation. Source - https://www.youtube.com/watch?v=Dn16HZ_oHEo
  flowRateCFM = OrificeCalibratedflowCFM * SQRT(diffPressure / OrificeCalibratedPressure);

  

/*** Pitot Style Bench ***/
// Diameter of pipe is required (set in config)
#elif defined PITOT_STYLE_BENCH // ratiometric

  /*
  
  https://www.youtube.com/watch?v=Dn16HZ_oHEo

  Vfpm = SQRT(P1-P2) * 40004.4 * Kf

  Where:
  P1 = Stagnation pressure [Pitot]
  P2 = Static pressure [PRef]
  P1-P2 = Dynamic pressure
  Kf = Correction factor for pitot tube (or test pressure )
  Vfpm = Velocity in feet per second

  CFM = Vfpm * Pi*R^2 

  Where:
  R = radius of pipe in feet


  */
  
  Vfpm = SQRT(P1-P2) * 40004.4 * Kf;

  flowRateCFM = convertVelocityToVolumetric(Vfpm, PIPE_RADIUS_IN_FEET);


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
double Calculations::convertFlowDepression(double oldPressure, double newPressure, double inputFlowCFM) {

  Calculations _calculations;
  Hardware _hardware;

  if (_hardware.benchIsRunning()) {
    double outputFlow;
    double pressureRatio = (newPressure / oldPressure);
    outputFlow = sqrt(pressureRatio) * inputFlowCFM;
    return outputFlow;
  } else {
    return 0.0;
  }
  
}



/***********************************************************
 * @brief Convert velocity to volumetric flow
 * @param velocity Velocity of air in feet per minute
 * @param pipeRadius radius of pipe in feet
 ***/

double Calculations::convertVelocityToVolumetric(double velocityFpm, double pipeRadiusFt) {

  double CFM;
  
  CFM = velocityFpm * pow(PI * pipeRadiusFt, 2) ;

  return CFM;
  
}
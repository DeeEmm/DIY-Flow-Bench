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

    case ATM:
      inputPressureKpa = inputPressure * 101.325;
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

    case ATM:
      inputPressureKpa = inputPressure * 0.00986923;
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
 * @brief CALCULATE VAPOUR PRESSURE (nominally 101.325kPa)
 * 
 * @note Tetans Equation
 * @example P(kPa) = 0.61078 * EXP((17.27 * Temp(C) / Temp(C) + 237.3)
 * 
 * @note Magnus-Tetan Equation
 * @example P(kPa) = 0.61094 * EXP((17.625 * Temp(C) / Temp(C) + 243.04)
 * 
 ***/
double Calculations::calculateVaporPressure(int units) {

  extern struct SensorData sensorVal;

  double vapourPressureKpa;
  double vapourPressurePsia;

  // vapourPressureKpa = 0.61078 * exp((17.27 * sensorVal.TempDegC) / (sensorVal.TempDegC + 237.3)); // Tetan 
  vapourPressureKpa = 0.61094 * exp((17.625 * sensorVal.TempDegC) / (sensorVal.TempDegC + 243.04)); // Magnus

  switch (units)
  {
    case PASCALS:
      return vapourPressureKpa * 1000;
      break;

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
 * @brief CALCULATE ABSOLUTE HUMIDITY 
 * 
 * AH = (RH × P)/(Rw × T × 100)
 * 
 * Where:
 * AH = Absolute Humdity g/m³  
 * RH = Relative Humidity
 * Rw = specific gas constant for water
 * P = Saturation Vapour Pressure in Pascals
 * T = Temperature 
 * 
 ***/
double Calculations::calculateAbsoluteHumidity() {

  extern struct SensorData sensorVal;
  
  double vapourPressure = this->calculateVaporPressure(KPA);

  double absoluteHumidity = (convertRelativeHumidity(sensorVal.RelH, PERCENT) * vapourPressure) / SPECIFIC_GAS_CONSTANT_WATER_VAPOUR * sensorVal.TempDegC;

  return absoluteHumidity;

}




/***********************************************************
 * @brief CALCULATE SPECIFIC GRAVITY (ratio of densities of air to water)
 * 
 * SG = 1 - 0.378 RHa PVa / Pb
 * Where:
 * SG = specific gravity 
 * RHa = relative humidity in % (e.g. 0.36)
 * PVa = Vapor pressure of water at actual temperature in psia
 * Pb = barometric pressure in psia
 * 
 * SG = 1-(0.378 * RH(%) * vaporPressure(psia)) / baroPressure(psia)
 * 
 ***/
double Calculations::calculateSpecificGravity() {

  extern struct SensorData sensorVal;
  double specificGravity;

  double vaporPressurePsia = this->calculateVaporPressure(PSIA);
  double relativeHumidity = this->convertRelativeHumidity(sensorVal.RelH, DECI);
  double baroPressurePsia = this->convertPressure(sensorVal.BaroKPA, PSIA);

  specificGravity = 1 - (0.378 * relativeHumidity * vaporPressurePsia) / baroPressurePsia;

  return specificGravity;

}




/***********************************************************
 * @brief CALCULATE DENSITY OF AIR
 * 
 * @todo needs review - https://en.wikipedia.org/wiki/Density_of_air
 * 
 * @note Ideal gas law
 * @example airDensity(kg/m3) = absPressurePa / (SPECIFIC_GAS_CONSTANT_DRY_AIR * absTempKelvin)
 * 
 * 
 * ρ = [(Pd / (Rd * T)) + (Pv / (Rv * T))]
 * where:
 * Pd = Pressure of dry air
 * Pv = Water vapor pressure 
 * T = Temperature in Kelvins
 * Rd = Specific gas constant for dry air [287.058 J/(kg·K)]
 * Rv = Specific gas constant for water [461.495 J/(kg·K)]
 * 
 * 
 ***/
double Calculations::calculateAirDensity() {

  extern struct SensorData sensorVal;

  // original 
  // double airDensity = ((convertPressure(sensorVal.BaroKPA, PSIA)) * MOLAR_MASS_DRY_AIR + calculateVaporPressure(PSIA) * MOLAR_MASS_WATER_VAPOUR) / UNIVERSAL_GAS_FLOW_CONSTANT * convertTemperature(sensorVal.TempDegC, KELVIN);

  double airDensity = (convertPressure(sensorVal.BaroKPA, PSIA) / (287.058 * convertTemperature(sensorVal.TempDegC, KELVIN))) + (calculateVaporPressure(PSIA) / (461.495 * convertTemperature(sensorVal.TempDegC, KELVIN)));

  return airDensity;

}



/***********************************************************
 * @brief CONVERT BETWEEN MASS FLOW UNITS
 * @param massflowKgh Mass flow in KG/H
 ***/
double Calculations::convertMassFlowUnits(double massFlowKGH, int units) {

  double convertedFlow = 0.0;

  switch (units) {

    case MG_S:
      convertedFlow = massFlowKGH * 277.8;
      break;

    case GM_M:
      convertedFlow = massFlowKGH * 16.667;
      break;

    case KG_H:
    default:
      convertedFlow = massFlowKGH;
      break;
  }

  return convertedFlow;

}



/***********************************************************
 * @brief CONVERT KGH TO CFM
 * @note Convert mass flow to volumetric using ideal gas law
 * 
 * Q = nRT/mP * ṁ
 * Where:
 * Q = Volumetric flow (SCCM)
 * n = number of moles of gas
 * R = Gas constant (litres . atm . mole . oK or cm3 . atm/mole . oK) | R = 82.1 (cm3 x atm) / (mole x K) | (m3 x Pa) / (mole x K)
 * T = absolute temperature in Kelvin
 * m = mass in grams
 * P = Pressure
 * ṁ = Mass flow (grams / min)
 * 
 * 
 ***/
double Calculations::convertKGHtoCFM(double massFlowKGH) {

  extern struct SensorData sensorVal;

  double FlowCFM;
  double flowCMM;
  double massFlowGM_M = convertMassFlowUnits(massFlowKGH, GM_M);

  double tempInKelvin = this->convertTemperature(sensorVal.TempDegC, DEGC, KELVIN);
  double baroPressureHpa = this->convertPressure(sensorVal.BaroKPA, HPA);
  double absoluteHumidity = calculateAbsoluteHumidity(); // grams / m3 of water vapour in air

  // adjust molar mass based on volume of water vapour in air
  double molarMassAir = ((100 - absoluteHumidity) * MOLAR_MASS_DRY_AIR / 100) + (absoluteHumidity * MOLAR_MASS_WATER_VAPOUR / 100);

  flowCMM  = massFlowGM_M * 82.1 * tempInKelvin / molarMassAir * baroPressureHpa;

  // convert cm3/min to CFM
  FlowCFM = flowCMM * 0.000035314666721489 * -0.001;

  return FlowCFM;

}


/***********************************************************
 * @brief CONVERT MASS FLOW TO VOLUMETRIC FLOW
 * @param massflowKgh Mass flow in KG/h
 * @note Calculate flow using ideal gas law:
 * V̇  = ṁ nRT / mP
 * Q = mRTZ/144P
 * where:
 * Q = flow in CFM
 * P = pressure in PSIA
 * m = mass flow in lb/min
 * R = universal gas constant Ru/MW
 * T = temperature in Rankine
 * Z = compressibility 
 * flowCFM = (massFlow * UniversalGasFlowConstant * GasTempInRankine * CompressibilityFactor(1))/ 144 * PressurePSIA
 * @ref https://www.pdblowers.com/tech-talk/volume-and-mass-flow-calculations-for-gases/
 * @todo review !!!!!!
 ***/
// DEPRECATED Replaced by convertKGHtoCFM conversion
double Calculations::convertMassFlowToVolumetric(double massFlowKgh) {
  
  extern struct SensorData sensorVal;

  double mafFlowCFM;
  double gasPressure;

  double tempInRankine = this->convertTemperature(sensorVal.TempDegC, DEGC, RANKINE);
  double specificGravity = this->calculateSpecificGravity(); //tested ok
  double molecularWeightAir = MOLAR_MASS_DRY_AIR * specificGravity; //tested ok
  double baroPressurePsia = this->convertPressure(sensorVal.BaroKPA, PSIA);
  double refPressurePsia = this->convertPressure(sensorVal.PRefKPA, PSIA);
  double massFlowLbm = massFlowKgh * 0.03674371036415;
  double universalGasConstant = 1545 / molecularWeightAir;

  // gasPressure = baroPressurePsia + refPressurePsia; // Adjusted value includes reference depression
  gasPressure = baroPressurePsia;

  // mafFlowCFM = ((massFlowLbm * 1545 * tempInRankine) / (molecularWeightAir * 144 * gasPressure)); // original
  mafFlowCFM = ((massFlowLbm * universalGasConstant * tempInRankine) / (144 * gasPressure)); // this is a test
  mafFlowCFM = mafFlowCFM * -1; // get rid of negative value from test

  return mafFlowCFM;

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
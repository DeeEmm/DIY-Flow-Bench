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
// #include "pins.h"

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
    case PASCALS:
      inputPressureKpa = inputPressure * 0.001;
      break;

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

    case KPA:
    default:
      inputPressureKpa = inputPressure;
      break;

  }

  // Then convert kPa to required output and return result
  switch (unitsOut)
  {
    case PASCALS:
      convertedPressure = inputPressureKpa * 1000;
      return convertedPressure;
      break;

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
 * @brief CONVERT BETWEEN MASS FLOW UNITS
 * @param unitsOut Default is Mass flow in KG/H
 * @param unitsIn Default is MassFlow in MG_S
 * 
 * 1g/m = 277.778mg/s
 * 1g/m = 0.06kg/h
 ***/
double Calculations::convertMassFlowUnits(double refFlow, int unitsOut, int unitsIn) {

  double massFlowKGH = 0.0;
  double convertedFlow = 0.0;

  switch (unitsIn) {

    case MG_S:
      massFlowKGH = refFlow * 277.778;
      break;

    case GM_M:
      massFlowKGH = refFlow * 16.6667;
      break;

    case KG_H:
    default:
      massFlowKGH = refFlow;
      break;
  }


  switch (unitsOut) {

    case MG_S:
      convertedFlow = massFlowKGH * 0.0036;
      break;

    case GM_M:
      convertedFlow = massFlowKGH * 0.06;
      break;

    case KG_H:
    default:
      convertedFlow = massFlowKGH;
      break;
  }


  return convertedFlow;

}





/***********************************************************
 * @brief CONVERT FLOW
 * @details Simplified conversion from KG/H to CFM
 * 
 * From Conversation #138 https://github.com/DeeEmm/DIY-Flow-Bench/discussions/138#discussioncomment-5590135
 * https://www.engineeringtoolbox.com/density-air-d_680.html
 * https://www.omnicalculator.com/physics/air-density
 * 
 ***/
double Calculations::convertFlow(double massFlowKGH) {

  extern struct SensorData sensorVal;

  double airDensity = 0.0; // kg/m3
  double waterVaporDensity = 0.0; // kg/m3
  double waterVaporPressure = 0.0; // kg/m3
  double dryAirPressure = 0.0; // kg/m3
  double flowM3H = 0.0; // m3/hr
  double flowCFM = 0.0;

  // TODO validate reference pressure adjustment - do we add it or subtract it? Should be baro pressure less vac amount
  double refPressurePascals = sensorVal.BaroPA - this->convertPressure(sensorVal.PRefKPA, PASCALS) ;

  airDensity = calculateAirDensity(sensorVal.TempDegC, refPressurePascals, sensorVal.RelH);

  // Multiply mass by density to get volume (m3/hr)
  flowM3H = massFlowKGH / airDensity; 

  // Convert to CFM
  flowCFM = flowM3H * 0.58858;

  // only return value if valid posotive value received
  if ( massFlowKGH > 0 ) {
    return flowCFM;
  } else {
    return 0.0;
  }

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
  double outputFlow;
  double pressureRatio;
  double scaleFactor;

  if (_hardware.benchIsRunning()) {
    if (newPressure == oldPressure) {
      return inputFlowCFM;
    } else {
      pressureRatio = (abs(newPressure) / abs(oldPressure));
    }
    scaleFactor = sqrt(pressureRatio);
    outputFlow =  inputFlowCFM * scaleFactor;
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
  
  CFM = velocityFpm * (PI * pow(pipeRadiusFt, 2));

  return CFM;
  
}


/***********************************************************
 * @brief Calculate air density
 * @return air density
 * @param TempC - Reference temperature
 * @param baroKPA - Reference pressure
 * @param relH - Reference humidity
 ***/

double Calculations::calculateAirDensity(double TempC, double refPressurePascals, double relH) {

  extern struct SensorData sensorVal;

  double airDensity = 0.0; // kg/m3
  double waterVaporDensity = 0.0; // kg/m3
  double waterVaporPressure = 0.0; // kg/m3
  double dryAirPressure = 0.0; // kg/m3

  double tempInKelvin = this->convertTemperature(TempC, KELVIN, DEGC);

  // Calculate saturation vapor pressure
  waterVaporPressure = 0.61078 * exp((7.5 * TempC) / (TempC + 237.3)) * relH;

  // Calculate Dry air pressure
  dryAirPressure = refPressurePascals - waterVaporPressure;

  // Calculate air density from ratio of dry to wet air
  airDensity = (dryAirPressure / (SPECIFIC_GAS_CONSTANT_DRY_AIR * tempInKelvin)) + (waterVaporPressure / (SPECIFIC_GAS_CONSTANT_WATER_VAPOUR * tempInKelvin));

  return airDensity;


}




/***********************************************************
 * @brief Convert flow to standard CFM (SCFM)
 * @note Translates current flow to standardised flow based on international standards
 * @note Default standard for project is ISO 1585 - Automotive engine testing
 ***/

double Calculations::convertToSCFM(double flowCFM, int standard) {
  
  extern struct SensorData sensorVal; 

  double SCFM;
  double tStd;
  double pStd;
  double rhStd;
  double airDensityStd;
  double airDensityAct;

  switch (standard) {

    case ISO_1585:
      tStd = 25;
      pStd = 100;
      rhStd = 0;
    break;

    case ISA:
      tStd = 15;
      pStd = 101.325;
      rhStd = 0;
    break;

    case ISO_13443:
      tStd = 15;
      pStd = 100.325;
      rhStd = 0;
    break;

    case ISO_5011:
      tStd = 20;
      pStd = 100.3;
      rhStd = 50;
    break;

    case ISO_2533:
      tStd = 15;
      pStd = 100.325;
      rhStd = 0;
    break;

    default: // ISO_1585
      tStd = 25;
      pStd = 100;
      rhStd = 0;
    break;

  }

  // From Issue #208 
  // SCFM = sensorVal.FlowCFM * (sensorVal.PRefKPA / pStd) * (tStd / sensorVal.TempDegC) * (1 / (1 - ( sensorVal.RelH / 100)));

  // From https://neutrium.net/general-engineering/conversion-of-standard-volumetric-flow-rates-of-gas/
  airDensityAct = calculateAirDensity(sensorVal.TempDegC, sensorVal.BaroPA , sensorVal.RelH);
  airDensityStd = calculateAirDensity(tStd, convertPressure(pStd,PASCALS) , rhStd);

  // SCFM = sensorVal.FlowCFM * (airDensityAct / airDensityStd);
  SCFM = flowCFM * (airDensityAct / airDensityStd);

  return SCFM;
  
}


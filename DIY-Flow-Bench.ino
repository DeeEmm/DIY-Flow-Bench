/****************************************
 * The DIY Flow Bench project
 * A basic flow bench to measure and display volumetric air flow using an Arduino and common automotive MAF sensor.
 * 
 * For more information please visit our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * Version 1.0.0-alpha
 * 
 * Menu system and displays handed by tcMenu library - https://github.com/davetcc/tcMenu
 * If you want to modify the display or menus, the tcMenu project file is provided in distro - menu.emf
 * 
 */

// Development and release version
#define VERSION "V1.0-Alpha"
#define BUILD "19120201"

#include "DIY-Flow-Bench_menu.h"
#include <EEPROM.h>
#include <Arduino.h>

#include "configuration.h"


/****************************************
 * DECLARE CONSTANTS
 ***/

#define INWG 1
#define KPA 2

/****************************************
 * DECLARE GLOBALS
 ***/

float startupBaroPressure;
int mafFlowThresholdCFM;


/****************************************
 * INITIALISATION
 ***/
void setup ()
{
    setupMenu();// Set up the menu + display system

    //take pressure sensor reading for baro correction if dedicated baro sensor not used
    int startupBaroPressure = analogRead(REF_PRESSURE_PIN);
}


/****************************************
 * GET BOARD VOLTAGE
 ***/
float getSupplyMillivolts()
{   
    int rawVoltageValue = analogRead(VOLTAGE_PIN);
    int supplyMillivolts = rawVoltageValue * (5.0 / 1023.0) * 100;

    return supplyMillivolts;
}




/****************************************
 * GET BAROMETRIC CORRECTION
 ***/
float getBaroPressure()
{   

    float baroPressureKpa;
    int supplyMillivolts = getSupplyMillivolts();
    int rawBaroValue = analogRead(REF_BARO_PIN);
    int baroMillivolts = (rawBaroValue * (5.0 / 1023.0)) * 1000;

    #ifdef BARO_MPX4115
        // Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
        // P = ((Vout / VS ) - 0.095) / 0.009 --- Formula transposed for P
        baroPressureKpa = ((baroMillivolts / supplyMillivolts ) - 0.095) / 0.009; 
    #elif BARO_BMP280
        //TODO
    #else
        // No baro sensor so use value grabbed at startup from reference pressure sensor
        // NOTE will only work for absolute style pressure sensor
        baroPressureKpa = (startupBaroPressure * startupBaroScalingFactor) + startupBaroScalingOffset; 
    #endif

    return baroPressureKpa;
}




/****************************************
 * CALCULATE FLOW in CFM
 * Convert RAW MAF sensor data into flow rate and display it
 *
 * Linear interpolation formula Y=Y1+(X−X1)(Y2−Y1/X2−X1)
 *
 * TONYS TEST RESULTS = X:2.2 Y:27.6 | X:3.2 Y:106.9
 ***/
float getMafFlowCFM()
{   
    float mafFlowRateCFM;
    int rawMafValue = analogRead(MAF_PIN);
    int mafMillivolts = (rawMafValue * (5.0 / 1023.0)) * 1000;

    #ifdef SIEMENS__5WK9605
        
        mafFlowRateCFM = 27.6 + ((mafMillivolts / 1000)-2.2) * 79.3;   
        mafFlowThresholdCFM = 2;
    
    #elif SOME_OTHER_SENSOR
        //SOME OTHER SENSORS CALCS
    #elif ADD_YOUR_OWN_SENSOR
        //YOUR SENSORS CALCS
    #endif

    return mafFlowRateCFM;
    
}



/****************************************
 * CALCULATE REFERENCE PRESSURE IN/WG
 * Convert RAW pressure sensor data
 ***/
float getRefPressure (int units) 
{   

    float refPressureKpa;
    float refPressureInWg;
    int rawRefPressValue = analogRead(REF_PRESSURE_PIN);
    int refPressMillivolts = (rawRefPressValue * (5.0 / 1023.0)) * 1000;
    int boardMillivolts = getSupplyMillivolts();

    #ifdef REF_MPXV7007
        // Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
        // P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
        refPressureKpa = ((refPressMillivolts / boardMillivolts ) - 0.5) / 0.057; 

    #elif REF_BMP280
        //TODO
    #endif

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
    }
    
}



/****************************************
 * CALCULATE TEMPERATURE
 * Convert RAW temperature sensor data
 ***/
float getTemperature()
{   
    float temperature;

    //TODO - get temperature from sensor
    temperature = 21;

    return temperature;
}



/****************************************
 * CALCULATE PITOT PROBE
 * Convert RAW temperature sensor data
 ***/
float getPitotPressure(int units)
{   
    float refPressureKpa;
    float refPressureInWg;
    int rawRefPressValue = analogRead(PITOT_PIN);
    int refPressMillivolts = (rawRefPressValue * (5.0 / 1023.0)) * 1000;
    int boardMillivolts = getSupplyMillivolts();

    #ifdef REF_MPXV7007
        // Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
        // P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
        refPressureKpa = ((refPressMillivolts / boardMillivolts ) - 0.5) / 0.057; 

    #elif REF_BMP280
        //TODO
    #endif

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
    }
    
}

/****************************************
 * CONVERT FLOW
 * Convert flow values between different reference pressures
 * Flow at the new pressure drop = (the square root of (new pressure drop/old pressure drop)) times CFM at the old pressure drop.
 ***/
float convertMafFlowInWg(float inputPressure = 10, int outputPressure = 28, float inputFlow = 0 )
{
    float outputFlow;
    
        outputFlow = (sqrt(outputPressure / inputPressure) * inputFlow);

    return outputFlow;

}


/****************************************
 * UPDATE DISPLAYS
 *
 * Displays driven by tcMenu library
 ***/
void updateDisplays()
{

    int desiredRefPressureInWg = menuDesiredRef.getCurrentValue();
    float mafFlowCFM = getMafFlowCFM();
    float refPressure = getRefPressure(INWG);

    // Main Menu
    // Flow Rate
    if (mafFlowCFM > mafFlowThresholdCFM)
    {
        menuFlowRate.setCurrentValue(mafFlowCFM);   
    } else {
        menuFlowRate.setCurrentValue(0);   
    }
    // Reference pressure
    menuRefPressure.setCurrentValue(refPressure);
    // Temperature
    menuTemperature.setCurrentValue(getTemperature());
    // Pitot
    menuPitot.setCurrentValue(getPitotPressure(INWG));
    // Adjusted Flow
    menuAdjustedFlow.setCurrentValue(convertMafFlowInWg(refPressure, desiredRefPressureInWg, mafFlowCFM));


    //Settings Menu
    menuSettingsCodeVersion.setTextValue(VERSION);
    menuSettingsBuildNumber.setTextValue(BUILD);
    

    #ifdef CFM_4X7SEG
        //TODO
    #endif

    #ifdef PITOT_4X7SEG
        //TODO
    #endif

    #ifdef MPXV7007 
        menuRefPressure.setCurrentValue(refPressureInWg);
    #elif BMP280

    #endif


}



/****************************************
 * MENU CALLBACK FUNCTIONS
 *
 * Menus driven by tcMenu library
 * --------------------------------------
 * NOTE: Menu names are used for reference 
 * Spaces are removed and name converted to camelCase
 * For example:
 * bool changed = menuFlowRefCal.isChanged();
 * 
 * MENU NAMES USED
 * --------------------------------------
 * menuFlowRate
 * menuRefPressure
 * menuTemperature
 * menPitot
 * menuDesiredRef
 * menuAdjustedFlow
 * menuSettingsCodeVersion
 * menuSettingsBuild
 * menuSettingsLowFlowCal
 * menuSettingsHighFlowCal
 * menuSettingsLeakTestCal
 * menuSettingsLeakTestCheck
 *
 * AVAILABLE METHODS
 * --------------------------------------
 * menuFoo.setCurrentValue(newValue);
 * NOTE: You can also set without calling the callback (silent set) like this...
 * menuFoo.setCurrentValue(newValue, true);   
 * bool changed = menuFoo.isChanged();
 * int val = menuFoo.getCurrentValue();
 * menuFoo.setBoolean(newValue)
 * bool b = menuFoo.getBoolean();
 * menuFloat.setFloatValue(1.23456);
 * float fl = menuFloat.getFloatValue();
 * const char * charString = "Wilma";
 * menuText.setTextValue(charString);
 * const char* text = menuText.getTextValue();
 * 
 ***/



/****************************************
 * MENU CALLBACK FUNCTION
 * setLowFlowCalibrationValue
 ***/
void CALLBACK_FUNCTION setLowFlowCalibrationValue(int id) {

    float MafFlowCFM = getMafFlowCFM();
    float RefPressure = getRefPressure(INWG);
    float convertedMafFlowCFM = convertMafFlowInWg(RefPressure, 28,  MafFlowCFM);
    float flowCalibrationValue = calibrationPlateLowCFM - convertedMafFlowCFM;
    //Store data in EEPROM
    EEPROM.write(NVM_LOW_FLOW_CAL_ADDR, flowCalibrationValue);

}

/****************************************
 * MENU CALLBACK FUNCTION
 * setHighFlowCalibrationValue
 ***/
void CALLBACK_FUNCTION setHighFlowCalibrationValue(int id) {

    float MafFlowCFM = getMafFlowCFM();
    float RefPressure = getRefPressure(INWG);
    float convertedMafFlowCFM = convertMafFlowInWg(RefPressure, 28,  MafFlowCFM);
    float flowCalibrationValue = calibrationPlateHighCFM - convertedMafFlowCFM;
    //Store data in EEPROM
    EEPROM.write(NVM_HIGH_FLOW_CAL_ADDR, flowCalibrationValue);

}

/****************************************
 * MENU CALLBACK FUNCTION
 * setLeakCalibrationValue
 ***/
void CALLBACK_FUNCTION setLeakCalibrationValue(int id) {

    float RefPressure = getRefPressure(INWG);  
    //Store data in EEPROM
    EEPROM.write(NVM_LEAK_CAL_ADDR, RefPressure);
}

/****************************************
 * MENU CALLBACK FUNCTION
 * checkLeakCalibration
 ***/
void CALLBACK_FUNCTION checkLeakCalibration(int id) {

    int leakCalibrationValue = 0; 
    leakCalibrationValue = EEPROM.read(NVM_LEAK_CAL_ADDR);
    int refPressure = getRefPressure(INWG);

    //compare calibration data from NVM
    if (leakCalibrationValue < (refPressure - leakTestTolerance))
    {
       menuSettingsLeakTestCheck.setTextValue("TEST OK");
    } else {
       menuSettingsLeakTestCheck.setTextValue("TEST FAIL");
    }

}


/****************************************
 * MAIN PROGRAM LOOP
 ***/
void loop ()
{
    taskManager.runLoop(); //run tcMenu
    updateDisplays();

}
















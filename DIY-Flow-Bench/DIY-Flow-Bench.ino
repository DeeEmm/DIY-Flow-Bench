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
 * Menu system and displays handled by tcMenu library - https://github.com/davetcc/tcMenu
 * If you want to modify the display or menus, the tcMenu project file is provided in distro - menu.emf
 * For more information see - https://github.com/DeeEmm/DIY-Flow-Bench/wiki/Customisation
 * 
 ***/

// Development and release version - Don't forget to update the changelog!!
#define VERSION "V1.0-Alpha"
#define BUILD "19120804"

#include "DIY-Flow-Bench_menu.h"
#include <EEPROM.h>
#include <Arduino.h>
#include "configuration.h"
#include "EN_Language.h"


/****************************************
 * DECLARE CONSTANTS
 ***/

// Pressure units
#define INWG 1
#define KPA 2

// Error handler codes
#define REF_PRESS_LOW 1
#define LEAK_TEST_FAILED 2
#define LEAK_TEST_PASS 3



/****************************************
 * DECLARE GLOBALS
 ***/

float startupBaroPressure;
int mafFlowThresholdCFM;
int errorVal = 0;

bool benchIsRunning();
//void errorHandler();
//void onDialogFinished();

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
        //TODO(#4) Add support for stand alone baro sensor
    #else
        // No baro sensor so use value grabbed at startup from reference pressure sensor
        // NOTE will only work for absolute style pressure sensor
        baroPressureKpa = (startupBaroPressure * startupBaroScalingFactor) + startupBaroScalingOffset; 
    #endif

    return baroPressureKpa;
}




/****************************************
 * GET MAF FLOW in CFM
 * Lookup CFM value from MAF data array
 ***/
float getMafFlowCFM ()
{
    // NOTE mafMapData is global array declared in mafData files
    float mafFlowRateCFM;
    int rawMafValue = analogRead(MAF_PIN);
    int mafMillivolts = (rawMafValue * (5.0 / 1023.0)) * 1000;
    int mafMapDataLength = sizeof(mafMapData) / sizeof(int); 
    int arrayPos;
    int lookupValue;
    float calibrationOffset;
    

    // determine what kind of array data we have
    if (mafMapDataLength >= 1023) {
        // we have a raw analog data array so we use the rawMafValue for the lookup
        int lookupValue = rawMafValue;
    } else {
        // we have a mV / cfm array so we use the mafMillivolts value for the lookup
        int lookupValue = mafMillivolts;
    }

    // traverse the array until we find the lookupValue
    for (int loopCount = 0; loopCount < mafMapDataLength; loopCount++) {

        // check to see if exact match is found 
        if (lookupValue == mafMapData[loopCount][1]) {
            // we've got the exact value
            mafFlowRateCFM = mafMapData[loopCount][2];
            break;

        // if there is no match we need to interpolate using the next highest / lowest values
        } else if ( (lookupValue > mafMapData[loopCount][1])) {
            // NOTE: Linear interpolation formula Y=Y1+(X-X1)(Y2-Y1/X2-X1)

            mafFlowRateCFM = (mafMapData[loopCount-1][2] + (lookupValue - mafMapData[loopCount-1][1]) * ((mafMapData[loopCount][2] - mafMapData[loopCount-1][2]) / (mafMapData[loopCount][1] - mafMapData[loopCount-1][1])));
            break;
        }
    }

    // Get calibration offset from NVM
    EEPROM.get( NVM_CD_CAL_OFFSET_ADDR, calibrationOffset ); //

    // convert stored CFM datavalue back into cfm
    mafFlowRateCFM = (mafFlowRateCFM / 1000) + calibrationOffset; 
    
    return mafFlowRateCFM;
    
}



/****************************************
 * CALCULATE REFERENCE PRESSURE
 * Convert RAW pressure sensor data to In/WG or kPa
 ***/
float getRefPressure (int units) 
{   

    float refPressureKpa;
    float refPressureInWg;
    int rawRefPressValue = analogRead(REF_PRESSURE_PIN);
    int refPressMillivolts = (rawRefPressValue * (5.0 / 1023.0)) * 1000;
    int supplyMillivolts = getSupplyMillivolts();

    #ifdef REF_MPXV7007
        // Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
        // P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
        refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057; 

    #elif REF_BMP280
        //TODO(#1) Add support for additional pressure sensors
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

    //TODO(#2) Add support for temperature sensor
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
    int supplyMillivolts = getSupplyMillivolts();

    #ifdef REF_MPXV7007
        // Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
        // P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
        refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057; 

    #elif REF_BMP280
        //TODO(#1) Add support for additional pressure sensors
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
 * CHECK REFERENCE PRESSURE
 * Make sure that reference pressure is within limits
 ***/
void refPressureCheck()
{
    float refPressure = getRefPressure(INWG);

    // Check that pressure does not fall below limit set by minTestPressurePercentage when bench is running
    if ((refPressure > (calibrationRefPressure * (minTestPressurePercentage / 100))) && (benchIsRunning()))
    {
        errorVal = REF_PRESS_LOW;
    }
}



/****************************************
 * ERROR CHECKING
 * 
 * Check for system errors and display message to user on screen
 ***/
void errorHandler(int errorVal)
{
    switch (errorVal)
    {
        case REF_PRESS_LOW:
            displayDialog(LANG_WARNING, LANG_FLOW_LIMIT_EXCEEDED);
        break;

        case LEAK_TEST_FAILED:
            displayDialog(LANG_WARNING, LANG_LEAK_TEST_FAILED);
        break;

        case LEAK_TEST_PASS:
            displayDialog(LANG_WARNING, LANG_LEAK_TEST_PASS);
        break;
    }

}


/****************************************
 * BENCH IS RUNNING
 ***/
 bool benchIsRunning()
{
    float refPressure = getRefPressure(INWG);
    if (refPressure > minRefPressure)
    {
        return true;
    } else {
        return false;
    }
}


/****************************************
 * MENU DIALOG CALLBACK HANDLER
 ***/
void onDialogFinished(ButtonType btnPressed, void* /*userdata*/) {        
    if(btnPressed != BTNTYPE_OK) {
        // do something if OK was pressed.
        // Reset errorVal
        errorVal = 0;
    } else {
      
    }
}



/****************************************
 * DISPLAY POPUP DIALOG ON MAIN DISPLAY
 ***/
void displayDialog(const char *dialogTitle, const char *dialogMessage)
{
        // Display popup dialog on display
        // https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-menu-item-types-tutorial/
        BaseDialog* dlg = renderer.getDialog();
        dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL, 1);
        dlg->show(dialogTitle, showRemoteDialogs, onDialogFinished);
        dlg->copyIntoBuffer(dialogMessage);
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
        //TODO(#6) Add support for additional displays
    #endif

    #ifdef PITOT_4X7SEG
        //TODO(#6) Add support for additional displays
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
 * menuSettingsMidFlowCal
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

//TODO(#7) Update calibration functions
//TODO(#8) - Add menu item to be able to select reference pressure value




/****************************************
 * MENU CALLBACK FUNCTION
 * setFlowOffsetCalibrationValue
 ***/
void CALLBACK_FUNCTION setCalibrationOffset(int id) {

    float MafFlowCFM = getMafFlowCFM();
    float RefPressure = getRefPressure(INWG);
    float convertedMafFlowCFM = convertMafFlowInWg(RefPressure, calibrationRefPressure,  MafFlowCFM);
    float flowCalibrationOffset = calibrationFlowRate - convertedMafFlowCFM;

    char flowCalibrationOffsetText[12]; // Buffer big enough?
    dtostrf(flowCalibrationOffset, 6, 2, flowCalibrationOffsetText); // Leave room for too large numbers!
      
    //Store data in EEPROM
    EEPROM.write(NVM_CD_CAL_OFFSET_ADDR, flowCalibrationOffset);

    // Display the value on the main screen
    displayDialog(LANG_CAL_OFFET_VALUE, flowCalibrationOffsetText);

}


/****************************************
 * MENU CALLBACK FUNCTION
 * setLowFlowCalibrationValue
 ***/
void CALLBACK_FUNCTION setLowFlowCalibrationValue(int id) {

    float MafFlowCFM = getMafFlowCFM();
    float RefPressure = getRefPressure(INWG);
    float convertedMafFlowCFM = convertMafFlowInWg(RefPressure, calibrationRefPressure,  MafFlowCFM);
    float flowCalibrationOffset = calibrationPlateLowCFM - convertedMafFlowCFM;
    //Store data in EEPROM
    EEPROM.write(NVM_LOW_FLOW_CAL_ADDR, flowCalibrationOffset);

}


/****************************************
 * MENU CALLBACK FUNCTION
 * setHighFlowCalibrationValue
 ***/
void CALLBACK_FUNCTION setHighFlowCalibrationValue(int id) {

    float MafFlowCFM = getMafFlowCFM();
    float RefPressure = getRefPressure(INWG);
    float convertedMafFlowCFM = convertMafFlowInWg(RefPressure, calibrationRefPressure,  MafFlowCFM);
    float flowCalibrationOffset = calibrationPlateHighCFM - convertedMafFlowCFM;

    //Store data in EEPROM
    EEPROM.write(NVM_HIGH_FLOW_CAL_ADDR, flowCalibrationOffset);

}

/****************************************
 * MENU CALLBACK FUNCTION
 * setRefPressCalibrationValue
 ***/
void CALLBACK_FUNCTION setRefPressCalibrationValue(int id) {

    float RefPressure = getRefPressure(INWG);
    
    //Store data in EEPROM
    EEPROM.write(NVM_REF_PRESS_CAL_ADDR, RefPressure);

}

/****************************************
 * MENU CALLBACK FUNCTION
 * setLeakCalibrationValue
 ***/
void CALLBACK_FUNCTION setLeakCalibrationValue(int id) {

    float RefPressure = getRefPressure(INWG);  
    char RefPressureText[12]; // Buffer big enough?
    dtostrf(RefPressure, 6, 2, RefPressureText); // Leave room for too large numbers!
    
    //Store data in EEPROM
    EEPROM.write(NVM_LEAK_CAL_ADDR, RefPressure);

    // Display the value on the main screen
    displayDialog(LANG_LEAK_CAL_VALUE, RefPressureText);    
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
    if (leakCalibrationValue > (refPressure - leakTestTolerance))
    {
    // Display the value on the main screen
      errorVal = LEAK_TEST_FAILED;       
    } else {
      errorVal = LEAK_TEST_PASS;       
    }

}




/****************************************
 * MAIN PROGRAM LOOP
 ***/
void loop ()
{
    taskManager.runLoop(); //run tcMenu
    refPressureCheck();
    if (errorVal != 0) errorHandler(errorVal);
    updateDisplays();

}

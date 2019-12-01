
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
 * If you want to modify the display or menus tcMenu project file located in /tcMenu folder
 * 
 */

// Development and release version
#define VERSION "V1.0-Alpha"
#define BUILD "19120101"

#include "DIY-Flow-Bench_menu.h"
#include <Arduino.h>

//#include "controls.h"
#include "configuration.h"


/****************************************
 * DECLARE GLOBALS
 ***/
int currentVacValue;
int mafFlowRateCFM;

void calculateMafFlow();

/****************************************
 * INITIALISATION
 ***/
void setup ()
{
    setupMenu();// Set up the menu + display system
}


/****************************************
 * MAIN PROGRAM LOOP
 ***/
void loop ()
{
    taskManager.runLoop(); //run tcMenu
    calculateMafFlow();
//    calculateSensor1Pressure();
//    calculateSensor2Pressure();

}



/****************************************
 * MENU CALLBACK FUNCTIONS
 *
 * NOTE: Menu names are used for reference 
 * bool changed = menuFoo.isChanged();
 * becomes 
 * bool changed = menuFlowRefCal.isChanged();
 * 
 * MENU NAMES USED
 * menuFlowRate
 * menuRefPressure
 * menuTemperature
 * menPitot
 * menuSettingsCodeVersion
 * menuSettingsBuild
 * menuSettingsFlowRefCal
 * menuSettingsFlowRefCheck
 * menuSettingsLeakTestCal
 * menuSettingsLeakTestCheck
 *
 * AVAILABLE METHODS
 * bool changed = menuFoo.isChanged();
 * menuFoo.setCurrentValue(newValue);
 * NOTE: You can also set without calling the callback (silent set) like this...
 * menuFoo.setCurrentValue(newValue, true);   
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

//TODO
void CALLBACK_FUNCTION checkFlowCalibration(int id) {
    //Retrieve calibration data from NVM
//    return flowCalibrationValue;
}

//TODO
void CALLBACK_FUNCTION setLeakCalibrationValue(int id) {

    //we need to save the calibration data to non-volatile memory on the arduino

    // we can return boolean to indicate that write has been successful (error checking)
}

//TODO
void CALLBACK_FUNCTION setFlowCalibrationValue(int id) {

    // we need to save the calibration data to non-volatile memory on the arduino

    // we can return boolean to indicate that write has been successful (error checking)
}

//TODO
void CALLBACK_FUNCTION checkLeakCalibration(int id) {

    int leakCalibrationValue = 0; 
    int rawvacValue;
    float vacVoltage;
    rawvacValue = analogRead(REF_VAC_PIN);

    // get value from NVM
    // (need to add this)
    //leakCalibrationValue = scaled value from vac signal;

    // lower the test value to define test tolerance
    // NOTE, this will essentially determine bench accuracy
    leakCalibrationValue -= calibrationToleranceCFM;

    //compare calibration data from NVM
    if (leakCalibrationValue >= currentVacValue )
    {
 //       return true;
    } else {
//        return false;
    }

}




/****************************************
 * CALCULATE FLOW
 * Convert RAW MAF sensor data into flow rate and display it
 ***/
void calculateMafFlow()
{   
    int rawMafValue;
    float MafVoltage;
    rawMafValue = analogRead(MAF_PIN);
    MafVoltage = (rawMafValue * (5.0 / 1023.0))*100;

    #ifdef SIEMENS__5WK9605
        mafFlowRateCFM = MafVoltage * 0.1821 + 2;
    #endif

    #ifdef SOME_OTHER_SENSOR
        //SOME OTHER SENSORS CALCS
    #endif
    
    #ifdef ADD_YOUR_OWN_SENSOR
        //YOUR SENSORS CALCS
    #endif
    
}


/****************************************
 * CONVERT FLOW
 * Convert flow values between different reference pressures
 ***/
float convertMafFlow(int inputPressure = 10, int outputPressure = 28 , int inputFlow = 0 )
{
    float outputFlow = 0;
    
        //TODO

    return outputFlow;

}


/****************************************
 * UPDATE DISPLAYS
 ***/
void updateDisplays()
{
    //Main Menu
    menuFlowRate.setCurrentValue(mafFlowRateCFM);   
    //Settings Menu
    menuSettingsCodeVersion.setTextValue(VERSION);
    menuSettingsDevBuild.setTextValue(BUILD);
    

    #ifdef CFM_4X7SEG
        //TODO
    #endif

    #ifdef PITOT_4X7SEG
        //TODO
    #endif


}






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
 * Default display used is DFRobot 1602 LCD Keypad Shield
 * 
 ***/

// Development and release version - Don't forget to update the changelog!!
#define VERSION "V1.0-Beta"
#define BUILD "19122301"


/****************************************
 * CORE LIBRARIES
 ***/

#include "DIY-Flow-Bench_menu.h"
#include <EEPROM.h>
#include <Arduino.h>
#include "configuration.h"
#include "EN_Language.h"



/****************************************
 * OPTIONAL LIBRARIES
 * Add pre-directives for all optional libraries to preserve memory
 ***/

// Support for ADAFRUIT_BMP280 temp & pressure sensor
// https://www.adafruit.com/product/2651
#if defined REF_ADAFRUIT_BMP280 || defined TEMP_ADAFRUIT_BMP280 || defined BARO_ADAFRUIT_BMP280
    #include <BMP280_DEV.h> 
    BMP280_DEV adafruitBmp280; // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)
#endif

// Support for ILIB_BMP280 temp & pressure sensors
// https://github.com/orgua/iLib
#if defined REF_ILIB_BMP280 || defined TEMP_ILIB_BMP280 || defined BARO_ILIB_BMP280
    #include <i2c_BMP280.h> 
    BMP280 ilibBmp280; 
#endif

// Support for DHT11 humidity / temperature sensors
// https://github.com/winlinvip/SimpleDHT
#if defined RELH_DHT11 || defined TEMP_DHT11
    #include <SimpleDHT.h>  
#endif


/****************************************
 * DECLARE CONSTANTS
 ***/

// standard units
#define INWG 1
#define KPA 2
#define PSIA 3
#define DEGC 4
#define DEGF 5
#define RANKINE 6


// Error handler codes
#define REF_PRESS_LOW 1
#define LEAK_TEST_FAILED 2
#define LEAK_TEST_PASS 3

// Other constants
#define MOLECULAR_WEIGHT_DRY_AIR 28.964



/****************************************
 * DECLARE GLOBALS
 ***/

float startupBaroPressure;
int mafFlowThresholdCFM;
int errorVal = 0;

extern long int mafMapData[][2];

bool benchIsRunning();



/****************************************
 * INITIALISATION
 ***/
void setup ()
{
    // Initialise libraries
    
    // Adafruit or derivative BMP Pressure & temp transducer
    #if defined REF_ADAFRUIT_BMP280 || defined TEMP_ADAFRUIT_BMP280 || defined BARO_ADAFRUIT_BMP280
        adafruitBmp280.begin();          
        //adafruitBmp280.begin(FORCED_MODE, BMP280_I2C_ALT_ADDR);	// Initialise the BMP280 in FORCED_MODE with the alternate I2C address (0x76)                       // Default initialisation, place the BMP280 into SLEEP_MODE 
        //adafruitBmp280.setPresOversampling(OVERSAMPLING_X4);    // Set the pressure oversampling to X4
        //adafruitBmp280.setTempOversampling(OVERSAMPLING_X1);    // Set the temperature oversampling to X1
        //adafruitBmp280.setIIRFilter(IIR_FILTER_4);              // Set the IIR filter to setting 4
        adafruitBmp280.setTimeStandby(TIME_STANDBY_2000MS);     // Set the standby time to 2 seconds
        adafruitBmp280.startNormalConversion();  
    #endif

    // ILIB BMP Pressure & temp transducer
    #if defined REF_ILIB_BMP280 || defined TEMP_ILIB_BMP280 || defined BARO_ILIB_BMP280
        ilibBmp280.initialize();
        ilibBmp280.setEnabled(); //TODO - VERIFY - Have removed zero as it appears zero initialises library as a one-shot read - DM 22.12.19
        ilibBmp280.triggerMeasurement(); 
    #endif

    // Support for DHT11 humidity / temperature sensors
    #if defined RELH_DHT11 || defined TEMP_DHT11
        SimpleDHT11 dht11(HUMIDITY_PIN);
   #endif
 

    // Set up the menu + display system
    setupMenu(); 

    // set reference pressure to default
    menuARef.setCurrentValue(calibrationRefPressure); 

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
 * GET BAROMETRIC pressure (kPa)
 * NOTE: Sensor must return an absolute value
 ***/
float getBaroPressure(int units)
{   

    float baroPressureKpa;
    float baroPressurePsia;
    float baroPressureRaw;
    float refTempRaw;
    float refAltRaw;
    int supplyMillivolts = getSupplyMillivolts();
    int rawBaroValue = analogRead(REF_BARO_PIN);
    int baroMillivolts = (rawBaroValue * (5.0 / 1023.0)) * 1000;

    #if defined BARO_MPX4115
        // Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
        // P = ((Vout / VS ) - 0.095) / 0.009 --- Formula transposed for P
        baroPressureKpa = ((baroMillivolts / supplyMillivolts ) - 0.095) / 0.009; 

    #elif defined BARO_ADAFRUIT_BMP280
        adafruitBmp280.getMeasurements(refTempRaw, baroPressureRaw, refAltRaw); // Deg C | hPa | M
        baroPressureKpa = baroPressureRaw * 10;

    #elif defined BARO_ILIB_BMP280
//        ilibBmp280.awaitMeasurement();
        ilibBmp280.getPressure(baroPressureRaw);
//        ilibBmp280.triggerMeasurement(); 
        baroPressureKpa = baroPressureRaw / 1000;

    #elif defined USE_REF_PRESS
        // No baro sensor defined so use value grabbed at startup from reference pressure sensor
        // NOTE will only work for absolute style pressure sensor else code may need to be changed
        baroPressureKpa = (startupBaroPressure * startupBaroScalingFactor) + startupBaroScalingOffset; 
    #else
        // we dont have any sensor so use standard sealevel baro pressure (14.7 psi)
        baroPressureKpa = 101.3529;
    #endif


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

}



/****************************************
 * CALCULATE TEMPERATURE
 * Convert RAW temperature sensor data
 ***/
float getTemperature(int units)
{   
    float refAltRaw;
    float refPressureRaw;
    float refTempRaw;
    float refTempDegC;
    float refTempDegF;
    float refTempRankine;
    float relativeHumidity;

    #if defined TEMP_ADAFRUIT_BMP280
        adafruitBmp280.getMeasurements(refTempRaw, refPressureRaw, refAltRaw);
        refTempDegC = roundf(refTempRaw*100.0)/100.0;

    #elif defined TEMP_ILIB_BMP280
//        bmp280.awaitMeasurement();
        ilibBmp280.getTemperature(refTempRaw);
//        bmp280.triggerMeasurement();
        refTempDegC = roundf(refTempRaw*100.0)/100.0;

    #elif defined TEMP_DHT11
        // NOTE DHT11 sampling rate is 1HZ. we may need to slow down read rate to every few secs
        dht11.read(&refTempDegC, &relativeHumidity, NULL)

    #else
        // We don't have any temperature input so we will assume ambient
        refTempDegC = 21;
    #endif

     switch (units)
    {
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
}



/****************************************
 * GET RELATIVE HUMIDITY %
 ***/
float getRelativeHumidity()
{   
    float relativeHumidity;
    float tempDegC;

    #if defined RELH_DHT11
        // NOTE DHT11 sampling rate is 1HZ. we may need to slow down read rate to every few secs
        dht11.read(&tempDegC, &relativeHumidity, NULL)

    #else
        //we dont have any sensor so use standard value
        relativeHumidity = 0.36;
    #endif

    return relativeHumidity;
    
}  
    



/****************************************
 * GET VAPOR PRESSURE
 ***/
float getVaporPressureKpa(int units)
{   
    float airTemp = getTemperature(DEGC);
    float molecularWeightOfDryAir = 28.964;
    float vapourPressureKpa =(0.61078 * exp((17.27 * airTemp)/(airTemp + 237.3))); // Tetans Equasion
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

}  
    



/****************************************
 * GET SPECIFIC GRAVITY
 ***/
float getSpecificGravity()
{   
    float specificGravity;
    float relativeHumidity = getRelativeHumidity();
    float vaporPressurePsia = getVaporPressureKpa(PSIA);
    float baroPressure = getBaroPressure(PSIA);

    specificGravity = (1-(0.378 * relativeHumidity * vaporPressurePsia) / baroPressure);
    
    return specificGravity;
}  
    



/****************************************
 * CONVERT MASS FLOW TO VOLUMETRIC FLOW
 ***/
float convertMassFlowToVolumetric(float massFlowKgh)
{   
    float mafFlowCFM;
    float tempInRankine = getTemperature(RANKINE);
    float specificGravity = getSpecificGravity();
    float molecularWeight = MOLECULAR_WEIGHT_DRY_AIR * specificGravity;
    float baroPressure = getBaroPressure(PSIA); 

    mafFlowCFM = (massFlowKgh * 1545 * tempInRankine) / (molecularWeight * 144 * baroPressure);


    return mafFlowCFM;
}  
    



/****************************************
 * GET MAF FLOW in CFM
 * Lookup CFM value from MAF data array
 ***/
float getMafFlowCFM()
{
    // NOTE mafMapData is global array declared in mafData files
    float mafFlowRateCFM;
    float mafFlowRateKGH;
    float calibrationOffset;
    int mafFlowRaw = analogRead(MAF_PIN);
    int mafMillivolts = (mafFlowRaw * (5.0 / 1023.0)) * 1000;
    int arrayPos;
    int lookupValue;
    int mafMapDataLength = sizeof(mafMapData) / 2;  

    // determine what kind of MAF data array we have
    if (mafMapDataLength >= 1023) {
        // we have a raw analog data array so we use the mafFlowRaw for the lookup
        int lookupValue = mafFlowRaw;
    } else {
        // we have a mV / cfm array so we use the mafMillivolts value for the lookup
        int lookupValue = mafMillivolts;
    }

    // TODO - Make sure that we are reading MAF data array from lowest value to highest - Add array sort function (qsort?)
    // traverse the array until we find the lookupValue
    for (int loopCount = 0; loopCount < mafMapDataLength; loopCount++) {

        // check to see if exact match is found 
        if (lookupValue == mafMapData[loopCount][1]) {
            // we've got the exact value
            mafFlowRateKGH = mafMapData[loopCount][2];
            break;

        // if there is no match we need to interpolate using the next highest / lowest values
        } else if ( (lookupValue > mafMapData[loopCount][1])) {
            // NOTE: Linear interpolation formula Y=Y1+(X-X1)(Y2-Y1/X2-X1)
            mafFlowRateKGH = (mafMapData[loopCount-1][2] + (lookupValue - mafMapData[loopCount-1][1]) * ((mafMapData[loopCount][2] - mafMapData[loopCount-1][2]) / (mafMapData[loopCount][1] - mafMapData[loopCount-1][1])));
            break;
        }
    }

    // Get calibration offset from NVM
    EEPROM.get( NVM_CD_CAL_OFFSET_ADDR, calibrationOffset ); 

    // convert stored CFM datavalue back into kg/h
    mafFlowRateKGH = (mafFlowRateKGH / 1000) + calibrationOffset; 

    mafFlowRateCFM = convertMassFlowToVolumetric(mafFlowRateKGH);

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
    float refPressureRaw;
    float refTempDegRaw;
    float refAltRaw;
    int rawRefPressValue = analogRead(REF_PRESSURE_PIN);
    int refPressMillivolts = (rawRefPressValue * (5.0 / 1023.0)) * 1000;
    int supplyMillivolts = getSupplyMillivolts();

    #if defined REF_MPXV7007
        // Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
        // P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
        refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057; 

    #elif defined REF_ADAFRUIT_BMP280
        adafruitBmp280.getMeasurements(refTempDegRaw, refPressureRaw, refAltRaw);
        refPressureKpa = refPressureRaw * 10;

    #elif defined REF_ILIB_BMP280
//        ilibBmp280.awaitMeasurement();
        ilibBmp280.getPressure(refPressureRaw);
//        ilibBmp280.triggerMeasurement();       
        refPressureKpa = refPressureRaw / 1000;

    #else
        // No reference pressure sensor used so lets return zero
        refPressureKpa = 0;

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
 * CALCULATE PITOT PROBE
 * Convert RAW differential pressure sensor data
 ***/
float getPitotPressure(int units)
{   
    float pitotPressureKpa;
    float pitotPressureInWg;
    int rawPitotPressValue = analogRead(PITOT_PIN);
    int pitotPressMillivolts = (rawPitotPressValue * (5.0 / 1023.0)) * 1000;
    int supplyMillivolts = getSupplyMillivolts();

    #if defined PITOT_MPXV7007DP
        // Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
        // P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
        pitotPressureKpa = ((pitotPressMillivolts / supplyMillivolts ) - 0.5) / 0.057; 

    #elif defined PITOT_OTHER_TYPE
        // add your sensor data here

    #else
        // No pitot probe used so lets return a zero value
        pitotPressureKpa = 0;

    #endif

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
 * BENCH IS RUNNING
 ***/
bool benchIsRunning()
{
    float refPressure = getRefPressure(INWG);
    float mafFlowRateCFM = getMafFlowCFM();

    if ((refPressure < minRefPressure) && (mafFlowRateCFM > minFlowRate))
    {
        return true;
    } else {
        return false;
    }
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
 * ERROR CHECKING
 * 
 * Check for system errors and display message to user on screen
 ***/
void errorHandler(int errorVal)
{
    if (!showAlarms) return;

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
 * UPDATE DISPLAYS
 *
 * Displays driven by tcMenu library
 ***/
void updateDisplays()
{

    int desiredRefPressureInWg = menuARef.getCurrentValue();
    float mafFlowCFM = getMafFlowCFM();
    float refPressure = getRefPressure(INWG);

    // Main Menu
    // Flow Rate
    if (mafFlowCFM > mafFlowThresholdCFM)
    {
        menuFlow.setFromFloatingPointValue(mafFlowCFM);   
    } else {
        menuFlow.setFromFloatingPointValue(0);   
    }
    // Reference pressure
    menuPRef.setFromFloatingPointValue(refPressure);
    // Temperature
    menuTemp.setFromFloatingPointValue(getTemperature(DEGC));
    // Pitot
    menuPitot.setCurrentValue(getPitotPressure(INWG));
    // Adjusted Flow

    menuAFlow.setFromFloatingPointValue(convertMafFlowInWg(refPressure, desiredRefPressureInWg, mafFlowCFM));

    //Settings Menu
    menuSettingsVer.setTextValue(VERSION);
    menuSettingsBld.setTextValue(BUILD);
    

    #if defined CFM_4X7SEG
        //TODO(#6) Add support for additional displays
    #endif

    #if defined PITOT_4X7SEG
        //TODO(#6) Add support for additional displays
        //menuPitot.setCurrentValue(getPitotPressure(INWG));
    #endif

    #if defined MPXV7007 
        //menuPRef.setFromFloatingPointValue(refPressureInWg);
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
 * menuFlow
 * menuRefe
 * menuTemp
 * menuPitot
 * menuARef
 * menuAFlow
 * menuSettingsVersion
 * menuSettingsBuild
 * menuSettingsLeakTestCal
 * menuSettingsLeakTestChk
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
void CALLBACK_FUNCTION checkLeakCalibrationValue(int id) {

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

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
#define BUILD "20010101"


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
#if defined SIMPLE_RELH_DHT11 || defined SIMPLE_TEMP_DHT11
    #include <SimpleDHT.h>  
    SimpleDHT11 dht11(HUMIDITY_PIN);    
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
#define DHT11_READ_FAIL 4

// Other constants
#define MOLECULAR_WEIGHT_DRY_AIR 28.964



/****************************************
 * DECLARE GLOBALS
 ***/

float startupBaroPressure;
int errorVal = 0;
bool benchIsRunning();

extern long mafMapData[][2];





/****************************************
 * INITIALISATION
 ***/
void setup ()
{

    #ifdef DEV_MODE 
        Serial.begin(9600);
    #endif
  
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
 
    // Set up the menu + display system
    setupMenu(); 
    renderer.setResetIntervalTimeSeconds(-1);

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
    int supplyMillivolts = rawVoltageValue * (5.0 / 1024.0) * 1000;

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
    int baroMillivolts = (rawBaroValue * (5.0 / 1024.0)) * 1000;

    #if defined BARO_MPX4115
        // Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
        // P = ((Vout / VS ) - 0.095) / 0.009 --- Formula transposed for P
        baroPressureKpa = (((float)baroMillivolts / (float)supplyMillivolts ) - 0.095) / 0.009; 

    #elif defined BARO_ADAFRUIT_BMP280
        adafruitBmp280.getMeasurements(refTempRaw, baroPressureRaw, refAltRaw); // Deg C | hPa | M
        baroPressureKpa = baroPressureRaw * 10;

    #elif defined BARO_ILIB_BMP280
        ilibBmp280.awaitMeasurement();
        ilibBmp280.getPressure(baroPressureRaw);
        ilibBmp280.triggerMeasurement(); 
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
    byte refTemp;
    byte refRelh;

    #if defined TEMP_ADAFRUIT_BMP280
        adafruitBmp280.getMeasurements(refTempRaw, refPressureRaw, refAltRaw);
        refTempDegC = roundf(refTempRaw*100.0)/100.0;

    #elif defined TEMP_ILIB_BMP280
        ilibBmp280.awaitMeasurement();
        ilibBmp280.getTemperature(refTempRaw);
        ilibBmp280.triggerMeasurement();
        refTempDegC = roundf(refTempRaw*100.0)/100.0;

    #elif defined SIMPLE_TEMP_DHT11
        // NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
        int err = SimpleDHTErrSuccess;
        if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
          int errorVal = DHT11_READ_FAIL; // Set error to display on screen
          refTempDegC = 0;        
        } else {
          refTempDegC = refTemp;
        }
        
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
    byte refTemp;
    byte refRelh;

    #if defined SIMPLE_RELH_DHT11
        // NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
        int err = SimpleDHTErrSuccess;
        if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
          int errorVal = DHT11_READ_FAIL; // Set error to display on screen
          relativeHumidity = 0;        
        } else {
          relativeHumidity = refRelh;
        }

    #else
        //we dont have any sensor so use standard value
        relativeHumidity = 0.36;
    #endif

    return relativeHumidity;
    
}  
    



/****************************************
 * GET VAPOR PRESSURE
 ***/
float getVaporPressure(int units)
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
    float vaporPressurePsia = getVaporPressure(PSIA);
    float baroPressurePsia = getBaroPressure(PSIA);

    specificGravity = (1-(0.378 * relativeHumidity * vaporPressurePsia) / baroPressurePsia);
    
    return specificGravity;


}  
    



/****************************************
 * CONVERT MASS FLOW TO VOLUMETRIC FLOW
 ***/
float convertMassFlowToVolumetric(float massFlowKgh)
{   
  float mafFlowCFM;
  float tempInRankine = getTemperature(RANKINE); //tested ok
  float specificGravity = getSpecificGravity(); //tested ok
  float molecularWeight = MOLECULAR_WEIGHT_DRY_AIR * specificGravity; //tested ok
  float baroPressure = getBaroPressure(PSIA); 
  float massFlowLbm = massFlowKgh * 0.03674371036415;

  mafFlowCFM = ((massFlowLbm * 1545 * tempInRankine) / (molecularWeight * 144 * baroPressure)); 

  return mafFlowCFM;


}  
    



/****************************************
 * GET MAF FLOW in CFM
 * Lookup CFM value from MAF data array
 ***/
float getMafFlowCFM()
{
    // NOTE mafMapData is global array declared in mafData files
    float calibrationOffset;
    float mafFlowRateCFM;
    float mafFlowRateKGH;
    float mafFlowRateKghRAW;
    int mafFlowRaw = analogRead(MAF_PIN);
    int mafMillivolts = (mafFlowRaw * (5.0 / 1024.0)) * 1000;
    int lookupValue;
    int numRows = sizeof(mafMapData)/sizeof(mafMapData[0]);
    //int numCols = sizeof(mafMapData[0])/sizeof(mafMapData[0][0]);

    if (mafMillivolts < MIN_MAF_MILLIVOLTS) {
      return 0;
    }

    // determine what kind of MAF data array we have //TODO - DO WE NEED TO KNOW????????
    if (numRows >= 1024) {
        // we have a raw analog data array so we use the mafFlowRaw for the lookup
        lookupValue = mafFlowRaw;
    } else {
        // we have a mV / cfm array so we use the mafMillivolts value for the lookup
        lookupValue = mafMillivolts;
    }


    // TODO - Make sure that we are reading MAF data array from lowest value to highest - Add array sort function (qsort?)
    // traverse the array until we find the lookupValue
    for (int rowNum = 0; rowNum <= numRows; rowNum++) {
      
        // check to see if exact match is found 
        if (lookupValue == mafMapData[rowNum][0]) {
            // we've got the exact value
            mafFlowRateKghRAW = mafMapData[rowNum][1];
            break;

        // if there is no match we need to interpolate using the next highest / lowest values
        } else if ( (mafMapData[rowNum][0]) > lookupValue) {
            // NOTE: Linear interpolation formula Y=Y0+(((X-X0)(Y1-Y0))/(X1-X0)) where Y = flow and X = Volts

            if (rowNum == 0) {
              // Flow lower than lowest value in table so we set it to zero and consider it no flow
              mafFlowRateKghRAW = 0;
            } else {
              // Flow value is valid so lets convert it.
              mafFlowRateKghRAW = mafMapData[rowNum-1][1] + (((lookupValue - mafMapData[rowNum-1][0]) * (mafMapData[rowNum][1] - mafMapData[rowNum-1][1])) / (mafMapData[rowNum][0] - mafMapData[rowNum-1][0]));            
            }
            break;
        }

    }



    // Get calibration offset from NVM
    EEPROM.get( NVM_CD_CAL_OFFSET_ADDR, calibrationOffset ); 

    // convert RAW datavalue back into kg/h
    mafFlowRateKGH = float(mafFlowRateKghRAW / 1000); 

    // convert kg/h into cfm
    mafFlowRateCFM = convertMassFlowToVolumetric(mafFlowRateKGH);// + calibrationOffset; // add calibration offset to value //TODO #21 Need to validate and test calibration routine

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
    int supplyMillivolts = getSupplyMillivolts();
    int rawRefPressValue = analogRead(REF_PRESSURE_PIN);
    int refPressMillivolts = (rawRefPressValue * (5.0 / 1024.0)) * 1000;

    #if defined REF_MPXV7007
        // Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
        // P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
        refPressureKpa = (((float)refPressMillivolts / (float)supplyMillivolts ) - 0.5) / 0.057; 

    #elif defined REF_ADAFRUIT_BMP280
        adafruitBmp280.getMeasurements(refTempDegRaw, refPressureRaw, refAltRaw);
        refPressureKpa = refPressureRaw * 10;

    #elif defined REF_ILIB_BMP280
        ilibBmp280.awaitMeasurement();
        ilibBmp280.getPressure(refPressureRaw);
        ilibBmp280.triggerMeasurement();       
        refPressureKpa = refPressureRaw / 1000;

    #else
        // No reference pressure sensor used so lets return zero
        //refPressureKpa = 6.97448943333324; //28"
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
    float pitotPressureKpa = 0.00;
    float pitotPressureInWg;
    int supplyMillivolts = getSupplyMillivolts() / 1000;
    int rawPitotPressValue = analogRead(PITOT_PIN);     
    int pitotPressMillivolts = (rawPitotPressValue * (5.0 / 1024.0)) * 1000;

    #if defined PITOT_MPXV7007DP
        // sensor characteristics from datasheet
        // Vout = VS x (0.057 x P + 0.5)

        pitotPressureKpa = ((float)rawPitotPressValue / (float)1024 - 0.5) / 0.057;

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

    if ((refPressure < MIN_BENCH_PRESSURE) && (mafFlowRateCFM > MIN_FLOW_RATE))
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

        case DHT11_READ_FAIL:
            displayDialog(LANG_WARNING, LANG_DHT11_READ_FAIL);
        break;

       
    }

}




/****************************************
 * UPDATE DISPLAYS
 *
 * Displays driven by tcMenu library
 * 
 * NOTE: display values need to be multiplied by divisor setting in TC menu to display decimal points
 ***/
void updateDisplays()
{

    float mafFlowCFM = getMafFlowCFM();
    float refPressure = getRefPressure(INWG);   

    // Main Menu
    // Flow Rate
    if (mafFlowCFM > MIN_FLOW_RATE)
    {
        menuFlow.setCurrentValue(mafFlowCFM * 100);   
    } else {
        menuFlow.setCurrentValue(0);   
    }

    // Temperature
    menuTemp.setCurrentValue(getTemperature(DEGC) * 10);   
    
    // Pitot
    menuPitot.setFloatValue(getPitotPressure(INWG));  
    
    // Reference pressure
    menuPRef.setCurrentValue(refPressure * 10);   
    
    // Adjusted Flow
    int desiredRefPressureInWg = menuARef.getCurrentValue();
    menuAFlow.setCurrentValue(convertMafFlowInWg(refPressure, desiredRefPressureInWg, mafFlowCFM) * 100);    //Scale output for display

    // Relative Humidity
    menuRelH.setCurrentValue(getRelativeHumidity() * 10);   

    //Maf Voltage
    int mafFlowRaw = analogRead(MAF_PIN);
    int mafVolts = (mafFlowRaw * (5.0 / 1024.0));
    menuMafVolts.setFloatValue(mafVolts);

    //Pitot Voltage
    int pitotRaw = analogRead(PITOT_PIN);
    int pitotVolts = (pitotRaw * (5.0 / 1024.0));
    menuPitotVolts.setFloatValue(pitotVolts);


    //Settings Menu
    menuSettingsVer.setTextValue(VERSION);
    menuSettingsBld.setTextValue(BUILD);
    

    // Additional Displays

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

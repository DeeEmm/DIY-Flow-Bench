/****************************************
 * The DIY Flow Bench project
 * A basic flow bench to measure and display volumetric air flow using an Arduino and common automotive MAF sensor.
 * 
 * For more information please visit our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums over aat https://diyflowbench.com 
 *
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * Menu system and displays handled by tcMenu library - https://github.com/davetcc/tcMenu
 * If you want to modify the display or menus, the tcMenu project file is provided in distro - menu.emf
 * Default display used is DFRobot 1602 LCD Keypad Shield
 * 
 ***/

// Development and release version - Don't forget to update the changelog & README Versions!!

#define MAJOR_VERSION "1"
#define MINOR_VERSION "0"
#define BUILD_NUMBER "20080704"
#define RELEASE "V.1.0-beta.14"



/****************************************
 * CORE LIBRARIES
 ***/

#include "DIY-Flow-Bench_menu.h"
#include <EEPROM.h>
#include <Arduino.h>
#include "configuration.h"
#include "pins.h"
#include "EN_Language.h"

/****************************************
 * OPTIONAL LIBRARIES
 * Add pre-directives for all optional libraries to preserve memory
 ***/


// Support for ADAFRUIT_BME280 temp, pressure & Humidity sensors
// https://github.com/adafruit/Adafruit_BME280_Library
#if defined REF_ADAFRUIT_BME280 || defined TEMP_ADAFRUIT_BME280 || defined BARO_ADAFRUIT_BME280
    #include <Adafruit_BME280_DEV.h> 
    Adafruit_BME280 adafruitBme280; // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)
#endif

// Support for SPARKFUN_BME280 temp, pressure & Humidity sensors
// https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide?_ga=2.39864294.574007306.1596270790-134320310.1596270790
#if defined RELH_SPARKFUN_BME280 || defined TEMP_SPARKFUN_BME280 || defined BARO_SPARKFUN_BME280
    #include "SparkFunBME280.h"
    #include <Wire.h>
    BME280 SparkFunBME280;
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
#define RAW 0
#define DEC 0
#define INWG 1
#define KPA 2
#define PSIA 3
#define DEGC 4
#define DEGF 5
#define RANKINE 6
#define PERCENT 7
#define BAR 8

// MAF Data Files
#define VOLTAGE 1
#define FREQUENCY 2
#define KEY_VALUE 1
#define RAW_ANALOG 2
#define KG_H 1
#define MG_S 2

// Error handler codes
#define REF_PRESS_LOW 1
#define LEAK_TEST_FAILED 2
#define LEAK_TEST_PASS 3
#define DHT11_READ_FAIL 4

// Other constants
#define MOLECULAR_WEIGHT_DRY_AIR 28.964
#define ABS_REF_PRESS_SENSOR


/****************************************
 * DECLARE GLOBALS
 ***/

bool APIEnabled = true;
bool DEBUG_MAF_DATA = false;
float startupBaroPressure;
int errorVal = 0;
bool benchIsRunning();
int serialData = 0;
extern long mafMapData[][2]; // mafData key > value array
extern long mafMapAnalogData[]; // mafData analog value array





/****************************************
 * INITIALISATION
 ***/
void setup ()
{

    #if defined USE_REF_PRESS_AS_BARO && defined REF_MPX4250
        startupBaroPressure = getRefPressure(KPA);
    #else
        startupBaroPressure = DEFAULT_BARO;
    #endif

    //Serial API
    if (APIEnabled) {
        Serial.begin(SERIAL_BAUD_RATE);      
    }
  
    // Initialise libraries

    //Sparkfun BME280 (Testing and working 03.08.20 /DM)
    #if defined RELH_SPARKFUN_BME280 || defined TEMP_SPARKFUN_BME280 || defined BARO_SPARKFUN_BME280
        Wire.begin();
        SparkFunBME280.setI2CAddress(BME280_I2C_ADDR); 
        if (SparkFunBME280.beginI2C() == false) //Begin communication over I2C
        {
            // TODO - need to replace this with error handler
            Serial.println("The sensor did not respond. Please check wiring.");
            //while(1); //Freeze
        }
    #endif
    
    // Adafruit or derivative BME Pressure, humidity & temp transducer
    #if defined TEMP_ADAFRUIT_BME280 || defined BARO_ADAFRUIT_BME280 || defined RELH_ADAFRUIT_BME280
        //I2C address - BME280_I2C_ADDR
        if (!adafruitBme280.begin()) {  
            Serial.println("The sensor did not respond. Please check wiring!");
            while (1);
        }
    #endif

 
    // Set up the menu + display system
    setupMenu(); 
    renderer.setResetIntervalTimeSeconds(-1);

    // set reference pressure to default
    menuARef.setCurrentValue(calibrationRefPressure); 


}


/****************************************
 * GET BOARD VOLTAGE
 ***/
float getSupplyMillivolts()
{   
    int rawVoltageValue = analogRead(VOLTAGE_PIN);
    float supplyMillivolts = rawVoltageValue * (5.0 / 1024.0) * 1000;

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
        // Datasheet - https://html.alldatasheet.es/html-pdf/5178/MOTOROLA/MPX4115/258/1/MPX4115.html
        // Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
        // P = ((Vout / VS ) - 0.095) / 0.009 --- Formula transposed for P
        baroPressureKpa = (((float)baroMillivolts / (float)supplyMillivolts ) - 0.095) / 0.009; 

    #elif defined BARO_ADAFRUIT_BME280
        baroPressureKpa =  adafruitBme280.readPressure() / 1000; //Pa

    #elif defined BARO_SPARKFUN_BME280
        baroPressureKpa =  SparkFunBME280.readFloatPressure() / 1000; // Pa

    #elif defined USE_REF_PRESS_AS_BARO
        // No baro sensor defined so use value grabbed at startup from reference pressure sensor
        // NOTE will only work for absolute style pressure sensor like the MPX4250
        baroPressureKpa = startupBaroPressure; 
    #else
        // we dont have any sensor so use default - standard sealevel baro pressure (14.7 psi)
        baroPressureKpa = DEFAULT_BARO;
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
float getTemp(int units)
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


    #if defined TEMP_ADAFRUIT_BME280
        refTempDegC  =  adafruitBme280.readTemperature();

    #elif defined TEMP_SPARKFUN_BME280
        refTempDegC =  SparkFunBME280.readTempC();

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
        // We don't have any temperature input so we will assume default
        refTempDegC = DEFAULT_TEMP;
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
float getRelativeHumidity(int units = 0)
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

    #elif defined RELH_ADAFRUIT_BME280
        relativeHumidity = adafruitBme280.readHumidity(); //%

    #elif defined RELH_SPARKFUN_BME280
        relativeHumidity =  SparkFunBME280.readFloatHumidity();

    #else
        //we dont have any sensor so use standard value 
        relativeHumidity = DEFAULT_RELH; // (36%)
    #endif


    switch (units)
    {
        case DEC:
            return relativeHumidity / 100;
        break;

        case PERCENT:
            return relativeHumidity;
        break;


    }  

}  
    



/****************************************
 * GET VAPOR PRESSURE
 ***/
float getVaporPressure(int units)
{   
    float airTemp = getTemp(DEGC);
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
    float relativeHumidity = getRelativeHumidity(DEC);
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
  float tempInRankine = getTemp(RANKINE); //tested ok
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
    float mafFlowRateRAW;
    int mafFlowRaw = analogRead(MAF_PIN);
    float mafMillivolts = (mafFlowRaw * (5.0 / 1024.0)) * 1000;
    int lookupValue;
    int numRows;

    if (mafMillivolts < MIN_MAF_MILLIVOLTS) {
      return 0;
    }

    if (MAFoutputType == FREQUENCY){
        // TODO #29 - MAF Data File configuration variables - add additional decode variables
        // Add support for frequency based sensors
    }

    // determine what kind of MAF data array we have 
    if (MAFdataFormat == RAW_ANALOG){

        // we have a raw analog data array so we use the mafFlowRaw for the lookup
        lookupValue = mafFlowRaw;

        // get the value directly from the data array
        mafFlowRateRAW = mafMapAnalogData[mafFlowRaw];

    } else {

        //Set size of array
       numRows = sizeof(mafMapData)/sizeof(mafMapData[0]);

        // we have a mV / flow array so we use the mafMillivolts value for the lookup
        lookupValue = mafMillivolts;

        // then traverse the array until we find the lookupValue
        for (int rowNum = 0; rowNum <= numRows; rowNum++) {
        
            // lets check to see if exact match is found 
            if (lookupValue == mafMapData[rowNum][0]) {
                // we've got the exact value
                mafFlowRateRAW = mafMapData[rowNum][1];
                break;

            // We've overshot so lets use the previous value
            } else if ( mafMapData[rowNum][0] > lookupValue ) {

                if (rowNum == 0) {
                    // we was on the first row so lets set the value to zero and consider it no flow
                    return 0;

                } else {
                    // Flow value is valid so lets convert it.
                    // lets use a linear interpolation formula to calculate the actual value
                    // NOTE: Y=Y0+(((X-X0)(Y1-Y0))/(X1-X0)) where Y = flow and X = Volts
                    mafFlowRateRAW = mafMapData[rowNum-1][1] + (((lookupValue - mafMapData[rowNum-1][0]) * (mafMapData[rowNum][1] - mafMapData[rowNum-1][1])) / (mafMapData[rowNum][0] - mafMapData[rowNum-1][0]));            
                }
                break;
            }

        }

    }

    // Get calibration offset from NVM
    EEPROM.get( NVM_CD_CAL_OFFSET_ADDR, calibrationOffset ); 

    if (MAFdataUnit == KG_H) {

        // convert RAW datavalue back into kg/h
        mafFlowRateKGH = float(mafFlowRateRAW / 1000); 

    } else if (MAFdataUnit == MG_S) {

        //  convert mg/s value into kg/h
        mafFlowRateKGH = float(mafFlowRateRAW * 0.0036); 
    }


    // convert kg/h into cfm (NOTE this is approx 0.4803099 cfm per kg/h @ sea level)
    mafFlowRateCFM = convertMassFlowToVolumetric(mafFlowRateKGH);// + calibrationOffset; // add calibration offset to value //TODO #21 Need to validate and test calibration routine

    if (DEBUG_MAF_DATA == true) {
        Serial.print(mafMillivolts);
        Serial.print("mv = ");
        if (MAFdataUnit == KG_H) {
            Serial.print(mafFlowRateRAW / 1000);
            Serial.print("kg/h = ");
        } else if (MAFdataUnit == MG_S) {
            Serial.print(mafFlowRateRAW);
            Serial.print("mg/s = ");
        }
        Serial.print(mafFlowRateCFM );
        Serial.print("cfm \r\n");
    }

    return mafFlowRateCFM;
}



/****************************************
 * CALCULATE REFERENCE PRESSURE
 * Convert RAW pressure sensor data to In/WG or kPa
 ***/
float getRefPressure(int units) 
{   

    float refPressureKpa;
    float refPressureInWg;
    float refPressureRaw;
    float refTempDegRaw;
    float refAltRaw;
    float supplyMillivolts = getSupplyMillivolts();
    int rawRefPressValue = analogRead(REF_PRESSURE_PIN);
    float refPressMillivolts = (rawRefPressValue * (5.0 / 1024.0)) * 1000;

    #if defined REF_MPXV7007
        // Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
        // Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007DP Datasheet)
        // P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
        refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057; 


    #elif defined REF_MPX2050
        // NOTE: Untested
        // Datasheet - https://au.mouser.com/datasheet/2/302/MPX2050-1152068.pdf
        refPressureKpa = (refPressMillivolts / supplyMillivolts ) * 50;         
    

    #elif defined REF_MPX4250
        // NOTE: Untested.  Also not best choice of sensor
        // Datasheet - https://www.nxp.com/files-static/sensors/doc/data_sheet/MPX4250.pdf
        // Vout = VS x (0.00369 x P + 0.04) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
        // P = ((Vout / VS ) - 0.04) / 0.00369 --- Formula transposed for P
        // Note we use the baro value as this is an absolute sensor, so to prevent circular references we need to know
        // if we actually have a Baro sensor installed
        #if defined USE_REF_PRESS_AS_BARO 
            // we don't have a baro value so use the value hardcoded in the config to offset the sensor value
            refPressureKpa = (((refPressMillivolts / supplyMillivolts ) - 0.04) / 0.00369) - DEFAULT_BARO;  
  //          ABS_REF_PRESS_SENSOR       
        #else
            // use the current baro value to offset the sensor value
            refPressureKpa = (((refPressMillivolts / supplyMillivolts ) - 0.04) / 0.00369) - getBaroPressure(KPA);         
        #endif

    #else
        // No reference pressure sensor used so lets return 1 (so as not to throw maths out)
        //refPressureKpa = 6.97448943333324; //28"
        refPressureKpa = DEFAULT_REF_PRESS;

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

        case BAR:
            // 1kpa = 0.01 bar
            return refPressureKpa  * 0.01 ;
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
 *
 * Convert flow values between different reference pressures
 * Flow at the new pressure drop = (the square root of (new pressure drop/old pressure drop)) times CFM at the old pressure drop.
 * An example of the above formula would be to convert flow numbers taken at 28" of water to those which would occur at 25" of water.
 * (25/28) = .89286
 * Using the square root key on your calculator and inputting the above number gives .94489 which can be rounded off to .945.
 * We can now multiply our CFM values at 28" of water by .945 to obtain the theoretical CFM values at 25" of water.
 * Source: http://www.flowspeed.com/cfm-numbers.htm
 ***/
double convertFlowDepression(float oldPressure = 10, int newPressure = 28, float inputFlow = 0 )
{
    double outputFlow;
    double pressureRatio = (newPressure / oldPressure);
    outputFlow = (sqrt(pressureRatio) * inputFlow);

    return outputFlow;

}


/****************************************
 * BENCH IS RUNNING
 ***/
bool benchIsRunning()
{
    float refPressure = getRefPressure(INWG);
    float mafFlowRateCFM = getMafFlowCFM();

    if ((refPressure > MIN_BENCH_PRESSURE) && (mafFlowRateCFM > MIN_FLOW_RATE))
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
    // note alarm commented out in alarm function as 'nag' can get quite annoying
    // Is this a redundant check?
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
//            displayDialog(LANG_WARNING, LANG_REF_PRESS_LOW);
//            Serial.write(LANG_REF_PRESS_LOW);
        break;

        case LEAK_TEST_FAILED:
            displayDialog(LANG_WARNING, LANG_LEAK_TEST_FAILED);
            Serial.write(LANG_LEAK_TEST_FAILED);
        break;

        case LEAK_TEST_PASS:
            displayDialog(LANG_WARNING, LANG_LEAK_TEST_PASS);
            Serial.write(LANG_LEAK_TEST_PASS);
        break;

        case DHT11_READ_FAIL:
            displayDialog(LANG_WARNING, LANG_DHT11_READ_FAIL);
            Serial.write(LANG_DHT11_READ_FAIL);
        break;

       
    }

}


/****************************************
 * SET CALIBRATION OFFSET
 * 
 ***/
 float setCalibrationOffset() {

    float MafFlowCFM = getMafFlowCFM();
    float RefPressure = getRefPressure(INWG);
    float convertedMafFlowCFM = convertFlowDepression(RefPressure, calibrationRefPressure,  MafFlowCFM);
    float flowCalibrationOffset = calibrationFlowRate - convertedMafFlowCFM;

    char flowCalibrationOffsetText[12]; // Buffer big enough?
    dtostrf(flowCalibrationOffset, 6, 2, flowCalibrationOffsetText); // Leave room for too large numbers!
      
    //Store data in EEPROM
    EEPROM.write(NVM_CD_CAL_OFFSET_ADDR, flowCalibrationOffset);

    return flowCalibrationOffset;
}



/****************************************
 * GET CALIBRATION OFFSET
 * 
 ***/
 float getCalibrationOffset() {


 }






/****************************************
 * leakTestCalibration
 ***/
float leakTestCalibration() {

    float RefPressure = getRefPressure(INWG);  
    char RefPressureText[12]; // Buffer big enough?
    dtostrf(RefPressure, 6, 2, RefPressureText); // Leave room for too large numbers!
    
    //Store data in EEPROM
    EEPROM.write(NVM_LEAK_CAL_ADDR, RefPressure);

    // Display the value on the main screen
    displayDialog(LANG_LEAK_CAL_VALUE, RefPressureText);  

    return RefPressure;
}


/****************************************
 * leakTest
 ***/
int  leakTest() {

    int leakCalibrationValue = 0; 
    leakCalibrationValue = EEPROM.read(NVM_LEAK_CAL_ADDR);
    int refPressure = getRefPressure(INWG);

    //compare calibration data from NVM
    if (leakCalibrationValue > (refPressure - leakTestTolerance))
    {   
       return LEAK_TEST_FAILED;
    } else {     
       return LEAK_TEST_PASS;
    }

}


/****************************************
 * CREATE CHECKSUM
 *
 * Create checksum from data passed by value
 * 
 ***/
char createChecksum(char data)
{
//    byte data[20] = ".M+205".
//    int length = 6;                    // or strlen(data)

//    int count = sizeof(data);
//  
//    byte checksum = 0;
//    for ( count i=0; i<length; i++) {
//      checksum += data[i];
//    }
//  
//    byte lowNibble = checksum & 0x0F;                // take the 4 lowest bits
//    byte highNibble = (checksum >> 4) & 0x0F;   // take the 4 highest bits
//  
//    // Convert the lowNibble and highNibble to a readable ascii character
//    if ( lowNibble >= 0x0A)
//      lowNibble += ('A' - 0x0A);
//    else
//      lowNibble += '0';
//  
//    if ( highNibble >= 0x0A)
//      highNibble += ('A' -  - 0x0A);
//    else
//      highNibble += '0';
//  
//    // Add them to the end of the data, high nibble first
//    data[length++] = highNibble;
//    data[length++] = lowNibble;
//  
//    // transmit data with 'length' number of bytes
//
//    return 
//
//
////simple method
//
//    uint8_t result = 0;
//    uint16_t sum = 0;
//    
//    for(uint8_t i = 0; i < (bufferSize - 1); i++){
//      sum += buffer[i];
//    }
//    result = sum & 0xFF;
//    
//    if(originalResult == result){
//       return 1;
//    }else{
//       return 0;
//    }

return 10;  

}



/****************************************
 * PARSE API
 *
 * handle API responses:
 * 
 * General Commmands
 * 'V' - Return firmware version
 * 'F' - Return flow value in cfm
 * 'T' - Return temperature value in deg C
 * 'H' - Return humidity value in RH
 * 'R' - Return reference pressure value in in/h2o
 * 'B' - Return barometric Pressure in KPa
 * Debug Commands
 * 'M' - Return MAF Data (NOTE: will only return data if flow > 0)
 * 'D' - Debug MAF on
 * 'd' - Debug MAF off
 * 'v' - System voltage
 * 'm' - Return MAF sensor voltage
 * 'b' - Return Baro sensor voltage
 * 'r' - Return reference pressure sensor voltage
 * 'h' - Return humidity sensor voltage
 * 't' - Return temperature sensor voltage
 * Calibration commands
 * 'L' - Perform Leak test calibration [+return ok/nok]
 * 'l' - Perform leak test [+return ok/nok]
 * 'O' - Perform offset calibration [+return ok/nok]
 * 
 ***/
void parseAPI(byte serialData)
{

  String serialResponse;
  char checksum = 0;
  double flowCFM = 0.01;
  int intFlowCFM = 0;


  switch (serialData)
  {
      case 'V': // Get Version 'VMmYYMMDDXX\r\n'
          Serial.print('V');
          Serial.print('.');
          Serial.print(MAJOR_VERSION);
          Serial.print('.');
          Serial.print(MINOR_VERSION);
          Serial.print('.');
          Serial.print(BUILD_NUMBER);
          Serial.print("\r\n");
      break;

      case 'L': // Perform Leak Test Calibration 'L\r\n'
          Serial.print('L');
          Serial.print('.');
          Serial.print(leakTestCalibration());
          Serial.print("\r\n");
          // TODO confirm Leak Test Calibration success in response
      break;

      case 'l': // Perform Leak Test 'l\r\n'      
          Serial.print('l');
          Serial.print('.');
          Serial.print(leakTest());
          Serial.print("\r\n");
          // TODO confirm Leak Test success in response
      break;

      case 'O': // Flow Offset Calibration  'O\r\n'        
          Serial.print('O');
          Serial.print('.');
          Serial.print(setCalibrationOffset());
          Serial.print("\r\n");
          // TODO confirm Flow Offset Calibration success in response
      break;

      case 'F': // Get measured Flow 'F123.45\r\n'

          //String stringOne =  String(5.698, 3);                                // using a float and the decimal
      
          Serial.print('F');
          Serial.print('.');
          // Truncate to 2 decimal places
          flowCFM = getMafFlowCFM() * 100;
          intFlowCFM = flowCFM;
          flowCFM = flowCFM / 100;
          // Add preceding zeros
          if (flowCFM < 10) { 
            Serial.print(0);
            Serial.print(0);
          } else if (flowCFM < 100) {
            Serial.print(0);              
          } 
          // Print the value
          Serial.print(flowCFM);
          Serial.print("\r\n");
          // TODO calculate and send checksum
      break;

      case 'M': // Get MAF sensor data'
          Serial.print('M');
          Serial.print('.');
          if (DEBUG_MAF_DATA == false) {
              DEBUG_MAF_DATA = true;
              getMafFlowCFM();
              DEBUG_MAF_DATA = false;         
          }
      break;

      case 'm': // Get MAF output voltage'
          Serial.print('m');
          Serial.print('.');
          Serial.print((analogRead(MAF_PIN) * (5.0 / 1024.0)) * 1000);
          Serial.print("\r\n");
      break;

      case 'T': // Get measured Temperature 'T.123.45\r\n'
          Serial.print('T');
          Serial.print('.');
          Serial.print(getTemp(DEGC));
          Serial.print("\r\n");
      break;

      case 't': // Get Temperature sensor output voltage'
          Serial.print('t');
          Serial.print('.');
          Serial.print((analogRead(TEMPERATURE_PIN) * (5.0 / 1024.0)) * 1000);
          Serial.print("\r\n");
      break;

      case 'H': // Get measured Humidity 'H.123.45\r\n'
          Serial.print('H');
          Serial.print('.');
          Serial.print(getRelativeHumidity(PERCENT));
          Serial.print("\r\n");
      break;

      case 'h': // Get Humidity sensor output voltage'
          Serial.print('h');
          Serial.print('.');
          Serial.print((analogRead(HUMIDITY_PIN) * (5.0 / 1024.0)) * 1000);
          Serial.print("\r\n");
      break;

      case 'R': // Get measured Reference Pressure 'R.123.45\r\n'
          Serial.print('R');
          Serial.print('.');
          Serial.print(getRefPressure(KPA));
          Serial.print("\r\n");
      break;

      case 'r': // Get Reference Pressure sensor output voltage'
          Serial.print('r');
          Serial.print('.');
          Serial.print((analogRead(REF_PRESSURE_PIN) * (5.0 / 1024.0)) * 1000);
          Serial.print("\r\n");
      break;

      case 'B': // Get measured Baro Pressure 'B.123.45\r\n'
          Serial.print('B');
          Serial.print('.');
          Serial.print(getBaroPressure(KPA));
          Serial.print("\r\n");
      break;

      case 'b': // Get Baro Pressure sensor output voltage'
          Serial.print('b');
          Serial.print('.');
          Serial.print((analogRead(REF_BARO_PIN) * (5.0 / 1024.0)) * 1000);
          Serial.print("\r\n");
      break;

      case 'v': // Get board supply voltage (mv) 'v.123.45\r\n'
          Serial.print('v');
          Serial.print('.');
          Serial.print(getSupplyMillivolts());
          Serial.print("\r\n");
      break;
      
      case 'D': // DEBUG MAF'
          Serial.print('D');
          Serial.print('.');
          DEBUG_MAF_DATA = true;
          Serial.print("\r\n");
      break;

      case 'd': // DEBUG OFF'
          Serial.print('d');
          Serial.print('.');
          DEBUG_MAF_DATA = false;
          Serial.print("\r\n");
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
        menuFlow.setCurrentValue(mafFlowCFM * 10);   
    } else {
        menuFlow.setCurrentValue(0);   
    }

    // Temperature
    menuTemp.setCurrentValue(getTemp(DEGC) * 10);   
    
    // Baro
    menuBaro.setCurrentValue(getBaroPressure(KPA) * 10);   
    
    // Relative Humidity
    menuRelH.setCurrentValue(getRelativeHumidity(PERCENT) * 10);   

    // Pitot
    double pitotPressure = getPitotPressure(INWG);
    // Pitot probe displays as a percentage of the reference pressure
    double pitotPercentage = ((getPitotPressure(INWG) / refPressure) * 10);
    menuPitot.setCurrentValue(pitotPercentage);  
    
    // Reference pressure
    menuPRef.setCurrentValue(refPressure * 10);   
    
    // Adjusted Flow
    // get the desired bench test pressure
    double desiredRefPressureInWg = menuARef.getCurrentValue();
    // convert from the existing bench test
    double adjustedFlow = convertFlowDepression(refPressure, desiredRefPressureInWg, mafFlowCFM);
    // Send it to the display
    menuAFlow.setCurrentValue(adjustedFlow * 10); 

    // Version and build
    String versionNumberString = String(MAJOR_VERSION) + '.' + String(MINOR_VERSION);
    String buildNumberString = String(BUILD_NUMBER);
    menuVer.setTextValue(versionNumberString.c_str());
    menuBld.setTextValue(buildNumberString.c_str());    

    // Ref Presure Voltage
    int refPressRaw = analogRead(REF_PRESSURE_PIN);
    double refMillivolts = (refPressRaw * (5.0 / 1024.0)) * 1000;
    menuSensorTestPRef.setCurrentValue(refMillivolts);

    // Maf Voltage
    int mafFlowRaw = analogRead(MAF_PIN);
    double mafMillivolts = (mafFlowRaw * (5.0 / 1024.0)) * 1000;
    menuSensorTestMAF.setCurrentValue(mafMillivolts);

    // Pitot Voltage
    int pitotRaw = analogRead(PITOT_PIN);
    double pitotMillivolts = (pitotRaw * (5.0 / 1024.0)) * 1000;
    menuSensorTestPitot.setCurrentValue(pitotMillivolts);

    // Additional Displays

    #if defined CFM_4X7SEG
        //TODO(#6) Add support for additional displays
    #endif

    #if defined PITOT_4X7SEG
        //TODO(#6) Add support for additional displays
    #endif

    #if defined MPXV7007 
        //TODO(#6) Add support for additional displays
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
 * menuCallback_Calibrate
 * menuCallback_leakTestCalibration
 * menuCallback_LeakTest
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

//TODO(#8) - Add menu item to be able to select reference pressure value




/****************************************
 * MENU CALLBACK FUNCTION
 * menuCallback_Calibrate
 ***/
void CALLBACK_FUNCTION menuCallback_Calibrate(int id) {

    // Perform calibration
    float flowCalibrationOffset = setCalibrationOffset();

    char flowCalibrationOffsetText[12]; // Buffer big enough?
    dtostrf(flowCalibrationOffset, 6, 2, flowCalibrationOffsetText); // Leave room for too large numbers!

    // Display the value on the main screen
    displayDialog(LANG_CAL_OFFET_VALUE, flowCalibrationOffsetText);
}





/****************************************
 * MENU CALLBACK FUNCTION
 * menuCallback_leakTestCalibration
 ***/
void CALLBACK_FUNCTION menuCallback_leakTestCalibration(int id) {

    float calibrationValue = leakTestCalibration();


    char calibrationValueText[12]; // Buffer big enough?
    dtostrf(calibrationValue, 6, 2, calibrationValueText); // Leave room for too large numbers!

    // Display the value on popup on the main screen
    displayDialog(LANG_LEAK_CAL_VALUE, calibrationValueText);  
    Serial.print(calibrationValue); 
}


/****************************************
 * MENU CALLBACK FUNCTION
 * menuCallback_LeakTest
 ***/
void CALLBACK_FUNCTION menuCallback_LeakTest(int id) {

    int testResult = leakTest();

    //compare calibration data from NVM
    if (testResult == LEAK_TEST_PASS)
    {
       // Display result on the main screen
       errorVal = LEAK_TEST_PASS;      
    } else {
       // Display result on the main screen 
       errorVal = LEAK_TEST_FAILED;       
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


    // If API enabled, read serial data
    if (APIEnabled) {
      if (Serial.available() > 0) {
        serialData = Serial.read();
        parseAPI(serialData);
      }
    }

}


































































































































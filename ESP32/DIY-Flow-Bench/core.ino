/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * core.ino - Core flow bench functions
 *
 * Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
 * 
 * For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions 
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * 
 ***/


/****************************************
 * DECLARE CONSTANTS
 ***/

#define ABS_REF_PRESS_SENSOR

const float MOLECULAR_WEIGHT_DRY_AIR = 28.964;

// standard units
const int RAW = 0;
const int DECI = 0;
const int INWG = 1;
const int KPA = 2;
const int PSIA = 3;
const int DEGC = 4;
const int DEGF = 5;
const int RANKINE = 6;
const int PERCENT = 7;
const int BAR = 8;

// MAF Data Files
const int VOLTAGE = 1;
const int FREQUENCY = 2;
const int KEY_VALUE = 1;
const int RAW_ANALOG = 2;
const int KG_H = 1;
const int MG_S = 2;

// Error handler codes
const int NO_ERROR = 0;
const int REF_PRESS_LOW = 1;
const int LEAK_TEST_FAILED = 2;
const int LEAK_TEST_PASS = 3;
const int DHT11_READ_FAIL = 4;
const int BME280_READ_FAIL = 5;
const int BENCH_RUNNING = 6;



/****************************************
 * DECLARE VARS
 ***/
bool streamMafData = false;
float startupBaroPressure;
int statusVal = 0;
//bool benchIsRunning();
String statusMessage = "Starting...";

extern long mafMapData[][2]; // mafData key > value array
extern long mafMapAnalogData[]; // mafData analog value array

extern ConfigSettings config;


/****************************************
 * INITIALISE HARDWARE
 ***/
 void initialiseHardware() {


    // Support for ADAFRUIT_BME280 temp, pressure & Humidity sensors
    // https://github.com/adafruit/Adafruit_BME280_Library
    #if defined REF_ADAFRUIT_BME280 || defined TEMP_ADAFRUIT_BME280 || defined BARO_ADAFRUIT_BME280
        #include <Adafruit_BME280_DEV.h> 
        Adafruit_BME280 adafruitBme280; // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)

        //I2C address - BME280_I2C_ADDR
        if (!adafruitBme280.begin()) {  
            int statusVal = BME280_READ_FAIL;
          Serial.println("Adafruit BME280 Initialisation failed");      
        } else {
          Serial.println("Adafruit BME280 Initialised");      
        }
    #endif

    // Support for SPARKFUN_BME280 temp, pressure & Humidity sensors
    // https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide?_ga=2.39864294.574007306.1596270790-134320310.1596270790
    #if defined RELH_SPARKFUN_BME280 || defined TEMP_SPARKFUN_BME280 || defined BARO_SPARKFUN_BME280
        #include "SparkFunBME280.h"
        #include <Wire.h>
        BME280 SparkFunBME280;

        Wire.begin();
        SparkFunBME280.setI2CAddress(BME280_I2C_ADDR); 
        if (SparkFunBME280.beginI2C() == false) //Begin communication over I2C
        {
            int statusVal = BME280_READ_FAIL;
          Serial.println("Sparkfun BME280 Initialisation failed");      
        } else {
          Serial.println("Sparkfun BME280 Initialised");      
        }
    #endif

    // Support for DHT11 humidity / temperature sensors
    // https://github.com/winlinvip/SimpleDHT
    #if defined SIMPLE_RELH_DHT11 || defined SIMPLE_TEMP_DHT11
        #include <SimpleDHT.h>  
        SimpleDHT11 dht11(HUMIDITY_PIN);    
    #endif

    #if defined USE_REF_PRESS_AS_BARO && defined REF_MPX4250
        startupBaroPressure = getRefPressure(KPA);
    #else
        startupBaroPressure = DEFAULT_BARO;
    #endif

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

        case PSIA:
             refPressureKpa = refPressureKpa * 0.145038;
            return refPressureKpa  * 0.01 ;
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
          int statusVal = DHT11_READ_FAIL; // Set error to display on screen
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
          int statusVal = DHT11_READ_FAIL; // Set error to display on screen
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
        case DECI:
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
    float relativeHumidity = getRelativeHumidity(DECI);
    float vaporPressurePsia = getVaporPressure(PSIA);
    float baroPressurePsia = getBaroPressure(PSIA);

    specificGravity = (1-(0.378 * relativeHumidity * vaporPressurePsia) / baroPressurePsia);
    
    return specificGravity;


}  
    



/****************************************
 * CONVERT MASS FLOW TO VOLUMETRIC FLOW
 *
 * Calcualted using ideal gas law:
 * https://www.pdblowers.com/tech-talk/volume-and-mass-flow-calculations-for-gases/
 ***/
float convertMassFlowToVolumetric(float massFlowKgh)
{   
  float mafFlowCFM;
  float gasPressure;
  float tempInRankine = getTemp(RANKINE); //tested ok
  float specificGravity = getSpecificGravity(); //tested ok
  float molecularWeight = MOLECULAR_WEIGHT_DRY_AIR * specificGravity; //tested ok
  float baroPressure = getBaroPressure(PSIA); 
  float refPressure = getRefPressure(PSIA);
  float massFlowLbm = massFlowKgh * 0.03674371036415;

  gasPressure = baroPressure + refPressure; // TODO need to validate refPressure (should be a negative number)

  mafFlowCFM = ((massFlowLbm * 1545 * tempInRankine) / (molecularWeight * 144 * gasPressure)); 

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

    if (mafMillivolts < config.maf_min_millivolts) {
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
//    EEPROM.get( NVM_CD_CAL_OFFSET_ADDR, calibrationOffset ); 

    if (MAFdataUnit == KG_H) {

        // convert RAW datavalue back into kg/h
        mafFlowRateKGH = float(mafFlowRateRAW / 1000); 

    } else if (MAFdataUnit == MG_S) {

        //  convert mg/s value into kg/h
        mafFlowRateKGH = float(mafFlowRateRAW * 0.0036); 
    }


    // convert kg/h into cfm (NOTE this is approx 0.4803099 cfm per kg/h @ sea level)
    mafFlowRateCFM = convertMassFlowToVolumetric(mafFlowRateKGH);// + calibrationOffset; // add calibration offset to value //TODO #21 Need to validate and test calibration routine

    if (streamMafData == true) {
        sendSerial(String(mafMillivolts));
        sendSerial("mv = ");
        if (MAFdataUnit == KG_H) {
            sendSerial(String(mafFlowRateRAW / 1000));
            sendSerial("kg/h = ");
        } else if (MAFdataUnit == MG_S) {
            sendSerial(String(mafFlowRateRAW));
            sendSerial("mg/s = ");
        }
        sendSerial(String(mafFlowRateCFM ));
        sendSerial("cfm \r\n");
    }

    return mafFlowRateCFM;
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

    if ((refPressure > config.min_bench_pressure) && (mafFlowRateCFM > config.min_flow_rate))
    {
        statusVal = BENCH_RUNNING;
        return true;
    } else {
        statusVal = NO_ERROR;
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

    // Check that pressure does not fall below limit set by MIN_TEST_PRESSURE_PERCENTAGE when bench is running
    // note alarm commented out in alarm function as 'nag' can get quite annoying
    // Is this a redundant check? Maybe a different alert would be more appropriate
    if ((refPressure < (config.cal_ref_press * (MIN_TEST_PRESSURE_PERCENTAGE / 100))) && (benchIsRunning()))
    {
        statusVal = REF_PRESS_LOW;
    }
}





/****************************************
 * STATUS MESSAGE HANDLER
 * 
 ***/
void statusMessageHandler(int statusVal)
{
    if (!showAlarms) return;

    switch (statusVal)
    {
        case NO_ERROR:
            statusMessage = LANG_NO_ERROR;
        break;

        case BENCH_RUNNING:
            statusMessage = LANG_BENCH_RUNNING;
            Serial.println(LANG_BENCH_RUNNING);
        break;

        case REF_PRESS_LOW:
            // This alarm can get really annoying as it pops up every time the bench is off
            // need to add function to be able to enable / disable from menu
            // statusMessage = LANG_WARNING, LANG_REF_PRESS_LOW);
            // Serial.println(LANG_REF_PRESS_LOW);
        break;

        case LEAK_TEST_FAILED:
            statusMessage = LANG_LEAK_TEST_FAILED;
            Serial.println(LANG_LEAK_TEST_FAILED);
        break;

        case LEAK_TEST_PASS:
            statusMessage = LANG_LEAK_TEST_PASS;
            Serial.println(LANG_LEAK_TEST_PASS);
        break;

        case DHT11_READ_FAIL:
            statusMessage = LANG_DHT11_READ_FAIL;
            Serial.println(LANG_DHT11_READ_FAIL);
        break;

        case BME280_READ_FAIL:
            statusMessage = LANG_BME280_READ_FAIL;
            Serial.println(LANG_BME280_READ_FAIL);
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
    float convertedMafFlowCFM = convertFlowDepression(RefPressure, config.cal_ref_press,  MafFlowCFM);
    float flowCalibrationOffset = config.cal_flow_rate - convertedMafFlowCFM;

    char flowCalibrationOffsetText[12]; // Buffer big enough?
    dtostrf(flowCalibrationOffset, 6, 2, flowCalibrationOffsetText); // Leave room for too large numbers!
      
    // Store data in EEPROM
    // EEPROM.write(NVM_CD_CAL_OFFSET_ADDR, flowCalibrationOffset);

// TODO - store in JSON 

    return flowCalibrationOffset;
}



/****************************************
 * GET CALIBRATION OFFSET
 * 
 ***/
 float getCalibrationOffset() {

// TODO - cal data loaded with json config

 }






/****************************************
 * leakTestCalibration
 ***/
float leakTestCalibration() {

    float RefPressure = getRefPressure(INWG);  
    char RefPressureText[12]; // Buffer big enough?
    dtostrf(RefPressure, 6, 2, RefPressureText); // Leave room for too large numbers!
    
    //Store data in EEPROM
//    EEPROM.write(NVM_LEAK_CAL_ADDR, RefPressure);

    // Display the value on the main screen
    statusMessage = LANG_LEAK_CAL_VALUE, RefPressureText;  

    return RefPressure;
}


/****************************************
 * leakTest
 ***/
int  leakTest() {

    int leakCalibrationValue = 0; 
//    leakCalibrationValue = EEPROM.read(NVM_LEAK_CAL_ADDR);
    int refPressure = getRefPressure(INWG);

    //compare calibration data from NVM
    if (leakCalibrationValue > (refPressure - config.leak_test_tolerance))
    {   
       return LEAK_TEST_FAILED;
    } else {     
       return LEAK_TEST_PASS;
    }

}


/****************************************
 * Get JSON Data
 ***/
String getDataJson(){

    float mafFlowCFM = getMafFlowCFM();
    float refPressure = getRefPressure(INWG);   
    DynamicJsonDocument  dataJson(1024);    

    dataJson["STATUS_MESSAGE"] = String(statusMessage);

    // Flow Rate
    if (mafFlowCFM > config.min_flow_rate)
    {
          dataJson["FLOW"] = String(mafFlowCFM);        
    } else {
          dataJson["FLOW"] = String(0);        
    }

    // Temperature
    dataJson["TEMP"] = String(getTemp(DEGC));        
    
    // Baro
    dataJson["BARO"] = String(getBaroPressure(KPA));        
    
    // Relative Humidity
    dataJson["RELH"] = String(getRelativeHumidity(PERCENT));

    // Pitot
    double pitotPressure = getPitotPressure(INWG);
    // Pitot probe displays as a percentage of the reference pressure
    double pitotPercentage = (getPitotPressure(INWG) / refPressure);
    dataJson["PITOT"] = String(pitotPercentage);
    
    // Reference pressure
    dataJson["PREF"] = String(refPressure);
    
    // Adjusted Flow
    // get the desired bench test pressure
//    double desiredRefPressureInWg = menuARef.getCurrentValue(); //TODO: Add ref pressure setting to UI & Config
    // convert from the existing bench test
//    double adjustedFlow = convertFlowDepression(refPressure, desiredRefPressureInWg, mafFlowCFM);
    // Send it to the display
//    dataJson["AFLOW"] = String(adjustedFlow);

    // Version and build
    String versionNumberString = String(MAJOR_VERSION) + '.' + String(MINOR_VERSION);
    String buildNumberString = String(BUILD_NUMBER);
    dataJson["RELEASE"] = String(RELEASE);
    dataJson["BUILD_NUMBER"] = String(BUILD_NUMBER);

    // Ref Presure Voltage
    int refPressRaw = analogRead(REF_PRESSURE_PIN);
    double refMillivolts = (refPressRaw * (5.0 / 1024.0)) * 1000;
    dataJson["PREF_MV"] = String(refMillivolts);

    // Maf Voltage
    int mafFlowRaw = analogRead(MAF_PIN);
    double mafMillivolts = (mafFlowRaw * (5.0 / 1024.0)) * 1000;
    dataJson["MAF_MV"] = String(mafMillivolts);

    // Pitot Voltage
    int pitotRaw = analogRead(PITOT_PIN);
    double pitotMillivolts = (pitotRaw * (5.0 / 1024.0)) * 1000;
    dataJson["PITOT_MV"] = String(pitotMillivolts);

    char jsonString[1024];

    serializeJson(dataJson, jsonString);

    return jsonString;

}


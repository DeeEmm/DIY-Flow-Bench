/***********************************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * maths.cpp - Core mathematical functions class
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
 
#include <Arduino.h>
#include "maths.h"

#include "constants.h"
#include "pins.h"
#include "sensors.h"
#include "structs.h"
#include "hardware.h"
//#include MAF_SENSOR_FILE

const float MOLECULAR_WEIGHT_DRY_AIR = 28.964;

Maths::Maths() {
	
	extern String mafSensorType;
	// extern int MAFoutputType;
	// extern int MAFdataFormat;
	extern int MAFdataUnit;
	extern long mafLookupTable[][2];
	 
	this->_mafDataUnit = MAFdataUnit;
	
	//memcpy (b, a, 50*50*sizeof(float));
	//memcpy(p, q, 13*15*sizeof(*p));
	memcpy(this->_mafLookupTable, mafLookupTable, sizeof this->_mafLookupTable);
	
	//this->_mafLookupTable[][2] = mafLookupTable[][2];
}



/***********************************************************
 * CALCULATE BAROMETRIC pressure (kPa)
 * NOTE: Sensor must return an absolute value
 ***/
float Maths::calculateBaroPressure(int units) {   
	
	Hardware _hardware;

	float baroPressureKpa;
	float baroPressurePsia;
	// UNUSED: float baroPressureRaw;
	// UNUSED: float refTempRaw;
	// UNUSED: float refAltRaw;
	// UNUSED: int supplyMillivolts = _hardware.getSupplyMillivolts();
	// UNUSED: int rawBaroValue = analogRead(REF_BARO_PIN);
	// UNUSED: int baroMillivolts = (rawBaroValue * (5.0 / 1024.0)) * 1000;

	#ifdef BARO_SENSOR_TYPE_MPX4115
		// Datasheet - https://html.alldatasheet.es/html-pdf/5178/MOTOROLA/MPX4115/258/1/MPX4115.html
		// Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
		// P = ((Vout / VS ) - 0.095) / 0.009 --- Formula transposed for P
		baroPressureKpa = (((float)baroMillivolts / (float)supplyMillivolts ) - 0.095) / 0.009; 

	#elif defined BARO_SENSOR_TYPE_ADAFRUIT_BME280
		baroPressureKpa =  adafruitBme280.readPressure() / 1000; //Pa

	#elif defined BARO_SENSOR_TYPE_SPARKFUN_BME280
		baroPressureKpa =  SparkFunBME280.readFloatPressure() / 1000; // Pa

	#elif defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		// No baro sensor defined so use value grabbed at startup from reference pressure sensor
		// NOTE will only work for absolute style pressure sensor like the MPX4250
		baroPressureKpa = startupBaroPressure; 
	#else
		// we don't have any sensor so use default - // NOTE: standard sea level baro pressure is 14.7 psi
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
	
	return baroPressureKpa;
}



/***********************************************************
 * CALCULATE REFERENCE PRESSURE
 * Convert RAW pressure sensor data to In/WG or kPa
 ***/
float Maths::calculateRefPressure(int units) {   
	
	Hardware _hardware;

	float refPressureKpa = 0;
	float refPressureInWg;
	// UNUSED: float refPressureRaw;
	// UNUSED: float refTempDegRaw;
	// UNUSED: float refAltRaw;
	// UNUSED: float supplyMillivolts = _hardware.getSupplyMillivolts();
	// UNUSED: int rawRefPressValue = analogRead(REF_PRESSURE_PIN);
	// UNUSED: float refPressMillivolts = (rawRefPressValue * (5.0 / 1024.0)) * 1000;

	#ifdef PREF_SENSOR_TYPE_MPXV7007
		// Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
		// Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007DP Datasheet)
		// P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
		refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057;  

	#elif defined PREF_SENSOR_TYPE_PX4250
		// NOTE: Untested.  Also not best choice of sensor
		// Datasheet - https://www.nxp.com/files-static/sensors/doc/data_sheet/MPX4250.pdf
		// Vout = VS x (0.00369 x P + 0.04) --- Where VS = Supply Voltage (Formula from MPXV7007 Datasheet)
		// P = ((Vout / VS ) - 0.04) / 0.00369 --- Formula transposed for P
		// Note we use the baro value as this is an absolute sensor, so to prevent circular references we need to know
		// if we actually have a Baro sensor installed
		#if defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO 
			// we don't have a baro value so use the value hardcoded in the config to offset the pressure sensor value
			refPressureKpa = (((refPressMillivolts / supplyMillivolts ) - 0.04) / 0.00369) - DEFAULT_BARO;  
		#elif defined BARO_SENSOR_TYPE_FIXED_VALUE            
			refPressureKpa = DEFAULT_BARO;
		#else
			// use the current baro value to offset the sensor value
			refPressureKpa = (((refPressMillivolts / supplyMillivolts ) - 0.04) / 0.00369) - this->calculateBaroPressure(KPA);         
		#endif

	#elif defined PREF_SENSOR_NOT_USED 
	#else
		// No reference pressure sensor used so lets return a value (so as not to throw maths out)
		// refPressureKpa = 6.97448943333324; //28"
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
	
	return refPressureKpa;
	
}



/***********************************************************
 * CALCULATE TEMPERATURE
 * Convert RAW temperature sensor data
 ***/
float Maths::calculateTemperature(int units) {   
	
	// UNUSED: float refAltRaw;
	// UNUSED: float refPressureRaw;
	// UNUSED: float refTempRaw;
	float refTempDegC;
	float refTempDegF;
	float refTempRankine;
	// UNUSED: float relativeHumidity;
	// UNUSED: byte refTemp;
	// UNUSED: byte refRelh;


	#ifdef TEMP_SENSOR_TYPE_ADAFRUIT_BME280
		refTempDegC  =  adafruitBme280.readTemperature();

	#elif defined TEMP_SENSOR_TYPE_SPARKFUN_BME280
		refTempDegC =  SparkFunBME280.readTempC();

	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(LANG_DHT11_READ_FAIL); // Set error to display on screen
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
	
	return refTempDegC;
}



/***********************************************************
 * CALCULATE RELATIVE HUMIDITY %
 ***/
float Maths::calculateRelativeHumidity(int units) {   
	
	float relativeHumidity;
	// UNUSED: float tempDegC;
	// UNUSED: byte refTemp;
	// UNUSED: byte refRelh;

	#ifdef RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(LANG_DHT11_READ_FAIL); // Set error to display on screen
		  relativeHumidity = 0;        
		} else {
		  relativeHumidity = refRelh;
		}

	#elif defined RELH_SENSOR_TYPE_ADAFRUIT_BME280
		relativeHumidity = adafruitBme280.readHumidity(); //%

	#elif defined RELH_SENSOR_TYPE_SPARKFUN_BME280
		relativeHumidity =  SparkFunBME280.readFloatHumidity();

	#else
		//we don't have any sensor so use standard value 
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
	
	return relativeHumidity;

}  
	



/***********************************************************
 * CALCULATE VAPOR PRESSURE
 ***/
float Maths::calculateVaporPressure(int units) {
	 
	 
	 
	float airTemp = this->calculateTemperature(DEGC);
	// UNUSED: float molecularWeightOfDryAir = 28.964;
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
	
	return vapourPressureKpa;

}  
	



/***********************************************************
 * CALCULATE SPECIFIC GRAVITY
 ***/
float Maths::calculateSpecificGravity() {
	 
	float specificGravity;
	float relativeHumidity = this->calculateRelativeHumidity(DECI);
	float vaporPressurePsia = this->calculateVaporPressure(PSIA);
	float baroPressurePsia = this->calculateBaroPressure(PSIA);

	specificGravity = (1-(0.378 * relativeHumidity * vaporPressurePsia) / baroPressurePsia);
	
	return specificGravity;


}  
	



/***********************************************************
 * CONVERT MASS FLOW TO VOLUMETRIC FLOW
 *
 * Calculated using ideal gas law:
 * https://www.pdblowers.com/tech-talk/volume-and-mass-flow-calculations-for-gases/
 ***/
float Maths::convertMassFlowToVolumetric(float massFlowKgh) {   
  float mafFlowCFM;
  float gasPressure;
  float tempInRankine = this->calculateTemperature(RANKINE); //tested ok
  float specificGravity = this->calculateSpecificGravity(); //tested ok
  float molecularWeight = MOLECULAR_WEIGHT_DRY_AIR * specificGravity; //tested ok
  float baroPressure = this->calculateBaroPressure(PSIA); 
  float refPressure = this->calculateRefPressure(PSIA);
  float massFlowLbm = massFlowKgh * 0.03674371036415;

  gasPressure = baroPressure + refPressure; // TODO: need to validate refPressure (should be a negative number)

  mafFlowCFM = ((massFlowLbm * 1545 * tempInRankine) / (molecularWeight * 144 * gasPressure)); 

  return mafFlowCFM;


}  
	



/***********************************************************
 * GET MAF FLOW in CFM
 * Lookup CFM value from MAF data array
 *
 * NOTE: mafLookupTable is global array declared in the MAFDATA files
 ***/
float Maths::calculateMafFlowCFM() {
	
	extern CalibrationSettings calibration;


	Sensors _sensors;
	Hardware _hardware;
		
	float mafFlowRateCFM;
	float mafFlowRateKGH = 0;
	double mafFlowRateRAW;
	
	int lookupValue;
	int numRows;


	/* 
	* GET VALUE FROM LOOKUP TABLE USING THE SENSOR VALUE AS LOOKUP KEY
	* THE TYPE OF SENSOR IS IRRELEVANT AS LONG AS THE CORRESPONDING LOOKUP TABLE IS USED.
	* I.E 500HZ OR 500MV WILL GIVE THE CORRECT VALUE PROVIDED THE RIGHT TABLE IS CALLED.
	***/

	//Set size of array
    numRows = sizeof(this->_mafLookupTable)/sizeof(this->_mafLookupTable[0]) -1;  

	lookupValue = _sensors.getMAF();

	// Traverse the array until we find the lookupValue
	for (int rowNum = 0; rowNum <= numRows; rowNum++) {
	
		// Lets check to see if exact match is found 
		if (lookupValue == this->_mafLookupTable[rowNum][0]) {
			// we've got the exact value
			mafFlowRateRAW = this->_mafLookupTable[rowNum][1];
			break;

		// We've overshot so lets use the previous value
		} else if ( this->_mafLookupTable[rowNum][0] > lookupValue ) {

			if (rowNum == 0) {
				// we were on the first row so lets set the value to zero and consider it no flow
				return 0;

			} else {
				// Flow value is valid so let's convert it.
				// We use a linear interpolation formula to calculate the actual value
				// NOTE: Y=Y0+(((X-X0)(Y1-Y0))/(X1-X0)) where Y = flow and X = Volts
				mafFlowRateRAW = this->_mafLookupTable[rowNum-1][1] + (((lookupValue - this->_mafLookupTable[rowNum-1][0]) * (this->_mafLookupTable[rowNum][1] - this->_mafLookupTable[rowNum-1][1])) / (this->_mafLookupTable[rowNum][0] - this->_mafLookupTable[rowNum-1][0]));            
			}
			break;
		}

	}


   	// NOW THAT WE HAVE A VALUE, WE NEED TO SCALE IT AND CONVERT IT

	if (this->_mafDataUnit == KG_H) {

		// convert RAW datavalue back into kg/h
		mafFlowRateKGH = float(mafFlowRateRAW / 1000); 

	} else if (this->_mafDataUnit == MG_S) {

		//  convert mg/s value into kg/h
		mafFlowRateKGH = float(mafFlowRateRAW * 0.0036); 
	}

	// convert kg/h into cfm (NOTE this is approx 0.4803099 cfm per kg/h @ sea level)
	mafFlowRateCFM = convertMassFlowToVolumetric(mafFlowRateKGH) + calibration.flow_offset; 

	// lets stream data to the serial port
	if (streamMafData == true) {
		Serial.print(String(lookupValue));
		Serial.println(" (raw) = ");
		if (this->_mafDataUnit == KG_H) {
			Serial.print(String(mafFlowRateRAW / 1000));
			Serial.println("kg/h = ");
		} else if (this->_mafDataUnit == MG_S) {
			Serial.print(String(mafFlowRateRAW));
			Serial.println("mg/s = ");
		}
		Serial.print(String(mafFlowRateCFM ));
		Serial.println("cfm \r\n");
	}

	return mafFlowRateCFM;
}



/***********************************************************
 * CALCULATE PITOT PROBE
 * Convert RAW differential pressure sensor data
 ***/
float Maths::calculatePitotPressure(int units) {
	
	Hardware _hardware;
	 
	float pitotPressureKpa = 0.00;
	float pitotPressureInWg;
	

	#ifdef PITOT_SENSOR_TYPE_MPXV7007DP
	
		float rawPitotPressValue = analogRead(PITOT_PIN);   
		// UNUSED: int supplyMillivolts = _hardware.getSupplyMillivolts() / 1000;
		// UNUSED: int pitotPressMillivolts = (rawPitotPressValue * (5.0 / 1024.0)) * 1000;
		// sensor characteristics from datasheet
		// Vout = VS x (0.057 x P + 0.5)

		pitotPressureKpa = (rawPitotPressValue / (float)1024 - 0.5) / 0.057;

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
	
	return pitotPressureKpa;
	
}



/***********************************************************
* CONVERT FLOW
*
* Convert flow values between different reference pressures
* Flow at the new pressure drop = (the square root of (new pressure drop/old pressure drop)) times CFM at the old pressure drop.
* An example of the above formula would be to convert flow numbers taken at 28" of water to those which would occur at 25" of water.
* (25/28) = .89286
* Using the square root key on your calculator and inputting the above number gives .94489 which can be rounded off to .945.
* We can now multiply our CFM values at 28" of water by .945 to obtain the theoretical CFM values at 25" of water.
* Source: http://www.flowspeed.com/cfm-numbers.htm
*/

double Maths::convertFlowDepression(float oldPressure, int newPressure, float inputFlow) {
		

  double outputFlow;
  double pressureRatio = (newPressure / oldPressure);
  outputFlow = (sqrt(pressureRatio) * inputFlow);

  return outputFlow;

}










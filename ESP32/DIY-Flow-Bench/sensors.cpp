/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* Sensors.cpp - Sensors class
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
* NOTE: Methods return sensor values in (SI) standard units. This allows any sensor to be integrated into the system
*
* TODO: Would like to implement PCNT pulse counter for frequency measurement
* https://github.com/DavidAntliff/esp32-freqcount/blob/master/frequency_count.c
***/

#include "Arduino.h"

#include "configuration.h"
#include "constants.h"
#include "pins.h"
#include "structs.h"

#include <Wire.h>
#include "hardware.h"
#include "sensors.h"
#include "messages.h"
#include "driver/pcnt.h"
#include LANGUAGE_FILE

#ifdef MAF_IS_ENABLED
#include MAF_DATA_FILE
#endif


#ifdef BME_IS_ENABLED
#define TINY_BME280_I2C
#include "TinyBME280.h"
tiny::BME280 _BMESensor;
TwoWire I2CBME = TwoWire(0);
#endif



Sensors::Sensors() {

}


/***********************************************************
 * @brief Sensor begin method
*/
void Sensors::begin () {

	Messages _message;
	_message.serialPrintf("Initialising Sensors \n");
	Sensors::initialise ();
	_message.serialPrintf("Sensors Initialised \n");
}


// NOTE: Voodoo needed to get interrupt to work within class structure for frequency measurement. 
// We declare a new instance of the Sensor class so that we can use it for the MAF ISR
// https://forum.arduino.cc/t/pointer-to-member-function/365758
// Sensors __mafVoodoo;



/***********************************************************
 * @brief Sensor initialise method 
*/
void Sensors::initialise () {

	Messages _message;
	#ifdef MAF_IS_ENABLED
	MafData _mafdata;
	#endif

	extern struct DeviceStatus status;
    extern struct Translator translate;
	// extern int mafOutputType;

	// _mafdata.begin();
	
	//initialise BME
	#ifdef BME_IS_ENABLED
	
		_message.serialPrintf("Initialising BME280 \n");	
		
		if (_BMESensor.beginI2C(BME280_I2C_ADDR) == false)
		{
			_message.serialPrintf("BME sensor did not respond. \n");
			_message.serialPrintf("Please check wiring and I2C address\n");
			_message.serialPrintf("BME I2C address %s set in configuration.h. \n", BME280_I2C_ADDR);
			while(1); //Freeze
		} else {
			_message.serialPrintf("BME280 Initialised\n");
		}

		_BMESensor.setTempOverSample(1);
		_BMESensor.setPressureOverSample(1);
		_BMESensor.setHumidityOverSample(1);
		_BMESensor.setStandbyTime(0);
		_BMESensor.setFilter(0);
		_BMESensor.setMode(tiny::Mode::NORMAL); // NORMAL / FORCED / SLEEP

	#endif


	// Set up the MAF ISR if required
	// Note Frequency based MAFs are required to be attached direct to MAF pin for pulse counter to work
	// This means 5v > 3.3v signal conditioning is required on MAF pin
/* TODO: temp disabled - need to reenable	
	if (_mafdata.mafOutputType == FREQUENCY) {	
		__mafVoodoo.mafSetupISR(MAF_PIN, []{__mafVoodoo.mafFreqCountISR();}, FALLING);
		timer = timerBegin(0, 2, true);                                  
		timerStart(timer);	
	}
*/	


	// Definitions for system status pane
	// MAF Sensor
	#if defined MAF_IS_ENABLED && defined MAF_SRC_IS_ADC && defined ADC_IS_ENABLED
		this->_mafSensorType = _mafdata.mafSensorType;
	#elif defined MAF_IS_ENABLED && defined MAF_SRC_IS_PIN
		this->_mafSensorType = _mafdata.mafSensorType + " on GPIO:" + MAF_PIN;
	#else
		this->_mafSensorType = translate.LANG_VAL_NOT_ENABLED;
	#endif

	// Baro Sensor
	#if defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		this->startupBaroPressure = this->getPRefValue();
		this->_baroSensorType = translate.LANG_VAL_START_REF_PRESSURE;
	#elif defined BARO_SENSOR_TYPE_FIXED_VALUE
		this->startupBaroPressure = DEFAULT_BARO_VALUE;
		this->_baroSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_baroSensorType += DEFAULT_BARO_VALUE;
	#elif defined BARO_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_baroSensorType = "BME280";
	#elif defined BARO_SENSOR_TYPE_MPX4115
		this->_baroSensorType = "MPX4115";
	#elif defined BARO_SENSOR_TYPE_LINEAR_ANALOG
		this->_baroSensorType = "ANALOG PIN: " + REF_BARO_PIN;
	#else 
		this->_baroSensorType = translate.LANG_VAL_FIXED_VALUE + DEFAULT_BARO_VALUE;
	#endif
	
	//Temp Sensor
	#ifdef TEMP_SENSOR_NOT_USED
		this->startupBaroPressure = this->getPRefValue();
		this->_tempSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined TEMP_SENSOR_TYPE_FIXED_VALUE
		this->_tempSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_tempSensorType += DEFAULT_TEMP_VALUE;
	#elif defined TEMP_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_tempSensorType = "BME280";
	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_tempSensorType = "Simple DHT11";
	#elif defined TEMP_SENSOR_TYPE_LINEAR_ANALOG
		this->_tempSensorType = "ANALOG PIN: " + TEMPERATURE_PIN;
	#else 
		this->_tempSensorType = translate.LANG_VAL_FIXED_VALUE + DEFAULT_TEMP_VALUE;
	#endif
	
	// Rel Humidity Sensor
	#ifdef RELH_SENSOR_NOT_USED
		this->startupBaroPressure = this->getPRefValue();
		this->_relhSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined RELH_SENSOR_TYPE_FIXED_VALUE
		this->_relhSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_relhSensorType += DEFAULT_RELH_VALUE;
	#elif defined RELH_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_relhSensorType = "BME280";
	#elif defined RELH_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_relhSensorType = "Simple DHT11";
	#elif defined RELH_SENSOR_TYPE_LINEAR_ANALOG
		this->_relhSensorType = "ANALOG PIN: " + HUMIDITY_PIN;
	#else 
		this->_relhSensorType = translate.LANG_VAL_FIXED_VALUE + DEFAULT_RELH_VALUE;
	#endif

	// reference pressure
	#ifdef PREF_SENSOR_NOT_USED
		this->_prefSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PREF_SENSOR_TYPE_MPXV7007 && defined ADS_IS_ENABLED
		this->_prefSensorType = "SMPXV7007";
	#elif defined PREF_SENSOR_TYPE_MPX4250 && defined ADS_IS_ENABLED
		this->_prefSensorType = "MPX4250";
	#elif defined PREF_SENSOR_TYPE_LINEAR_ANALOG
		this->_prefSensorType = "ANALOG PIN: " + REF_PRESSURE_PIN;
	#else 
		this->_prefSensorType = translate.LANG_VAL_NOT_ENABLED;
	#endif
	
	// differential pressure
	#ifdef PDIFF_SENSOR_NOT_USED
		this->_pdiffSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PDIFF_SENSOR_TYPE_MPXV7007 && defined ADS_IS_ENABLED
		this->_pdiffSensorType = "SMPXV7007";
	#elif defined PDIFF_SENSOR_TYPE_LINEAR_ANALOG
		this->_pdiffSensorType = "ANALOG PIN: " + DIFF_PRESSURE_PIN;
	#else 
		this->_pdiffSensorType = translate.LANG_VAL_NOT_ENABLED;
	#endif
	
	// pitot pressure differential
    #ifdef PITOT_SENSOR_NOT_USED
		this->_pitotSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PITOT_SENSOR_TYPE_MPXV7007 && defined ADS_IS_ENABLED
		this->_pitotSensorType = "SMPXV7007";
	#elif defined PITOT_SENSOR_TYPE_LINEAR_ANALOG
		this->_pitotSensorType = "ANALOG PIN: " + PITOT_PIN;
	#else 
		this->_pitotSensorType = translate.LANG_VAL_NOT_ENABLED;
	#endif
	
	// Set status values for GUI
	status.mafSensor = this->_mafSensorType;
	status.baroSensor = this->_baroSensorType;
	status.tempSensor = this->_tempSensorType;
	status.relhSensor = this->_relhSensorType;
	status.prefSensor = this->_prefSensorType;
	status.pdiffSensor = this->_pdiffSensorType;
	status.pitotSensor = this->_pitotSensorType;

	// END System status definitions

}


/***********************************************************
 * Set up MAF ISR
 *
 * We cannot call a non-static member function directly so we need to encapsulate it
 * This is part of the Voodoo
 ***/
/* TODO: temp disabled - need to reenable	
void Sensors::mafSetupISR(uint8_t irq_pin, void (*ISR_callback)(void), int value) {
  attachInterrupt(digitalPinToInterrupt(irq_pin), ISR_callback, value);
}
*/



/***********************************************************
* Interrupt Service Routine for MAF Frequency measurement
*
* Determine how long since last triggered (Resides in RAM memory as it is faster)
***/
// TODO: Add IRAM_ATTR to Nova-Arduino 
/* TODO: temp disabled - need to reenable	
void IRAM_ATTR Sensors::mafFreqCountISR() {
    uint64_t TempVal = timerRead(timer);            
    PeriodCount = TempVal - StartValue;             
    StartValue = TempVal;                           
}
*/



/***********************************************************
 * @brief getMafMillivolts: Returns MAF signal in Millivolts
 ***/
double Sensors::getMafMillivolts() {
	
	Hardware _hardware;
	
	#if defined MAF_SRC_IS_ADC && defined ADC_IS_ENABLED
		mafMillivolts = _hardware.getADCMillivolts(MAF_ADC_CHANNEL);
				
	#elif defined MAF_SRC_IS_PIN	
		long mafRaw = analogRead(MAF_PIN);
		mafMillivolts = (mafRaw * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
	#else
		mafMillivolts = 1;
		return 1;
	#endif
	
	// Lets make sure we have a valid value to return
	if (mafMillivolts > 0) {
		mafMillivolts += MAF_MV_TRIMPOT;
		return mafMillivolts;
	} else {
		return 1;
	}	
	
}



/***********************************************************
 * @brief Returns RAW MAF Sensor value
 *
 * @note MAF decode is done in Calculations.cpp
 ***/
double Sensors::getMafValue() {
	
	Hardware _hardware;
	#ifdef MAF_IS_ENABLED
	MafData _mafdata;
	
	double mafFlow = 0.0;

	switch (_mafdata.mafOutputType) {
		
		case VOLTAGE:
		{
			int mafMillivolts = 0;
			
			#if defined MAF_SRC_IS_ADC && defined ADC_IS_ENABLED
				//mafFlowRaw = _hardware.getADCRawData(MAF_ADC_CHANNEL);
				mafMillivolts = _hardware.getADCMillivolts(MAF_ADC_CHANNEL);
				
			#elif defined MAF_SRC_IS_PIN	
				long mafFlowRaw = analogRead(MAF_PIN);
				mafMillivolts = (mafFlowRaw * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
			#else
				mafMillivolts = 1;
				return 1;
			#endif

			mafFlow = mafMillivolts + MAF_MV_TRIMPOT;			
			return mafFlow;
		}
		break;
		
		case FREQUENCY:
		{  
			// TODO
			mafFlow = 40000000.00 / PeriodCount;
			//mafFlow = 40000000.00 / __mafVoodoo.PeriodCount; // NOTE: Do we need Voodoo?
			mafFlow += MAF_MV_TRIMPOT;
			return mafFlow;
		}
		break;
		
	}	
	
	return mafFlow;

	#endif

	return 1; // MAF is disabled so lets return 1
}




/***********************************************************
 * @name getTempValue
 * @brief Returns temperature in Deg C
 * @return refTempDegC
 ***/
double Sensors::getTempValue() {
	
	Hardware _hardware;
	Messages _message;


	double refTempDegC;
	
	#ifdef TEMP_SENSOR_TYPE_LINEAR_ANALOG
	
		long rawTempValue = analogRead(TEMPERATURE_PIN);	
		double tempMillivolts = (rawTempValue * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;	
		tempMillivolts += TEMP_MV_TRIMPOT;		
		refTempDegC = tempMillivolts * TEMP_ANALOG_SCALE;
		refTempDegC +=  TEMP_FINE_ADJUST;
		
	#elif defined TEMP_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED

		refTempDegC = _BMESensor.readFixedTempC() / 100.00F;

	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(translate.LANG_VAL_DHT11_READ_FAIL); // Set error to display on screen
		  refTempDegC = 0;        
		} else {
		  refTempDegC = refTemp;
		}	
		refTempDegC +=  TEMP_FINE_ADJUST;

	#else
		// We don't have any temperature input so we will assume default
		refTempDegC = DEFAULT_TEMP_VALUE;
	#endif
	
	if (TEMP_FINE_ADJUST != 0) refTempDegC += TEMP_FINE_ADJUST;
	return refTempDegC;
}



/***********************************************************
 * @name getBaroValue
 * @brief Barometric pressure in hPa
 * @returns baroPressureHpa
 * @note 1 kPa = 10 hPa | 1 hPa = 1 millibar
 ***/
double Sensors::getBaroValue() {
	
	Hardware _hardware;
	double baroPressureHpa;
		
	#if defined BARO_SENSOR_TYPE_LINEAR_ANALOG
		
		long rawBaroValue = analogRead(REF_BARO_PIN);
		double baroMillivolts = (rawBaroValue * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 10000;
		baroMillivolts += BARO_MV_TRIMPOT;		
		baroPressureHpa = baroMillivolts * BARO_ANALOG_SCALE;
		baroPressureHpa += BARO_FINE_ADJUST;
	
	#elif defined BARO_SENSOR_TYPE_MPX4115
		// Datasheet - https://html.alldatasheet.es/html-pdf/5178/MOTOROLA/MPX4115/258/1/MPX4115.html
		// Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
		baroPressureHpa = map(_hardware.getADCRawData(BARO_ADC_CHANNEL), 0, 4095, 15000, 115000);
		baroPressureHpa += BARO_FINE_ADJUST;

	#elif defined BARO_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED

		baroPressureHpa = _BMESensor.readFixedPressure() / 100.00F; 
		
	#elif defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		// No baro sensor defined so use value grabbed at startup from reference pressure sensor
		// NOTE will only work for absolute style pressure sensor like the MPX4250
		baroPressureKpa = startupBaroPressure; 
		baroPressureKpa += BARO_FINE_ADJUST;
	#else
		// we don't have any sensor so use default - // NOTE: standard sea level baro pressure is 14.7 psi
		baroPressureKpa = DEFAULT_BARO_VALUE;
	#endif

	// Truncate to 2 decimal places
	// int value = baroPressureKpa * 100 + .5;
    // return (double)value / 100;

	if (BARO_FINE_ADJUST != 0) baroPressureKpa += BARO_FINE_ADJUST;
	return baroPressureHpa;

}



/***********************************************************
 * @name getRelHValue
 * @brief Returns Relative Humidity in %
 * @returns relativeHumidity
 ***/
double Sensors::getRelHValue() {
	
	Hardware _hardware;

	double relativeHumidity;

	  // extern struct Translator translate;
		
	#ifdef RELH_SENSOR_TYPE_LINEAR_ANALOG
	
		long rawRelhValue = analogRead(HUMIDITY_PIN);
		double relhMillivolts = (rawRelhValue * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
		relhMillivolts += RELH_MV_TRIMPOT;		
		relativeHumidity = relhMillivolts * RELH_ANALOG_SCALE;
		relativeHumidity += RELH_FINE_ADJUST;
	
	#elif defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11
	
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(translate.LANG_VAL_DHT11_READ_FAIL); // Set error to display on screen
		  relativeHumidity = 0;        
		} else {
		  relativeHumidity = refRelh;
		}
		relativeHumidity + RELH_FINE_ADJUST;

	#elif defined RELH_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED

		relativeHumidity = _BMESensor.readFixedHumidity() / 1000.00F;
		
	#else
		// we don't have a sensor so use nominal fixed value 
		relativeHumidity = DEFAULT_RELH_VALUE; // (36%)
	
	#endif

	if (RELH_FINE_ADJUST != 0) relativeHumidity += RELH_FINE_ADJUST;
	return relativeHumidity;
	
}


/***********************************************************
 * Returns altitude based on pressure difference from local sea level
 ***/
double Sensors::getAltitude() {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude. See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  //TODO: BREAK PRESSURE OUT TO CONFIG VALUE THAT CAN BE SET IN GUI
  double atmospheric = getRelHValue() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / SEALEVELPRESSURE_HPA, 0.1903));
}



/***********************************************************
 * getPRefMillivolts
 * Returns Reference pressure in Millivolts
 ***/
double Sensors::getPRefMillivolts() {

	Hardware _hardware;

	#if defined PREF_SRC_ADC && defined ADC_IS_ENABLED

		refPressMillivolts = _hardware.getADCMillivolts(PREF_ADC_CHANNEL);
				
	#elif defined PREF_SRC_PIN	
		long refPressRaw = analogRead(REF_PRESSURE_PIN);
		refPressMillivolts = (refPressRaw * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
	#else	
		refPressMillivolts = 1;
		return 1;
	#endif

	// Lets make sure we have a valid value to return
	if (refPressMillivolts > 0) {
		refPressMillivolts += PREF_MV_TRIMPOT;
		return refPressMillivolts;
	} else {
		return 1;
	}


}

/***********************************************************
 * getPRefValue
 * Returns Reference pressure in kPa
 ***/
double Sensors::getPRefValue() {

	Hardware _hardware;
	
	double refPressureKpa = 0.0;
	
	#if defined PREF_SENSOR_TYPE_LINEAR_ANALOG
		refPressureKpa = getPRefMillivolts() * PREF_ANALOG_SCALE;
	
	#elif defined PREF_SENSOR_TYPE_MPXV7007
		// Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
		// Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007DP Datasheet)
		// refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057;  
		
		// REVIEW Not sure if we need to error check here
//		if (!(Sensors::getPRefMillivolts() > 0) | !(_hardware.get5vSupplyMillivolts() > 0)) return 1; 

		refPressureKpa = (((this->getPRefMillivolts() / _hardware.get5vSupplyMillivolts() ) - 0.5) / 0.057);  

	#else
		// No reference pressure sensor used so lets return a fixed value (so as not to throw maths out)
		// refPressureKpa = 6.97448943333324; //28"
		refPressureKpa = DEFAULT_REF_PRESS_VALUE;

	#endif
	
	return refPressureKpa;

}




/***********************************************************
 * Returns Differential Pressure in Millivolts
 ***/
double Sensors::getPDiffMillivolts() {
	
	Hardware _hardware;
	
	#if defined PDIFF_SRC_IS_ADC && defined ADC_IS_ENABLED
		pDiffMillivolts = _hardware.getADCMillivolts(PDIFF_ADC_CHANNEL);
				
	#elif defined PDIFF_SRC_IS_PIN	
		long pDiffRaw = analogRead(DIFF_PRESSURE_PIN);
		pDiffMillivolts = (pDiffRaw * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
	#else
		pDiffMillivolts = 1;
		return 1;
	#endif
	
	// Lets make sure we have a valid value to return
	if (pDiffMillivolts > 0) {
		pDiffMillivolts += PDIFF_MV_TRIMPOT;
		return pDiffMillivolts;
	} else {
		return 1;
	}
	
}




/***********************************************************
 * Returns Pressure differential in kPa
 ***/
double Sensors::getPDiffValue() {

	Hardware _hardware;
	Messages _message;

	double diffPressureKpa = 0.0;
	int diffPressMillivolts;
	long diffPressRaw;	

	#if defined PDIFF_SRC_IS_ADC && defined ADC_IS_ENABLED
		diffPressMillivolts = getPDiffMillivolts();	
	#elif defined PDIFF_SRC_IS_PIN	
		diffPressRaw = analogRead(DIFF_PRESSURE_PIN);
		diffPressMillivolts = (diffPressRaw * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
	#else
		diffPressMillivolts = 1;
		return 1;
	#endif
	
//_message.serialPrintf("%s \n", diffPressRaw);	
	
	diffPressMillivolts += PDIFF_MV_TRIMPOT;

	#if defined PDIFF_SENSOR_TYPE_LINEAR_ANALOG
		diffPressureKpa = diffPressMillivolts * PDIFF_ANALOG_SCALE;
	
	#elif defined PDIFF_SENSOR_TYPE_MPXV7007
		// RECOMMENDED SENSOR
		// Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
		// Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007DP Datasheet)
		
		#if defined ADC_TYPE_ADS1115
			//scale for 16 bit ADC

			// REVIEW Not sure if we need to error check here
			// if (!(Sensors::getPDiffMillivolts() > 0) | !(_hardware.get5vSupplyMillivolts() > 0)) return 1; 
			
			diffPressureKpa = (((this->getPDiffMillivolts() / _hardware.get5vSupplyMillivolts() ) - 0.5) / 0.057);  
			
			
		#elif defined ADC_TYPE_ADS1015
			//scale for 12 bit ADC
			diffPressureKpa = map(_hardware.getADCRawData(PREF_ADC_CHANNEL), 0, 4095, -7000, 7000); 
		#endif
		
		 
	#elif defined PDIFF_SENSOR_NOT_USED 
	#else
		// No reference pressure sensor used so lets return a value (so as not to throw maths out)
		// refPressureKpa = 6.97448943333324; //28"
		diffPressureKpa = DEFAULT_PDIFF_PRESS;

	#endif

	return diffPressureKpa;

}




/***********************************************************
 * Returns Pitot Pressure in Millivolts
 ***/
double Sensors::getPitotMillivolts() {

	Hardware _hardware;
	
	#if defined PITOT_SRC_IS_ADC && defined ADC_IS_ENABLED
		pitotMillivolts = _hardware.getADCMillivolts(PITOT_ADC_CHANNEL);
	#elif defined PDIFF_SRC_IS_PIN	
		long pitotRaw = analogRead(PITOT_PIN);
		pitotMillivolts = (pitotRaw * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
	#else
		pitotMillivolts = 1;
		return 1;
	#endif
	
	// Lets make sure we have a valid value to return
	if (pitotMillivolts > 0) {
		pitotMillivolts += PITOT_MV_TRIMPOT;
		return pitotMillivolts;
	} else {
		return 1;
	}
	
	
}





/***********************************************************
 * Returns Pitot pressure differential in kPa
 ***/
double Sensors::getPitotValue() {
	
	Hardware _hardware;


	double pitotPressureKpa = 0.0;
 	long pitotPressRaw;
	double pitotMillivolts;
	
	
	#if defined PITOT_SRC_IS_ADC && defined ADC_IS_ENABLED
		pitotMillivolts = getPitotMillivolts();
	#elif defined PITOT_SRC_IS_PIN	
		pitotPressRaw = analogRead(PITOT_PIN);
		pitotMillivolts = (pitotPressRaw * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
	#else
		pitotMillivolts = 1;
		return 1;
	#endif		
	
	pitotMillivolts += PITOT_MV_TRIMPOT;
	
	#if defined PITOT_SENSOR_TYPE_LINEAR_ANALOG
		pitotPressureKpa = pitotMillivolts * PITOT_ANALOG_SCALE;
	
	#elif defined PITOT_SENSOR_TYPE_MPXV7007DP

		pitotMillivolts = this->getPitotMillivolts();
		
		// REVIEW Not sure if we need to error check here
		// if (!(pitotMillivolts > 0) | !(_hardware.get5vSupplyMillivolts() > 0)) return 1; 
		
		pitotPressureKpa = (((pitotMillivolts / _hardware.get5vSupplyMillivolts() ) - 0.5) / 0.057);  

	#else
		// No pitot probe used so lets return a fixed value
		pitotPressureKpa = DEFAULT_PITOT_VALUE;

	#endif

	return pitotPressureKpa;

}


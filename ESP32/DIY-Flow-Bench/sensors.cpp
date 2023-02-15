/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file sensors.cpp
 * 
 * @brief Sensors class
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
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

// #ifdef MAF_IS_ENABLED
// #include "mafData/maf.h"
// #endif

#ifdef MAF_IS_ENABLED
#include MAF_DATA_FILE
#endif



#ifdef BME_IS_ENABLED
#define TINY_BME280_I2C
#include "TinyBME280.h"
tiny::BME280 _BMESensor;
TwoWire I2CBME = TwoWire(0);
#endif


/***********************************************************
 * @brief Class constructor
 ***/
Sensors::Sensors() {

}


/***********************************************************
 * @brief Sensor begin method
 ***/
void Sensors::begin () {

	Messages _message;
	_message.serialPrintf("Initialising Sensors \n");
	Sensors::initialise ();
	_message.serialPrintf("Sensors Initialised \n");
}

// REVIEW - Frequency style MAF Begin
// NOTE: Voodoo needed to get interrupt to work within class structure for frequency measurement. 
// We declare a new instance of the Sensor class so that we can use it for the MAF ISR
// https://forum.arduino.cc/t/pointer-to-member-function/365758
// Sensors __mafVoodoo;


 
/***********************************************************
 * @brief Initialise sensors 
 ***/
void Sensors::initialise () {

	Messages _message;

	extern struct DeviceStatus status;
    extern struct Translator translate;
	extern int mafOutputType;

	// Initialise  MAF data
	#ifdef MAF_IS_ENABLED
		Maf _maf;
		
		// get size of the MAF datatable
  		status.mafDataTableRows = mafLookupTable.size() -1;

		// get highest MAF input value from data table
		status.mafDataValMax = mafLookupTable[status.mafDataTableRows][1];
		status.mafDataKeyMax = mafLookupTable[status.mafDataTableRows][0];
		
		// get MAF units
		status.mafUnits = _maf.mafUnits();
	#endif


	
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




	// REVIEW FREQUENCY BASED MAF - temporarily disabled - need to fix & reenable
	// Set up the MAF ISR if required
	// Note Frequency based MAFs are required to be attached direct to MAF pin for pulse counter to work
	// This means 5v > 3.3v signal conditioning is required on MAF pin (possible simple voltage divider + filter)
	// Need to check out use of ESP PCNT function - https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/pcnt.html
	// ESP32 has max PCNT frequency of 40MHz (half clock speed - 80MHz)
	// Typical MAF is 30Hz -  150Hz
	// Example - https://github.com/espressif/esp-idf/blob/v4.4/examples/peripherals/pcnt/pulse_count_event/main/pcnt_event_example_main.c
	// also this... https://blog.adafruit.com/2018/12/27/frequency-measurement-on-an-esp32-in-micropython-esp32-micropython-python/
	// and this... https://forum.micropython.org/viewtopic.php?f=18&t=5724&p=32921
	// (which has example without interrupts)

/* temp disabled - need to reenable	
	if (_maf.outputType == FREQUENCY) {	
		__mafVoodoo.mafSetupISR(MAF_PIN, []{__mafVoodoo.mafFreqCountISR();}, FALLING);
		timer = timerBegin(0, 2, true);                                  
		timerStart(timer);	
	}
*/	


	// Sensor definitions for system status pane
	// MAF Sensor
	#if defined MAF_IS_ENABLED && defined MAF_SRC_IS_ADC && defined ADC_IS_ENABLED
		this->_mafSensorType = _maf.sensorType();
	#elif defined MAF_IS_ENABLED && defined MAF_SRC_IS_PIN
		this->_mafSensorType = _maf.sensorType() + " on GPIO:" + MAF_PIN;
	#else
		this->_mafSensorType = translate.LANG_VAL_NOT_ENABLED;
	#endif

	// Baro Sensor
	#if defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		this->startupBaroPressure = this->getPRefValue();
		this->_baroSensorType = translate.LANG_VAL_START_REF_PRESSURE;
	#elif defined BARO_SENSOR_TYPE_FIXED_VALUE
		this->startupBaroPressure = FIXED_BARO_VALUE;
		this->_baroSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_baroSensorType += FIXED_BARO_VALUE;
	#elif defined BARO_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_baroSensorType = "BME280";
	#elif defined BARO_SENSOR_TYPE_MPX4115
		this->_baroSensorType = "MPX4115";
	#elif defined BARO_SENSOR_TYPE_LINEAR_ANALOG
		this->_baroSensorType = "ANALOG PIN: " + REF_BARO_PIN;
	#else 
		this->_baroSensorType = translate.LANG_VAL_FIXED_VALUE + FIXED_BARO_VALUE;
	#endif
	
	//Temp Sensor
	#ifdef TEMP_SENSOR_NOT_USED
		this->startupBaroPressure = this->getPRefValue();
		this->_tempSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined TEMP_SENSOR_TYPE_FIXED_VALUE
		this->_tempSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_tempSensorType += FIXED_TEMP_VALUE;
	#elif defined TEMP_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_tempSensorType = "BME280";
	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_tempSensorType = "Simple DHT11";
	#elif defined TEMP_SENSOR_TYPE_LINEAR_ANALOG
		this->_tempSensorType = "ANALOG PIN: " + TEMPERATURE_PIN;
	#else 
		this->_tempSensorType = translate.LANG_VAL_FIXED_VALUE + FIXED_TEMP_VALUE;
	#endif
	
	// Rel Humidity Sensor
	#ifndef RELH_IS_ENABLED
		this->startupBaroPressure = this->getPRefValue();
		this->_relhSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined RELH_SENSOR_TYPE_FIXED_VALUE
		this->_relhSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_relhSensorType += FIXED_RELH_VALUE;
	#elif defined RELH_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_relhSensorType = "BME280";
	#elif defined RELH_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_relhSensorType = "Simple DHT11";
	#elif defined RELH_SENSOR_TYPE_LINEAR_ANALOG
		this->_relhSensorType = "ANALOG PIN: " + HUMIDITY_PIN;
	#else 
		this->_relhSensorType = translate.LANG_VAL_FIXED_VALUE + FIXED_RELH_VALUE;
	#endif

	// reference pressure
	#ifndef PREF_IS_ENABLED
		this->_prefSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PREF_SENSOR_TYPE_MPXV7007 && defined ADC_IS_ENABLED
		this->_prefSensorType = "SMPXV7007";
	#elif defined PREF_SENSOR_TYPE_MPX4250 && defined ADC_IS_ENABLED
		this->_prefSensorType = "MPX4250";
	#elif defined PREF_SENSOR_TYPE_LINEAR_ANALOG
		this->_prefSensorType = "ANALOG PIN: " + REF_PRESSURE_PIN;
	#else 
		this->_prefSensorType = translate.LANG_VAL_NOT_ENABLED;
	#endif
	
	// differential pressure
	#ifndef PDIFF_IS_ENABLED
		this->_pdiffSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PDIFF_SENSOR_TYPE_MPXV7007 && defined ADC_IS_ENABLED
		this->_pdiffSensorType = "SMPXV7007";
	#elif defined PDIFF_SENSOR_TYPE_LINEAR_ANALOG
		this->_pdiffSensorType = "ANALOG PIN: " + DIFF_PRESSURE_PIN;
	#else 
		this->_pdiffSensorType = translate.LANG_VAL_NOT_ENABLED;
	#endif
	
	// pitot pressure differential
    #ifndef PITOT_IS_ENABLED
		this->_pitotSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PITOT_SENSOR_TYPE_MPXV7007 && defined ADC_IS_ENABLED
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

// REVIEW - MAF Interrupt service routine setup
/***********************************************************
  * @brief Set up MAF ISR
  *
  * We cannot call a non-static member function directly so we need to encapsulate it
  * This is part of the Voodoo
  ***/
/* TODO: temp disabled - need to reenable	
void Sensors::mafSetupISR(uint8_t irq_pin, void (*ISR_callback)(void), int value) {
  attachInterrupt(digitalPinToInterrupt(irq_pin), ISR_callback, value);
}
*/


// REVIEW - MAF Interrupt service routine
/***********************************************************
 * @brief Interrupt Service Routine for MAF Frequency measurement
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
 * @brief Returns RAW MAF Sensor value (ADC Value)
 *
 ***/
long Sensors::getMafRaw() {
	
	Hardware _hardware;
	extern struct SensorData sensorVal;

	#ifdef MAF_IS_ENABLED
	Maf _maf;

	#if defined MAF_SRC_IS_ADC && defined ADC_IS_ENABLED
		sensorVal.MafRAW = _hardware.getADCRawData(MAF_ADC_CHANNEL);
		
	#elif defined MAF_SRC_IS_PIN	
		long mafFlowRaw = analogRead(MAF_PIN);
	#else
		mafFlowRaw = 1;
	#endif

	return sensorVal.MafRAW;

	#else

	return 0; // MAF is disabled so lets return 1

	#endif
}




/***********************************************************
 * @brief getMafVolts: Returns MAF signal in Volts
 ***/
double Sensors::getMafVolts() {
	
	Hardware _hardware;

	double sensorVolts = 0.00F;
	
	#if defined MAF_SRC_IS_ADC && defined MAF_IS_ENABLED && defined ADC_IS_ENABLED
		sensorVolts = _hardware.getADCVolts(MAF_ADC_CHANNEL);
				
	#elif defined MAF_SRC_IS_PIN	
		long mafRaw = analogRead(MAF_PIN);
		sensorVolts = mafRaw * (_hardware.get3v3SupplyVolts() / 4095.0);
	#else
		sensorVolts = 1.0;
		return sensorVolts;
	#endif
	
	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
		sensorVolts += MAF_MV_TRIMPOT;
		return sensorVolts;
	} else {
		return 0.0;
	}	
	
}





/***********************************************************
 * @brief Returns MAF mass flow value in KG/H 
 * @note Interpolates lookupvalue from datatable key>value pairs
 *
 ***/
int32_t Sensors::getMafFlow(int units) {

	extern struct DeviceStatus status;
	extern struct SensorData sensorVal;

	Hardware _hardware;

	double flowRateCFM = 0.0;
	double flowRateMGS = 0.0;
	double flowRateKGH = 0.0;
	double lookupValue = 0.0;

	// scale sensor reading to data table size using map function (0-5v : 0-keymax)
	long refValue =  map(this->getMafVolts(), 0, _hardware.get5vSupplyVolts(), 0, status.mafDataKeyMax ); 

	for (int rowNum = 0; rowNum < status.mafDataTableRows; rowNum++) { // iterate the data table comparing the Lookup Value to the refValue for each row

		long Key = mafLookupTable[rowNum][0]; // Key Value for this row (x2)
		long Val = mafLookupTable[rowNum][1]; // Flow Value for this row (y2)

		// Did we get a match??
		if (refValue == Key) { // Great!!! we've got the exact key value

		lookupValue = Val; // lets use the associated lookup value
		break;
		
		// } else if ( Key > refValue && rowNum == 0) { // we were only on the first row so there is no previous value to interpolate with, so lets set the flow value to zero and consider it no flow

		//   return 0.0; 
		//   break;

		} else if (Key > refValue && rowNum > 0) { // The value is somewhere between this and the previous key value so let's use linear interpolation to calculate the actual value: 

		long KeyPrev = mafLookupTable[rowNum - 1][0]; // Key value for the previous row (x1)
		long ValPrev = mafLookupTable[rowNum - 1][1]; // Flow value for the previous row (y1)

		// Linear interpolation y = y1 + (x-x1)((y2-y1)/(x2-x1)) where x1+y1 are coord1 and x2_y2 are coord2
		lookupValue = ValPrev + (refValue - KeyPrev)*((Val-ValPrev)/(Key-KeyPrev));
		break;   
		}

	} 


	// Now that we have a flow value, we need to scale it and convert it.
	if (status.mafUnits == KG_H) {

		// convert RAW datavalue back into kg/h
		flowRateKGH = double(lookupValue / 1000);

	} else if (status.mafUnits == MG_S) {

		// Convert RAW datavalue back into mg/s
		// flowRateMGS = lookupValue / 10;
		flowRateMGS = lookupValue;

		// convert mg/s value into kg/h
		flowRateKGH = flowRateMGS / 277.8;

	}


	return flowRateKGH;


}

/***********************************************************
 * @brief get Reference Pressure sensor voltage
 * @returns current PRef sensor value in Volts
 ***/
double Sensors::getPRefVolts() {

	Hardware _hardware;

	double sensorVolts = 0.0;

	#if defined PREF_SRC_ADC && defined PREF_IS_ENABLED && defined ADC_IS_ENABLED // use ADC value

		sensorVolts = _hardware.getADCVolts(PREF_ADC_CHANNEL);	

	#elif defined PREF_SRC_PIN && defined PREF_IS_ENABLED // use raw pin value

		long refPressRaw = analogRead(REF_PRESSURE_PIN);
		sensorVolts = refPressRaw * (_hardware.get3v3SupplyVolts() / 4095.0);

	#else // return a fixed value
	
		sensorVolts = 1.0;
		return sensorVolts;
	#endif

	// Lets make sure we have a valid value to return
	if (sensorVolts > 0.0) { 
		sensorVolts += PREF_MV_TRIMPOT;
		return sensorVolts;
	} else { 
		return 0.0;
	}


}





/***********************************************************
 * @brief Process Reference Pressure sensor value depending on sensor type
 * @returns Reference pressure in kPa
 * @note Default sensor MPXV7007DP - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
 ***/
double Sensors::getPRefValue() {

	Hardware _hardware;
	
	double sensorVal = 0.0;
	double sensorVolts = this->getPRefVolts();
	
	// Convert value to kPa according to sensor type
	#if defined PREF_SENSOR_TYPE_LINEAR_ANALOG
		sensorVal = this->getPRefVolts() * PREF_ANALOG_SCALE;
	
	#elif defined PREF_SENSOR_TYPE_MPXV7007

		// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
		// sensorVal (kPa) = (sensorVolts - 0.5 * _hardware.get5vSupplyVolts() ) / (0.057 * _hardware.get5vSupplyVolts() / 1000);
		sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;

	#else

		sensorVal = FIXED_REF_PRESS_VALUE;

	#endif

	// Lets make sure we have a valid value to return
	if (sensorVal > 0) { 
		return sensorVal;
	} else { 
		return 0.0001; // return small non zero value to prevent divide by zero errors (will be truncated to zero in display)
	}


}





/***********************************************************
 * @brief Get PDiff Volts
 * @returns Differential Pressure in Volts
 ***/
double Sensors::getPDiffVolts() {
	
	Hardware _hardware;

	double sensorVolts = 0.0;
	
	#if defined PDIFF_SRC_IS_ADC && defined PDIFF_IS_ENABLED && defined ADC_IS_ENABLED // use ADC value

		sensorVolts = _hardware.getADCVolts(PDIFF_ADC_CHANNEL);
				
	#elif defined PDIFF_SRC_IS_PIN	// Use raw pin value

		long pDiffRaw = analogRead(DIFF_PRESSURE_PIN);
		sensorVolts = pDiffRaw * (_hardware.get3v3SupplyVolts() / 4095.0);

	#else // return a fixed value

		sensorVolts = 1.0;
		return sensorVolts;

	#endif
	
	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
		sensorVolts += PDIFF_MV_TRIMPOT;
		return sensorVolts;
	} else {
		return 0;
	}
	
}




/***********************************************************
 * @brief Get differential pressure in kPa
 * @returns Differential pressure in kPa
 * @note Default sensor is MPXV7007DP - Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
 ***/
double Sensors::getPDiffValue() {

	Hardware _hardware;

	double sensorVal = 0.0;
	double sensorVolts = this->getPDiffVolts();

	// Convert value to kPa according to sensor type
	#if defined PDIFF_SENSOR_TYPE_LINEAR_ANALOG

		diffPressureKpa = getPDiffVolts() * PDIFF_ANALOG_SCALE;
	
	#elif defined PDIFF_SENSOR_TYPE_MPXV7007 // Recommended sensor

		// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
		// sensorVal = (sensorVolts - 0.5 * _hardware.get5vSupplyVolts() ) / (0.057 * _hardware.get5vSupplyVolts() / 1000);
		sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;

	#else // use fixed value

		sensorVal = FIXED_PDIFF_PRESS;

	#endif

	// Lets make sure we have a valid value to return
	if (sensorVal > 0) {
		return sensorVal;
	} else {
		return 0.0001;
	}

}




/***********************************************************
 * @brief Get pitot Volts
 * @returns Pitot Pressure in Volts
 ***/
double Sensors::getPitotVolts() {

	Hardware _hardware;

	double sensorVolts = 0.0;
	
	#if defined PITOT_SRC_IS_ADC && defined PITOT_IS_ENABLED && defined ADC_IS_ENABLED // use ADC value

		sensorVolts = _hardware.getADCVolts(PITOT_ADC_CHANNEL);

	#elif defined PDIFF_SRC_IS_PIN	// use raw pin value

		long pitotRaw = analogRead(PITOT_PIN);
		sensorVolts = pitotRaw * (_hardware.get3v3SupplyVolts() / 4095.0);

	#else // use fixed value

		sensorVolts = 1.0;
		return sensorVolts;

	#endif
	
	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
		sensorVolts += PITOT_MV_TRIMPOT;
		return sensorVolts;
	} else {
		return 0.0;
	}
	
	
}





/***********************************************************
 * @brief get Pitot value in kPa
 * @returns Pitot pressure differential in kPa
 * @note Default sensor MPXV7007DP - Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
 ***/
double Sensors::getPitotValue() {
	
	Hardware _hardware;

	double sensorVal = 0.0;
	double sensorVolts = this->getPitotVolts();
	
	// Convert value to kPa according to sensor type
	#if defined PITOT_SENSOR_TYPE_LINEAR_ANALOG

		sensorVal = getPitotVolts() * PITOT_ANALOG_SCALE;
	
	#elif defined PITOT_SENSOR_TYPE_MPXV7007
		
		// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
		// sensorVal = (sensorVolts - 0.5 * _hardware.get5vSupplyVolts() ) / (0.057 * _hardware.get5vSupplyVolts() / 1000);
		sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;


	#else // use fixed value

		sensorVal = 0.0001;

	#endif

	// Lets make sure we have a valid value to return
	if (sensorVal > 0) {
		return sensorVal;
	} else {
		return 0.0001;
	}

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
		double tempVolts = rawTempValue * (_hardware.get3v3SupplyVolts() / 4095.0);	
		tempVolts += TEMP_MV_TRIMPOT;		
		refTempDegC = tempVolts * TEMP_ANALOG_SCALE;
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
		refTempDegC = FIXED_TEMP_VALUE;
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
		double baroVolts = rawBaroValue * (_hardware.get3v3SupplyVolts() / 4095.0);
		baroVolts += BARO_MV_TRIMPOT;		
		baroPressureHpa = baroVolts * BARO_ANALOG_SCALE;
		baroPressureHpa += BARO_FINE_ADJUST;
	
	#elif defined BARO_SENSOR_TYPE_MPX4115 && defined ADC_IS_ENABLED
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
		baroPressureKpa = FIXED_BARO_VALUE;
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
		
	#ifdef RELH_SENSOR_TYPE_LINEAR_ANALOG
	
		long rawRelhValue = analogRead(HUMIDITY_PIN);
		double relhVolts = rawRelhValue * (_hardware.get3v3SupplyVolts() / 4095.0);
		relhVolts += RELH_MV_TRIMPOT;		
		relativeHumidity = relhVolts * RELH_ANALOG_SCALE;
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
		relativeHumidity = FIXED_RELH_VALUE; // (36%)
	
	#endif

	if (RELH_FINE_ADJUST != 0) relativeHumidity += RELH_FINE_ADJUST;
	return relativeHumidity;
	
}


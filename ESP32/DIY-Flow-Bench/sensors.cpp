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

#include <Adafruit_ADS1X15.h>
#include <SFunBME280.h>
#include <Wire.h>
#include "Arduino.h"
#include "configuration.h"
#include "constants.h"
#include "pins.h"
#include "structs.h"
#include "hardware.h"
#include "sensors.h"
#include "messages.h"
#include "driver/pcnt.h"
#include LANGUAGE_FILE
#include MAF_DATA_FILE

//#include "bme280.h"



Sensors::Sensors() {

	// mafData _mafData;
	
	// extern char* mafSensorType;
	// extern int mafOutputType;
	// extern int mafDataUnit;	
	
	// this->_mafSensorType = mafSensorType;
	// this->_mafOutputType  = mafOutputType;
	// this->_mafDataUnit = mafDataUnit;



}


void Sensors::begin () {

	Messages _message;
	
	_message.serialPrintf((char*)"Initialising Sensors \n");
	
	Sensors::initialise ();
	
	_message.serialPrintf((char*)"Sensors Initialised \n");

}



// NOTE: Voodoo needed to get interrupt to work within class structure for frequency measurement. 
// We declare a new instance of the Sensor class so that we can use it for the MAF ISR
// https://forum.arduino.cc/t/pointer-to-member-function/365758
Sensors __mafVoodoo;



void Sensors::initialise () {

	Messages _message;
	MafData _mafdata;

	extern struct DeviceStatus status;
    // extern struct translator translate;
	// extern int mafOutputType;

	// _mafdata.begin();
	
	//initialise BME
	#ifdef BME_IS_ENABLED
	
		//BME280 _bme280;
		BME280 _BMESensor;
		
		_message.serialPrintf((char*)"Initialising BME280 \n");	
		
		_BMESensor.setMode(0); // Sleep mode - required when writing settings to BME device
		_BMESensor.setReferencePressure(SEALEVELPRESSURE_HPA * 100);
		_BMESensor.setI2CAddress(BME280_I2C_ADDR);
		_BMESensor.beginI2C();
		_BMESensor.setPressureOverSample(16);
		_BMESensor.setHumidityOverSample(4);
		_BMESensor.setTempOverSample(16);
		_BMESensor.setStandbyTime(1);
		_BMESensor.setFilter(16);
		_BMESensor.setMode(2); // Normal
		
		//_bme280.initialise(BME280_I2C_ADDR);
//		//_bme280.setControl(0x11); // 0x11 - Normal Mode
	
	
		// if (!this->BME280init()) {
		// 	_message.statusPrintf((char*)"BME280 device Error!! \n");
		// } else {
		// 	_message.statusPrintf((char*)"BME280 device initialised \n");		
		// }
	#endif

	// Baro Sensor
	#ifdef BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		this->startupBaroPressure = this->getPRefValue();
		this->_baroSensorType = translate.LANG_VAL_START_REF_PRESSURE;
	#elif defined BARO_SENSOR_TYPE_FIXED_VALUE
		this->startupBaroPressure = DEFAULT_BARO_VALUE;
		this->_baroSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_baroSensorType += DEFAULT_BARO_VALUE;
	#elif defined BARO_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_baroSensorType = (char*)"BME280";
	#elif defined BARO_SENSOR_TYPE_MPX4115
		this->_baroSensorType = "MPX4115";
	#elif defined BARO_SENSOR_TYPE_LINEAR_ANALOG
		this->_baroSensorType = "ANALOG PIN: " + REF_BARO_PIN;
	#endif
	
	//Temp Sensor
	#ifdef TEMP_SENSOR_NOT_USED
		this->startupBaroPressure = this->getPRefValue();
		this->_tempSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined TEMP_SENSOR_TYPE_FIXED_VALUE
		this->_tempSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_tempSensorType += DEFAULT_TEMP_VALUE;
	#elif defined TEMP_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_tempSensorType = (char*)"BME280";
	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_tempSensorType = (char*)"Simple DHT11";
	#elif defined TEMP_SENSOR_TYPE_LINEAR_ANALOG
		this->_tempSensorType = (char*)"ANALOG PIN: " + TEMPERATURE_PIN;
	#endif
	
	// Rel Humidity Sensor
	#ifdef RELH_SENSOR_NOT_USED
		this->startupBaroPressure = this->getPRefValue();
		this->_relhSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined RELH_SENSOR_TYPE_FIXED_VALUE
		this->_relhSensorType = translate.LANG_VAL_FIXED_VALUE;
		this->_relhSensorType += DEFAULT_RELH_VALUE;
	#elif defined RELH_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_relhSensorType = (char*)"BME280";
	#elif defined RELH_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_relhSensorType = "Simple DHT11";
	#elif defined RELH_SENSOR_TYPE_LINEAR_ANALOG
		this->_relhSensorType = (char*)"ANALOG PIN: " + HUMIDITY_PIN;
	#endif

	// reference pressure
	#ifdef PREF_SENSOR_NOT_USED
		this->_prefSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PREF_SENSOR_TYPE_MPXV7007
		this->_prefSensorType = (char*)"SMPXV7007";
	#elif defined PREF_SENSOR_TYPE_MPX4250
		this->_prefSensorType = (char*)"MPX4250";
	#elif defined PREF_SENSOR_TYPE_LINEAR_ANALOG
		this->_prefSensorType = (char*)"ANALOG PIN: " + REF_PRESSURE_PIN;
	#endif
	
	// differential pressure
	#ifdef PDIFF_SENSOR_NOT_USED
		this->_pdiffSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PPDIFF_SENSOR_TYPE_MPXV7007
		this->_pdiffSensorType = (char*)"SMPXV7007";
	#elif defined PDIFF_SENSOR_TYPE_LINEAR_ANALOG
		this->_pdiffSensorType = (char*)"ANALOG PIN: " + DIFF_PRESSURE_PIN;
	#endif
	
	// pitot pressure differential
    #ifdef PITOT_SENSOR_NOT_USED
		this->_pitotSensorType = translate.LANG_VAL_NOT_ENABLED;
	#elif defined PITOT_SENSOR_TYPE_MPXV7007
		this->_pitotSensorType = (char*)"SMPXV7007";
	#elif defined PITOT_SENSOR_TYPE_LINEAR_ANALOG
		this->_pitotSensorType = (char*)"ANALOG PIN: " + PITOT_PIN;
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
	// Set status values for GUI
	status.mafSensor = _mafdata.mafSensorType;
	status.baroSensor = this->_baroSensorType;
	status.tempSensor = this->_tempSensorType;
	status.relhSensor = this->_relhSensorType;
	status.prefSensor = this->_prefSensorType;
	status.pdiffSensor = this->_pdiffSensorType;
	status.pitotSensor = this->_pitotSensorType;

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
 * Returns MAF signal in Millivolts
 ***/
float Sensors::getMafMillivolts() {
	
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
 * Returns RAW MAF Sensor value
 *
 * MAF decode is done in Calculations.cpp
 ***/
float Sensors::getMafValue() {
	
	Hardware _hardware;
	MafData _mafdata;
	
	float mafFlow = 0.0;

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
}




/***********************************************************
 * Returns temperature in Deg C
 ***/
float Sensors::getTempValue() {
	
	Hardware _hardware;
	Messages _message;
	//BME280 _bme280;
	BME280 _BMESensor;

	float refTempDegC;
	
	   // extern struct translator translate;
	
	#ifdef TEMP_SENSOR_TYPE_LINEAR_ANALOG
	
		long rawTempValue = analogRead(TEMPERATURE_PIN);	
		float tempMillivolts = (rawTempValue * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;	
		tempMillivolts += TEMP_MV_TRIMPOT;		
		refTempDegC = tempMillivolts * TEMP_ANALOG_SCALE;
		
	#elif defined TEMP_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		
		//refTempDegC = _bme280.getTemperature();
		
		refTempDegC = _BMESensor.readTempC() / 100.00F;
	
// //REMOVE: DEBUG_PRINT	
// _message.debugPrintf((char*)"refTempDegC - %f \n", refTempDegC);

	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(translate.LANG_VAL_DHT11_READ_FAIL); // Set error to display on screen
		  refTempDegC = 0;        
		} else {
		  refTempDegC = refTemp;
		}	
	#else
		// We don't have any temperature input so we will assume default
		refTempDegC = DEFAULT_TEMP_VALUE;
	#endif
	
	
//REMOVE:
_message.debugPrintf((char*)"TEMP =  %f /n", refTempDegC);
	
	return refTempDegC + TEMP_FINE_ADJUST;
}



/***********************************************************
* Returns Barometric pressure in kPa
* NOTE: Should we use Hpa?
***/
float Sensors::getBaroValue() {
	
	Hardware _hardware;
	// BME280 _bme280;
	BME280 _BMESensor;
		
	#if defined BARO_SENSOR_TYPE_LINEAR_ANALOG
		
		long rawBaroValue = analogRead(REF_BARO_PIN);
		float baroMillivolts = (rawBaroValue * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
		baroMillivolts += BARO_MV_TRIMPOT;		
		baroPressureKpa = baroMillivolts * BARO_ANALOG_SCALE;
	
	#elif defined BARO_SENSOR_TYPE_MPX4115
		// Datasheet - https://html.alldatasheet.es/html-pdf/5178/MOTOROLA/MPX4115/258/1/MPX4115.html
		// Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
		refPressureKPa = map(_hardware.getADCRawData(PREF_ADC_CHANNEL), 0, 4095, 15000, 115000);

	#elif defined BARO_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
			
		// baroPressureKpa = _bme280.getBaro() / 100.0F;
		baroPressureHpa = _BMESensor.readFloatPressure() / 100.0F; // readFloatPressure = uint32_t
		
	#elif defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		// No baro sensor defined so use value grabbed at startup from reference pressure sensor
		// NOTE will only work for absolute style pressure sensor like the MPX4250
		baroPressureKpa = startupBaroPressure; 
	#else
		// we don't have any sensor so use default - // NOTE: standard sea level baro pressure is 14.7 psi
		baroPressureKpa = DEFAULT_BARO_VALUE;
	#endif

	return baroPressureKpa = baroPressureHpa  + BARO_FINE_ADJUST;
}



/***********************************************************
 * Returns Relative Humidity in %
 ***/
double Sensors::getRelHValue() {
	
	Hardware _hardware;
	// BME280 _bme280;
	BME280 _BMESensor;
	
	  // extern struct translator translate;
		
	#ifdef RELH_SENSOR_TYPE_LINEAR_ANALOG
	
		long rawRelhValue = analogRead(HUMIDITY_PIN);
		float relhMillivolts = (rawRelhValue * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
		relhMillivolts += RELH_MV_TRIMPOT;		
		relativeHumidity = relhMillivolts * RELH_ANALOG_SCALE;
	
	#elif defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11
	
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(translate.LANG_VAL_DHT11_READ_FAIL); // Set error to display on screen
		  relativeHumidity = 0;        
		} else {
		  relativeHumidity = refRelh;
		}

	#elif defined RELH_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
	
		// relativeHumidity = _bme280.getHumidity(); 
		relativeHumidity = _BMESensor.readFloatHumidity() * 10;
		

	#else
		// we don't have a sensor so use nominal fixed value 
		relativeHumidity = DEFAULT_RELH_VALUE; // (36%)
	
	#endif
	
	// DEPRECATED: Sanity check value (masks error - not really good)
	// if (relativeHumidity > 100.0) {
	//   relativeHumidity = 100.0;  
	// } else if (relativeHumidity < 0.0) {
	//     relativeHumidity = 0.0;
	// }

	return  relativeHumidity + RELH_FINE_ADJUST;
}


/***********************************************************
 * Returns altitude based on pressure difference from local sea level
 ***/
float Sensors::getAltitude() {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude. See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  //TODO: BREAK PRESSURE OUT TO CONFIG VALUE THAT CAN BE SET IN GUI
  float atmospheric = getRelHValue() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / SEALEVELPRESSURE_HPA, 0.1903));
}



/***********************************************************
 * getPRefMillivolts
 * Returns Reference pressure in Millivolts
 ***/
float Sensors::getPRefMillivolts() {

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
float Sensors::getPRefValue() {

	Hardware _hardware;
	Messages _message;
	
	float refPressureKpa = 0.0;
	
	#if defined PREF_SENSOR_TYPE_LINEAR_ANALOG
		refPressureKpa = getPRefMillivolts() * PREF_ANALOG_SCALE;
	
	#elif defined PREF_SENSOR_TYPE_MPXV7007
		// Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
		// Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007DP Datasheet)
		// refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057;  
		
		if (!(Sensors::getPRefMillivolts() > 0) | !(_hardware.get5vSupplyMillivolts() > 0)) return 1; 

		refPressureKpa = (((Sensors::getPRefMillivolts() / _hardware.get5vSupplyMillivolts() ) - 0.5) / 0.057);  

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
float Sensors::getPDiffMillivolts() {
	
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
float Sensors::getPDiffValue() {

	Hardware _hardware;
	Messages _message;

	float diffPressureKpa = 0.0;
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
	
//_message.serialPrintf((char*)"%s \n", diffPressRaw);	
	
	diffPressMillivolts += PDIFF_MV_TRIMPOT;

	#if defined PDIFF_SENSOR_TYPE_LINEAR_ANALOG
		diffPressureKpa = diffPressMillivolts * PDIFF_ANALOG_SCALE;
	
	#elif defined PDIFF_SENSOR_TYPE_MPXV7007
		// RECOMMENDED SENSOR
		// Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
		// Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007DP Datasheet)
		
		#if defined ADC_TYPE_ADS1115
			//scale for 16 bit ADC
			if (!(Sensors::getPDiffMillivolts() > 0) | !(_hardware.get5vSupplyMillivolts() > 0)) return 1; 
			
			diffPressureKpa = (((Sensors::getPDiffMillivolts() / _hardware.get5vSupplyMillivolts() ) - 0.5) / 0.057);  
			
			
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
float Sensors::getPitotMillivolts() {

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
float Sensors::getPitotValue() {
	
	Hardware _hardware;


	float pitotPressureKpa = 0.0;
 	long pitotPressRaw;
	float pitotMillivolts;
	
	
	#if defined PITOT_SRC_IS_ADC && defined ADC_IS_ENABLED
		pitotMillivolts = getPitotMillivolts();
	#elif defined PITOT_SRC_IS_PIN	
		pitotPressRaw = analogRead(PITOT_PIN);
		pitotMillivolts = (pitotPressRaw * (_hardware.get3v3SupplyMillivolts() / 4095.0)) * 1000;
	#else
		pitotMillivolts = 1;
		return 1;
	#endif
	
//Serial.println(pitotPressRaw);		
	
	pitotMillivolts += PITOT_MV_TRIMPOT;
	
	#if defined PITOT_SENSOR_TYPE_LINEAR_ANALOG
		pitotPressureKpa = pitotMillivolts * PITOT_ANALOG_SCALE;
	
	#elif defined PITOT_SENSOR_TYPE_MPXV7007DP
		
		if (!(Sensors::getPitotMillivolts() > 0) | !(_hardware.get5vSupplyMillivolts() > 0)) return 1; 
		
		pitotPressureKpa = (((Sensors::getPitotMillivolts() / _hardware.get5vSupplyMillivolts() ) - 0.5) / 0.057);  

	#else
		// No pitot probe used so lets return a fixed value
		pitotPressureKpa = DEFAULT_PITOT_VALUE;

	#endif

	return pitotPressureKpa;
}









//DEPRECATED:
/*


// https://github.com/Seeed-Studio/Grove_BME280/blob/master/Seeed_BME280.cpp

bool Sensors::BME280init() {
	
	Messages _message;
	
	uint8_t retry = 0;
	uint8_t chip_id = 0;
	
	// while ((retry++ < 5) && (chip_id != 0x60)) {
	// 	chip_id = Sensors::BME280Read8(BME280_REG_CHIPID);
	// 	_message.statusPrintf((char*)"BME280 Read chip ID: %s \n", chip_id);
	// 	delay(100);
	// }
	if (chip_id != 0x60){
		_message.statusPrintf((char*)"BME280 Read Chip ID fail! \n");
		return false;
	}
	
	dig_T1 = BME280Read16LE(BME280_REG_DIG_T1);
	dig_T2 = BME280ReadS16LE(BME280_REG_DIG_T2);
	dig_T3 = BME280ReadS16LE(BME280_REG_DIG_T3);
	
	dig_P1 = BME280Read16LE(BME280_REG_DIG_P1);
	dig_P2 = BME280ReadS16LE(BME280_REG_DIG_P2);
	dig_P3 = BME280ReadS16LE(BME280_REG_DIG_P3);
	dig_P4 = BME280ReadS16LE(BME280_REG_DIG_P4);
	dig_P5 = BME280ReadS16LE(BME280_REG_DIG_P5);
	dig_P6 = BME280ReadS16LE(BME280_REG_DIG_P6);
	dig_P7 = BME280ReadS16LE(BME280_REG_DIG_P7);
	dig_P8 = BME280ReadS16LE(BME280_REG_DIG_P8);
	dig_P9 = BME280ReadS16LE(BME280_REG_DIG_P9);
	
	dig_H1 = BME280Read8(BME280_REG_DIG_H1);
	dig_H2 = BME280Read16LE(BME280_REG_DIG_H2);
	dig_H3 = BME280Read8(BME280_REG_DIG_H3);
	dig_H4 = (BME280Read8(BME280_REG_DIG_H4) << 4) | (0x0F & BME280Read8(BME280_REG_DIG_H4 + 1));
	dig_H5 = (BME280Read8(BME280_REG_DIG_H5 + 1) << 4) | (0x0F & BME280Read8(BME280_REG_DIG_H5) >> 4);
	dig_H6 = (int8_t)BME280Read8(BME280_REG_DIG_H6);
	
	BME280WriteRegister(BME280_REG_CTRLHUMID, 0x05);  //Choose 16X oversampling
	BME280WriteRegister(BME280_REG_CTRL, 0xB7);  //Choose 16X oversampling
	
	return true;
}

float Sensors::BME280GetTemperature(void) {
	int32_t var1, var2;

	int32_t adc_T = BME280Read24(BME280_REG_TEMPDATA);
	// Check if the last transport succeeded
	if (!isTransport_OK) {
		return 0;
	}
	adc_T >>= 4;
	var1 = (((adc_T >> 3) - ((int32_t)(dig_T1 << 1))) *	((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	float T = (t_fine * 5 + 128) >> 8;
	return T / 100;
}


float Sensors::BME280getBaro(void) {
  int64_t var1, var2, p;
  // Call getTemperature to get t_fine
  BME280GetTemperature();
  // Check if the last transport succeeded
  if (!isTransport_OK) {
	return 0;
  }
  int32_t adc_P = BME280Read24(BME280_REG_PRESSUREDATA);
  adc_P >>= 4;
  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)dig_P6;
  var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
  var2 = var2 + (((int64_t)dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
  if (var1 == 0) {
	return 0; // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
  return (float)(p / 256.0);
}

float Sensors::BME280GetHumidity(void) {
  int32_t v_x1_u32r, adc_H;
  // Call getTemperature to get t_fine	
  BME280GetTemperature();
  // Check if the last transport succeeded
  if (!isTransport_OK) {
	return 0;
  }
  adc_H = BME280Read16(BME280_REG_HUMIDITYDATA);
  v_x1_u32r = (t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
  v_x1_u32r = v_x1_u32r >> 12;
  float h = v_x1_u32r / 1024.0;
  return h;
}


uint8_t Sensors::BME280Read8(uint8_t reg) {
	
	Wire.beginTransmission(BME280_I2C_ADDR);
	Wire.write(reg);
	Wire.endTransmission();

	Wire.requestFrom(BME280_I2C_ADDR, 1);
	// return 0 if slave didn't response
	if (Wire.available() < 1) {
		isTransport_OK = false;
		return 0;
	} else {
		isTransport_OK = true;
	}

	return Wire.read();
}


uint16_t Sensors::BME280Read16(uint8_t reg) {
	uint8_t msb, lsb;

	Wire.beginTransmission(BME280_I2C_ADDR);
	Wire.write(reg);
	Wire.endTransmission();

	Wire.requestFrom(BME280_I2C_ADDR, 2);
	// return 0 if slave didn't response
	if (Wire.available() < 2) {
		isTransport_OK = false;
		return 0;
	} else {
		isTransport_OK = true;
	}
	msb = Wire.read();
	lsb = Wire.read();

	return (uint16_t) msb << 8 | lsb;
}


uint16_t Sensors::BME280Read16LE(uint8_t reg) {
	uint16_t data = BME280Read16(reg);
	return (data >> 8) | (data << 8);
}

int16_t Sensors::BME280ReadS16(uint8_t reg) {
	return (int16_t)BME280Read16(reg);
}

int16_t Sensors::BME280ReadS16LE(uint8_t reg) {
	return (int16_t)BME280Read16LE(reg);
}

uint32_t Sensors::BME280Read24(uint8_t reg) {
	
	Messages _message;
	
	uint32_t data;

	Wire.beginTransmission(BME280_I2C_ADDR);
	Wire.write(reg);
	Wire.endTransmission();

	Wire.requestFrom(BME280_I2C_ADDR, 3);
	// return 0 if slave didn't respond
	// if (Wire.available() < 3) {
	// 	isTransport_OK = false;
	// 	return 0;
	// } else if (isTransport_OK == false) {
	// 	isTransport_OK = true;
	// 	if (!BME280init()) {
	// 		_message.statusPrintf((char*)"BME280 device not connected or broken! \n");
	// 	}
	// }
	data = Wire.read();
	data <<= 8;
	data |= Wire.read();
	data <<= 8;
	data |= Wire.read();

	return data;
}

void Sensors::BME280WriteRegister(uint8_t reg, uint8_t val) {
	Wire.beginTransmission(BME280_I2C_ADDR); // start transmission to device
	Wire.write(reg);       // send register address
	Wire.write(val);         // send value to write
	Wire.endTransmission();     // end transmission
}

*/

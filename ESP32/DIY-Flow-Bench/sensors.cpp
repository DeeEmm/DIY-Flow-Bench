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
#include "hardware.h"
#include "sensors.h"
#include "messages.h"
#include "driver/pcnt.h"

#include LANGUAGE_FILE


Sensors::Sensors() {

	extern String mafSensorType;
	extern int MAFoutputType;
	extern int MAFdataUnit;	
	
	this->_mafSensorType = mafSensorType;
	this->_mafOutputType  = MAFoutputType;
	this->_mafDataUnit = MAFdataUnit;
	
}


void Sensors::Begin () {

	Messages _message;
		
	// What hardware / sensors do we have? Lets initialise them 
	
	// Support for ADAFRUIT_BME280 temp, pressure & Humidity sensors
	// https://github.com/adafruit/Adafruit_BME280_Library
	#if defined TEMP_SENSOR_TYPE_ADAFRUIT_BME280 || defined BARO_SENSOR_TYPE_ADAFRUIT_BME280 || defined RELH_SENSOR_TYPE_ADAFRUIT_BME280 
		#include <Adafruit_BME280.h> 
		Adafruit_BME280 adafruitBme280; // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)
		//I2C address - BME280_I2C_ADDR
		if (!adafruitBme280.begin()) {  
			return BME280_READ_FAIL;			
		  _message.DebugPrintLn("Adafruit BME280 Initialisation failed");      
		} else {
		  _message.DebugPrintLn("Adafruit BME280 Initialised");      
		}
	#endif
		
		
	// Support for SPARKFUN_BME280 temp, pressure & Humidity sensors
	// https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide?_ga=2.39864294.574007306.1596270790-134320310.1596270790
	#if defined TEMP_SENSOR_TYPE_SPARKFUN_BME280 || defined BARO_SENSOR_TYPE_SPARKFUN_BME280 || defined RELH_SENSOR_TYPE_SPARKFUN_BME280 
		// TODO: There is an issue with this library. Was always a bit flakey but more so now it's inside this class.
		#include "SparkFunBME280.h"
		#include "Wire.h"
		BME280 SparkFunBME280;
		Wire.begin();
		SparkFunBME280.setI2CAddress(BME280_I2C_ADDR); 
		if (SparkFunBME280.beginI2C() == false) //Begin communication over I2C
		{
			return BME280_READ_FAIL;			
		  _message.DebugPrintLn("Sparkfun BME280 Initialisation failed");      
		} else {
		  _message.DebugPrintLn("Sparkfun BME280 Initialised");      
		}
	#endif

	// Support for DHT11 humidity / temperature sensors
	// https://github.com/winlinvip/SimpleDHT
	#if defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11 || defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11 
		#include <SimpleDHT.h>    		 
	#endif
}


// Voodoo needed to get interrupt to work within class structure for frequency measurement. 
// We declare a new instance of the Sensor class so that we can use it for the MAF ISR
// https://forum.arduino.cc/t/pointer-to-member-function/365758
Sensors __mafVoodoo;


void Sensors::Initialise () {

	extern struct DeviceStatus status;
	extern int MAFoutputType;

	// Baro Sensor
	#ifdef BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		this->startupBaroPressure = this->PRef(KPA);
		this->_baroSensorType = LANG_START_REF_PRESSURE;
	#elif defined BARO_SENSOR_TYPE_FIXED_VALUE
		this->startupBaroPressure = DEFAULT_BARO;
		this->_baroSensorType = LANG_FIXED_VALUE;
		this->_baroSensorType += DEFAULT_BARO;
	#elif defined BARO_SENSOR_TYPE_SPARKFUN_BME280
		this->_baroSensorType = "Sparkfun BME280";
	#elif defined BARO_SENSOR_TYPE_ADAFRUIT_BME280
		this->_baroSensorType = "Adafruit BME280";
	#elif defined BARO_SENSOR_TYPE_MPX4115
		this->_baroSensorType = "MPX4115";
	#elif defined BARO_SENSOR_TYPE_LINEAR_ANALOG
		this->_baroSensorType = "ANALOG PIN: " + REF_BARO_PIN;
	#endif
	
	//Temp Sensor
	#ifdef TEMP_SENSOR_NOT_USED
		this->startupBaroPressure = this->PRef(KPA);
		this->_tempSensorType = LANG_NOT_ENABLED;
	#elif defined TEMP_SENSOR_TYPE_FIXED_VALUE
		this->_tempSensorType = LANG_FIXED_VALUE;
		this->_tempSensorType += DEFAULT_TEMP;
	#elif defined TEMP_SENSOR_TYPE_SPARKFUN_BME280
		this->_tempSensorType = "Sparkfun BME280";
	#elif defined TEMP_SENSOR_TYPE_ADAFRUIT_BME280
		this->_tempSensorType = "Adafruit BME280";
	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_tempSensorType = "Simple DHT11";
	#elif defined TEMP_SENSOR_TYPE_LINEAR_ANALOG
		this->_tempSensorType = "ANALOG PIN: " + TEMPERATURE_PIN;
	#endif
	
	// Rel Humidity Sensor
	#ifdef RELH_SENSOR_NOT_USED
		this->startupBaroPressure = this->PRef(KPA);
		this->_relhSensorType = LANG_NOT_ENABLED;
	#elif defined RELH_SENSOR_TYPE_FIXED_VALUE
		this->_relhSensorType = LANG_FIXED_VALUE;
		this->_relhSensorType += DEFAULT_RELH;
	#elif defined RELH_SENSOR_TYPE_SPARKFUN_BME280
		this->_relhSensorType = "Sparkfun BME280";
	#elif defined RELH_SENSOR_TYPE_ADAFRUIT_BME280
		this->_relhSensorType = "Adafruit BME280";
	#elif defined RELH_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_relhSensorType = "Simple DHT11";
	#elif defined RELH_SENSOR_TYPE_LINEAR_ANALOG
		this->_relhSensorType = "ANALOG PIN: " + HUMIDITY_PIN;
	#endif

	// reference pressure
	#ifdef PREF_SENSOR_NOT_USED
		this->_prefSensorType = LANG_NOT_ENABLED;
	#elif defined PREF_SENSOR_TYPE_MPXV7007
		this->_prefSensorType = "SMPXV7007";
	#elif defined PREF_SENSOR_TYPE_MPX4250
		this->_prefSensorType = "MPX4250";
	#elif defined PREF_SENSOR_TYPE_LINEAR_ANALOG
		this->_prefSensorType = "ANALOG PIN: " + REF_PRESSURE_PIN;
	#endif
	
	// differential pressure
	#ifdef PDIFF_SENSOR_NOT_USED
		this->_pdiffSensorType = LANG_NOT_ENABLED;
	#elif defined PPDIFF_SENSOR_TYPE_MPXV7007
		this->_pdiffSensorType = "SMPXV7007";
	#elif defined PDIFF_SENSOR_TYPE_LINEAR_ANALOG
		this->_pdiffSensorType = "ANALOG PIN: " + DIFF_PRESSURE_PIN;
	#endif
	
	// pitot pressure differential
    #ifdef PITOT_SENSOR_NOT_USED
		this->_pitotSensorType = LANG_NOT_ENABLED;
	#elif defined PITOT_SENSOR_TYPE_MPXV7007
		this->_pitotSensorType = "SMPXV7007";
	#elif defined PITOT_SENSOR_TYPE_LINEAR_ANALOG
		this->_pitotSensorType = "ANALOG PIN: " + PITOT_PIN;
	#endif
	
	
	// Set up the MAF ISR if required
	if (MAFoutputType == FREQUENCY) {	
		__mafVoodoo.mafSetupISR(MAF_PIN, []{__mafVoodoo.mafFreqCountISR();}, FALLING);
		timer = timerBegin(0, 2, true);                                  
		timerStart(timer);	
	}
	
	// Set status values for GUI
	status.mafSensor = this->_mafSensorType;
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
void Sensors::mafSetupISR(uint8_t irq_pin, void (*ISR_callback)(void), int value) {
  attachInterrupt(digitalPinToInterrupt(irq_pin), ISR_callback, value);
}




/***********************************************************
* Interrupt Service Routine for MAF Frequency measurement
*
* Determine how long since last triggered (Resides in RAM memory as it is faster)
***/
// TODO: Add IRAM_ATTR to Nova-Arduino 
void IRAM_ATTR Sensors::mafFreqCountISR() {
    uint64_t TempVal = timerRead(timer);            
    PeriodCount = TempVal - StartValue;             
    StartValue = TempVal;                           
}




/***********************************************************
 * Returns RAW MAF Sensor value
 *
 * MAF decode is done in Maths.cpp
 ***/
float Sensors::getMafValue() {
	
	Hardware _hardware;
	
	float mafFlow = 0.0;
	
	switch (this->_mafOutputType) {
		
		case VOLTAGE:
		{
			int supplyMillivolts = _hardware.getSupplyMillivolts();
			int mafFlowRaw = analogRead(MAF_PIN);
			double mafMillivolts = (mafFlowRaw * (supplyMillivolts / 4095.0)) * 1000;
			mafFlow = mafMillivolts + MAF_TRIMPOT;
			return mafFlow;
		}
		break;
		
		case FREQUENCY:
		{  
			mafFlow = 40000000.00 / PeriodCount;
			//mafFlow = 40000000.00 / __mafVoodoo.PeriodCount; // NOTE: Do we need Voodoo?
			mafFlow += MAF_TRIMPOT;
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
	float refTempDegC;
	
	int supplyMillivolts = _hardware.getSupplyMillivolts();
	int rawTempValue = analogRead(TEMPERATURE_PIN);
	int tempMillivolts = (rawTempValue * (supplyMillivolts / 4095.0)) * 1000;
	tempMillivolts += TEMP_TRIMPOT;		

	#ifdef TEMP_SENSOR_TYPE_LINEAR_ANALOG
		refTempDegC = tempMillivolts * TEMP_ANALOG_SCALE;
		
	#elif TEMP_SENSOR_TYPE_ADAFRUIT_BME280
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
	
	return refTempDegC;
}



/***********************************************************
* Returns Barometric pressure in kPa
* NOTE: Sensor must return an absolute value
***/
float Sensors::getBaroValue() {
	
	Hardware _hardware;
	
	float baroPressureKpa;
	
	int supplyMillivolts = _hardware.getSupplyMillivolts();
	int rawBaroValue = analogRead(REF_BARO_PIN);
	int baroMillivolts = (rawBaroValue * (supplyMillivolts / 4095.0)) * 1000;
	baroMillivolts += BARO_TRIMPOT;		

	#ifdef BARO_SENSOR_TYPE_LINEAR_ANALOG
		baroPressureKpa = baroMillivolts * BARO_ANALOG_SCALE;
	
	#elif BARO_SENSOR_TYPE_MPX4115
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
	
	return baroPressureKpa;
}



/***********************************************************
 * Returns Relative Humidity in %
 ***/
float Sensors::getRelHValue() {
	
	Hardware _hardware;
	
	float relativeHumidity;
	
	int supplyMillivolts = _hardware.getSupplyMillivolts();
	int rawRelhValue = analogRead(HUMIDITY_PIN);
	int relhMillivolts = (rawRelhValue * (supplyMillivolts / 4095.0)) * 1000;
	relhMillivolts += BARO_TRIMPOT;		

	#ifdef RELH_SENSOR_TYPE_LINEAR_ANALOG
		relativeHumidity = relhMillivolts * RELH_ANALOG_SCALE;
	
	#elif RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11
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

	return relativeHumidity;
}



/***********************************************************
 * Returns Reference pressure in kPa
 ***/
float Sensors::getPRefValue() {
	
	Hardware _hardware;

	float refPressureKpa = 0;

	float supplyMillivolts = _hardware.getSupplyMillivolts();
	int rawRefPressValue = analogRead(REF_PRESSURE_PIN);
	float refPressMillivolts = (rawRefPressValue * (supplyMillivolts / 4095.0)) * 1000;
	refPressMillivolts += PREF_TRIMPOT;

	#ifdef PREF_SENSOR_TYPE_LINEAR_ANALOG
		refPressureKpa = refPressMillivolts * PREF_ANALOG_SCALE;
	
	#elif PREF_SENSOR_TYPE_MPXV7007
		// Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
		// Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007DP Datasheet)
		// P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
		refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057;  

	#elif defined PREF_SENSOR_TYPE_MPX4250
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

	return refPressureKpa;
}





/***********************************************************
 * Returns Pressure differential in kPa
 ***/
float Sensors::getPDiffValue() {
	
	Hardware _hardware;

	float diffPressureKpa = 0;

	float supplyMillivolts = _hardware.getSupplyMillivolts();
	int rawDiffPressValue = analogRead(DIFF_PRESSURE_PIN);
	float diffPressMillivolts = (rawDiffPressValue * (supplyMillivolts / 4095.0)) * 1000;
	diffPressMillivolts += PDIFF_TRIMPOT;

	#ifdef PDIFF_SENSOR_TYPE_LINEAR_ANALOG
		diffPressureKpa = diffPressMillivolts * PDIFF_ANALOG_SCALE;
	
	#elif PDIFF_SENSOR_TYPE_MPXV7007
		// Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
		// Vout = VS x (0.057 x P + 0.5) --- Where VS = Supply Voltage (Formula from MPXV7007DP Datasheet)
		// P = ((Vout / VS ) - 0.5) / 0.057 --- Formula transposed for P
		refPressureKpa = ((refPressMillivolts / supplyMillivolts ) - 0.5) / 0.057;  

	#elif defined PDIFF_SENSOR_NOT_USED 
	#else
		// No reference pressure sensor used so lets return a value (so as not to throw maths out)
		// refPressureKpa = 6.97448943333324; //28"
		diffPressureKpa = DEFAULT_PDIFF_PRESS;

	#endif

	return diffPressureKpa;
}






/***********************************************************
 * Returns Pitot pressure differential in kPa
 ***/
float Sensors::getPitotValue() {
	
	Hardware _hardware;

	float pitotPressureKpa = 0.0;
	
	// Pitot Voltage
	float supplyMillivolts = _hardware.getSupplyMillivolts();
	int pitotRaw = analogRead(PITOT_PIN);
	double pitotMillivolts = (pitotRaw * (supplyMillivolts / 4095.0)) * 1000;
	pitotMillivolts += PITOT_TRIMPOT;
	
	#ifdef PITOT_SENSOR_TYPE_LINEAR_ANALOG
		pitotPressureKpa = pitotMillivolts * PITOT_ANALOG_SCALE;
	
	#elif PITOT_SENSOR_TYPE_MPXV7007DP
		// sensor characteristics from datasheet
		// Vout = VS x (0.057 x P + 0.5)
		pitotPressureKpa = (pitotMillivolts / supplyMillivolts - 0.5) / 0.057;

	#else
		// No pitot probe used so lets return a zero value
		pitotPressureKpa = 0.0;

	#endif

	return pitotPressureKpa;
}


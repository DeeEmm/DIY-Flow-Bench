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


Sensors::Sensors() {
	
	Messages _message;

	extern String mafSensorType;
	extern int MAFoutputType;
	extern int MAFdataUnit;	
	
	this->_mafSensorType = mafSensorType;
	this->_mafOutputType  = MAFoutputType;
	this->_mafDataUnit = MAFdataUnit;
	
	
	unsigned int buffer[24];
	unsigned int data[8];
	unsigned int dig_H1 = 0;
	
	
}


void Sensors::begin () {

	Messages _message;


	/* BME280 START 
		Code from - http://www.esp32learning.com/code/esp32-and-bme280-temperature-sensor-example.php
	*/

	Wire.begin();

	
	/* BME280 END */


	// Support for DHT11 humidity / temperature sensors
	// https://github.com/winlinvip/SimpleDHT
	// #if defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11 || defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11 
	// 	#include <SimpleDHT.h>    		 
	// #endif
	
	Sensors::initialise ();
	
	
	
}




// Voodoo needed to get interrupt to work within class structure for frequency measurement. 
// We declare a new instance of the Sensor class so that we can use it for the MAF ISR
// https://forum.arduino.cc/t/pointer-to-member-function/365758
Sensors __mafVoodoo;




void Sensors::initialise () {

	extern struct DeviceStatus status;
	extern int MAFoutputType;

	// Baro Sensor
	#ifdef BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		this->startupBaroPressure = this->PRef(KPA);
		this->_baroSensorType = LANG_START_REF_PRESSURE;
	#elif defined BARO_SENSOR_TYPE_FIXED_VALUE
		this->startupBaroPressure = DEFAULT_BARO_VALUE;
		this->_baroSensorType = LANG_FIXED_VALUE;
		this->_baroSensorType += DEFAULT_BARO_VALUE;
	#elif defined BARO_SENSOR_TYPE_BME280
		this->_baroSensorType = "BME280";
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
		this->_tempSensorType += DEFAULT_TEMP_VALUE;
	#elif defined TEMP_SENSOR_TYPE_BME280
		this->_tempSensorType = "BME280";
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
		this->_relhSensorType += DEFAULT_RELH_VALUE;
	#elif defined RELH_SENSOR_TYPE_BME280
		this->_relhSensorType = "BME280";
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
	// Note Frequency based MAFs are required to be attached direct to MAF pin for pulse counter to work
	// This means 5v > 3.3v signal conditioning is required on MAF pin
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
			float mafMillivolts = 0.0;
			int mafFlowRaw;
			
			#if defined MAF_SRC_ADC_1015 || defined MAF_SRC_ADC_1015
				mafMillivolts = _hardware.getAdcMillivolts(MAF_ADC_CHANNEL);
			#elif defined PREF_SRC_PIN	
				mafFlowRaw = analogRead(REF_PRESSURE_PIN);
				mafMillivolts = (mafFlowRaw * (supplyMillivolts / 4095.0)) * 1000;
			#endif

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
 * Read BME280
 *
 * Based on: http://www.esp32learning.com/code/esp32-and-bme280-temperature-sensor-example.php
 ***/
void Sensors::getBME280RawData() {
	
	unsigned int buffer[24];
	
	for(int i = 0; i < 24; i++){
		// Start I2C Transmission
		Wire.beginTransmission(BME280_I2C_ADDR);
		// Select data register
		Wire.write((136+i));
		// Stop I2C Transmission
		Wire.endTransmission();
		 
		// Request 1 byte of data
		Wire.requestFrom(BME280_I2C_ADDR, 1);
		 
		// Read 24 bytes of data
		if(Wire.available() == 1){
			buffer[i] = Wire.read();
		}
	}
	 
	// Convert the data
	// temp coefficients
	this->dig_T1 = (buffer[0] & 0xff) + ((buffer[1] & 0xff) * 256);
	this->dig_T2 = buffer[2] + (buffer[3] * 256);
	this->dig_T3 = buffer[4] + (buffer[5] * 256);
	 
	// pressure coefficients
	this->dig_P1 = (buffer[6] & 0xff) + ((buffer[7] & 0xff ) * 256);
	this->dig_P2 = buffer[8] + (buffer[9] * 256);
	this->dig_P3 = buffer[10] + (buffer[11] * 256);
	this->dig_P4 = buffer[12] + (buffer[13] * 256);
	this->dig_P5 = buffer[14] + (buffer[15] * 256);
	this->dig_P6 = buffer[16] + (buffer[17] * 256);
	this->dig_P7 = buffer[18] + (buffer[19] * 256);
	this->dig_P8 = buffer[20] + (buffer[21] * 256);
	this->dig_P9 = buffer[22] + (buffer[23] * 256);
	 
	// Start I2C Transmission
	Wire.beginTransmission(BME280_I2C_ADDR);
	// Select data register
	Wire.write(161);
	// Stop I2C Transmission
	Wire.endTransmission();
	 
	// Request 1 byte of data
	Wire.requestFrom(BME280_I2C_ADDR, 1);
	 
	// Read 1 byte of data
	if(Wire.available() == 1){
		this->dig_H1 = Wire.read();
	}
	 
	for(int i = 0; i < 7; i++) {
		// Start I2C Transmission
		Wire.beginTransmission(BME280_I2C_ADDR);
		// Select data register
		Wire.write((225+i));
		// Stop I2C Transmission
		Wire.endTransmission();
		 
		// Request 1 byte of data
		Wire.requestFrom(BME280_I2C_ADDR, 1);
		 
		// Read 7 bytes of data
		if(Wire.available() == 1) {
			buffer[i] = Wire.read();
		}
	}
	 
	// Convert the data
	// humidity coefficients
	int dig_H2 = buffer[0] + (buffer[1] * 256);
	unsigned int dig_H3 = buffer[2] & 0xFF ;
	int dig_H4 = (buffer[3] * 16) + (buffer[4] & 0xF);
	int dig_H5 = (buffer[4] / 16) + (buffer[5] * 16);
	int dig_H6 = buffer[6];
	 
	// Start I2C Transmission
	Wire.beginTransmission(BME280_I2C_ADDR);
	// Select control humidity register
	Wire.write(0xF2);
	// Humidity over sampling rate = 1
	Wire.write(0x01);
	// Stop I2C Transmission
	Wire.endTransmission();
	 
	// Start I2C Transmission
	Wire.beginTransmission(BME280_I2C_ADDR);
	// Select control measurement register
	Wire.write(0xF4);
	// Normal mode, temp and pressure over sampling rate = 1
	Wire.write(0x27);
	// Stop I2C Transmission
	Wire.endTransmission();
	 
	// Start I2C Transmission
	Wire.beginTransmission(BME280_I2C_ADDR);
	// Select config register
	Wire.write(0xF5);
	// Stand_by time = 1000ms
	Wire.write(0xA0);
	// Stop I2C Transmission
	Wire.endTransmission();
	 
	for(int i = 0; i < 8; i++) {
		// Start I2C Transmission
		Wire.beginTransmission(BME280_I2C_ADDR);
		// Select data register
		Wire.write((247+i));
		// Stop I2C Transmission
		Wire.endTransmission();
		 
		// Request 1 byte of data
		Wire.requestFrom(BME280_I2C_ADDR, 1);
		 
		// Read 8 bytes of data
		if(Wire.available() == 1) {
			data[i] = Wire.read();
		}
	}
	
}




/***********************************************************
 * Returns temperature in Deg C
 ***/
float Sensors::getTempValue() {
	
	Hardware _hardware;
	float refTempDegC;
	
	int supplyMillivolts = _hardware.getSupplyMillivolts();


	#ifdef TEMP_SENSOR_TYPE_LINEAR_ANALOG
	
		int rawTempValue = analogRead(TEMPERATURE_PIN);	
		int tempMillivolts = (rawTempValue * (supplyMillivolts / 4095.0)) * 1000;	
		tempMillivolts += TEMP_TRIMPOT;		
		refTempDegC = tempMillivolts * TEMP_ANALOG_SCALE;
		
	#elif defined TEMP_SENSOR_TYPE_BME280
		 
		 getBME280RawData();
		 
		 // Convert temperature data to 19-bits
		 long adc_t = (((long)(this->data[3] & 0xFF) * 65536) + ((long)(this->data[4] & 0xFF) * 256) + (long)(this->data[5] & 0xF0)) / 16;
		 
		 // Temperature offset calculations
		 double var1 = (((double)adc_t) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
		 double var2 = ((((double)adc_t) / 131072.0 - ((double)dig_T1) / 8192.0) * (((double)adc_t)/131072.0 - ((double)dig_T1)/8192.0)) * ((double)dig_T3);
		 double t_fine = (long)(var1 + var2);
		 double cTemp = (var1 + var2) / 5120.0;
		 //double fTemp = cTemp * 1.8 + 32;
		 
		 refTempDegC = cTemp;

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
		refTempDegC = DEFAULT_TEMP_VALUE;
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
	
	#ifdef BARO_SENSOR_TYPE_LINEAR_ANALOG
		
		int rawBaroValue = analogRead(REF_BARO_PIN);
		int baroMillivolts = (rawBaroValue * (supplyMillivolts / 4095.0)) * 1000;
		baroMillivolts += BARO_TRIMPOT;		
		baroPressureKpa = baroMillivolts * BARO_ANALOG_SCALE;
	
	#elif BARO_SENSOR_TYPE_MPX4115
		// Datasheet - https://html.alldatasheet.es/html-pdf/5178/MOTOROLA/MPX4115/258/1/MPX4115.html
		// Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
		// P = ((Vout / VS ) - 0.095) / 0.009 --- Formula transposed for P
		baroPressureKpa = (((float)baroMillivolts / (float)supplyMillivolts ) - 0.095) / 0.009; 

	#elif defined BARO_SENSOR_TYPE_BME280
		
		getBME280RawData();
		
		// Convert pressure and temperature data to 19-bits
		long adc_p = (((long)(data[0] & 0xFF) * 65536) + ((long)(data[1] & 0xFF) * 256) + (long)(data[2] & 0xF0)) / 16;
		long adc_t = (((long)(data[3] & 0xFF) * 65536) + ((long)(data[4] & 0xFF) * 256) + (long)(data[5] & 0xF0)) / 16; 
		
		// Temperature offset calculations
		double var1 = (((double)adc_t) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
		double var2 = ((((double)adc_t) / 131072.0 - ((double)dig_T1) / 8192.0) * (((double)adc_t)/131072.0 - ((double)dig_T1)/8192.0)) * ((double)dig_T3);
		double t_fine = (long)(var1 + var2);
		
		var1 = ((double)t_fine / 2.0) - 64000.0;
		var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
		var2 = var2 + var1 * ((double)dig_P5) * 2.0;
		var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
		var1 = (((double) dig_P3) * var1 * var1 / 524288.0 + ((double) dig_P2) * var1) / 524288.0;
		var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
		double p = 1048576.0 - (double)adc_p;
		p = (p - (var2 / 4096.0)) * 6250.0 / var1;
		var1 = ((double) dig_P9) * p * p / 2147483648.0;
		var2 = p * ((double) dig_P8) / 32768.0;
		double pressure = (p + (var1 + var2 + ((double)dig_P7)) / 16.0) / 1000;
		
		baroPressureKpa = pressure;

	#elif defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		// No baro sensor defined so use value grabbed at startup from reference pressure sensor
		// NOTE will only work for absolute style pressure sensor like the MPX4250
		baroPressureKpa = startupBaroPressure; 
	#else
		// we don't have any sensor so use default - // NOTE: standard sea level baro pressure is 14.7 psi
		baroPressureKpa = DEFAULT_BARO_VALUE;
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

	#elif defined RELH_SENSOR_TYPE_BME280

		getBME280RawData();

		// Convert the humidity data
		long adc_h = ((long)(this->data[6] & 0xFF) * 256 + (long)(this->data[7] & 0xFF));
		
		
		// Humidity offset calculations
		double var_H = (((double)t_fine) - 76800.0);
		var_H = (adc_h - (dig_H4 * 64.0 + dig_H5 / 16384.0 * var_H)) * (dig_H2 / 65536.0 * (1.0 + dig_H6 / 67108864.0 * var_H * (1.0 + dig_H3 / 67108864.0 * var_H)));
		double humidity = var_H * (1.0 - dig_H1 * var_H / 524288.0);
		if(humidity > 100.0){
			humidity = 100.0;
		} else if(humidity < 0.0) {
			humidity = 0.0;
		}

		relativeHumidity = humidity;

	#else
		//we don't have any sensor so use standard value 
		relativeHumidity = DEFAULT_RELH_VALUE; // (36%)
	#endif

	return relativeHumidity;
}



/***********************************************************
 * Returns Reference pressure in kPa
 ***/
float Sensors::getPRefValue() {
	
	Sensors::begin();
	
	Hardware _hardware;
	
	float refPressureKpa = 0.0;
	float supplyMillivolts = _hardware.getSupplyMillivolts();
	float rawRefPressValue;
	float refPressMillivolts;

	#if defined PREF_SRC_ADC_1015 || defined PREF_SRC_ADC_1015
		refPressMillivolts = _hardware.getAdcMillivolts(PREF_ADC_CHANNEL);
	#elif defined PREF_SRC_PIN	
		rawRefPressValue = analogRead(REF_PRESSURE_PIN);
		refPressMillivolts = (rawRefPressValue * (supplyMillivolts / 4095.0)) * 1000;
	#endif
	
	refPressMillivolts += PREF_TRIMPOT;

	#ifdef PREF_SENSOR_TYPE_LINEAR_ANALOG
		refPressureKpa = refPressMillivolts * PREF_ANALOG_SCALE;
	
	#elif defined PREF_SENSOR_TYPE_MPXV7007
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
			refPressureKpa = (((refPressMillivolts / supplyMillivolts ) - 0.04) / 0.00369) - DEFAULT_BARO_VALUE;  
		#elif defined BARO_SENSOR_TYPE_FIXED_VALUE            
			refPressureKpa = DEFAULT_BARO_VALUE;
		#else
			// use the current baro value to offset the sensor value
			refPressureKpa = (((refPressMillivolts / supplyMillivolts ) - 0.04) / 0.00369) - this->calculateBaroPressure(KPA);         
		#endif

	#elif defined PREF_SENSOR_NOT_USED 
	#else
		// No reference pressure sensor used so lets return a value (so as not to throw maths out)
		// refPressureKpa = 6.97448943333324; //28"
		refPressureKpa = DEFAULT_REF_PRESS_VALUE;

	#endif

	return refPressureKpa;
}





/***********************************************************
 * Returns Pressure differential in kPa
 ***/
float Sensors::getPDiffValue() {
	
	Hardware _hardware;

	float diffPressureKpa = 0.0;
	float diffPressMillivolts;
	int rawDiffPressValue;
	float supplyMillivolts = _hardware.getSupplyMillivolts();
	

	#if defined PDIFF_SRC_ADC_1015 || defined PDIFF_SRC_ADC_1015
		diffPressMillivolts = _hardware.getAdcMillivolts(PDIFF_ADC_CHANNEL);
	#elif defined PDIFF_SRC_PIN	
		rawDiffPressValue = analogRead(DIFF_PRESSURE_PIN);
		diffPressMillivolts = (rawDiffPressValue * (supplyMillivolts / 4095.0)) * 1000;
	#endif
	
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
	float supplyMillivolts = _hardware.getSupplyMillivolts();
	int pitotRaw;
	double pitotMillivolts;
	
	
	#if defined PITOT_SRC_ADC_1015 || defined PITOT_SRC_ADC_1015
		pitotMillivolts = _hardware.getAdcMillivolts(PITOT_ADC_CHANNEL);
	#elif defined PITOT_SRC_PIN	
		pitotRaw = analogRead(PITOT_PIN);
		pitotMillivolts = (pitotRaw * (supplyMillivolts / 4095.0)) * 1000;
	#endif
	
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


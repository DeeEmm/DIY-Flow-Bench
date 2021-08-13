/****************************************
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
 *
 * NOTE: Methods return sensor values in (SI) standard units. This allows any sensor to be integrated into the system
 * 
 ***/

#include "Arduino.h"
#include "configuration.h"
#include "constants.h"
#include "pins.h"
#include "structs.h"
#include "sensors.h"
#include LANGUAGE_FILE






//Sensors::Sensors(int tempSensorType, int baroSensorType, int relhSensorType, int prefSensorType, int pdiffSensorType, int pitotSensorTyp) {
Sensors::Sensors() {

	
	// Zero out the sensor values
	this->_mafval = 0.0;
	this->_baro = 0.0;
	this->_relh = 0.0;
	this->_temp = 0.0;
	this->_pref = 0.0;
	this->_pdiff = 0.0;
	this->_pitot = 0.0;
	

}

void Sensors::Begin () {
	// What hardware / sensors do we have? Lets initialise them and report back to calling function
	
	// Support for ADAFRUIT_BME280 temp, pressure & Humidity sensors
	// https://github.com/adafruit/Adafruit_BME280_Library
	#if defined TEMP_SENSOR_TYPE_ADAFRUIT_BME280 || defined BARO_SENSOR_TYPE_ADAFRUIT_BME280 || defined RELH_SENSOR_TYPE_ADAFRUIT_BME280 
		// TODO: Getting some error messages
		//'Sensors::initialise()::TwoWire Wire', declared using local type 'Sensors::initialise()::TwoWire', is used but never defined 
		// #include <Adafruit_BME280.h> 
		// Adafruit_BME280 adafruitBme280; // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)
		// //I2C address - BME280_I2C_ADDR
		// if (!adafruitBme280.begin()) {  
		// 	return BME280_READ_FAIL;			
		//   Serial.println("Adafruit BME280 Initialisation failed");      
		// } else {
		//   Serial.println("Adafruit BME280 Initialised");      
		// }
	#endif
		
		
	// Support for SPARKFUN_BME280 temp, pressure & Humidity sensors
	// https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide?_ga=2.39864294.574007306.1596270790-134320310.1596270790
	#if defined TEMP_SENSOR_TYPE_SPARKFUN_BME280 || defined BARO_SENSOR_TYPE_SPARKFUN_BME280 || defined RELH_SENSOR_TYPE_SPARKFUN_BME280 
		// TODO: There is an issue with this library. Was always a bit flakey but more so now it's inside this class.
		// #include "SparkFunBME280.h"
		// #include "Wire.h"
		// BME280 SparkFunBME280;
		// Wire.begin();
		// SparkFunBME280.setI2CAddress(BME280_I2C_ADDR); 
		// if (SparkFunBME280.beginI2C() == false) //Begin communication over I2C
		// {
		// 	return BME280_READ_FAIL;			
		//   Serial.println("Sparkfun BME280 Initialisation failed");      
		// } else {
		//   Serial.println("Sparkfun BME280 Initialised");      
		// }
	#endif


	// Support for DHT11 humidity / temperature sensors
	// https://github.com/winlinvip/SimpleDHT
	#if defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11 || defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11 
		#include <SimpleDHT.h>    		 
	#endif

}


void Sensors::Initialise () {

	extern struct DeviceStatus status;
	
	// MAF Sensor 
	// TODO: Do we need to initialise different MAF types???
	this->_mafSensorType = MAF_SENSOR_TYPE;
	this->_mafSensorType.remove(0, 8);

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
	#endif


	#ifdef PREF_SENSOR_NOT_USED
		this->_prefSensorType = LANG_NOT_ENABLED;
	#elif defined PREF_SENSOR_TYPE_MPXV7007
		this->_prefSensorType = "SMPXV7007";
	#elif defined PREF_SENSOR_TYPE_PX4250
		this->_prefSensorType = "PX4250";
	#endif


    #ifdef PITOT_SENSOR_NOT_USED
		this->_pitotSensorType = LANG_NOT_ENABLED;
	#elif defined PITOT_SENSOR_TYPE_MPXV7007
		this->_pitotSensorType = "SMPXV7007";
	#endif
  
	

	// Set status values for GUI
	status.mafSensor = this->_mafSensorType;
	status.baroSensor = this->_baroSensorType;
	status.tempSensor = this->_tempSensorType;
	status.relhSensor = this->_relhSensorType;
	status.prefSensor = this->_prefSensorType;
	status.pdiffSensor = this->_pdiffSensorType;
	status.pitotSensor = this->_pitotSensorType;

}




// Getters and setters - read sensor data and return it in SI Standard units

/****************************************
 * Returns RAW MAF Sensor value
 *
 * MAF decode is done in Maths.cpp
 ***/
float Sensors::MAF() {

	// Maf Voltage
	  int mafFlowRaw = analogRead(MAF_PIN);
	  double mafMillivolts = (mafFlowRaw * (5.0 / 1024.0)) * 1000;
	
	// TODO: #26 Add MAF frequency style sensor 

	return _mafval;
}


/****************************************
 * Returns temperature in Deg C
 ***/
float Sensors::Temp() {

	//float refAltRaw;
	//float refPressureRaw;
	//float refTempRaw;
	float refTempDegC;
	//float refTempDegF;
	//float refTempRankine;
	//float relativeHumidity;
	byte refTemp;
	//byte refRelh;

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

	return _temp;
}



/****************************************
 * Returns Barometric pressure in kPa
 ***/
float Sensors::Baro() {
	
	return _baro;
}



/****************************************
 * Returns Relative Humidity in %
 ***/
float Sensors::RelH() {

	return _relh;
}



/****************************************
 * Returns Reference pressure in kPa
 ***/
float Sensors::PRef(int _unit) {

	// Ref Pressure Voltage
	  int refPressRaw = analogRead(REF_PRESSURE_PIN);
	  double refMillivolts = (refPressRaw * (5.0 / 1024.0)) * 1000;
	 

	return _pref;
}



/****************************************
 * Returns Pressure differential in kPa
 ***/
float Sensors::PDiff() {

	return _pdiff;
}



/****************************************
 * Returns Pitot pressure differential in kPa
 ***/
float Sensors::Pitot() {

	// Pitot Voltage
	  int pitotRaw = analogRead(PITOT_PIN);
	  double pitotMillivolts = (pitotRaw * (5.0 / 1024.0)) * 1000;
	  

	return _pitot;
}

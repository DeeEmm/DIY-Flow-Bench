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
 ***/

#include "Arduino.h"
#include "sensors.h"
//#include "configuration.h"

Sensors::Sensors(int tempSensorType, int baroSensorType, int relhSensorType, int prefSensorType, int pdiffSensorType, int pitotSensorType) {

	// Lets cater for a variety of sensor types
	this->_tempSensorType = tempSensorType;
	this->_baroSensorType = baroSensorType;
	this->_relhSensorType = relhSensorType;
	this->_prefSensorType = prefSensorType;
	this->_pdiffSensorType = pdiffSensorType;
	this->_pitotSensorType = pitotSensorType;
	
	// Zero out the sensor values
	this->_baro=0.0;
    this->_relh=0.0;
    this->_temp=0.0;
    this->_pref=0.0;
    this->_pdiff=0.0;
    this->_pitot=0.0;
}

int Sensors::begin () {
	// What hardware / sensors do we have? Lets initialise them and report back to calling function
	
	// Support for ADAFRUIT_BME280 temp, pressure & Humidity sensors
	// https://github.com/adafruit/Adafruit_BME280_Library
	if (this->_tempSensorType == ADAFRUIT_BME280 || this->_baroSensorType == ADAFRUIT_BME280 || this->_relhSensorType == ADAFRUIT_BME280) {
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
	}
		
		
	// Support for SPARKFUN_BME280 temp, pressure & Humidity sensors
	// https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide?_ga=2.39864294.574007306.1596270790-134320310.1596270790
	if (this->_tempSensorType == SPARKFUN_BME280 || this->_baroSensorType == SPARKFUN_BME280 || this->_relhSensorType == SPARKFUN_BME280) {
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
	}


	// Support for DHT11 humidity / temperature sensors
	// https://github.com/winlinvip/SimpleDHT
	if (this->_tempSensorType == SIMPLE_DHT11 || this->_relhSensorType == SIMPLE_DHT11) {
		#include <SimpleDHT.h>    		 
	}

	return 0;

}

float Sensors::Temp() {

	return _temp;
}

float Sensors::Baro() {
	
	return _baro;
}

float Sensors::RelH() {

	return _relh;
}

float Sensors::Pref() {

	return _pref;
}

float Sensors::Pdiff() {

	return _pdiff;
}

float Sensors::Pitot() {

	return _pitot;
}

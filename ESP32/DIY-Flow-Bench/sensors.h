/****************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* sensors.h - Sensors class header file
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
#ifndef SENSORS_H_INCLUDED
#define SENSORS_H_INCLUDED

// Sensors used
#define ADAFRUIT_BME280 1
#define SPARKFUN_BME280 2
#define SIMPLE_DHT11 3

// Error handler codes
#define  NO_ERROR 0
#define  REF_PRESS_LOW 1
#define  LEAK_TEST_FAILED 2
#define  LEAK_TEST_PASS 3
#define  DHT11_READ_FAIL 4
#define  BME280_READ_FAIL 5
#define  BENCH_RUNNING 6



class Sensors {

public:
  Sensors(int tempSensorType, int baroSensorType, int relhSensorType, int prefSensorType, int pdiffSensorType, int pitotSensorType);
  int begin();
  float Temp();
	float Baro();
	float RelH();
	float Pref();
	float Pdiff();
	float Pitot();
  
private:
	float _baro;
	float _relh;
	float _temp;
	float _pref;
	float _pdiff;
	float _pitot;
  int _tempSensorType;
  int _baroSensorType;
  int _relhSensorType;
  int _prefSensorType;
  int _pdiffSensorType;
  int _pitotSensorType;    
  
};

#endif
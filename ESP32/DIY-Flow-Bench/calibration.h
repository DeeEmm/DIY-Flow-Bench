/****************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* Calibration.h - Calibration header file
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
#ifndef CALIBRATION_H_INCLUDED
#define CALIBRATION_H_INCLUDED

// Error handler codes
#define  NO_ERROR 0
#define  REF_PRESS_LOW 1
#define  LEAK_TEST_FAILED 2
#define  LEAK_TEST_PASS 3
#define  DHT11_READ_FAIL 4
#define  BME280_READ_FAIL 5
#define  BENCH_RUNNING 6



class Calibration {

	public:
		Calibration();
		bool setFlowOffset();
		float getFlowOffset();
		bool setLeakTestPressure();
		float getLeakTestPressure();
	
	
	private:



};

#endif
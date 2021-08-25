/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* Constants.h - constant definitions header file
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
* NOTE: Many constants are simply named integers which makes reading through the code flow much easier to understand
* as instead of seeing things like select (case) 1, 2, 3, etc the code reads select (case) SAVE_CONFIG, FILE_DELETE, etc.
* It should be noted that these constants are used without parenthesis (which would turn them into strings).
* When adding values to a collection of constants, be sure to use a unique number for that collection. Each collection is
* grouped by usage / type under its own heading. Each number is arbitrary, the actual value is unimportant.
*
*
***/
#pragma once


/***********************************************************
 * Websocket Header Schema
 ***/
#define GET_FLOW_DATA 1
#define REC_FLOW_DATA 2
#define CALIBRATE 3
#define FILE_LIST 4
#define SYS_STATUS 5
#define SAVE_CONFIG 6 
#define LOAD_CONFIG 7
#define FILE_DOWNLOAD 8
#define FILE_DELETE 9
#define FILE_UPLOAD 10
#define START_BENCH 11
#define STOP_BENCH 12
#define LEAK_CAL 13
#define GET_CAL 14 


/***********************************************************
 * Standard Units
 ***/
#define  RAW 0
#define  DECI 0
#define  INWG 1
#define  KPA 2
#define  PSIA 3
#define  DEGC 4
#define  DEGF 5
#define  RANKINE 6
#define  PERCENT 7
#define  BAR 8



/***********************************************************
 * Error handler codes
 ***/
#define  NO_ERROR 0
#define  REF_PRESS_LOW 1
#define  LEAK_TEST_FAILED 2
#define  LEAK_TEST_PASS 3
#define  DHT11_READ_FAIL 4
#define  BME280_READ_FAIL 5
#define  BENCH_RUNNING 6


/***********************************************************
 * MAF Data File constants
 ***/

#define  VOLTAGE 1
#define  FREQUENCY 2
#define  KEY_VALUE 1
#define  RAW_ANALOG 2
#define  KG_H 1
#define  MG_S 2


/***********************************************************
 * Sensor types
 ***/
#define NOT_USED 1
#define REF_PRESS_AS_BARO 2
#define FIXED_VALUE 3
#define MPXV7007 4
#define MPX4250 5
#define MPXV7007DP 6
#define ADAFRUIT_BME280 7
#define SPARKFUN_BME280 8
#define MPX4115 9
#define SIMPLE_TEMP_DHT11 10


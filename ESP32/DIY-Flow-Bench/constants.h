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
 * General
 ***/
#define ARDUINOJSON_ENABLE_STD_STRING 1 // allow support for std::string


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
#define  HPA 9



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
#define _BME280 7
#define SPARKFUN_BME280 8
#define MPX4115 9
#define SIMPLE_TEMP_DHT11 10



/***********************************************************
 * BME Registers
 ***/
#define BME280_REG_DIG_T1    0x88
#define BME280_REG_DIG_T2    0x8A
#define BME280_REG_DIG_T3    0x8C

#define BME280_REG_DIG_P1    0x8E
#define BME280_REG_DIG_P2    0x90
#define BME280_REG_DIG_P3    0x92
#define BME280_REG_DIG_P4    0x94
#define BME280_REG_DIG_P5    0x96
#define BME280_REG_DIG_P6    0x98
#define BME280_REG_DIG_P7    0x9A
#define BME280_REG_DIG_P8    0x9C
#define BME280_REG_DIG_P9    0x9E

#define BME280_REG_DIG_H1    0xA1
#define BME280_REG_DIG_H2    0xE1
#define BME280_REG_DIG_H3    0xE3
#define BME280_REG_DIG_H4    0xE4
#define BME280_REG_DIG_H5    0xE5
#define BME280_REG_DIG_H6    0xE7

#define BME280_REG_CHIPID          0xD0
#define BME280_REG_VERSION         0xD1
#define BME280_REG_SOFTRESET       0xE0

#define BME280_REG_CAL26           0xE1

#define BME280_REG_CTRLHUMID        0xF2
#define BME280_REG_CTRL             0xF4
#define BME280_REG_CONFIG          0xF5
#define BME280_REG_PRESSUREDATA    0xF7
#define BME280_REG_TEMPDATA        0xFA
#define BME280_REG_HUMIDITYDATA    0xFD

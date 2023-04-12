/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file constants.h
 * 
 * @brief Define constants
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/
#pragma once

#include <stdint.h>


/***********************************************************
 * General
 ***/
#define ARDUINOJSON_ENABLE_STD_STRING 1 // allow support for std::string


/***********************************************************
 * Physical constants
 ***/
#define MOLAR_MASS_DRY_AIR 28.96469 // g/mole or 0.028964 kg/mol.
#define MOLAR_MASS_WATER_VAPOUR 18.01528 //g/mol or 0.0180153 kg/mol
#define SPECIFIC_GAS_CONSTANT_DRY_AIR 287.058  // J/ (kg.K).
#define SPECIFIC_GRAVITY_DRY_AIR 0.0013 // SG
#define UNIVERSAL_GAS_CONSTANT 8.3144598 // (48) J⋅mol^−1⋅K^−1.
#define SPECIFIC_GAS_CONSTANT_WATER_VAPOUR 461.5 //  J/(kg⋅K).


/***********************************************************
 * Standard Units
 ***/
#define  RAW 0
#define  DECI 0
#define  INH2O 1
#define  KPA 2
#define  PSIA 3
#define  DEGC 4
#define  DEGF 5
#define  RANKINE 6
#define  PERCENT 7
#define  BAR 8
#define  HPA 9
#define  KELVIN 10
#define  ATM 11
#define  PASCALS 12


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
#define  GM_M 3


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




/***********************************************************
 * Favicon
 ***/
#define favicon_ico_gz_len 1956
const uint8_t favicon_ico_gz[] = {
0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xED, 0x5A, 0x09, 0x6C, 0x15, 0x55, 
0x14, 0x1D, 0x16, 0x15, 0x11, 0xA5, 0x20, 0xA8, 0x88, 0x4A, 0x11, 0xC5, 0xAA, 0x89, 0xA2, 0x06, 
0x5C, 0x22, 0x8A, 0x7B, 0x8C, 0x4B, 0x34, 0x8A, 0x8A, 0xC1, 0x44, 0xE3, 0x12, 0xA2, 0x44, 0xC4, 
0xA8, 0xC4, 0x88, 0x04, 0xA3, 0x84, 0x16, 0x5A, 0x0A, 0xA5, 0x14, 0x4A, 0xCB, 0x52, 0xA0, 0xA5, 
0x6C, 0x42, 0x41, 0xA0, 0xEC, 0x7B, 0xA1, 0xAC, 0x15, 0xA8, 0xEC, 0x65, 0x11, 0x4A, 0xA1, 0x40, 
0x5B, 0x4A, 0x77, 0xDA, 0xE3, 0x3B, 0xFF, 0xFE, 0xD7, 0x99, 0xF6, 0xCF, 0x6F, 0x4B, 0x4B, 0xF8, 
0xFD, 0x71, 0x26, 0x39, 0xF9, 0x7F, 0xDE, 0xBC, 0x79, 0xF7, 0xBE, 0xFB, 0xEE, 0xBB, 0xEF, 0xCE, 
0xBD, 0xD7, 0x30, 0x9A, 0x18, 0xCD, 0x8C, 0x80, 0x00, 0x43, 0xFD, 0x06, 0x1A, 0xFD, 0x9A, 0x1B, 
0x46, 0x0F, 0xC3, 0x30, 0x02, 0x03, 0xE5, 0x3E, 0xA8, 0x8D, 0x61, 0x4C, 0x56, 0x6D, 0xDD, 0xBA, 
0xB9, 0x9F, 0x77, 0x35, 0x8C, 0x94, 0xF6, 0x86, 0x11, 0xA4, 0xFA, 0x04, 0xB0, 0x9F, 0x21, 0xED, 
0xDE, 0x2E, 0x04, 0x1B, 0x43, 0x14, 0x4A, 0x14, 0x70, 0x85, 0x28, 0x69, 0xC0, 0xBB, 0xD6, 0x31, 
0xEA, 0xFB, 0x6E, 0xE3, 0x40, 0x68, 0x0B, 0x60, 0xCC, 0xAD, 0x40, 0x48, 0x33, 0xB9, 0x0F, 0xBF, 
0x05, 0x08, 0xBB, 0x09, 0x18, 0x79, 0x83, 0xB4, 0x87, 0xB6, 0x34, 0xFB, 0xF0, 0x57, 0xB7, 0xB3, 
0x0F, 0xFB, 0xCF, 0x7C, 0x11, 0x38, 0xBC, 0x04, 0x48, 0x8B, 0x01, 0xA6, 0x3F, 0x03, 0x6C, 0x0D, 
0x07, 0x16, 0x7C, 0x00, 0x44, 0xDF, 0x07, 0x9C, 0x58, 0x0F, 0x4C, 0x7E, 0x54, 0xF5, 0x79, 0x09, 
0x38, 0x94, 0x04, 0x4C, 0x7C, 0x00, 0x98, 0xF2, 0x18, 0x70, 0x7C, 0x0D, 0xB0, 0x23, 0x52, 0xF5, 
0xEB, 0x0D, 0x44, 0x75, 0x02, 0xD6, 0xFD, 0x02, 0x8C, 0xB8, 0x0E, 0xD8, 0x13, 0x07, 0x2C, 0xFF, 
0x06, 0x98, 0xF4, 0x88, 0x8C, 0x7D, 0x6C, 0xB5, 0xA2, 0x77, 0x3D, 0x30, 0xBE, 0x33, 0xB0, 0x6B, 
0x02, 0x30, 0xB6, 0x83, 0xE2, 0x2F, 0x00, 0xF8, 0x67, 0x16, 0xB0, 0xE2, 0x5B, 0x60, 0x51, 0x5F, 
0xF3, 0x7D, 0xF6, 0xDB, 0x33, 0xCD, 0x7C, 0x7F, 0xDE, 0x3B, 0x8A, 0x97, 0x51, 0xC0, 0xD2, 0x2F, 
0xE5, 0xFD, 0xDD, 0x53, 0xE4, 0x97, 0xBC, 0xEF, 0x9B, 0x0D, 0x6C, 0x0E, 0x56, 0xBC, 0x3C, 0x2E, 
0xBC, 0x65, 0x2C, 0x03, 0xFE, 0x9E, 0x04, 0xC4, 0xF5, 0x00, 0xB6, 0x47, 0x00, 0x4B, 0x3E, 0x07, 
0xB2, 0x76, 0xAA, 0xBE, 0xED, 0x80, 0xA3, 0x2B, 0x80, 0xA4, 0x3E, 0xC0, 0x91, 0xA5, 0x32, 0xC6, 
0xC2, 0x8F, 0xD5, 0xBC, 0xD6, 0x01, 0xE9, 0x33, 0x85, 0x0E, 0x65, 0x12, 0xDE, 0xDA, 0x94, 0xE7, 
0xA8, 0x9B, 0x45, 0x36, 0xA3, 0xDB, 0x88, 0x4C, 0xF9, 0xAB, 0xEF, 0x89, 0xD0, 0x1B, 0xE5, 0x97, 
0xFC, 0xFA, 0x7A, 0xED, 0xAF, 0x0E, 0x1A, 0xBA, 0x7F, 0x1A, 0xB4, 0x7F, 0xBD, 0x1A, 0x86, 0x3A, 
0x5E, 0x41, 0x6E, 0x1B, 0xD3, 0xAB, 0x36, 0x3B, 0x33, 0xD4, 0x68, 0xAA, 0xE8, 0x0D, 0x56, 0xC8, 
0xBC, 0x86, 0xB2, 0xCD, 0x74, 0xD1, 0x34, 0x69, 0xFB, 0x6A, 0x8D, 0xAF, 0xF5, 0xBC, 0xED, 0xE4, 
0xE0, 0x2B, 0xDA, 0x0E, 0x1C, 0x38, 0x68, 0x0C, 0x98, 0x70, 0xAF, 0xF8, 0x10, 0xF4, 0x2D, 0xB4, 
0xFF, 0xA1, 0xC1, 0x73, 0x9D, 0xCF, 0x08, 0x7D, 0xB6, 0x46, 0xDD, 0x63, 0xB6, 0x11, 0xD5, 0xC7, 
0x21, 0xE8, 0x13, 0xE8, 0xF6, 0x71, 0x77, 0x9B, 0xED, 0xA4, 0x41, 0x5F, 0xC7, 0x4A, 0x23, 0x33, 
0x15, 0xA8, 0xA8, 0x00, 0xF2, 0x4E, 0x00, 0x65, 0x45, 0xC0, 0x81, 0x79, 0xCA, 0x27, 0xB9, 0x43, 
0x9E, 0x2D, 0xEF, 0x0F, 0x5C, 0x2E, 0x01, 0x72, 0x8E, 0x88, 0x4F, 0xC2, 0xB6, 0x6D, 0x63, 0x80, 
0xFC, 0x4C, 0xA0, 0xE4, 0xA2, 0xF2, 0x91, 0xFE, 0x52, 0xE3, 0xB5, 0x90, 0xF6, 0xD3, 0xDB, 0x64, 
0x9C, 0xEC, 0xBD, 0xCA, 0xEF, 0x88, 0x35, 0xC7, 0x4F, 0x4F, 0x80, 0xEB, 0xBA, 0x74, 0x1A, 0x28, 
0x38, 0x23, 0xEF, 0x2D, 0xFE, 0xCC, 0x7C, 0x1E, 0xFF, 0x9C, 0xD0, 0x75, 0xCD, 0x57, 0xF9, 0x26, 
0x87, 0x17, 0x2B, 0x3F, 0x65, 0x87, 0xE9, 0xBB, 0x9D, 0xDB, 0xA7, 0x7C, 0xA2, 0x4F, 0xAA, 0xF2, 
0xBC, 0xFE, 0x57, 0xA1, 0x6D, 0x6D, 0x4B, 0x50, 0x3E, 0x5B, 0xE9, 0x25, 0x4F, 0xF9, 0x8E, 0x0F, 
0x14, 0xFA, 0xF4, 0xB3, 0x78, 0xBF, 0xE6, 0x27, 0xE1, 0x21, 0xA4, 0xA9, 0x27, 0x7D, 0x22, 0xA2, 
0xBD, 0x9A, 0x73, 0x29, 0x90, 0xF8, 0x72, 0x0D, 0xF4, 0x87, 0x88, 0xEF, 0x54, 0x1F, 0xFA, 0x73, 
0xDE, 0x50, 0xE3, 0x17, 0x9B, 0xF3, 0xAB, 0x4E, 0x9F, 0x38, 0xBB, 0x07, 0x58, 0xF5, 0xBD, 0x3D, 
0x7D, 0xF2, 0x7D, 0x26, 0x4D, 0xD1, 0x2A, 0x30, 0xD7, 0x49, 0xD3, 0xE7, 0x5A, 0xAD, 0xFE, 0x51, 
0x7C, 0xBD, 0xEA, 0xF4, 0x53, 0x43, 0x15, 0x46, 0xCA, 0x3A, 0xAF, 0xFE, 0xC1, 0x5E, 0xFE, 0x95, 
0xF4, 0x77, 0xCB, 0x38, 0x76, 0xF4, 0xE9, 0x9F, 0x96, 0xE4, 0x2B, 0x7F, 0x35, 0x51, 0x7C, 0x51, 
0x2B, 0xFD, 0xF2, 0x52, 0xF1, 0x95, 0xD7, 0xFE, 0xEC, 0x49, 0x7F, 0xFB, 0x58, 0xF1, 0x47, 0x73, 
0x8F, 0x01, 0xB1, 0x0F, 0x79, 0xA7, 0x1F, 0x79, 0xA7, 0x8C, 0x33, 0xEB, 0x35, 0x4F, 0xFA, 0xD4, 
0xDD, 0xDC, 0x0C, 0x60, 0xE3, 0x50, 0xA5, 0xD7, 0x77, 0x01, 0x45, 0x39, 0xA6, 0x0C, 0xAA, 0xCB, 
0x3F, 0xAC, 0x95, 0xF2, 0x5B, 0x5B, 0x79, 0xCA, 0x3F, 0x3D, 0x5E, 0xF9, 0xBC, 0x53, 0xED, 0xE9, 
0xC7, 0x28, 0xBE, 0x4E, 0xA6, 0x28, 0x6C, 0xB2, 0xD7, 0xBF, 0x95, 0x03, 0x94, 0x1E, 0x9F, 0x11, 
0x7F, 0x98, 0xF7, 0xBB, 0xA2, 0x4D, 0x19, 0x54, 0xA7, 0x4F, 0xDF, 0x9E, 0xDF, 0x02, 0x56, 0xFA, 
0xE4, 0xE9, 0xD4, 0x16, 0x79, 0x4F, 0xF7, 0xCB, 0x52, 0x6B, 0x09, 0xB5, 0x6F, 0x8A, 0x73, 0x05, 
0x3B, 0xA3, 0x4C, 0xDF, 0x9B, 0x72, 0xE4, 0x9A, 0xE6, 0x9F, 0x52, 0x7A, 0xF3, 0xA6, 0x1A, 0xBF, 
0x50, 0xED, 0xA1, 0xB3, 0x42, 0xCB, 0xB5, 0xE7, 0xB6, 0x8B, 0xAE, 0xF2, 0xBB, 0x80, 0xFB, 0x8E, 
0xFB, 0x8F, 0x7B, 0x8C, 0x28, 0x2F, 0x93, 0x6F, 0x97, 0x23, 0xC9, 0x42, 0xBF, 0xF0, 0x9C, 0x8C, 
0xC5, 0x6F, 0x00, 0xDA, 0x90, 0xCA, 0xF5, 0xE9, 0x2C, 0xB6, 0x83, 0xF2, 0xD4, 0x7B, 0xA2, 0xD2, 
0xFE, 0xB4, 0x93, 0x67, 0x04, 0x6D, 0x93, 0xF5, 0xBF, 0xCB, 0xE6, 0xB8, 0xDF, 0xA5, 0x3C, 0xF4, 
0x38, 0x56, 0xD0, 0x66, 0x91, 0x96, 0xBE, 0xE7, 0xDA, 0xFA, 0xCA, 0xCE, 0x3A, 0x70, 0xE0, 0xA0, 
0xB1, 0xC1, 0xD7, 0xDF, 0x5F, 0xBE, 0xFD, 0xFE, 0xF4, 0xF1, 0xF7, 0x77, 0x43, 0xE3, 0x0C, 0xFE, 
0x7E, 0x31, 0x4E, 0xD2, 0x4D, 0xE1, 0x53, 0xA3, 0xE6, 0x38, 0x09, 0x2F, 0x25, 0xB3, 0xD6, 0x18, 
0x66, 0xF4, 0xC5, 0x70, 0x23, 0x45, 0xFD, 0x2F, 0x6C, 0x04, 0x7B, 0xC7, 0x1B, 0x0A, 0x5D, 0x3C, 
0x92, 0x57, 0xC5, 0x73, 0x25, 0xEF, 0xC3, 0x8D, 0x71, 0xEA, 0xB7, 0xB8, 0x11, 0xF0, 0x57, 0x57, 
0x14, 0xBB, 0x79, 0x6E, 0xAD, 0xD0, 0xD7, 0xCF, 0x78, 0x37, 0xE7, 0x10, 0x5C, 0xA9, 0x33, 0xBE, 
0xE6, 0xA5, 0x7E, 0x68, 0xFC, 0xFA, 0x5E, 0x1B, 0xFC, 0x99, 0x77, 0x07, 0x0E, 0x1C, 0x38, 0x70, 
0xE0, 0xC0, 0x81, 0x83, 0xFA, 0x83, 0x71, 0xD2, 0xF8, 0xE7, 0xA5, 0xB6, 0x81, 0x98, 0xFE, 0xB4, 
0xC4, 0xFC, 0x82, 0x9B, 0x78, 0xF6, 0x65, 0x7C, 0x2F, 0xE1, 0x05, 0xB3, 0x6F, 0xEC, 0xC3, 0x66, 
0x3C, 0x94, 0x79, 0xFE, 0x19, 0x3D, 0xCD, 0x67, 0x1A, 0x8C, 0x73, 0xEA, 0xF7, 0x23, 0x3B, 0x54, 
0xA5, 0x45, 0x30, 0xCE, 0x1A, 0x71, 0x9B, 0xD9, 0x87, 0x31, 0xCE, 0x19, 0xCF, 0x56, 0xED, 0x93, 
0xD0, 0x0B, 0x88, 0xEE, 0x0A, 0x8C, 0x68, 0xEE, 0xC9, 0x13, 0x73, 0x06, 0xC5, 0x39, 0x12, 0xCB, 
0x64, 0xCC, 0xB3, 0xAC, 0x40, 0x62, 0x9C, 0x59, 0xBB, 0x80, 0xA4, 0x8F, 0x80, 0x51, 0xB7, 0x98, 
0x7D, 0xE3, 0x9E, 0x04, 0x4A, 0xF3, 0x25, 0xAE, 0xCA, 0x98, 0x27, 0xF3, 0x11, 0x3A, 0x66, 0x3B, 
0xFB, 0x75, 0xE0, 0xFC, 0x7E, 0xA0, 0x24, 0xCF, 0x3D, 0x56, 0xB1, 0xC4, 0x4B, 0x59, 0x6F, 0xA1, 
0xE7, 0xB8, 0xF4, 0x0B, 0x89, 0xC5, 0xBA, 0x9E, 0xAB, 0xF7, 0x8B, 0x2E, 0x48, 0x0E, 0x64, 0xFE, 
0x7B, 0x26, 0x0D, 0xC6, 0x60, 0xF3, 0x8E, 0x4B, 0x1F, 0xF2, 0x51, 0x56, 0x28, 0xA8, 0xB8, 0x0C, 
0x5C, 0x38, 0x0C, 0xCC, 0x7B, 0xB7, 0x6A, 0x3D, 0x04, 0xF3, 0x27, 0x8C, 0x09, 0x57, 0x94, 0x03, 
0xFB, 0xE6, 0x48, 0x8C, 0x7E, 0xE3, 0x6F, 0x40, 0xE6, 0x56, 0xA1, 0xC1, 0x98, 0x30, 0xE3, 0xAD, 
0xEC, 0xCB, 0xBC, 0xCF, 0xE6, 0x10, 0x19, 0xEF, 0xE8, 0x72, 0xA9, 0xC9, 0xD0, 0xBC, 0x31, 0xA6, 
0xCD, 0x1A, 0x0F, 0xE6, 0x66, 0x38, 0xD6, 0xFE, 0xB9, 0xB2, 0x1E, 0x11, 0xB7, 0x9B, 0x6B, 0xC9, 
0xB5, 0x48, 0x8B, 0x95, 0xE7, 0x8C, 0xD3, 0xCF, 0x7D, 0x5B, 0xEA, 0x40, 0x46, 0x07, 0x98, 0xFC, 
0x90, 0xD6, 0xCA, 0xEF, 0x84, 0x7F, 0xD6, 0x7E, 0x2C, 0xFB, 0x1A, 0x48, 0xFE, 0x0A, 0xD8, 0x12, 
0x22, 0xB1, 0x62, 0xC6, 0x99, 0x13, 0x5F, 0xAD, 0xBA, 0x06, 0xC9, 0xFD, 0x64, 0x7E, 0x6B, 0x06, 
0xC9, 0x3D, 0x79, 0x9A, 0xD0, 0x05, 0x38, 0xB5, 0x59, 0x62, 0xE8, 0xE4, 0x49, 0xC7, 0x90, 0x17, 
0x7C, 0x28, 0x79, 0x96, 0xB4, 0x58, 0xCF, 0xB5, 0x24, 0x18, 0x6B, 0x2F, 0x57, 0x63, 0x6D, 0xFA, 
0xDD, 0xFE, 0x39, 0x6B, 0x61, 0x28, 0x57, 0x9D, 0x87, 0xB0, 0x43, 0x5C, 0x77, 0xE1, 0x9F, 0x72, 
0x0D, 0x75, 0xE7, 0xA1, 0x28, 0x73, 0xC6, 0xE4, 0x39, 0xF7, 0x0D, 0x43, 0xAA, 0xEA, 0x77, 0x75, 
0xFE, 0x35, 0xA6, 0x3D, 0x25, 0xED, 0x19, 0xC9, 0x22, 0xFB, 0xBA, 0xF0, 0xCF, 0x1C, 0x06, 0x69, 
0x6C, 0xFA, 0xE3, 0xEA, 0xF2, 0xEF, 0x7A, 0xB7, 0xBF, 0xE8, 0x2E, 0x65, 0x63, 0x8D, 0xC9, 0x7B, 
0xE3, 0x9F, 0x73, 0x64, 0x9E, 0x82, 0xFA, 0xA8, 0x73, 0x7F, 0xB5, 0xF1, 0x3F, 0xFF, 0x7D, 0xC9, 
0x17, 0xB0, 0xCE, 0xC9, 0xCE, 0x06, 0x5C, 0x11, 0xFF, 0xD1, 0x52, 0x03, 0x44, 0xD9, 0xB3, 0x16, 
0x8A, 0x35, 0x52, 0xDC, 0x3F, 0xD6, 0xFD, 0x52, 0x23, 0xFF, 0x0A, 0x07, 0xE7, 0x4B, 0x4E, 0x6B, 
0x62, 0x50, 0xED, 0xFC, 0x93, 0x4E, 0x76, 0xBA, 0x69, 0x0B, 0x68, 0xAB, 0xEC, 0xF8, 0xA7, 0x0C, 
0x73, 0x8F, 0x02, 0xFF, 0x6E, 0x90, 0x7A, 0x28, 0xD6, 0x45, 0xD9, 0xF1, 0x5F, 0x74, 0x5E, 0xF2, 
0x27, 0xE7, 0xD2, 0x25, 0xD7, 0x42, 0xDE, 0x57, 0x0C, 0x90, 0x39, 0xD5, 0x95, 0xFF, 0x03, 0x7F, 
0xCA, 0x1E, 0x88, 0xA9, 0x85, 0xFF, 0x90, 0x26, 0x92, 0xDB, 0xA3, 0xEC, 0x99, 0x7B, 0xE5, 0x3E, 
0x3B, 0xB6, 0xCA, 0x93, 0x96, 0xE6, 0xBF, 0x20, 0x5B, 0xD9, 0xAB, 0x03, 0x92, 0xB7, 0x61, 0xEE, 
0xD0, 0x96, 0xFF, 0x1C, 0xB1, 0x69, 0x17, 0x0E, 0x0A, 0xEF, 0x87, 0x16, 0x2A, 0xDB, 0xDE, 0xD1, 
0x93, 0x47, 0x6F, 0xFC, 0x53, 0xC7, 0xA8, 0x3F, 0x9C, 0x3B, 0xF3, 0x34, 0x35, 0xF1, 0x4F, 0xFB, 
0x4C, 0x7E, 0x0A, 0xB3, 0x25, 0x47, 0x97, 0x3A, 0x42, 0xEC, 0x23, 0x6D, 0x8C, 0x87, 0xFE, 0x5C, 
0x96, 0x5C, 0x2D, 0x6D, 0x3D, 0xC1, 0x7A, 0x38, 0x3B, 0xFE, 0x49, 0x83, 0x35, 0x70, 0xA3, 0xDB, 
0x8A, 0xDC, 0x69, 0x9B, 0x59, 0x5B, 0x17, 0xD6, 0xB2, 0x6E, 0xFC, 0x27, 0xBE, 0x22, 0xED, 0xB4, 
0x95, 0xDA, 0x86, 0xDA, 0xF1, 0xCF, 0x73, 0x65, 0xFD, 0x60, 0xC9, 0x2F, 0xD2, 0x2E, 0x52, 0x8F, 
0x22, 0xDA, 0x89, 0xBD, 0xDB, 0x3B, 0xDD, 0x3C, 0x23, 0x6A, 0xD2, 0x7F, 0x57, 0x9E, 0xAB, 0x8B, 
0xF7, 0xFD, 0x1B, 0xD9, 0x51, 0xCE, 0x0A, 0xE6, 0x59, 0xB5, 0x2E, 0x78, 0xE3, 0x9F, 0x6B, 0xCE, 
0x31, 0x28, 0x4F, 0xE6, 0x0E, 0xF9, 0xBC, 0x26, 0xFB, 0xC9, 0xB5, 0xA1, 0xDE, 0xB3, 0x3D, 0xBA, 
0x8B, 0xD9, 0xCE, 0xB3, 0x8B, 0x3A, 0x3C, 0xB5, 0x7B, 0xCD, 0xFC, 0x73, 0x7E, 0xCC, 0x1D, 0x33, 
0xC7, 0x68, 0xC7, 0x3F, 0x75, 0x93, 0x6D, 0xD4, 0x03, 0xE6, 0x69, 0xA3, 0xEF, 0xB7, 0xCC, 0x3B, 
0x10, 0xD8, 0x11, 0x25, 0x36, 0xEF, 0x60, 0x92, 0xE4, 0x90, 0xB9, 0xEF, 0x73, 0x32, 0x24, 0x1F, 
0x9C, 0x32, 0xCC, 0xCC, 0x5B, 0xD2, 0xD7, 0x48, 0x0D, 0x93, 0xF3, 0x8B, 0x75, 0x91, 0x53, 0x9F, 
0x90, 0xB1, 0x48, 0x97, 0x7A, 0x4F, 0x3B, 0xAB, 0xF5, 0xCC, 0x65, 0x8B, 0x7A, 0x8B, 0x0E, 0x1D, 
0x5A, 0x24, 0x7D, 0xF9, 0x8C, 0xB5, 0x97, 0xA4, 0xC5, 0x7D, 0xC5, 0xB3, 0x89, 0xE0, 0xD9, 0x44, 
0xFD, 0x5E, 0x3B, 0x48, 0x72, 0x89, 0x2B, 0x07, 0x0A, 0xFF, 0xDC, 0xDF, 0xCC, 0xCD, 0x32, 0x5F, 
0xCA, 0xB3, 0x8C, 0x7A, 0xC7, 0xF7, 0xAD, 0xE7, 0x2F, 0xE5, 0x41, 0xB9, 0xE9, 0xF3, 0x9A, 0xA0, 
0xBD, 0xE1, 0xF9, 0x19, 0xDF, 0xB3, 0xEA, 0xFE, 0x9B, 0xFB, 0x96, 0x3C, 0xE3, 0xFE, 0xA3, 0xAE, 
0x6C, 0x0B, 0x97, 0x3D, 0x4B, 0x5B, 0xC3, 0x8B, 0xBE, 0x05, 0xCF, 0x67, 0xDD, 0xFF, 0xE2, 0x49, 
0x19, 0x8F, 0x6B, 0xCB, 0xF5, 0x5A, 0x35, 0x50, 0xD6, 0x53, 0xD3, 0xA2, 0x8F, 0x41, 0x94, 0xBB, 
0xDF, 0x5F, 0xD8, 0x47, 0x7C, 0x96, 0x4B, 0x59, 0x72, 0xCF, 0xF7, 0x5C, 0xCF, 0xD5, 0x6F, 0x71, 
0x9E, 0xE4, 0xE9, 0xE9, 0x67, 0x59, 0x75, 0x87, 0xF3, 0xE5, 0x39, 0x45, 0xBF, 0x6D, 0x9A, 0xF2, 
0x6F, 0x62, 0x1E, 0xF4, 0xAC, 0xE5, 0xD0, 0x18, 0xD3, 0x56, 0xFA, 0x69, 0x8C, 0xEF, 0x24, 0x7A, 
0x69, 0x6D, 0xB3, 0xE6, 0x80, 0x39, 0x9E, 0x6E, 0xE7, 0x3A, 0xB1, 0xAF, 0xA6, 0x65, 0x07, 0xFA, 
0x11, 0xAC, 0x93, 0xA0, 0x4F, 0x61, 0x6D, 0xE7, 0xDA, 0x59, 0xEB, 0x46, 0x1D, 0x38, 0x70, 0xE0, 
0xC0, 0x81, 0x03, 0x07, 0x0E, 0xFE, 0xBF, 0xF0, 0xE7, 0x1C, 0x64, 0xA1, 0xDF, 0xE7, 0xAF, 0x87, 
0xF9, 0x71, 0xFD, 0x80, 0xBB, 0x8E, 0xC3, 0x9F, 0xEB, 0x37, 0x2A, 0x6B, 0x50, 0x82, 0xFD, 0xA8, 
0x7E, 0x26, 0xD8, 0xAC, 0x9F, 0x71, 0x2E, 0xE7, 0x6A, 0xC8, 0xF5, 0x1F, 0x8C, 0xF1, 0x18, 0xA6, 
0x2E, 0x3C, 0x00, 0x00
};
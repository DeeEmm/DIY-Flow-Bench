/***********************************************************
 * DELPHI_AF10118.h
 *
 * Manufacturer: DELPHI
 * Part#: AF10118
 * MAF file for type: 
 * File units = 1000 * kg/hr
 * Comments: Example Data
 * Status: Unvalidated
 * Support: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/MAF-Data-Files
 * Discussion: https://github.com/DeeEmm/DIY-Flow-Bench/discussions/51
 ***/
#ifndef MAFDATACLASS
#define MAFDATACLASS

#include <Arduino.h>
#include "../constants.h"
#include "mafData/maf.h"



/***********************************************************
 * @brief MAF Class Constructor
 * @note: See exampleMafData.h for explaination and example
 * 
 ***/
Maf::Maf() {
}



/***********************************************************
 * @brief MAF Type
 * 
 * @note Description of MAF type
 *
 ***/
String Maf::sensorType() {
    return "DELPHI_AF10118";
}

 
 
/***********************************************************
 * @brief MAF Output Type
 *
 * @note Valid options:
 * VOLTAGE
 * FREQUENCY
 ***/
int Maf::outputType() {
    return VOLTAGE;
}



/***********************************************************
 * @brief MAF Units
 * 
 * @note: Valid options:
 * KG_H
 * MG_S
 ***/
int Maf::mafUnits() {
    return KG_H;
}



/***********************************************************
 * @brief mafLookupTable
 * @note Global vector of vectors containing MAF>Flow key>value pairs
 ***/
 std::vector<std::vector<int>> mafLookupTable = {{ 
{0,0},
{600,5620},
{800,8982},
{940,10711},
{1000,12440},
{1300,19597},
{1400,22382},
{1500,25456},
{1600,29251},
{1700,33526},
{1800,38137},
{1900,43036},
{2000,48175},
{2100,53987},
{2200,59222},
{2400,74112},
{2600,91259},
{2800,111144},
{3000,132950},
{3200,158262},
{3400,187177},
{3600,217724},
{3800,258503},
{4000,295631},
{4250,342461},
{4500,421040},
{4750,490925},
{5000,490925}
};

//#endif
#endif
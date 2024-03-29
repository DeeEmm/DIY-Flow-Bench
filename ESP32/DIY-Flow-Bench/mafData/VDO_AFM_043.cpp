/***********************************************************
 * VDO-AFM-043.cpp
 *
 * Manufacturer: VDO | Siemens
 * Part#: AFM-043 | 25008302 | A2C59506198
 * MAF file for type: GM GenIII LS1 / VT-VX-VY 5.7 Commodore / Monaro 
 * File units = 100 * gm / sec
 * Comments: Basic test data
 * Status: UNVALIDATED / UNTESTED
 * Support: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/MAF-Data-Files
 * Discussion: https://github.com/DeeEmm/DIY-Flow-Bench/discussions/51
 * Data Source: 
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
    return "VDO_AFM_043";
}



/***********************************************************
 * @brief MAF Output Type
 *
 * @note Valid options:
 * VOLTAGE
 * FREQUENCY
 ***/
int Maf::outputType() {
    return FREQUENCY;
}



/***********************************************************
 * @brief MAF Units
 * 
 * @note: Valid options:
 * KG_H
 * MG_S
 ***/
int Maf::mafUnits() {
    return MG_S;
}



/***********************************************************
 * @brief MAF scaling
 * 
 * @note: scaling factor for MAF Data:
 ***/
double Maf::mafScaling() {
    return 0.01;
}



/***********************************************************
 * @brief Original MAF Diameter in mm
 * 
 * @note Used to calculate MAF transfer function to transpose 
 * flow rates for different pipe diameters
 * @note unavalidated diameter - https://forums.justcommodores.com.au/threads/ve-maf-to-ls1.232264/page-2
 ***/
int Maf::mafDiameter() {
    return 74;
}



/***********************************************************
 * @brief mafLookupTable
 * @note Global vector of vectors containing MAF>Flow key>value pairs
 ***/
 std::vector<std::vector<long>> mafLookupTable = {{ 
{0,0},
{1500,214},
{1625,259},
{1750,307},
{1875,359},
{2000,415},
{2125,474},
{2250,540},
{2375,610},
{2500,685},
{2625,767},
{2750,855},
{2875,948},
{3000,1050},
{3125,1159},
{3250,1276},
{3375,1402},
{3500,1534},
{3625,1695},
{3750,1866},
{3875,2052},
{4000,2229},
{4125,2416},
{4250,2616},
{4375,2826},
{4500,3048},
{4625,3283},
{4750,3530},
{4875,3791},
{5000,4066},
{5125,4354},
{5250,4657},
{5375,4974},
{5500,5306},
{5625,5655},
{5750,6019},
{5875,6398},
{6000,6795},
{6125,7209},
{6250,7641},
{6375,8091},
{6500,8558},
{6625,9045},
{6750,9550},
{6875,10074},
{7000,10620},
{7125,11184},
{7250,11770},
{7375,12376},
{7500,13004},
{7625,13654},
{7750,14326},
{7875,15020},
{8000,15738},
{8125,16479},
{8250,17244},
{8375,18032},
{8500,18846},
{8625,19684},
{8750,20548},
{8875,21438},
{9000,22353},
{9125,23295},
{9250,24264},
{9375,25260},
{9500,26284},
{9625,27337},
{9750,28417},
{9875,29527},
{10000,30666},
{10125,31834},
{10250,33033},
{10375,34262},
{10500,35521},
{10625,36813},
{10750,38135},
{10875,39490},
{11000,40877},
{11125,42297},
{11250,43750},
{11375,45237},
{11500,46758},
{11625,48313},
{11750,49902},
{11875,51199},
{12000,51199}
}};
	

#endif
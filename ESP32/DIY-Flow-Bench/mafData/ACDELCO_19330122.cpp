/***********************************************************
 * ACDELCO_19330122.cpp
 *
 * Manufacturer: AC Delco
 * Part#: 19330122
 * MAF file for type: Corvette Z06  
 * File units = 100 * gm / sec
 * Comments: Basic test data
 * Status: UNVALIDATED / UNTESTED
 * Support: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/MAF-Data-Files
 * Discussion: https://github.com/DeeEmm/DIY-Flow-Bench/discussions/51
 * 
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
    return "ACDELCO_19330122";
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
 * @note Diameter from - https://www.corvetteforum.com/forums/c5-tech/2951897-upgrading-to-z06-maf.html
 ***/
int Maf::mafDiameter() {
    return 85;
}



/***********************************************************
 * @brief mafLookupTable
 * @note Global vector of vectors containing MAF>Flow key>value pairs
 ***/
 std::vector<std::vector<long>> mafLookupTable = {{ 
		{0,0},
		{1500,142},
		{1625,176},
		{1750,210},
		{1875,248},
		{2000,286},
		{2125,326},
		{2250,370},
		{2375,418},
		{2500,471},
		{2625,531},
		{2750,597},
		{2875,668},
		{3000,743},
		{3125,823},
		{3250,908},
		{3375,997},
		{3500,1091},
		{3625,1193},
		{3750,1305},
		{3875,1427},
		{4000,1559},
		{4125,1698},
		{4250,1847},
		{4375,2005},
		{4500,2172},
		{4625,2348},
		{4750,2534},
		{4875,2728},
		{5000,2934},
		{5125,3148},
		{5250,3374},
		{5375,3611},
		{5500,3862},
		{5625,4130},
		{5750,4417},
		{5875,4720},
		{6000,5040},
		{6125,5377},
		{6250,5730},
		{6375,6101},
		{6500,6488},
		{6625,6891},
		{6750,7312},
		{6875,7747},
		{7000,8197},
		{7125,8663},
		{7250,9145},
		{7375,9643},
		{7500,10159},
		{7625,10690},
		{7750,11236},
		{7875,11801},
		{8000,12383},
		{8125,12984},
		{8250,13605},
		{8375,14248},
		{8500,14913},
		{8625,15598},
		{8750,16306},
		{8875,17038},
		{9000,17792},
		{9125,18570},
		{9250,19373},
		{9375,20199},
		{9500,21052},
		{9625,21930},
		{9750,22834},
		{9875,23764},
		{10000,24723},
		{10125,25706},
		{10250,26716},
		{10375,27756},
		{10500,28825},
		{10625,29925},
		{10750,31055},
		{10875,32220},
		{11000,33416},
		{11125,34649},
		{11250,35916},
		{11375,37222},
		{11500,38566},
		{11625,39951},
		{11750,41377},
		{11875,42845},
		{12000,44360}
	}};


#endif
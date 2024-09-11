/***********************************************************
 * ACDELCO_92281162.cpp
 *
 * Manufacturer: AC Delco
 * Part#: 92281162
 * MAF file for type: GM / Holden VZ & VE / Corvette / LS2 engines
 * File units = 10 * mg/sec
 * Status: VALIDATED / UNTESTED
 * Support: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/MAF-Data-Files
 * Discussion: https://github.com/DeeEmm/DIY-Flow-Bench/discussions/142
 * Data Source www.microsquirt.info/maffactor_lsx.inc
 * 160777 mg/s is Approx 283cfm 
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
    return "ACDELCO_92281162";
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
 * @brief MAF scaling
 * 
 * @note: scaling factor for MAF Data:
 ***/
double Maf::mafScaling() {
    return 0.1;
}



/***********************************************************
 * @brief Original MAF Diameter in mm
 * 
 * @note Used to calculate MAF transfer function to transpose 
 * flow rates for different pipe diameters
 * @note Dimension Verified
 ***/
int Maf::mafDiameter() {
    return 94;
}



/***********************************************************
 * @brief mafLookupTable
 * @note Global vector of vectors containing MAF>Flow key>value pairs
 ***/
 std::vector<std::vector<long>> mafLookupTable = {{ 
    {	8	,	28	},
    {	15	,	34	},
    {	23	,	43	},
    {	31	,	52	},
    {	38	,	64	},
    {	46	,	78	},
    {	54	,	94	},
    {	62	,	113	},
    {	69	,	134	},
    {	77	,	156	},
    {	85	,	182	},
    {	92	,	210	},
    {	100	,	241	},
    {	108	,	276	},
    {	115	,	313	},
    {	123	,	353	},
    {	131	,	396	},
    {	138	,	443	},
    {	146	,	494	},
    {	154	,	550	},
    {	162	,	611	},
    {	169	,	677	},
    {	177	,	748	},
    {	185	,	824	},
    {	192	,	851	},
    {	200	,	941	},
    {	208	,	1040	},
    {	215	,	1148	},
    {	223	,	1266	},
    {	231	,	1395	},
    {	238	,	1535	},
    {	246	,	1687	},
    {	254	,	1852	},
    {	262	,	2029	},
    {	269	,	2221	},
    {	277	,	2426	},
    {	285	,	2647	},
    {	292	,	2883	},
    {	300	,	3136	},
    {	308	,	3405	},
    {	315	,	3692	},
    {	323	,	3998	},
    {	331	,	4323	},
    {	338	,	4667	},
    {	346	,	5032	},
    {	354	,	5418	},
    {	362	,	5826	},
    {	369	,	6257	},
    {	377	,	6711	},
    {	385	,	7189	},
    {	392	,	7692	},
    {	400	,	8220	},
    {	408	,	8776	},
    {	415	,	9358	},
    {	423	,	9968	},
    {	431	,	10607	},
    {	438	,	11276	},
    {	446	,	11975	},
    {	454	,	12705	},
    {	462	,	13467	},
    {	469	,	14262	},
    {	477	,	15091	},
    {	485	,	15954	},
    {	492	,	16852	},
    {	500	,	17787	}
    }};

#endif
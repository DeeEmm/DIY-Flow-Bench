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
#pragma once


 /***********************************************************
  * MAF Type
  *
  ***/
 #define MAF_SENSOR_TYPE "DELPHI_AF10118"
 
 
 /***********************************************************
 * MAF Output Type
 *
 * 1 = voltage
 * 2 = frequency
 ***/
int MAFoutputType = 1;


/***********************************************************
 * MAF Data format
 * 
 * 1 = key_value
 * 2 = rawAnalog (1024 data points)
 ***/
int MAFdataFormat = 1; 



/***********************************************************
 * MAF Units
 * 
 * 1 = kg_h
 * 2 = mg_s
 ***/
int MAFdataUnit = 1;


/***********************************************************
 * MAF Data
 ***/
long mafMapAnalogData[] = {0,0}; // Need to declare unused analog array to prevent code breaking
long mafMapData[][2] = {
{0,3650},
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

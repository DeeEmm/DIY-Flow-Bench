/***********************************************************
 * exampleMafData.h
 *
 * Manufacturer: Manufacturer
 * Part#: 123456
 * MAF file for type: Make / Model
 * File units = 10 * mg/sec
 * Comments: Example Data
 * Status: 
 * Support: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/MAF-Data-Files
 * Discussion: https://github.com/DeeEmm/DIY-Flow-Bench/discussions/51
***/
#ifndef MAFDATA
#define MAFDATA



/***********************************************************
* This is an example MAF Data file
*
* You may find that the first and last entries have an odd looking number on the datasheet. 
* This is the limp home value which will come into effect if the sensor is shorted or open circuit
* Simply change these values to 0 for the lowest value and match the highest value for the other value.
***/


/***********************************************************
* MAF Type
*
***/
 #define MAF_SENSOR_TYPE "YOUR_MAF_NAME"
 

/***********************************************************
* MAF Output Type
*
* VOLTAGE
* FREQUENCY
***/
int MAFoutputType = VOLTAGE;



/***********************************************************
* MAF Units
* 
* KG_H
* MG_S
***/

int MAFdataUnit = MG_S;



/***********************************************************
* MAF Data
*
* The lookup table takes the form of {key, value}
*
* Keys may represent analog units, millivolts, hertz, baked beans or or any other input value.
* Data table length is not fixed 
* Keys do not need to be contiguous nor linear
* Keys do not need to start at zero
* Keys do not need to finish at a specific value
* Whatever value you input into the getMafValue method will return the relevant lookup table value
*
*
* NOTE:
* Generally with analog sensor data the keys represent the sensor signal range and so are usually contiguous and to the order of bit^resolution.
* i.e. 10 bit data range has a table size of 2^10 = 1024 bits long, which means that there will be 1024 entries (as per this example).
* However, it should be noted that whilst the ESP32 inputs have 12 bit resolution, it is not necessary for the data table to share 
* this resolution. Lookup table results are interpolated and so input resolution does not need to match the lookup table size
*
***/

long mafLookupTable [][2] = {
{0,0},
{1,177},
{2,181},
{3,184},
{4,188},
{5,191},
{6,195},
{7,199},

... etc ...

{1018,157897},
{1019,158374},
{1020,158852},
{1021,159331},
{1022,159812},
{1023,160293},
{1024,160775},
{1025,160775}
};

#endif
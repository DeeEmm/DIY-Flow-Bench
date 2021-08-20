/***********************************************************
 * exampleKeyValueData.h
 *
 * Manufacturer: Manufacturer
 * Part#: 123456
 * MAF file for type: Make / Model
 * File units = 1000 * kg/hr
 * Comments: Example Data
 * Status: 
 * Support: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/MAF-Data-Files
 * Discussion: https://github.com/DeeEmm/DIY-Flow-Bench/discussions/51
 ***/
#pragma once



/***********************************************************
 * This is an example MAF Data file
 * You can either use volts versus kg/hr or ms/sec as below, or 1024 analog data points versus ms/sec or kg/hr
 * (refer to mafData/exampleAnalogData.h if yo have 1024 points of analog data)
 *
 * Just change the values below to suit. Array length and voltage intervals are not an issue, add extra items as necessary.
 * 
 * NOTE: all data values used for MAF lookup are integers. This is to reduce overheads and simplify code. 
 * It also allows us to use both millivolts and RAW analog references in the same code without having to worry about changing datatypes
 * So to populate the table you will probably have to translate your values as follows:
 * millivolts (volts/100), kgh/1000 
 * 
 * example - the first two entries below are 0.1 volts and 6.226 kg/hr
 * we multiply volts by 1000 to get millivolts - 0.1 x 1000 = 100
 * we multiply kg/hr by 1000 to provide 3 decimal places in integer format - 6.226 x 1000 = 6226
 * we multiply mg/sec by 10 to provide 1 decimal place in integer format - 62.6 x 10 = 626
 ***/


/***********************************************************
  * MAF Type
  *
  ***/
 #define MAF_SENSOR_TYPE "YourMafName"
 
 
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
{0,0},
{100,6226},
{200,6745},
{300,7307},
{400,7917},
{500,8577},
{600,9292},
{700,10067},
{800,10907},
{900,11816},
{1000,12802},
{1100,13869},
{1200,15026},
{1300,16279},
{1400,17637},
{1500,19108},
{1600,20701},
{1700,22428},
{1800,24298},
{1900,26324},
{2000,28520},
{2100,30898},
{2200,33475},
{2300,36266},
{2400,39291},
{2500,42567},
{2600,46117},
{2700,49963},
{2800,54130},
{2900,58644},
{3000,63535},
{3100,68833},
{3200,74574},
{3300,80793},
{3400,87531},
{3500,94830},
{3600,102739},
{3700,111307},
{3800,120589},
{3900,130646},
{4000,141541},
{4100,153345},
{4200,166133},
{4300,179988},
{4400,194998},
{4500,211260},
{4600,228878},
{4700,247965},
{4800,268644},
{4900,291048},
{5000,315320}
};

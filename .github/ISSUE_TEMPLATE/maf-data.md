---
name: MAF Data
about: Submit a MAF data file
title: "[MAF DATA]"
labels: enhancement
assignees: ''

---

// exampleSensor.h

// This is an example MAF Data file
// You can either use volts versus kg/hr as below, or 1024 analog data points versus kg/hr
// Just change the values below to suit. Array length and voltage intervals are not an issue, add extra items as necessary.

// NOTE: all data values used for MAF lookup are integers. This is to reduce overheads and simplify code. 
// It also allows us to use both millivolts and RAW analog references in the same code without having to worry about changing datatypes
// So to populate the table you will probably have to translate your values as follows:
// millivolts (volts/100), kgh/1000 

// example - the first two entries below are 0.1 volts and 2.304 kg/hr
// we multiply volts by 100 to get millivolts - 0.1 x 100 = 10
// we multiply kg/hr by 1000 to provide 3 decimal places in integer format - 2.304 x 1000 = 2304

int mafMapData[][2] = {
{	10	,	6226	},
{	20	,	6745	},
{	30	,	7307	},
{	40	,	7917	},
{	50	,	8577	},
{	60	,	9292	},
{	70	,	10067	},
{	80	,	10907	},
{	90	,	11816	},
{	100	,	12802	},
{	110	,	13869	},
{	120	,	15026	},
{	130	,	16279	},
{	140	,	17637	},
{	150	,	19108	},
{	160	,	20701	},
{	170	,	22428	},
{	180	,	24298	},
{	190	,	26324	},
{	200	,	28520	},
{	210	,	30898	},
{	220	,	33475	},
{	230	,	36266	},
{	240	,	39291	},
{	250	,	42567	},
{	260	,	46117	},
{	270	,	49963	},
{	280	,	54130	},
{	290	,	58644	},
{	300	,	63535	},
{	310	,	68833	},
{	320	,	74574	},
{	330	,	80793	},
{	340	,	87531	},
{	350	,	94830	},
{	360	,	102739	},
{	370	,	111307	},
{	380	,	120589	},
{	390	,	130646	},
{	400	,	141541	},
{	410	,	153345	},
{	420	,	166133	},
{	430	,	179988	},
{	440	,	194998	},
{	450	,	211260	},
{	460	,	228878	},
{	470	,	247965	},
{	480	,	268644	},
{	490	,	291048	},
{	500	,	315320	}
};

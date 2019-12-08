// exampleSensor.h

// This is an example MAF Data file
// You can either use volts versus cfm as below, or 1024 analog data points versus cfm
// Just change the values below to suit. Array length and voltage intervals are not an issue, add extra items as necessary.

// NOTE: all data values used for MAF lookup are integers. This is to reduce overheads and simplify code. 
// It also allows us to use both millivolts and RAW analog references in the same code without having to worry about changing datatypes
// So to populate the table you will probably have to translate your values as follows:
// millivolts (volts/100), CFM/1000 

// example - the first two entries below are 0.1 volts and 2.304 cfm
// we multiply volts by 100 to get millivolts - 0.1 x 100 = 10
// we multiply cfm by 1000 to provide 3 decimal places in integer format - 2.304 x 1000 = 2304

int mafMapData[][2] = {
{	10	,	2304	},
{	20	,	2512	},
{	30	,	2739	},
{	40	,	2985	},
{	50	,	3254	},
{	60	,	3548	},
{	70	,	3868	},
{	80	,	4216	},
{	90	,	4596	},
{	100	,	5010	},
{	110	,	5462	},
{	120	,	5954	},
{	130	,	6491	},
{	140	,	7076	},
{	150	,	7714	},
{	160	,	8409	},
{	170	,	9167	},
{	180	,	9993	},
{	190	,	10894	},
{	200	,	11876	},
{	210	,	12947	},
{	220	,	14114	},
{	230	,	15386	},
{	240	,	16772	},
{	250	,	18284	},
{	260	,	19932	},
{	270	,	21729	},
{	280	,	23687	},
{	290	,	25822	},
{	300	,	28150	},
{	310	,	30687	},
{	320	,	33453	},
{	330	,	36468	},
{	340	,	39755	},
{	350	,	43338	},
{	360	,	47244	},
{	370	,	51503	},
{	380	,	56145	},
{	390	,	61205	},
{	400	,	66722	},
{	410	,	72736	},
{	420	,	79292	},
{	430	,	86439	},
{	440	,	94230	},
{	450	,	102723	},
{	460	,	111982	},
{	470	,	122075	},
{	480	,	133078	},
{	490	,	145073	},
{	500	,	158149	}
};
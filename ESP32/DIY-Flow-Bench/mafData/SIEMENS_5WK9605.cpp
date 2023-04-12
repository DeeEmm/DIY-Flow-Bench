/***********************************************************
 * SIEMENS_5WK9605.cpp
 *
 * Manufacturer: Siemens
 * Part#: 5WK9605
 * MAF file for type: BMW M54B30 
 * File units = 10 * kg/hr
 * Status: VALIDATED / UNTESTED.
 * Support: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/MAF-Data-Files
 * Discussion: https://github.com/DeeEmm/DIY-Flow-Bench/discussions/141
 * Data Source - https://www.ms4x.net/index.php?title=Aftermarket_Upgrade_Sensor_Data
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
    return "SIEMENS_5WK9605";
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
    return 72;
}




/***********************************************************
 * @brief mafLookupTable
 * @note Global vector of vectors containing MAF>Flow key>value pairs
 ***/
 std::vector<std::vector<long>> mafLookupTable = {{ 
{	0	,	0	},
{	2	,	0	},
{	4	,	0	},
{	6	,	0	},
{	8	,	18	},
{	10	,	22	},
{	12	,	27	},
{	14	,	31	},
{	16	,	36	},
{	18	,	40	},
{	20	,	45	},
{	21	,	49	},
{	23	,	54	},
{	25	,	58	},
{	27	,	63	},
{	29	,	67	},
{	31	,	73	},
{	33	,	76	},
{	35	,	79	},
{	37	,	82	},
{	39	,	85	},
{	41	,	88	},
{	43	,	91	},
{	45	,	95	},
{	47	,	98	},
{	49	,	102	},
{	51	,	105	},
{	53	,	109	},
{	55	,	113	},
{	57	,	117	},
{	59	,	121	},
{	61	,	125	},
{	63	,	129	},
{	64	,	133	},
{	66	,	137	},
{	68	,	142	},
{	70	,	146	},
{	72	,	151	},
{	74	,	156	},
{	76	,	161	},
{	78	,	166	},
{	80	,	171	},
{	82	,	176	},
{	84	,	181	},
{	86	,	186	},
{	88	,	192	},
{	90	,	198	},
{	92	,	203	},
{	94	,	209	},
{	96	,	215	},
{	98	,	221	},
{	100	,	227	},
{	102	,	233	},
{	104	,	240	},
{	105	,	247	},
{	107	,	253	},
{	109	,	260	},
{	111	,	267	},
{	113	,	275	},
{	115	,	282	},
{	117	,	290	},
{	119	,	297	},
{	121	,	305	},
{	123	,	313	},
{	125	,	322	},
{	127	,	331	},
{	129	,	339	},
{	131	,	348	},
{	133	,	358	},
{	135	,	367	},
{	137	,	377	},
{	139	,	387	},
{	141	,	397	},
{	143	,	407	},
{	145	,	418	},
{	146	,	428	},
{	148	,	439	},
{	150	,	450	},
{	152	,	461	},
{	154	,	472	},
{	156	,	483	},
{	158	,	495	},
{	160	,	506	},
{	162	,	518	},
{	164	,	529	},
{	166	,	541	},
{	168	,	553	},
{	170	,	565	},
{	172	,	577	},
{	174	,	589	},
{	176	,	601	},
{	178	,	614	},
{	180	,	626	},
{	182	,	639	},
{	184	,	652	},
{	186	,	665	},
{	188	,	679	},
{	189	,	692	},
{	191	,	706	},
{	193	,	720	},
{	195	,	735	},
{	197	,	749	},
{	199	,	764	},
{	201	,	779	},
{	203	,	795	},
{	205	,	811	},
{	207	,	826	},
{	209	,	843	},
{	211	,	860	},
{	213	,	877	},
{	215	,	894	},
{	217	,	912	},
{	219	,	930	},
{	221	,	949	},
{	223	,	968	},
{	225	,	987	},
{	227	,	1007	},
{	229	,	1027	},
{	230	,	1047	},
{	232	,	1068	},
{	234	,	1089	},
{	236	,	1110	},
{	238	,	1132	},
{	240	,	1154	},
{	242	,	1177	},
{	244	,	1200	},
{	246	,	1223	},
{	248	,	1246	},
{	250	,	1270	},
{	252	,	1295	},
{	254	,	1319	},
{	256	,	1345	},
{	258	,	1370	},
{	260	,	1396	},
{	262	,	1422	},
{	264	,	1449	},
{	266	,	1476	},
{	268	,	1503	},
{	270	,	1531	},
{	271	,	1559	},
{	273	,	1588	},
{	275	,	1617	},
{	277	,	1646	},
{	279	,	1676	},
{	281	,	1706	},
{	283	,	1736	},
{	285	,	1768	},
{	287	,	1799	},
{	289	,	1831	},
{	291	,	1863	},
{	293	,	1896	},
{	295	,	1930	},
{	297	,	1963	},
{	299	,	1998	},
{	301	,	2033	},
{	303	,	2068	},
{	305	,	2104	},
{	307	,	2140	},
{	309	,	2177	},
{	311	,	2214	},
{	313	,	2252	},
{	314	,	2290	},
{	316	,	2329	},
{	318	,	2369	},
{	320	,	2409	},
{	322	,	2450	},
{	324	,	2491	},
{	326	,	2533	},
{	328	,	2575	},
{	330	,	2618	},
{	332	,	2662	},
{	334	,	2706	},
{	336	,	2751	},
{	338	,	2797	},
{	340	,	2843	},
{	342	,	2891	},
{	344	,	2938	},
{	346	,	2987	},
{	348	,	3036	},
{	350	,	3086	},
{	352	,	3137	},
{	354	,	3188	},
{	355	,	3240	},
{	357	,	3293	},
{	359	,	3347	},
{	361	,	3402	},
{	363	,	3458	},
{	365	,	3514	},
{	367	,	3572	},
{	369	,	3630	},
{	371	,	3690	},
{	373	,	3750	},
{	375	,	3811	},
{	377	,	3874	},
{	379	,	3937	},
{	381	,	4002	},
{	383	,	4068	},
{	385	,	4134	},
{	387	,	4202	},
{	389	,	4272	},
{	391	,	4342	},
{	393	,	4414	},
{	395	,	4487	},
{	396	,	4561	},
{	398	,	4637	},
{	400	,	4714	},
{	402	,	4793	},
{	404	,	4873	},
{	406	,	4955	},
{	408	,	5038	},
{	410	,	5123	},
{	412	,	5210	},
{	414	,	5298	},
{	416	,	5388	},
{	418	,	5481	},
{	420	,	5575	},
{	422	,	5671	},
{	424	,	5769	},
{	426	,	5870	},
{	428	,	5972	},
{	430	,	6077	},
{	432	,	6184	},
{	434	,	6293	},
{	436	,	6403	},
{	438	,	6515	},
{	439	,	6628	},
{	441	,	6742	},
{	443	,	6857	},
{	445	,	6973	},
{	447	,	7090	},
{	449	,	7208	},
{	451	,	7325	},
{	453	,	7443	},
{	455	,	7561	},
{	457	,	7679	},
{	459	,	7797	},
{	461	,	7915	},
{	463	,	8033	},
{	465	,	8151	},
{	467	,	8270	},
{	469	,	8389	},
{	471	,	8508	},
{	473	,	8627	},
{	475	,	8747	},
{	477	,	8868	},
{	479	,	8989	},
{	480	,	9111	},
{	482	,	9233	},
{	484	,	9356	},
{	486	,	9480	},
{	488	,	9605	},
{	490	,	9731	},
{	492	,	9857	},
{	494	,	9985	},
{	496	,	10114	},
{	498	,	10240	}
}};

#endif
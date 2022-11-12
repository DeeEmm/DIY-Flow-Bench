/***********************************************************
 * ACDELCO-92281162.h
 *
 * Manufacturer: AC Delco
 * Part#: 92281162
 * MAF file for type: GM / Holden VZ & VE / Corvette / LS2 engines
 * File units = 10 * mg/sec
 * Comments: Original data from Megasquirt website - www.microsquirt.info/maffactor_lsx.inc
 * Status: Validated
 * Support: https://github.com/DeeEmm/DIY-Flow-Bench/wiki/MAF-Data-Files
 * Discussion: https://github.com/DeeEmm/DIY-Flow-Bench/discussions/51
 ***/
#pragma once
#ifndef MAFDATA
#define MAFDATA

#include "../constants.h"


class MafData {

    public:

        MafData();
        static void begin();

        String mafSensorType;
        int mafOutputType;
        int mafDataUnit;
        int mafLookupTable[1024][2];

};
#endif
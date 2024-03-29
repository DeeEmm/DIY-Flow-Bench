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

#include <Arduino.h>
#include <vector>
#include "../constants.h"
#include "../configuration.h"

class Maf {

    friend class Sensors;
    friend class Calculations;

    protected: 

        // static std::vector<std::vector<int>> *mafLookupTable;
        // static const std::vector<std::vector<int>> mafLookupTable;

    public:

        Maf();

        String mafSensorType;
        int mafOutputType;
        int mafDataUnit;

        String sensorType();
        int outputType();
        int mafUnits();
        double mafScaling();
        int mafDiameter();

        static std::vector<std::vector<int>> mafLookupTable;
        // std::vector<std::vector<int>> mafLookupTable;


};

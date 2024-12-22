/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file mafData.h
 * 
 * @brief Define MAF data
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/
#pragma once

#include <array>
#include <iostream>
#include <vector>


class MafData {
    
private:
    static const int NUM_COEFFICIENTS = 7;
    static const int NUM_MAF_TYPES = 2;
    int currentMafType;

public:
    enum MafType {
        ACDELCO_92281162 = 0,
        BOSCH_0280218067 = 1
    };

    float mafCoeff[2][7] = {
        {624.306263f, 25.418081f, 0.365468f, 0.002299f, 0.000003f, -0.000000f, 0.000000f}, // ACDELCO_92281162
        {817.925606f, -15.237634f, 0.091194f, -0.000073f, -0.000000f, 0.000000f, -0.000000f} // BOSCH_0280218067
    };

    // MafData() : currentMafType(ACDELCO_92281162) {}

    // // Constructor with MafType parameter
    // MafData(MafType type) : currentMafType(type) {
    //     if (type >= NUM_MAF_TYPES) {
    //         currentMafType = ACDELCO_92281162; // Set default if invalid
    //     }
    // }

    // Constructor with int parameter
    MafData(int type) : currentMafType(type) {
        if (type >= NUM_MAF_TYPES) {
            currentMafType = ACDELCO_92281162; // Set default if invalid
        }
    }

    void setMafType(MafType type) {
        if (type < NUM_MAF_TYPES) {
            currentMafType = type;
        }
    }

    float getCoefficient(int index) const {
        if (index >= 0 && index < NUM_COEFFICIENTS) {
            return mafCoeff[currentMafType][index];
        }
        return 0.0f;
    }

    MafType getCurrentType() const {
        return static_cast<MafType>(currentMafType);
    }
};





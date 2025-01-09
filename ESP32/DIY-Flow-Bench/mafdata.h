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
    static const int NUM_MAF_TYPES = 3;
    int currentMafType;

public:
    enum MafType {
        ACDELCO_92281162 = 0,
        BOSCH_0280218067 = 1,
        DELPHI_AF10118 = 2
    };

    enum MafOutputType {
        Voltage = 0,
        Frequency = 1
    };

    float mafCoeff[NUM_MAF_TYPES][NUM_COEFFICIENTS] = {
        {0.762518f, 0.001883f, 0.000005f, 0.000000035f, 0.000000f, -0.000000f, 0.000000f}, // ACDELCO_92281162 (Data calculated from PY V1)
        {81.792561f, -0.152376f, 0.000091f, -0.000000f, -0.000000f, 0.000000f, -0.000000f}, // BOSCH_0280218067 (Data calculated from PY V1)
        {-147.198775f, 0.544108f, -0.000725f, 0.000000f, -0.000000f, 0.000000f, -0.000000f} // DELPHI_AF10118
    };


    int mafDiameter[NUM_MAF_TYPES] = {94, 82, 70}; // MAF diameter in mm

    double mafScaling[NUM_MAF_TYPES] = {0.1f, 0.1f, 0.1f}; // MAF scaling factor

    int mafOutputType[NUM_MAF_TYPES] = {Voltage, Voltage, Voltage}; // MAF output type

    int mafMaxKGH[NUM_MAF_TYPES] = {1607, 1805, 491}; // MAF kg/h value at 5 volts

    String mafType[NUM_MAF_TYPES] = {
        "ACDELCO 92281162",
        "BOSCH 0280218067",
        "DELPHI_AF10118"
    };

    String mafLink[NUM_MAF_TYPES] = {
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/142",
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/138",
        ""
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

    String getType() const {
        return mafType[currentMafType];
    }

    double getScaling() const {
        return mafScaling[currentMafType];
    }

    int getDiameter() const {
        return mafDiameter[currentMafType];
    }

    int getOutputType() const {
        return mafOutputType[currentMafType];
    }

    int getMaxKGH() const {
        return mafMaxKGH[currentMafType];
    }

    String getMafLink() const {
        return mafLink[currentMafType];
    }

    float calculateFlow(float mafVolts) const {
        float flow = 0.0f;
        float v_power = 1.0f;  // V^n starts at V^0
        
        // Calculate polynomial using coefficients
        for(int i = 0; i < NUM_COEFFICIENTS; i++) {
            flow += mafCoeff[currentMafType][i] * v_power;
            v_power *= mafVolts;  // Increment power for next term
        }
        
        return flow;
    }

};





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
    static const int NUM_MAF_TYPES = 8;
    int currentMafType;

public:

    enum MafOutputType {
        Voltage = 0,
        Frequency = 1
    };

    enum MafType {
        ACDELCO_92281162 = 0,
        BOSCH_0280218067 = 1,
        DELPHI_AF10118 = 2,
        BOSCH_0280002421 = 3,
        BOSCH_0280218008 = 4,
        BOSCH_0280217531 = 5,
        BOSCH_0280218019 = 6,
        BOSCH_0280217123 = 7     
    };

    enum MafStatus {
        Untested = 0,
        Tested = 1,
        Validated = 2,
        Invalid = 3
    };

    // Polynomial Coefficients for MAF transfer function (C0, C1, C2, C3, C4, C5, C6)
    // Best way to generate polynomials is using excel sheet as it returns more accurate data
    // Enter data in exponential notation as it is generated in excel sheet (e.g. 1.2345e-05)
    double mafCoeff[NUM_MAF_TYPES][NUM_COEFFICIENTS] = {
        {1.734046571f, 0.007062544f, 1.01476E-05, 6.3883E-09, 9.63923E-13, -3.24416E-17, 1.11433E-21}, // ACDELCO_92281162 (Data calculated from PY V1)
        {0.970294436f, -0.078166434f, 0.000138556f, -8.51019E-08, 3.12389E-11, -5.15648E-15, 3.82478E-19}, // BOSCH_0280218067 (Excel data positive flow only)
        {-1.006588218f, 0.080079937f, -0.000180578, 1.68028E-07, -6.82173E-11, 1.31768E-14, -9.55601E-19}, // DELPHI_AF10118 (Excel data)
        {18.5392475f, -0.094665483f, 0.000123116f, -5.85313e-08, 1.82179e-11, -2.42405e-15, 1.69842E-19}, // BOSCH_0280002421 (HFM5 plugin 82mm) (Excel data)
        {215.8330171f, -0.526769493f, 0.00048859f, -2.15008E-07, 5.33313E-11, -6.74228E-15, 3.87965E-19}, // BOSCH_0280218008 (HFM5 plugin 78mm) (Excel data)
        {58.36807578f, -0.164211479f, 0.00016265f, -6.89421E-08, 1.76501E-11, -2.26324E-15, 1.45971E-19}, // BOSCH_0280217531 (HFM5 plugin 71mm) (Excel data)
        {146.5075402f, -0.344050939f, 0.000308055f, -1.30212E-07, 3.06369E-11, -3.58939E-15, 1.91244E-19}, // BOSCH_0280218019 (HFM5 plugin 62mm) (Excel data)
        {-0.00047495f, -0.005188161f, 4.25563E-06, 2.94949E-09, -8.80773E-13, 1.35382E-16, 4.28461E-21} // BOSCH_0280217123 (HFM5 plugin 50mm) (Excel data)
    };

    // When adding a sensor, dont forget to add it to the iMAF_SENS_TYP dropdown in config.html


    int mafStatus[NUM_MAF_TYPES] = {Untested, Tested, Tested, Tested, Tested ,Tested ,Tested ,Tested}; // MAF Status

    int mafDiameter[NUM_MAF_TYPES] = {94, 82, 70, 82, 78, 71, 62, 50 }; // Original MAF diameter in mm

    double mafScaling[NUM_MAF_TYPES] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}; // MAF scaling factor

    int mafOutputType[NUM_MAF_TYPES] = {Voltage, Voltage, Voltage, Voltage, Voltage, Voltage, Voltage, Voltage}; // MAF output type

    int mafMaxKGH[NUM_MAF_TYPES] = {1607, 1805, 491, 1000, 850, 640, 480, 370}; // MAX MAF kg/h value

    String mafType[NUM_MAF_TYPES] = {
        "ACDELCO 92281162",
        "BOSCH 0280218067",
        "DELPHI AF10118",
        "BOSCH 0280002421",
        "BOSCH 0280218008",
        "BOSCH 0280217531",
        "BOSCH 0280218019",
        "BOSCH 0280217123"

    };

    String mafLink[NUM_MAF_TYPES] = {
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/142",
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/138",
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/319",
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/339",
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/346",
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/347",
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/348",
        "https://github.com/DeeEmm/DIY-Flow-Bench/discussions/349"
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
        if (type > NUM_MAF_TYPES) {
            currentMafType = BOSCH_0280218067; // Set default if invalid
        }
    }

    void setMafType(MafType type) {
        if (type <= NUM_MAF_TYPES) {
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

    int getNumSensors() const {
        return NUM_MAF_TYPES;
    }

    int getStatus() const {
        return mafStatus[currentMafType];
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
            v_power *= mafVolts;  // Increment power for next scan
        }
        
        return flow;
    }

};





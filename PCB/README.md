<img width="2032" alt="Screenshot 2023-02-22 at 1 37 35 pm" src="https://user-images.githubusercontent.com/3038710/231706881-e8f968e5-44ca-4bdc-a4ba-b6cb024abd6a.png">

## The DIY Flow Bench project

### https://diyflowbench.com

### DIYFB ESP32 SHIELD V2.0-BETA

Open source flow bench project to measure and display volumetric air flow using an ESP32 / MAF.

For more information please visit the [WIKI](https://github.com/DeeEmm/DIY-Flow-Bench/wiki) Or join our [support forums](https://github.com/DeeEmm/DIY-Flow-Bench/discussions) 

This project and all associated files are provided for use under the CERN–OHL–W license which allows people to make their own derivative designs provided that attribution is provided and that any derivative design is also made publicly available. 

## Notes

- PCB is designed to fit standard Hammond 100mm x 120mm 1455 series cases such as the 1455K1201 (100mm x 120mm x 40mm)
- Standard tolerance components can be used but more accurate results may be obtained by using higher tolerance components
- Both MPXV7007DP (U13,U14,U15) and MPX4250AP (U3,U4,U5) are connected to the same inputs using different footprints. This is to allow either style of sensor to be used. U13 is connected to U3, U14 to U4 and U15 to U5 Sensor designations are printed on the PCB: U3/13 is Ref Pressure. U4/14 is Differential Pressure. U5/15 is Pitot
- Pololu 'Step Stick' style stepper drivers are used for stepper drivers and can be either A4988 or DRV8825 (Preferred). The DRV8825 can handle higher motor currents
- Edge connectors are standard IDE style ribbon connectors. This allows pre-made ribbon cables to be used for wiring loom. Pin spacing is standard 2.54mm so is also compatible with other pin headers or direct cable soldering ULN2003A Darlington Array allows switching of 12v devices at mas 500mA such as relays for control of vac motors and other peripherals. Please note that the ULN2003 switches the 0v line of the load to ground
- DC power jack is same as ESP / Arduino. You can desolder and re-use the connector that came on your ESP32
- U8 and U9 are LM2596S DC-DC buck style converters. You wil need one 3.3v and one 5v version. Alternatively you can use adjustable units and dial the output voltages in as required
- Standard blade fuse holder with 3 amp fuse is required for board protection. Fuse can be derated as required.
- BME / ADC / PCA footprints may differ from whatever BOB you purchase. BOBs can easlily be connected using jumper wires if required.

## Schematic

![Schematic_DIY-FLOW-BENCH-V2](https://user-images.githubusercontent.com/3038710/231708453-fe83ac02-28ee-4325-a34d-cd31aa10870b.png)
[Schematic_DIY-FLOW-BENCH-V2.pdf](https://github.com/DeeEmm/DIY-Flow-Bench/files/11220199/Schematic_DIY-FLOW-BENCH-V2.pdf)


## BOM

![BOM_DIY-FLOW-BENCH-V2](https://user-images.githubusercontent.com/3038710/231706321-c94b84f1-5c3f-42c6-91a3-65d81e70514c.png)
[BOM_DIY-FLOW-BENCH-V2.pdf](https://github.com/DeeEmm/DIY-Flow-Bench/files/11220123/BOM_DIY-FLOW-BENCH-V2.pdf)


## Official DIYFB Shield

The official shield PCB will be available for purchase once testing has been finalised.


## Changelog

- 13.04.23 - V2 Shield assembed and testing under way 
- 01.03.23 - V2 design updated to address issues found with V1 board
- 26.08.21 - Currently design is untested, prototype boards have been ordered. Will update once prototype has been reviewed

<img width="2032" alt="PCB 3D View" src="https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/PCB/PCB%203D.png?raw=true">

## The DIY Flow Bench project

### https://diyflowbench.com

### DIYFB ESP32 SHIELD V2.3-BETA

Open source flow bench project to measure and display volumetric air flow using an ESP32 / MAF.

For more information please visit the [WIKI](https://github.com/DeeEmm/DIY-Flow-Bench/wiki) Or join our [support forums](https://github.com/DeeEmm/DIY-Flow-Bench/discussions) 

The PCB design and all associated files are provided for use under the CERN–OHL–W license which allows people to make their own derivative designs provided that attribution is provided and that any derivative design is also made publicly available. 

The DIYFB Software is provided under the GPL3 license. More details can be found in the README file in the main project.

## Notes

- PCB is designed to fit standard Hammond 100mm x 120mm 1455 series cases such as the 1455K1201 (100mm x 120mm x 40mm)
- Standard tolerance components can be used but more accurate results may be obtained by using higher tolerance components
- Pressure sensors MPXV7007DP (U100,U110,U120), MPX4250AP (U101,U111,U121) and XGZ6899A (U102,U112,U122) are connected to the same inputs using different footprints. This is to allow you to use any style of sensor. i.e. U100 is connected to U101 and U102. Sensor designations are printed on the PCB: u100/U101/102 are Ref Pressure. U110/U111/U112 are Differential Pressure. U120/U121/U122 are Pitot
- Pololu 'Step Stick' style stepper drivers are used for stepper drivers and can be either A4988 or DRV8825 (Preferred). The DRV8825 can handle higher motor currents
- Edge connectors are standard IDE style ribbon connectors. This allows pre-made ribbon cables to be used for wiring loom. Pin spacing is standard 2.54mm so is also compatible with other pin headers or direct cable soldering 
- ULN2003A Darlington Array allows switching of 12v devices at max 500mA such as relays for control of vac motors and other peripherals. Please note that the ULN2003 switches the 0v line of the load to ground. You can jumper from the ULN to the breadboard area for custom applications.
- DC power jack is same as ESP / Arduino. You can desolder and re-use the connector that came on your ESP32
- U8 and U9 are LM2596S DC-DC buck style converters. You will need one 3.3v and one 5v version. Alternatively you can use adjustable units and dial the output voltages in manually.
- Standard blade fuse holder with 3 amp fuse is required for board protection. Fuse can be derated as required.
- BME / ADC / PCA footprints may differ from whatever BOB you purchase. BOBs can easlily be connected using jumper wires if required. We have tried to include layout for mosst commmon BME versions.

## Schematic

![Schematic_DIY-FLOW-BENCH-V2-3](https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/PCB/Schematic_DIY-FLOW-BENCH-V2-3.jpg?raw=true)
[Schematic_DIY-FLOW-BENCH-V2-3.pdf](https://github.com/DeeEmm/DIY-Flow-Bench/files/11220199/Schematic_DIY-FLOW-BENCH-V2-3.pdf)


## BOM

You can view the BOM at the following link 

https://htmlpreview.github.io/?https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/docs/iBom-DIY-FLOW-BENCH-V2-3.html

This will open up an interactive BOM and schematic layout which allows you to view component locations on the PCB


## Official DIYFB Shield

The V2.3 shield PCB is available for purchase for AU$10 + Shipping. DM me for more details.


## Changelog

- 11.09.24 - V2.3 shield update
- 13.04.23 - V2 Shield assembed and testing under way 
- 01.03.23 - V2 design updated to address issues found with V1 board
- 26.08.21 - Currently design is untested, prototype boards have been ordered. Will update once prototype has been reviewed

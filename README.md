# DIY-Flow-Bench


This repository contains the hardware schematics and software code needed to build a volumetric flow bench based on an Arduino controller and common automotive MAF sensor


Version Information
===================

Version 1.0



Contributors
============

Michael Percy AKA DeeEmm
Tony Donn AKA ModChipMan
Hans Ake Rahm



What is it?
===========

The DIY Flow Bench is a basic system to measure and display volumetric air flow.

The Flow Bench is used as a tool for measuring and quantifying modifications and improvements made to items such as automotive cylinder heads and carburettors by measuring air flow improvements.


Project Goals
=============

- Affordable & easy to source components.
- Can be built & operated by a layman.
- Generates results comparable with commercial flow bench.
- Open source.


Project Status
==============

The project is currently working towards a version 1 release. At present there is no stable development version but the original aplha release is available from the master branch.


Roadmap
=======

Version 1 Roadmap
  

The V1 release is aimed at creating a basic functional flow bench. This comprises of determining hardware requirements and testing different setups to validate the repeatability and accuracy of a MAF based flow bench.

The basic bench design comprises of a MAF sensor mounted in a flow tube connected to the test piece. The other end of the flow tube is connected to the vacuum source (The need for plenums needs to be determined). The MAF sensor signal is processed and scaled by an Arduino based controller to display the flow rate in CFM

The V1 project focus is to determine a repeatable hardware design along with a basic MAF interface that will display flow results in CFM. It is important that the hardware design can be easily replicated and produces the same results for a variety of vacuum sources and setups. Results should be verified with calibrated orifice plates. Successful validation of results across several benches is required.

The project comprises of the following key outcomes:

##Specify project constraints:

Determine design constraints e.g. Price, size, complexity, materials, etc
Define functional constraints - Only basic & necessary functions to be included in V1 release.

##Hardware:

Component selection (MAF / Pressure sensors / Arduino type / PSU etc)
Bench design (Physical size / pipework dimensions / materials / etc)
Creation of basic Arduino shield design to integrate pressure sensors / MAF / LCD display.

##Software

Creation of modular code structure
Integration of 4 line LCD display libraries

##Functionality

Display flow rate in CFM
Display reference signal value IN/WG
Display calibration data
Calibrate to orifice plate / reference pressure
Adjust scaling for MAF sensor
Startup test (Leak test)

#Validation

3rd party calibration of test orifices
Calibration of bench to test orifice
Verification of correct flow rates for other test orifice sizes
Results obtained from more than one system

For more details on the current status of the V1 development please visit the project page at https://github.com/DeeEmm/DIY-Flow-Bench/projects/1


License
=======

This project is released under the GPLV3 license. For details on usage please refer to the license file included with this project


Further Information
===================

Join the project community - https://www.facebook.com/groups/diyflowbench/ 

If you would like to get involved and controbute to the project please let us know.



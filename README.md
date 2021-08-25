# DIY-Flow-Bench

## Version Information
###

General Version V2.0-RC.1

For current build version please check the BUILD_NUMBER in docs/changelog



## What is a Flow Bench?
###

A Flow Bench is a basic system to measure and display volumetric air flow.

It is used as a tool for measuring and quantifying modifications and improvements made to items like automotive cylinder heads and carburettors by measuring the amount of air that they can flow. By measuring the volume of air that can be passed through the item the user can determine if modifications help improve air flow characteristics.

Traditionally constructed of a manometer and calibrated flow orifices, volumetric flow was calculated using mathematical methods. Modern flow benches use sensors to detect flow and reference pressures, flow calculations are now undertaken by microprocessor.

## What is the DIY Flow Bench Project?
###

The DIY Flow Bench project (DIYFB) is a professional quality flow bench controller that produces results comparable with, and in some cases exceeding commercial grade systems. It uses high grade sensors and a WiFi enabled ESP32 microprocessor to display flow data and control bench operation via a web browser on any web enabled device. 

This repository contains the hardware schematics and software code needed to build a volumetric flow bench based on an ESP32 controller and commonly available automotive sensors.

Support is included for the following systems architectures.

- MAF based benches
- Orifice based benches
- Pitot style benches

The DIYFB system can be used on any flow bench and makes an ideal retrofit for older manometer based benches as well as being the perfect choice for new builds.


## Project Goals
###
- Validated software and hardware design
- Affordable & easy to source components.
- Can be built & operated by a layman.
- Generates results comparable with commercial flow bench.
- Open source software and hardware design


## Download
###

You can download the latest release from our Githib repository at https://github.com/DeeEmm/DIY-Flow-Bench/releases


## Development Info

This branch contains current development version for the ESP32 port.

There is no official ESP32 release at this stage and whilst commits are undertaken to ensure functional code is uploaded, code may on occasion break.

The main goal of the project is to develop functional ESP port with WiFi access and WEB-GUI that will replace the existing Arduino based main development branch.

Current status of the project can be viewed at

- https://github.com/DeeEmm/DIY-Flow-Bench/projects/4

- Discussion for this version - https://github.com/DeeEmm/DIY-Flow-Bench/discussions/49
- Discord Chat - https://discord.gg/eAbktJj



## Support

No official support is offered for development versions however basic ESP32 installation information can be found at

- [https://github.com/DeeEmm/DIY-Flow-Bench/wiki/Installation](https://github.com/DeeEmm/DIY-Flow-Bench/wiki/Installation)

If you get stuck, feel free to hit us up over on the Discord channel.


## License
###

This project is released under the GPLV3 license. For details on usage please refer to the license file included with this project



## Further Information
###

- The latest information can be found in the project WIKI - https://github.com/DeeEmm/DIY-Flow-Bench/wiki
- Github repository - https://github.com/DeeEmm/DIY-Flow-Bench
- Support forums - https://github.com/DeeEmm/DIY-Flow-Bench/discussions
- Discord - https://discord.gg/eAbktJj
- Join our project community - https://www.facebook.com/groups/diyflowbench/ 

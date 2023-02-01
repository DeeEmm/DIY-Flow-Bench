# DIY-Flow-Bench

![DIY Flow Bench Web Interface](https://user-images.githubusercontent.com/3038710/126745200-fead302f-e0aa-48b6-8165-f72902b1edac.png)

![GitHub forks](https://img.shields.io/github/forks/deeemm/diy-flow-bench?style=social) &nbsp;&nbsp; ![GitHub Repo stars](https://img.shields.io/github/stars/deeemm/diy-flow-bench?style=social) &nbsp;&nbsp; ![GitHub all releases](https://img.shields.io/github/downloads/DeeEmm/DIY-Flow-Bench/total?logo=github) &nbsp;&nbsp; ![GitHub issues](https://img.shields.io/github/issues-raw/deeemm/diy-flow-bench?logo=github) &nbsp;&nbsp; ![GitHub contributors](https://img.shields.io/github/contributors/deeemm/diy-flow-bench?logo=github) &nbsp;&nbsp; ![Discord](https://img.shields.io/discord/762654320444440587?logo=discord) &nbsp;&nbsp; ![GitHub](https://img.shields.io/github/license/deeemm/diy-flow-bench?logo=gnu) 

## Version Information
###

General Version V2.0-RC.X


[Download the latest official release](https://github.com/DeeEmm/DIY-Flow-Bench/releases)


NOTE: This project is still in active development and is currently at Release Candidate stage. We are working towards a stable software and hardware release. If you are interested in contributing with testing please head over to our [Discord](https://discord.gg/eAbktJj) channel or [discussion forums](https://github.com/DeeEmm/DIY-Flow-Bench/discussions). 


## What is a Flow Bench?
###

A Flow Bench is a basic system to measure and display volumetric air flow.

It is used as a tool for measuring and quantifying modifications and improvements made to items like automotive cylinder heads and carburettors by measuring the amount of air that they can flow. By measuring the volume of air that can be passed through the item the user can determine if modifications made to the item help improve air flow characteristics.

Traditionally constructed of a manometer and calibrated flow orifices, volumetric flow was calculated using mathematical methods. Modern flow benches use sensors to detect flow and reference pressures and flow calculations are now undertaken by microprocessor.

## What is the DIY Flow Bench Project?
###

The DIY Flow Bench project (DIYFB) is a professional quality flow bench controller that produces results comparable with, and in some cases exceeding commercial grade systems. It uses high grade sensors and a WiFi enabled ESP32 microprocessor to display flow data and control bench operation via a web browser on any web enabled device. 

This repository contains the hardware schematics and software code needed to build a volumetric flow bench based on an ESP32 controller and commonly available automotive sensors.

Support is included for the following systems architectures.

- MAF based benches
- Orifice based benches
- Pitot style benches
- Venturi style benches

The DIYFB system can be used on any flow bench and makes an ideal retrofit for older manometer based benches as well as being the perfect choice for new builds. We include DIY hardware plans so you can build a basic MAF based flow bench that uses a shop vac for the vacuum source. This is a great entry level bench and perfect for most shops and hobbyists. We also provide support for larger and more complex bench designs.


## Project Goals
###
- Validated software and hardware design.
- Affordable & easy to source components.
- Can be built & operated by a layman.
- Generates results comparable with commercial flow bench.
- Open source software and hardware design.
- DIY Shield kits available to purchase.


## Development Info

This branch contains the current release candidate and is based on the ESP32 port of the original project.

After unit and peer testing is concluded and all bugs are resolved, this version will become the official DIYFB V2 release.

If you use this code and have found issues or have comments relating to the release, please raise an issue in the bugtracker, comment in the support thread for the current version or hit us up at our Discord channel.

- [Bugtracker](https://github.com/DeeEmm/DIY-Flow-Bench/issues)
- [Discussion for current version](https://github.com/DeeEmm/DIY-Flow-Bench/discussions) (Pinned to top of page)
- [Discord Chat](https://discord.gg/eAbktJj)

Please note that support is only provided for the current release so make sure that you are up to date and running the most [current version](https://github.com/DeeEmm/DIY-Flow-Bench/releases).


## Support

Documentation is provided for installation, configuration and use in the [WIKI](https://github.com/DeeEmm/DIY-Flow-Bench/wiki).

If you get stuck, feel free to create a [discussion](https://github.com/DeeEmm/DIY-Flow-Bench/discussions/) in the support forum, or see if anyone is available over on the [Discord](https://discord.gg/eAbktJj) channel.

If you find an issue, or have a feature request, please [raise an issue](https://github.com/DeeEmm/DIY-Flow-Bench/issues)


## License
###

This software is released under the GPLV3 license. For details on usage please refer to the license file included with this project

## Demo
###

You can view an online demo of the Web UI at the followng link

[https://diyflowbench.com/demo/](https://diyflowbench.com/demo/)


## Further Information
###

- [Project WIKI]( https://github.com/DeeEmm/DIY-Flow-Bench/wiki)
- [Github repository](https://github.com/DeeEmm/DIY-Flow-Bench)
- [Support forums](https://github.com/DeeEmm/DIY-Flow-Bench/discussions) 
- [Discord](https://discord.gg/eAbktJj)
- [Join our project community](https://www.facebook.com/groups/diyflowbench/)
